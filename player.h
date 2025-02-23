#pragma once

#define __STDC_CONSTANT_MACROS
#define SDL_MAIN_HANDLED

extern "C"
{
#include "libavutil/avutil.h"
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavutil/avstring.h"
#include "libswresample/swresample.h"
#include "libavutil/time.h"
#include "libavutil/imgutils.h"

#include "SDL2/SDL.h"
}

#define MAX_AUDIO_BUF_SIZE 19200
#define MAX_VIDEO_PACKET_SIZE (1024 * 1024)
#define MAX_AUDIO_PACKET_SIZE (1024 * 64)
#define PICTURE_QUEUE_SIZE 2
#define VIDEO_REFRESH_EVENT (SDL_USEREVENT)
#define VIDEO_QUIT_EVENT (SDL_USEREVENT + 1)

//压缩数据包队列结构体
struct PacketList
{
    AVPacket pkt;
    struct PacketList *next;
};

//压缩数据包队列
struct PacketQueue
{
    int size;//队列数据包个数
    PacketList *head, *tail;//首尾指针
    SDL_mutex *qMutex;//队列互斥锁，防止同时访问共享内存空间
    SDL_cond *qCond;//队列信号量，用于同步
};

//视频输出图像数据结构
struct VideoPicture
{
    uint8_t *buffer;//为frameYUV提供缓存空间
    AVFrame *frameYUV;//一帧处理完成的YUV格式图像
    int allocated, width, height;//该空间分配的标志以及图像的宽和高
    double pts;//该帧图像播放的时间戳
};

//记录打开的视频文件动态信息
struct VideoInf
{
    //视频文件的全局参数
    int quit;//关闭视频的标志
    int resized;//窗口尺寸改变的标志
    char file_name[1024];//视频文件名
    AVFormatContext *avFormatCtx;//视频文件上下文
    int video_idx, audio_idx;//视音频流下标
    AVCodecContext *vCodecCtx, *aCodecCtx;//视音频解码器上下文
    AVCodec *vCodec, *aCodec;//视音频解码器
    AVStream *vStream, *aStream;//视音频流
    SDL_Thread *parse_tid, *decode_tid, *refresh_tid;//视频文件解析线程、视频流解码线程id和视频刷新线程id
    int volume;//音量大小，范围为[0, SDL_MIX_MAXVOLUME]
    int fullScreen;//全屏状态
    double speed;//播放速度
    int pause;//是否暂停
    int seeking;//跳转播放处理中的标志
    int seek_flag;//快进/快退的标志
    double tar_pts;//跳转播放的目标时间戳
    SDL_mutex *screen_mutex;//屏幕锁，用于改变屏幕大小等过程
    SDL_Window *screen;//视频播放窗口
    SDL_Renderer *renderer;//视频播放渲染器
    SDL_Texture *texture;//图片纹理

    //视频相关数据
    VideoPicture picture_queue[PICTURE_QUEUE_SIZE];//图像缓存队列，存放解码并转码后用于播放的图像数据
//    VideoPicture picture_buf;//保存当前正在显示的图像缓存，用于刷新界面
    int picq_ridx, picq_widx, picq_size;//图像缓存队列的读写下标以及队列的大小
    SDL_mutex *picq_mutex;//图像缓存队列互斥锁
    SDL_cond *picq_cond_write;//图像缓存队列写同步信号量
    SDL_mutex *refresh_mutex;//用于视频刷新线程的互斥锁
    SDL_cond *refresh_cond;//用于控制刷新下一帧的同步信号量
    PacketQueue video_queue;//视频压缩数据包队列
    int width, height;//视频图像的宽和高
    SwsContext *swsCtx;//视频格式转换上下文
    double video_pts;//记录播放视频帧的时间戳
    double video_time;//将要播放视频帧的播放时间，以系统时间为基准
    double pre_vpts, pre_vdelay;//记录上一个视频帧的时间戳和延迟

    //音频相关数据
    PacketQueue audio_queue;//音频压缩数据包队列
    AVPacket aPacket;//正在解码的音频数据包
    Uint8 *audio_buf;//正在播放的音频数据
    int audio_buf_idx, audio_buf_size;//已经送入播放器缓存的解码数据量和该帧总的数据量
    SwrContext *swrCtx;//音频重采样上下文
    double audio_pts;//当前解码音频帧的时间戳（播放完成时），经过时基换算
};

//后台播放器类，负责实现视频播放的功能
class Player
{
public:
    Player();//构造函数
    ~Player();//析构函数

public:
    //以下公共函数都是为前台调用的功能函数，会判断操作的合法性，按需调用即可
    bool Playing();//是否打开了视频文件
    void Play(const char input_file[], void *wid = NULL);//播放视频，输入视频文件路径和窗口控件的winID
    int Pausing();//判断是否处于暂停状态，返回1表示暂停中，返回0表示播放中，返回-1表示没有打开任何视频
    void Pause();//暂停-播放切换功能
    double GetTotalDuration();//获取视频的总长度，没有打开任何视频就返回-1
    double GetCurrentTime();//获取当前播放进度，没有打开任何视频就返回-1
    bool Jump(double play_time);//跳转播放，输入跳转到的时间点，单位是秒，返回是否跳转成功
    void Backward();//快退，每次跳转8秒
    void Forward();//快进，每次跳转8秒
    bool SetSpeed(double speed);//设置播放速度，暂时限制[0.5, 3]，返回是否设置成功
    void SpeedUp();//加快播放速度，每次速度提升0.1，范围[0.5, 2]
    void SpeedDown();//降低播放速度，每次速度降低0.1，范围[0.5, 2]
    bool SetVolume(int volume);//设置音量，输入值范围[0, 100]，返回是否设置成功
    void VolumeUp();//提高音量，每次提升2音量
    void VolumeDown();//降低音量，每次降低2音量
    void Quit();//退出播放器，即关闭当前视频文件

private:
    void Init();//初始化类
    VideoInf *av;

};

