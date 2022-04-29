#include <stdio.h>
#include <math.h>
#include <iostream>
#include "player.h"
#include "sonic.h"
#include <stdlib.h>
using namespace std;

sonicStream sncStream;//PCM数据倍速处理流
AVPacket flush_pkt;//包标志，标志跳转播放模式（快进/快退等）

//创建压缩数据包队列
void CreatePacketQueue(PacketQueue *q)
{
    memset(q, 0, sizeof(PacketQueue));
    q->qMutex = SDL_CreateMutex();
    q->qCond = SDL_CreateCond();
}

//清空压缩数据包队列
void ClearPacketQueue(PacketQueue *q)
{
    SDL_LockMutex(q->qMutex);//给共享内存区上锁
    AVPacketList *pktList;
    while (q->head)
    {
        pktList = q->head;
        q->head = q->head->next;
        if (pktList->pkt.data)
        {
            av_packet_unref(&pktList->pkt);
        }
        av_free(pktList);
    }
    q->size = 0;
    q->tail = q->head = NULL;
    SDL_UnlockMutex(q->qMutex);
}

//销毁压缩数据包队列，释放空间
void DestroyPacketQueue(PacketQueue *q)
{
    ClearPacketQueue(q);
    SDL_DestroyMutex(q->qMutex);
    SDL_DestroyCond(q->qCond);
    q->qMutex = NULL;
    q->qCond = NULL;
}

//将压缩数据包pkt进队
void PacketQueuePut(PacketQueue *q, AVPacket *pkt)
{
    AVPacketList *pktList = (AVPacketList *)av_mallocz(sizeof(AVPacketList));
    pktList->pkt = *pkt;
    pktList->next = NULL;

    //访问共享内存区
    SDL_LockMutex(q->qMutex);
    if (q->size == 0)//队列中无数据
    {
        q->head = pktList;
        q->tail = pktList;
    }
    else
    {
        q->tail->next = pktList;
        q->tail = pktList;
    }
    q->size++;
    SDL_CondSignal(q->qCond);
    SDL_UnlockMutex(q->qMutex);
}

//从数据包队列中取出数据放到pkt中，成功放回0
int PacketQueueGet(VideoInf *av, PacketQueue *q, AVPacket *pkt)
{
    AVPacketList *pktList;
    //访问共享内存区
    SDL_LockMutex(q->qMutex);
    while (q->size == 0)//队列中无数据
    {
        SDL_CondWait(q->qCond, q->qMutex);//阻塞等待，并暂时释放互斥锁
        if(av->quit)
        {
            printf("user shuts the video!\n");
            SDL_UnlockMutex(q->qMutex);
;           return -1;
        }
    }
    if (q->head == NULL)
    {
        printf("packet queue is empty, failed!\n");
        return -1;
    }
    pktList = q->head;
    *pkt = q->head->pkt;
    q->head = q->head->next;
    q->size--;
    if (q->size == 0)
    {
        q->tail = NULL;
    }
    av_free(pktList);
    SDL_UnlockMutex(q->qMutex);

    return 0;
}

//获取实际的音频时间
double GetAudioTime(VideoInf *av)
{
    double pts = av->audio_pts;
    int nb_samples = av->audio_buf_size / av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO) / 2;//缓存帧的采样数
    double frame_time = 1.0 * nb_samples / av->aCodecCtx->sample_rate;//缓存帧的时长
    pts -= frame_time * (av->audio_buf_size - av->audio_buf_idx) / av->audio_buf_size;

    return pts;
}

