//
// Created by zz on 2021/1/8.
//

#ifndef BUILD_FFMPEG_LEARN_RECORD_VIDEO_HPP
#define BUILD_FFMPEG_LEARN_RECORD_VIDEO_HPP
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include "common.hpp"
#define WIDTH 1280
#define HEIGHT 720
bool g_brec = true;
void setflags(int sig) {
    g_brec = false;
}

void timer() {
    struct sigaction act;
    act.sa_flags = 0;
    act.sa_handler = setflags;
    sigaction(SIGALRM, &act, NULL);

    struct itimerval t;
    t.it_value.tv_sec = 10; //sec run
    t.it_value.tv_usec = 0;
    t.it_interval.tv_usec = 0;
    t.it_interval.tv_sec = 10; //real interval
    setitimer(ITIMER_REAL, &t, NULL);
}




AVCodecContext *open_encode(int width, int height) {
    //@@@1 get encoder
    AVCodec *codec = avcodec_find_encoder_by_name("libx264");
    if (!codec) {
        printf("codec libx264 not found.\n");
        exit(1);
    }
    //@@@2 allocate AVCodecContext, user use encode by AVCodecContext
    AVCodecContext *enc_ctx = avcodec_alloc_context3(codec);
    if (!enc_ctx) {
        av_log(NULL, AV_LOG_ERROR, "Allocate AVCodecContext failure.\n");
        return NULL;
    }
    //@@@@3 Initialize AVCodeContext , 编码成什么样，需要用户设置
        ///set resolution
    enc_ctx->width = width;
    enc_ctx->height = height;
        ///set sps pps
    enc_ctx->profile = FF_PROFILE_H264_HIGH_444;///encode profile
    enc_ctx->level = 50;                        ///对视频的描述
        ///set GOP
    enc_ctx->gop_size = 250;
    enc_ctx->keyint_min = 25;
        ///set YUV, encode need know YUV format
    enc_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
        ///set biterate
    enc_ctx->bit_rate = 1000000;
        ///set fps
    enc_ctx->time_base = (AVRational){1, 25};
    enc_ctx->framerate = (AVRational){25, 1};
        ///set b frame num, option
    enc_ctx->max_b_frames = 3;
    enc_ctx->has_b_frames = 1;
        ///set reference frame num, option
    enc_ctx->refs = 3;

    //@@@4 open encoder
    int ret = avcodec_open2(enc_ctx, codec, NULL);
    if (ret != 0) {
        av_log(NULL, AV_LOG_ERROR, "open avcodec failure.\n");
        return NULL;
    }
    return enc_ctx;
}

AVFrame *create_frame(int width, int height) {
    AVFrame *frame = av_frame_alloc();
    if (!frame) return NULL;
    frame->width = width;
    frame->height = height;
    frame->format = AV_PIX_FMT_YUV420P;//here is data, open_encode is encode
    av_frame_get_buffer(frame, 32);
    return frame;
}

void encode_yuv_frame(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *enc_pkt, FILE *store_file) {
    int ret = avcodec_send_frame(enc_ctx, frame);
    if (ret < 0) {
        return ;
    }
    while (ret >= 0) {
        ret = avcodec_receive_packet(enc_ctx, enc_pkt);
        if (ret == AVERROR(EAGAIN) || ret == EOF) { ///EAGAIN, end again, current frame is not available, need send frame
            return ;
        }
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "encode end. %s\n", av_err2str(ret));
            exit(1);
        }
        fwrite(enc_pkt->data, 1, enc_pkt->size, store_file);
        av_packet_unref(enc_pkt);
    }
    return ;
}

