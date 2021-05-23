//
// Created by zz on 2021/4/27.
//

#ifndef BUILD_FFMPEG_LEARN_RECORD_AUDIO_HPP
#define BUILD_FFMPEG_LEARN_RECORD_AUDIO_HPP
#include "common.hpp"
#include "record_video.hpp"
/// @brief open avfoundation device
/// @return
AVFormatContext *open_audio_dev() {
    // @@@1 set device 0:means camera
    char *device_name = ":0";
    // @@@2 register device
    avdevice_register_all();
    show_avfoundation();
    // @@@3 get format
    AVInputFormat *format = av_find_input_format("avfoundation");///AVFoundation是苹果在iOS和OS X系统中用于处理基于时间的媒体数据的Objective-C框架
    AVDictionary *option = NULL;
   // av_dict_set(&option, "video_size", "1280*720", 0);
   // av_dict_set(&option, "framerate", "30", 0);
   // av_dict_set(&option, "pixel_format", "nv12", 0);
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


AVFrame *alloc_frame() {
    AVFrame *frame = av_frame_alloc();
    frame->channel_layout = 2;
    frame->nb_samples = 512;
    frame->format = AV_SAMPLE_FMT_FLT;
    av_frame_get_buffer(frame, 0);
    return frame;
}

void record_audio() {

    char *file = "record_audio.pcm";
    FILE *fd = fopen(file, "wb+");

    AVFormatContext *format_ctx= open_audio_dev();

    AVStream *stream = format_ctx->streams[0];
    AVCodecParameters *audio_param = stream->codecpar;
    //This struct describes the properties of an encoded stream.
    //sample rate: 48000, channels: 65, format: 3(AV_SAMPLE_FMT_FLT)
    //printf("sample rate: %d, channels: %d, format: %d\n", audio_param->sample_rate, audio_param->channels, audio_param->format);
    AVPacket pkt;
    av_init_packet(&pkt);
    timer();
    while (g_brec) {
        int ret = av_read_frame(format_ctx, &pkt);
        if (ret == AVERROR(EAGAIN)) {
            // av_log(NULL, AV_LOG_INFO, "Resource temporarily unavailable.\n");
            continue;
        }else if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "read frame from device failure. %d\n", ret);
            break;
        }
        av_log(NULL, AV_LOG_INFO, "read pkt size = %d\n", pkt.size);
        fwrite(pkt.data, 1, pkt.size, fd);
        av_packet_unref(&pkt);
    }
    printf("finished.\n");

    avformat_free_context(format_ctx);
    fclose(fd);

    return;
}


#endif //BUILD_FFMPEG_LEARN_RECORD_AUDIO_HPP