//音频倍速处理函数
//输入采样点数nb_samples
//将倍速处理后的数据写回av->audio_buf中并返回处理后的nb_samples
int ChangeAudioSpeed(VideoInf *av, int nb_samples)
{
    int nb_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);//立体声

    if (!sncStream)
    {	// 参数为采样率和声道数
        sncStream = sonicCreateStream(av->aCodecCtx->sample_rate, nb_channels);
    }

    // 设置流的速率
    sonicSetSpeed(sncStream, float(av->speed));
    // 向流中写入audio_buf
    int ret = sonicWriteShortToStream(sncStream, (short *)av->audio_buf, nb_samples);
    // 计算处理后的采样点数
    int numSamples = int(nb_samples / av->speed);
    if (ret)
    {	// 从流中读取处理好的数据
        nb_samples = sonicReadShortFromStream(sncStream, (short *)av->audio_buf, numSamples);
    }

    return nb_samples;
}

//解码一帧音频数据，保存到av->audio_buf中
int DecodeAudioPacket(VideoInf *av)
{
    if(av->quit)
    {
        return -1;
    }

    AVPacket *pkt = &av->aPacket;
    AVFrame *frame = av_frame_alloc();

    for (;;)
    {
        //从解码器中解码得到数据，进行处理后交由音频回调函数送入声卡播放，否则到队列中取包
        while(avcodec_receive_frame(av->aCodecCtx, frame) == 0)
        {
            //重采样
            int nb_channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);
            int nb_samples = swr_convert(av->swrCtx, (uint8_t **)&av->audio_buf, MAX_AUDIO_BUF_SIZE,
                (const uint8_t **)frame->data, frame->nb_samples);
            av->audio_pts += 1.0 * nb_samples / av->aCodecCtx->sample_rate;

            //进行变速处理
            nb_samples = ChangeAudioSpeed(av, nb_samples);

            //计算audio_buf大小
            av->audio_buf_size = av_samples_get_buffer_size(NULL, nb_channels, nb_samples, AV_SAMPLE_FMT_S16, 1);
            if (av->audio_buf_size <= 0)
            {
                continue;
            }
            av->audio_buf_idx = 0;

            av_frame_free(&frame);
            return 0;
        }

        //正在解码数据包已经解码完成，需要读取下一个包
        if (pkt->data)//pkt中有数据，释放缓存空间，防止内存泄露
        {
            av_packet_unref(pkt);
        }
        if (PacketQueueGet(av, &av->audio_queue, pkt) != 0)//从队列中读取压缩数据包
        {
            printf("get pkt failed!\n");
            av_frame_free(&frame);
            return -1;
        }
        //读取到跳转处理的标志，重置解码器
        if (pkt->data == flush_pkt.data)
        {
            avcodec_flush_buffers(av->aCodecCtx);
            continue;
        }
        avcodec_send_packet(av->aCodecCtx, pkt);//将拿到的数据包送到解码器

        if (pkt->pts != AV_NOPTS_VALUE)//更新音频时间戳
        {
            av->audio_pts = pkt->pts * av_q2d(av->aStream->time_base);
        }
    }
    av_frame_free(&frame);
    return -1;
}

//音频回调函数
void AudioCallback(void *user_data, Uint8 *stream, int len)
{
    VideoInf *av = (VideoInf *)user_data;

    SDL_memset(stream, 0, len);
    while (len > 0)
    {
        if (av->audio_buf_idx >= av->audio_buf_size)//缓存数据已全部送入播放器缓存，需要重新解码压缩数据包
        {
            //获取压缩数据包失败，
            if(DecodeAudioPacket(av) != 0)
            {
                return;
            }
        }
        //将解码的数据写入播放器缓存中
        int write_len = av->audio_buf_size - av->audio_buf_idx;
        if (write_len > len)
        {
            write_len = len;
        }
        SDL_MixAudio(stream, av->audio_buf + av->audio_buf_idx, write_len, av->volume);
        stream += write_len;
        len -= write_len;
        av->audio_buf_idx += write_len;
    }
}

