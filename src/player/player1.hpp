//
// Created by zz on 2021/3/9.
//
#ifndef BUILD_FFMPEG_LEARN_PLAYER1_HPP
#define BUILD_FFMPEG_LEARN_PLAYER1_HPP
#include "common.hpp"
#include <SDL/SDL.h>

int sdl_event() {
   SDL_Event event;
   SDL_PollEvent(&event);
   switch (event.type) {
       case SDL_QUIT: {
          return -1;
       }
      default: {
         return 0;
      }
   }
}

void sdl_create(SDL_Window **pWindow, SDL_Renderer **pRenderer, SDL_Texture **pTexture, int width, int height) {
   SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);
   *pWindow = SDL_CreateWindow("play video", 0, 0, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
   *pRenderer = SDL_CreateRenderer(*pWindow, -1, 0);
   *pTexture = SDL_CreateTexture(*pRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, width, height);
}



/// @brief parse yuv data from mp4 video, by ffmpeg and sdl
/// @return
//根据输入文件获取format上下文，并对流做检查，获取流索引
//通过流索引 找到编码上下文，打开编码器。
//准备数据，循环解码数据。并由SDL处理显示
int play_mp4_video(char *video_path) {

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
   for (int i = 0; i < fmt_ctx->nb_streams; ++i) {
      if (fmt_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
         video_index = i;
      }
   }
//   int ret = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
//   video_index = ret;  这种方式也可以，是新出的方法
   SDL_Log("find video index = %d\n", video_index);
   if (video_index == -1) {
      SDL_Log("find stream index fail.\n");
      return -1;
   }

   //通过流索引 找到编码上下文，打开编码器
   AVCodecContext *codec_ctx_orig = fmt_ctx->streams[video_index]->codec;
   AVCodec *decodec = avcodec_find_decoder(codec_ctx_orig->codec_id);
   if (NULL == decodec) {
      SDL_Log("find decoder fail.\n");
      return -1;
   }
   AVCodecContext *decodec_ctx = avcodec_alloc_context3(decodec);
   avcodec_copy_context(decodec_ctx, codec_ctx_orig);
   if (avcodec_open2(decodec_ctx, decodec, NULL) < 0) {
      SDL_Log("open decodec fail.\n");
      return -1;
   }

   //SDL创建
   SDL_Window *win = NULL;
   SDL_Renderer *render = NULL;
   SDL_Texture *texture = NULL;

   sdl_create(&win, &render, &texture, decodec_ctx->width, decodec_ctx->height);

   //准备接收数据, 循环解码数据处理
   AVFrame *raw_frame = av_frame_alloc();
   AVPacket *packet = av_packet_alloc();

   SwsContext *sws_ctx = sws_getContext(decodec_ctx->width, decodec_ctx->height, decodec_ctx->pix_fmt,
                                        decodec_ctx ->width, decodec_ctx->height, AV_PIX_FMT_YUV420P, SWS_BILINEAR,
                                        NULL, NULL, NULL);
   AVPicture *picture = (AVPicture *)malloc(sizeof(AVPicture));
   avpicture_alloc(picture, AV_PIX_FMT_YUV420P, decodec_ctx->width, decodec_ctx->height);
   int finished = 0;
   while (av_read_frame(fmt_ctx, packet) >= 0) {
      if (packet->stream_index == video_index) {
         avcodec_decode_video2(decodec_ctx, raw_frame, &finished, packet);
         if (finished) { //finished one frame
            sws_scale(sws_ctx, (uint8_t const * const *)raw_frame->data,
                      raw_frame->linesize, 0, decodec_ctx->height,
                      picture->data, picture->linesize);
            SDL_UpdateYUVTexture(texture, NULL,
                                 picture->data[0], picture->linesize[0],
                                 picture->data[1], picture->linesize[1],
                                 picture->data[2], picture->linesize[2]);
            SDL_Rect rect;
            rect.x = 0;
            rect.y = 0;
            rect.w = decodec_ctx->width;
            rect.h = decodec_ctx->height;
            SDL_RenderClear(render);
            SDL_RenderCopy(render, texture, NULL, &rect);
            SDL_RenderPresent(render);
            SDL_Delay(40); //不加这个播放的很快
         }
      }

      if (sdl_event() != 0) {
         break;
      }
      av_packet_unref(packet);
   }

   av_frame_free(&raw_frame);
   // Close the codec
   avcodec_close(decodec_ctx);
   avcodec_close(codec_ctx_orig);
   avformat_close_input(&fmt_ctx);
   avpicture_free(picture);
   free(picture);
   SDL_DestroyWindow(win);
   SDL_DestroyRenderer(render);
   SDL_DestroyTexture(texture);
   SDL_Quit();
   return 0;
}



#endif //BUILD_FFMPEG_LEARN_PLAYER1_HPP
