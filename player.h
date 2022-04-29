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

//压缩数据包队列
struct PacketQueue
{
    int size;//队列数据包个数
    AVPacketList *head, *tail;//首尾指针
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
    char file_name[1024];//视频文件名
    AVFormatContext *avFormatCtx;//视频文件上下文
    int video_idx, audio_idx;//视音频流下标
    AVCodecContext *vCodecCtx, *aCodecCtx;//视音频解码器上下文
    AVCodec *vCodec, *aCodec;//视音频解码器
    AVStream *vStream, *aStream;//视音频流
    SDL_Thread *parse_tid, *decode_tid;//视频文件解析线程id和视频流解码线程id
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
    int picq_ridx, picq_widx, picq_size;//图像缓存队列的读写下标以及队列的大小
    SDL_mutex *picq_mutex;//图像缓存队列互斥锁
    SDL_cond *picq_cond_write;//图像缓存队列写同步信号量
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
    void init();//初始化类
    bool playing();//是否正在播放视频中
    void play(const char input_file[]);//播放视频

private:
    VideoInf *av;

};