//处理视频帧的pts并更新av中的视频时间戳
double synchronize_video(VideoInf *av, AVFrame *src_frame, double pts)
{
    //检查显示时间戳
    if (pts != 0) //检查显示时间戳是否有效
    {
        av->video_pts = pts;//用显示时间戳更新已播放时间
    }
    else //若获取不到显示时间戳
    {
        pts = av->video_pts;//用已播放时间更新显示时间戳
    }
    //更新视频时间戳
    double frame_delay = av_q2d(av->vCodecCtx->time_base);//该帧显示完将要花费的时间
    //若存在重复帧，则在正常播放的前后两帧图像间安排渲染重复帧
    frame_delay += src_frame->repeat_pict*(frame_delay*0.5);//计算渲染重复帧的时值(类似于音符时值)
    av->video_pts += frame_delay;//更新视频播放时间

    return pts;//此时返回的值即为下一帧将要开始显示的时间戳
}

//视频解码线程函数
int DecodeThread(void *user_data)
{
    VideoInf *av = (VideoInf *)user_data;
    AVFrame *frame = av_frame_alloc();
    AVPacket *pkt = av_packet_alloc();

    //不断地从视频压缩数据包队列中读取数据包，进行解码，格式转换后放到图像缓存队列中
    for (;;)
    {
        if (pkt->data)//释放缓存的空间，防止内存泄漏
        {
            av_packet_unref(pkt);
        }
        if (PacketQueueGet(av, &av->video_queue, pkt) != 0)
        {
            printf("get pkt failed!\n");
            if(av->quit)
            {
                printf("user shuts the video!\n");
                av_frame_free(&frame);
                av_packet_free(&pkt);
                return 0;
            }
            else
            {
                continue;
            }
        }
        //读取到跳转处理的标志，重置解码器
        if (pkt->data == flush_pkt.data)
        {
            avcodec_flush_buffers(av->vCodecCtx);
            continue;
        }
        //解码一帧视频压缩数据，得到视频像素数据
        if(avcodec_send_packet(av->vCodecCtx, pkt) == 0)
        {
            //一个pkt中可能有多帧数据，需要循环读取
            while (avcodec_receive_frame(av->vCodecCtx, frame) == 0)
            {
                //检查队列是否已满
                SDL_LockMutex(av->picq_mutex);
                while (av->picq_size >= PICTURE_QUEUE_SIZE)
                {
                    SDL_CondWait(av->picq_cond_write, av->picq_mutex);
                    if(av->quit)
                    {
                        printf("user shuts the video!\n");
                        av_frame_free(&frame);
                        av_packet_unref(pkt);
                        av_packet_free(&pkt);
                        SDL_UnlockMutex(av->picq_mutex);
            ;           return 0;
                    }
                }
                SDL_UnlockMutex(av->picq_mutex);

                VideoPicture *vp = &av->picture_queue[av->picq_widx];

                //该空间缓存是否未分配或者图像的宽高不正确，需要重新分配空间
                if (!vp->allocated || vp->width != av->width || vp->height != av->height)
                {
                    if (vp->buffer)//确保没有分配空间，防止内存泄漏
                    {
                        av_free(vp->buffer);
                    }
                    if (vp->frameYUV)//确保没有分配空间，防止内存泄漏
                    {
                        av_frame_free(&vp->frameYUV);
                    }

                    vp->allocated = 1;
                    vp->width = av->width;
                    vp->height = av->height;

                    //分配空间并给frameYUV挂上buffer缓存
                    vp->buffer = (uint8_t *)av_mallocz(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, av->width, av->height, 1));
                    vp->frameYUV = av_frame_alloc();
                    av_image_fill_arrays(vp->frameYUV->data, vp->frameYUV->linesize, vp->buffer, AV_PIX_FMT_YUV420P, av->width, av->height, 1);
                }

                //获取该帧的时间戳
                double pts = frame->pts * av_q2d(av->vStream->time_base);
                vp->pts = synchronize_video(av, frame, pts);

                //对图像进行格式转换
                sws_scale(av->swsCtx, (const uint8_t *const*)frame->data, frame->linesize, 0, av->height,
                    vp->frameYUV->data, vp->frameYUV->linesize);

                //队列长度+1，写位置指针也移动
                SDL_LockMutex(av->picq_mutex);
                av->picq_size++;
                av->picq_widx++;
                if (av->picq_widx >= PICTURE_QUEUE_SIZE)
                {
                    av->picq_widx = 0;
                }
                SDL_UnlockMutex(av->picq_mutex);
            }
        }
    }

    return 0;
}

