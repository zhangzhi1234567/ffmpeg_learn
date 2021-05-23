//
// Created by zz on 2021/3/12.
//

#ifndef BUILD_FFMPEG_LEARN_SDL_3_PLAY_VIDEO_HPP
#define BUILD_FFMPEG_LEARN_SDL_3_PLAY_VIDEO_HPP

#include <SDL/SDL.h>

#define video_w 1280
#define video_h 720

#define REFRESH_EVENT (SDL_USEREVENT + 1)
int screen_w = video_w;
int screen_h = video_h;
int thread_exit = 0;

int refresh_video(void *data) {
    SDL_Event event;
    while (!thread_exit) {
        event.type = REFRESH_EVENT;
        SDL_PushEvent(&event);
        SDL_Delay(40);
    }
    event.type = SDL_QUIT;
    SDL_PushEvent(&event);
    return 0;
}

int sdl_play_video_yuv() {

    //初始化，创建操作
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("play yuv", 200, 200, video_w, video_h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
    //IYUV: Y + U + V  (3 planes)
    //YV12: Y + V + U  (3 planes)
    Uint32  pixformat = SDL_PIXELFORMAT_IYUV;
    SDL_Texture *texture = SDL_CreateTexture(renderer, pixformat, SDL_TEXTUREACCESS_STREAMING, video_w, video_h);

    //读取数据，循环显示
    FILE *fd_video = fopen("./record_1280_720_25_yuv420p.yuv", "rb+");
    int read_frame_len = video_w * video_h * 12 / 8;
    char *buffer = (char *)malloc(read_frame_len);
    int tmp = read_frame_len;
    if (read_frame_len & 0xF) {
        read_frame_len = (tmp & 0xFFF0) + 0x10;
    }
    fread(buffer, 1, read_frame_len, fd_video);
    char *read_pos = buffer;
    SDL_CreateThread(refresh_video, "refresh_video", NULL);
    SDL_Rect rect;
    do {
        SDL_Event event;
        SDL_WaitEvent(&event);
        switch (event.type) {
            case REFRESH_EVENT: {
                rect.x = 0;
                rect.y = 0;
                rect.w = screen_w;
                rect.h = screen_h;
                SDL_UpdateTexture(texture, NULL, read_pos, video_w);

                int len = fread(buffer, 1, read_frame_len, fd_video);
                if (len <= 0) {
                    thread_exit = 1;
                }
                read_pos = buffer;
                SDL_RenderClear(renderer);
                SDL_RenderCopy(renderer, texture, NULL, &rect);
                SDL_RenderPresent(renderer);
                break;
            }
            case SDL_WINDOWEVENT: {
                SDL_GetWindowSize(window, &screen_w, &screen_h);
                break;
            }
            case SDL_QUIT: {
                thread_exit = 1;
                break;
            }
        }

    }while(!thread_exit);


    fclose(fd_video);
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyTexture(texture);
    SDL_Quit();

    return 0;
}

#endif //BUILD_FFMPEG_LEARN_SDL_3_PLAY_VIDEO_HPP