/// @brief show avfoundation devices
/// @eg
/// AVFoundation video devices:
/// [0] FaceTime高清摄像头（内建）
/// [1] Capture screen 0
/// AVFoundation audio devices:
/// [0] 聚集设备
/// [1] Soundflower (64ch)
/// [2] MacBook Pro麦克风
/// [3] Soundflower (2ch)
void show_avfoundation() {
    AVFormatContext *fmt = avformat_alloc_context();
    AVDictionary *option = NULL;
    av_dict_set(&option, "list_devices", "true", NULL);
    AVInputFormat *inputFmt = av_find_input_format("avfoundation");
    avformat_open_input(&fmt, "", inputFmt, &option);
}
/// @brief open avfoundation device
/// @return
AVFormatContext *open_input_dev() {
    // @@@1 set device 0:means camera
    char *device_name = "0";
    // @@@2 register device
    avdevice_register_all();
    show_avfoundation();
    // @@@3 get format
    AVInputFormat *format = av_find_input_format("avfoundation");///AVFoundation是苹果在iOS和OS X系统中用于处理基于时间的媒体数据的Objective-C框架
    AVDictionary *option = NULL;
    av_dict_set(&option, "video_size", "1280*720", 0);
    av_dict_set(&option, "framerate", "30", 0);
    av_dict_set(&option, "pixel_format", "nv12", 0);
    // @@@4 open device
    AVFormatContext *fmt_ctx = avformat_alloc_context();///必须调用
    int ret = avformat_open_input(&fmt_ctx, device_name, format, &option);
    if (ret != 0) {
        av_log(NULL, AV_LOG_ERROR, "open device failure, ret = %d\n", ret);
        return NULL;
    }
    if(avformat_find_stream_info(fmt_ctx,NULL)<0)
    {
        printf("Couldn't find stream information.\n");
        return NULL;
    }

    return fmt_ctx;
}
//0 open device
//1 open encode
//2 nv12 video capture
//3 nv12 transform YUV420
//--> 4 YUV420 write file
//--> 4 YUV420 send to encode
//5 get encoded data
/// @brief capture video data from device
/// @return
void record_video() {
    char *file_path = "./record_file.h264";
    char *file_yuv_path = "./record_file.yuv";
    FILE *store_file = fopen(file_path, "wb+");
    FILE *store_yuv_file = fopen(file_yuv_path, "wb+");
    AVPacket pkt;
    memset(&pkt, 0, sizeof(pkt));
    AVFormatContext *fmt_ctx = open_input_dev();

    //@@@1 open encode
    AVCodecContext *enc_ctx = open_encode(WIDTH, HEIGHT);
    //@@@2 nv12 video capture ///capture which format by device.
        ///frame stored YUV data
    AVFrame *frame = create_frame(WIDTH, HEIGHT);
        ///newpky stored encoded data
    AVPacket *newpkt = av_packet_alloc();
    int base = 0;
    timer();//open timer
    while(g_brec) {
        int ret = av_read_frame(fmt_ctx, &pkt);
        if (ret == AVERROR(EAGAIN)) {
           // av_log(NULL, AV_LOG_INFO, "Resource temporarily unavailable.\n");
            continue;
        }else if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "read frame from device failure. %d\n", ret);
            break;
        }
        av_log(NULL, AV_LOG_INFO, "read pkt, size = %d\n", pkt.size);
        //fwrite(pkt.data, 1, 921600*1.5, store_nv12_file);
        //fflush(store_nv12_file);
        //@@@3 nv12 transform YUV420
        memcpy(frame->data[0], pkt.data, WIDTH*HEIGHT); ///copy Y data
        for (int i = 0; i < WIDTH*HEIGHT/4; ++i) {
           frame->data[1][i] = pkt.data[WIDTH*HEIGHT + 2*i];
           frame->data[2][i] = pkt.data[WIDTH*HEIGHT + 2*i + 1];
        }
        //--> 4 YUV420 write file / option
        fwrite(frame->data[0], 1, 921600, store_yuv_file);
        fwrite(frame->data[1], 1, 921600/4, store_yuv_file);
        fwrite(frame->data[2], 1, 921600/4, store_yuv_file);

        //--> 4 YUV420 send to encode
        //@@@5 get encoded data
        base += 1;
        frame->pts = base;
        encode_yuv_frame(enc_ctx, frame, newpkt, store_file);
        av_packet_unref(&pkt);
    }
    printf("finish.\n");
    encode_yuv_frame(enc_ctx, NULL, newpkt, store_file);

    //free resource
    avformat_free_context(fmt_ctx);
    fclose(store_yuv_file);
    fclose(store_file);
    return ;
}
#endif //BUILD_FFMPEG_LEARN_RECORD_VIDEO_HPP