//视频刷新定时器回调函数
static Uint32 RefreshTimerCallBack(Uint32 interval, void *user_data)
{
    SDL_Event event;
    event.user.data1 = user_data;
    event.type = VIDEO_REFRESH_EVENT;
    SDL_PushEvent(&event);
    return 0;
}

//视频刷新定时器
void VideoRefreshTimer(VideoInf *av, int delay)
{
    SDL_AddTimer(delay, RefreshTimerCallBack, av);
}

//视频刷新函数，刷新屏幕并开启下一帧的定时器
void VideoRefresh(void *user_data)
{
    VideoInf *av = (VideoInf *)user_data;

    //用户已经关闭播放，停止定时器
    if (av->quit)
    {
        return;
    }

    //1.-------刷新显示当前帧到屏幕上---------
        //检查队列是否有数据
    if (av->pause || av->picq_size == 0)
    {//没有图像数据，1ms后再尝试
        VideoRefreshTimer(av, 1);
        return;
    }

    //将图片显示到屏幕上
    VideoPicture *vp = &av->picture_queue[av->picq_ridx];
    if (vp->frameYUV)
    {
        SDL_LockMutex(av->screen_mutex);

        av->video_time = av_gettime() / 1000000.0;//更新当前帧显示时对应的系统时间

        //将图片更新到屏幕上
        SDL_UpdateTexture(av->texture, NULL, vp->frameYUV->data[0], vp->frameYUV->linesize[0]);
        SDL_RenderCopy(av->renderer, av->texture, NULL, NULL);
        SDL_RenderPresent(av->renderer);

        SDL_UnlockMutex(av->screen_mutex);
    }

    //改变缓存队列大小和移动读指针
    SDL_LockMutex(av->picq_mutex);
    av->picq_ridx++;
    if (av->picq_ridx >= PICTURE_QUEUE_SIZE)
    {
        av->picq_ridx = 0;
    }
    av->picq_size--;
    SDL_CondSignal(av->picq_cond_write);
    SDL_UnlockMutex(av->picq_mutex);

    //2.-------计算下一帧的显示时间，并启动定时器--------
    double delay = vp->pts - av->pre_vpts;
    if (delay <= 0 || delay >= 1.0)//判断是否合理,否则沿用上一帧的延迟
    {
        delay = av->pre_vdelay;
    }
    av->pre_vdelay = delay;
    av->pre_vpts = vp->pts;

    //判断视频时间戳和音频时间戳的相对快慢，并进行相应调整
    double diff = vp->pts - GetAudioTime(av);
    if (fabs(diff) < 5)//时差过大的话就已经不是同步问题了
    {
        if (diff < -delay)//视频慢了，快速播放下一帧
        {
            delay = 0;
        }
        else if (diff > delay)//视频快了，延迟播放下一帧
        {
            delay = 2 * delay;
        }
    }

    //根据调整得到的delay开启定时器
    av->video_time += delay / av->speed;
    double actual_delay = av->video_time - av_gettime() / 1000000.0;
    if (actual_delay < 0)
    {
        actual_delay = 0;
    }
    VideoRefreshTimer(av, int(actual_delay * 1000 + 0.5));
}

//将播放时间设置跳转到pts
void JumpToPts(VideoInf *av, double pts)
{
    //检验pts的合理性
    if (pts < 0)
    {
        pts = 0;
    }
    else if (pts > av->avFormatCtx->duration)
    {
        pts = av->avFormatCtx->duration;
    }

    av->tar_pts = pts;
    double cur_pts = GetAudioTime(av);
    av->seek_flag = av->tar_pts < cur_pts ? AVSEEK_FLAG_BACKWARD : 0;
}

