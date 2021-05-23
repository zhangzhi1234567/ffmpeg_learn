//
// Created by zz on 2021/3/9.
//
#ifndef BUILD_FFMPEG_LEARN_PLAYER2_HPP
#define BUILD_FFMPEG_LEARN_PLAYER2_HPP
#include <SDL/SDL.h>
#include "common.hpp"
#include "player1.hpp"
#include "queue.hpp"

PacketQueue_t g_audio_q;

void fill_audio_data2(void *userdata, Uint8 * stream, int len) {
    SDL_memset(stream, 0, len);


}
void sdl_open_audio(AVCodecContext *aCodecctx) {

    SDL_AudioSpec want;
    want.channels = 1;
    want.format = AUDIO_S16SYS;
    //want.samples = SDL_AUDIO_BUFFER_SIZE;
    want.freq = 44100;
    want.callback = fill_audio_data2;
    want.userdata = static_cast<void *>(aCodecctx);
    SDL_OpenAudio(&want, NULL);

}

/// @brief parse yuv data from mp4 video, by ffmpeg and sdl
/// @return
//根据输入文件获取format上下文，并对流做检查，获取流索引
//通过流索引 找到编码上下文，打开编码器。
//准备数据，循环解码数据。并由SDL处理显示
int play_mp4_video_and_audio(char *video_path) {

    AVFormatContext *fmt_ctx = NULL;
    if (avformat_open_input(&fmt_ctx, video_path, NULL, NULL) != 0) {
       SDL_Log("open input fail.\n");
       return -1;
    }
    if (avformat_find_stream_info(fmt_ctx, NULL) < 0) {
       SDL_Log("find stream info fail.\n");
       return -1;
    }
    av_dump_format(fmt_ctx, 0, video_path, 0);

    int video_index = -1;
    int audio_index = -1;
    for (int i = 0; i < fmt_ctx->nb_streams; ++i) {
       if (fmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
          video_index = i;
       }
       if (fmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO) {
          audio_index = i;
       }
    }
//    int ret = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
//    video_index = ret;  这种方式也可以，是新出的方法
    SDL_Log("find video index = %d\n", video_index);
    if (video_index == -1 || audio_index == -1) {
       SDL_Log("find stream index fail.\n");
       return -1;
    }

    //通过流索引 找到编码上下文，打开编码器 //视频和音频步骤一致
    /// @brief 视频：获取编码器上下文，根据上下文获取编码器, 打开编码器
    AVCodecContext *v_codec_ctx_orig = fmt_ctx->streams[video_index]->codec;
    AVCodec *v_decodec = avcodec_find_decoder(v_codec_ctx_orig->codec_id);
    if (NULL == v_decodec) {
       SDL_Log("find decoder fail.\n");
       return -1;
    }
    AVCodecContext *v_decodec_ctx = avcodec_alloc_context3(v_decodec);
    avcodec_copy_context(v_decodec_ctx, v_codec_ctx_orig);
    if (avcodec_open2(v_decodec_ctx, v_decodec, NULL) < 0) {
       SDL_Log("open v_decodec fail.\n");
       return -1;
    }
     /// @brief  音频： 获取编码器上下文，打开编码器
    AVCodecContext *a_codec_ctx_orig = fmt_ctx->streams[audio_index]->codec;
    AVCodec *a_decodec = avcodec_find_decoder(a_codec_ctx_orig->codec_id);
    if (NULL == a_decodec) {
        SDL_Log("find a_decoder fail.\n");
        return -1;
    }
    AVCodecContext *a_decodc_ctx = avcodec_alloc_context3(a_decodec);
    avcodec_copy_context(a_decodc_ctx, a_codec_ctx_orig);
    if (avcodec_open2(a_decodc_ctx, a_decodec, NULL) < 0) {
        SDL_Log("open a_decodec fail.\n");
        return -1;
    }
    //音频相关SDL创建
    sdl_open_audio(a_decodc_ctx);
    packet_queue_init(&g_audio_q);

    //视频相关SDL创建，
    SDL_Window *win = NULL;
    SDL_Renderer *render = NULL;
    SDL_Texture *texture = NULL;

    sdl_create(&win, &render, &texture, v_decodec_ctx->width, v_decodec_ctx->height);

    //准备接收数据, 循环解码数据处理
    AVFrame *raw_frame = av_frame_alloc();
    AVPacket *packet = av_packet_alloc();

    SwsContext *sws_ctx = sws_getContext(v_decodec_ctx->width, v_decodec_ctx->height, v_decodec_ctx->pix_fmt,
                                         v_decodec_ctx ->width, v_decodec_ctx->height, AV_PIX_FMT_YUV420P, SWS_BILINEAR,
                                         NULL, NULL, NULL);
    AVPicture *picture = (AVPicture *)malloc(sizeof(AVPicture));
    avpicture_alloc(picture, AV_PIX_FMT_YUV420P, v_decodec_ctx->width, v_decodec_ctx->height);
    int finished = 0;
    while (av_read_frame(fmt_ctx, packet) >= 0) {

        ///@1 视频数据直接交给视频解码器解码
       if (packet->stream_index == video_index) {
          avcodec_decode_video2(v_decodec_ctx, raw_frame, &finished, packet);
          if (finished) { //finished one frame
             sws_scale(sws_ctx, (uint8_t const * const *)raw_frame->data,
                       raw_frame->linesize, 0, v_decodec_ctx->height,
                       picture->data, picture->linesize);
             SDL_UpdateYUVTexture(texture, NULL,
                                  picture->data[0], picture->linesize[0],
                                  picture->data[1], picture->linesize[1],
                                  picture->data[2], picture->linesize[2]);
             SDL_Rect rect;
             rect.x = 0;
             rect.y = 0;
             rect.w = v_decodec_ctx->width;
             rect.h = v_decodec_ctx->height;
             SDL_RenderClear(render);
             SDL_RenderCopy(render, texture, NULL, &rect);
             SDL_RenderPresent(render);
             SDL_Delay(40); //不加这个播放的很快
          }
          ///@2 音频交给音频队列，从队列中取数据解码
       }else if (packet->stream_index == audio_index) {
           packet_queue_put(&g_audio_q, packet);
       }

       if (sdl_event() != 0) {
          break;
       }
       av_packet_unref(packet);
    }

    av_frame_free(&raw_frame);
    // Close the codec
    avcodec_close(v_decodec_ctx);
    avcodec_close(v_codec_ctx_orig);
    avformat_close_input(&fmt_ctx);
    avpicture_free(picture);
    free(picture);
    SDL_DestroyWindow(win);
    SDL_DestroyRenderer(render);
    SDL_DestroyTexture(texture);
    SDL_Quit();
    return 0;
}



#endif //BUILD_FFMPEG_LEARN_PLAYER2_HPP