//全屏/退出全屏
void FullScreen(VideoInf *av)
{
    //还没有生成窗口，退出操作
    if (!av->texture)
    {
        return;
    }

    SDL_LockMutex(av->screen_mutex);
    av->fullScreen = !av->fullScreen;
    SDL_DestroyTexture(av->texture);
    SDL_DestroyRenderer(av->renderer);
    if (av->fullScreen)
    {
        SDL_SetWindowFullscreen(av->screen, SDL_WINDOW_FULLSCREEN_DESKTOP);
    }
    else
    {
        SDL_SetWindowFullscreen(av->screen, SDL_WINDOW_OPENGL);
    }
    av->renderer = SDL_CreateRenderer(av->screen, -1, 0);
    av->texture = SDL_CreateTexture(av->renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, av->width, av->height);
    SDL_UnlockMutex(av->screen_mutex);
}

//关闭视频
void Video_Quit(VideoInf *av)
{
    av->quit = 1;//设置状态

    //发送一下信号量，防止进程阻塞无法退出
    if (av->picq_cond_write)
    {
        SDL_CondSignal(av->picq_cond_write);
    }
    if (av->video_queue.qCond)
    {
        SDL_CondSignal(av->video_queue.qCond);
    }
    if (av->audio_queue.qCond)
    {
        SDL_CondSignal(av->audio_queue.qCond);
    }

    //关闭音频播放
    SDL_CloseAudio();

    //关闭视频窗口
    if (av->screen) SDL_HideWindow(av->screen);
}


//接受窗口的消息进行处理
int ProcessThread(VideoInf *av)
{
    //等待消息到来
    SDL_Event event;
    for (;;)
    {
        SDL_WaitEvent(&event);
        switch (event.type)
        {
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym)
            {
            case SDLK_RIGHTBRACKET://播放速度增加
                av->speed += 0.1;
                if (av->speed > 2)
                {
                    av->speed = 2;
                }
                break;
            case SDLK_LEFTBRACKET://播放速度减少
                av->speed -= 0.1;
                if (av->speed < 0.5)
                {
                    av->speed = 0.5;
                }
                break;
            case SDLK_LEFT://快退8秒
                if (!av->seeking && !av->pause)
                {
                    double tar_pts = GetAudioTime(av) - 8;
                    JumpToPts(av, tar_pts);
                    av->seeking = 1;
                }
                break;
            case SDLK_RIGHT://快进8秒
                if (!av->seeking && !av->pause)
                {
                    double tar_pts = GetAudioTime(av) + 8;
                    JumpToPts(av, tar_pts);
                    av->seeking = 1;
                }
                break;
            case SDLK_SPACE://播放/暂停
                av->pause = !av->pause;
                SDL_PauseAudio(av->pause);
                break;
            case SDLK_UP://提高音量
                av->volume += 2;
                if (av->volume > SDL_MIX_MAXVOLUME)
                {
                    av->volume = SDL_MIX_MAXVOLUME;
                }
                break;
            case SDLK_DOWN://降低音量
                av->volume -= 2;
                if (av->volume < 0)
                {
                    av->volume = 0;
                }
                break;
            case SDLK_f://全屏
                FullScreen(av);
                break;
            case SDLK_ESCAPE://关闭播放器
                if (av->fullScreen)
                {
                    FullScreen(av);
                }
                else//关闭视频
                {
                    Video_Quit(av);
                }
                break;
            default:
                break;
            }
            break;
        case SDL_QUIT://关闭视频
            Video_Quit(av);
            break;
        case VIDEO_REFRESH_EVENT://刷新视频图像，由当前帧计算延迟启动定时器触发事件
            if (av->quit) return 0;
            VideoRefresh(event.user.data1);
            break;
        default:
            break;
        }
    }
    return -1;
}

//视频解析线程函数，打开视频文件找到码流并打开音频播放和视频解码线程
int ParseThread(void *user_data)
{
    VideoInf *av = (VideoInf *)user_data;
    AVPacket *avPacket;

    //初始化变量
    av->avFormatCtx = avformat_alloc_context();
    avPacket = av_packet_alloc();

    //打开视频文件
    if (avformat_open_input(&av->avFormatCtx, av->file_name, NULL, NULL) != 0)
    {
        printf("Could not open file!\n");
        return -1;
    }

    //获取视频文件信息
    if (avformat_find_stream_info(av->avFormatCtx, NULL) < 0)
    {
        printf("Could not find stream info!\n");
        return -1;
    }

    //打印视频信息
//    av_dump_format(av->avFormatCtx, 0, NULL, 0);

    //找到视音频流下标
    av->video_idx = av->audio_idx = -1;
    for (unsigned int i = 0; i < av->avFormatCtx->nb_streams; ++i)
    {
        if (av->avFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO && av->video_idx < 0)
        {
            av->video_idx = i;
            continue;
        }
        if (av->avFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO && av->audio_idx < 0)
        {
            av->audio_idx = i;
            continue;
        }
    }

    //没有视频和音频，退出
    if (av->video_idx == -1 && av->audio_idx == -1)
    {
        printf("Could not find video or audio streams!\n");
        exit(0);
    }

    //设置默认配置，其他参数默认值为0
    av->speed = 1;//默认原速播放
    av->volume = 128;//默认最大音量播放

    //打开解码视频流的相关组件，初始化av结构体中的相关视频信息
    if (av->video_idx >= 0 && av->video_idx < av->avFormatCtx->nb_streams)
    {
        av->vStream = av->avFormatCtx->streams[av->video_idx];
        av->vCodecCtx = avcodec_alloc_context3(av->vCodec);
        avcodec_parameters_to_context(av->vCodecCtx, av->vStream->codecpar);
        av->vCodec = avcodec_find_decoder(av->vCodecCtx->codec_id);//根据codec_id找到解码器
        if (av->vCodec == NULL)
        {
            printf("Could not find decoder!\n");
            return -1;
        }
        if (avcodec_open2(av->vCodecCtx, av->vCodec, NULL) < 0)//打开解码器
        {
            printf("Could not open decoder!\n");
            return -1;
        }
        //初始化数据
        av->width = av->vCodecCtx->width;
        av->height = av->vCodecCtx->height;
        CreatePacketQueue(&av->video_queue);
        av->picq_mutex = SDL_CreateMutex();
        av->picq_cond_write = SDL_CreateCond();
        av->renderer = SDL_CreateRenderer(av->screen, -1, 0);
        av->texture = SDL_CreateTexture(av->renderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, av->width, av->height);
        //设置视频图像格式转换规则
        av->swsCtx = sws_getContext(av->width, av->height, av->vCodecCtx->pix_fmt,
            av->width, av->height, AV_PIX_FMT_YUV420P, SWS_BILINEAR, NULL, NULL, NULL);
        av->decode_tid = SDL_CreateThread(DecodeThread, "DecodeThread", av);
        VideoRefreshTimer(av, 1);
        av->video_pts = 0;
        av->pre_vpts = 0;
        av->pre_vdelay = 40e-3;
        av->video_time = av_gettime() / 1000000.0;
    }

    //打开解码音频流的相关组件，初始化av结构体中的相关音频信息
    if (av->audio_idx >= 0 && av->audio_idx < av->avFormatCtx->nb_streams)
    {
        av->aStream = av->avFormatCtx->streams[av->audio_idx];
        av->aCodecCtx = avcodec_alloc_context3(av->aCodec);
        avcodec_parameters_to_context(av->aCodecCtx, av->aStream->codecpar);
        av->aCodec = avcodec_find_decoder(av->aCodecCtx->codec_id);//根据codec_id找到解码器
        if (av->aCodec == NULL)
        {
            printf("Could not find decoder!\n");
            return -1;
        }
        if (avcodec_open2(av->aCodecCtx, av->aCodec, NULL) < 0)//打开解码器
        {
            printf("Could not open decoder!\n");
            return -1;
        }
        av->aCodecCtx->pkt_timebase = av->aStream->time_base;
        //初始化数据
        CreatePacketQueue(&av->audio_queue);
        av->audio_buf = (Uint8 *)av_mallocz(MAX_AUDIO_BUF_SIZE * 2);
        av->audio_buf_idx = 0;
        av->audio_buf_size = 0;
        //设置重采样规则
        int64_t in_channel_layout = av_get_default_channel_layout(av->aCodecCtx->channels);
        av->swrCtx = swr_alloc();
        swr_alloc_set_opts(av->swrCtx, AV_CH_LAYOUT_STEREO, AV_SAMPLE_FMT_S16, av->aCodecCtx->sample_rate,
            in_channel_layout, av->aCodecCtx->sample_fmt, av->aCodecCtx->sample_rate, 0, NULL);
        swr_init(av->swrCtx);
        //打开播放器
        SDL_AudioSpec wanted_spec;//播放参数
        wanted_spec.freq = av->aCodecCtx->sample_rate;//采样率
        wanted_spec.format = AUDIO_S16;//2bytes
        wanted_spec.channels = av_get_channel_layout_nb_channels(AV_CH_LAYOUT_STEREO);//立体声
        wanted_spec.samples = 1024;
        wanted_spec.callback = AudioCallback;
        wanted_spec.userdata = av;
        wanted_spec.silence = 0;
        if (SDL_OpenAudio(&wanted_spec, NULL))
        {
            printf("Could not open audio player!\n");
            return -1;
        }
        SDL_PauseAudio(0);
    }

    //获取每一帧数据包
    for (;;)
    {
        //判断是否退出播放
        if (av->quit)
        {
            av_packet_free(&avPacket);
            printf("user shuts the video!\n");
            return 0;
        }

        //判断是否需要跳转播放位置
        if (av->seeking)
        {
            //选择有码流信息的码流作为跳转基准
            int stream_idx = av->video_idx >= 0 ? av->video_idx : av->audio_idx;

            //将目标时间戳转换到对应码流时基单位的时间戳
            AVRational tb;
            tb.num = 1;
            tb.den = AV_TIME_BASE;
            int64_t tar_pts = av->tar_pts * AV_TIME_BASE;
            tar_pts = av_rescale_q(tar_pts, tb, av->avFormatCtx->streams[stream_idx]->time_base);

            //跳转到目标时间戳
            av_seek_frame(av->avFormatCtx, stream_idx, tar_pts, av->seek_flag);

            //清空队列缓存并且加入flush_pkt包通知音视频解码线程有跳转操作
            if (av->video_idx >= 0)
            {
                ClearPacketQueue(&av->video_queue);
                PacketQueuePut(&av->video_queue, &flush_pkt);
            }
            if (av->audio_idx >= 0)
            {
                ClearPacketQueue(&av->audio_queue);
                PacketQueuePut(&av->audio_queue, &flush_pkt);
            }

            av->seeking = 0;
        }

        //队列满就延迟添加
        if (av->video_queue.size >= MAX_VIDEO_PACKET_SIZE || av->audio_queue.size >= MAX_AUDIO_PACKET_SIZE)
        {
            SDL_Delay(100);
            continue;
        }

        //没有读取到帧，延迟40ms等待数据传入
        if (av_read_frame(av->avFormatCtx, avPacket) < 0)
        {
            SDL_Delay(40);
            continue;
        }

        //将视音频压缩数据包扔进对应的队列中等待播放
        if (avPacket->stream_index == av->video_idx)//视频数据包
        {
            PacketQueuePut(&av->video_queue, avPacket);
        }
        else if (avPacket->stream_index == av->audio_idx)//音频数据包
        {
            PacketQueuePut(&av->audio_queue, avPacket);
        }
        else//其他数据不做处理，清空包中的缓存
        {
            av_packet_unref(avPacket);
        }
    }

    return 0;
}

//释放图像缓存队列空间
void Free_Picture_Queue(VideoInf *av)
{
    for (int i = 0; i < PICTURE_QUEUE_SIZE; ++i)
    {
        VideoPicture *vp = &av->picture_queue[i];
        if (vp->buffer)
        {
            av_free(vp->buffer);
            vp->buffer = NULL;
        }
        if (vp->frameYUV)
        {
            av_frame_free(&vp->frameYUV);
        }
    }
}

//构造函数
Player::Player()
{
    av = (VideoInf *)av_mallocz(sizeof(VideoInf));
    memset(av, 0, sizeof(VideoInf));
    av->quit = 1;
}

//析构函数
Player::~Player()
{
    if (av)
    {
        init();
        av_free(av);
    }
}

//是否正在播放视频中
bool Player::playing()
{
    return av->quit == 0;
}

//初始化类，清空av记录的信息以及释放其中申请的空间，并将av填充为0
void Player::init()
{
    //释放视频格式上下文空间
    if(av->avFormatCtx)
    {
        avformat_close_input(&av->avFormatCtx);
    }

    //释放视音频解码器上下文空间
    if(av->vCodecCtx)
    {
        avcodec_close(av->vCodecCtx);
        avcodec_free_context(&av->vCodecCtx);
    }
    if(av->aCodecCtx)
    {
        avcodec_close(av->aCodecCtx);
        avcodec_free_context(&av->aCodecCtx);
    }

    //释放互斥锁变量，信号量等
    if(av->screen_mutex)
    {
        SDL_DestroyMutex(av->screen_mutex);
        av->screen_mutex = NULL;
    }
    if(av->picq_mutex)
    {
        SDL_DestroyMutex(av->picq_mutex);
        av->picq_mutex = NULL;
    }
    if(av->picq_cond_write)
    {
        SDL_DestroyCond(av->picq_cond_write);
        av->picq_cond_write = NULL;
    }

    //销毁SDL相关组件
    if(av->screen)
    {
        SDL_DestroyWindow(av->screen);
        av->screen = NULL;
    }
    if(av->renderer)
    {
        SDL_DestroyRenderer(av->renderer);
        av->renderer = NULL;
    }
    if(av->texture)
    {
        SDL_DestroyTexture(av->texture);
        av->texture = NULL;
    }

    //销毁压缩数据包队列
    if(av->video_queue.qMutex)
    {
        DestroyPacketQueue(&av->video_queue);
    }
    if(av->audio_queue.qMutex)
    {
        DestroyPacketQueue(&av->audio_queue);
    }

    //释放图片缓存队列空间
    Free_Picture_Queue(av);

    //释放重采样、图片格式转换上下文
    if(av->swsCtx)
    {
        sws_freeContext(av->swsCtx);
        av->swsCtx = NULL;
    }
    if(av->swrCtx)
    {
        swr_free(&av->swrCtx);
    }

    //释放音频缓存空间
    if(av->audio_buf)
    {
        av_free(av->audio_buf);
        av->audio_buf = NULL;
    }

    //将av置0
    memset(av, 0, sizeof(VideoInf));
}

//播放视频
void Player::play(const char input_file[])
{
    init();//初始化类中参数

    strcpy_s(av->file_name, input_file);

    //初始化SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
    {
        printf("Could not init SDL!\n");
        return;
    }

    av->screen = SDL_CreateWindow("MyPlayer", 50, 50, 1280, 720, SDL_WINDOW_OPENGL);
    SDL_ShowWindow(av->screen);
    av->screen_mutex = SDL_CreateMutex();

    //创建视频解析线程
    av->parse_tid = SDL_CreateThread(ParseThread, "ParseThread", av);

    //循环接收用户控制和视频显示等消息的线程
    ProcessThread(av);
}
