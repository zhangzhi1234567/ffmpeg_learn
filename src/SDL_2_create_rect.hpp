//
// Created by zz on 2021/3/11.
//

#ifndef BUILD_FFMPEG_LEARN_SDL_2_CREATE_RECT_HPP
#define BUILD_FFMPEG_LEARN_SDL_2_CREATE_RECT_HPP
#include <SDL/SDL.h>

void create_sdl_rand_rect() {
    int quit = 1;
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("rect_windows", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480,SDL_WINDOW_SHOWN);
    if (!window) {
        printf("create windows fail.\n");
        return;
    }
    SDL_Renderer *render = SDL_CreateRenderer(window, -1, 0);
    //create texture for renderer
    SDL_Texture  *texture = SDL_CreateTexture(render, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 400, 450);
    SDL_Rect rect;
    rect.h = 20;
    rect.w = 20;
    do {
        SDL_Event event;
        //SDL_WaitEvent(&event); 用等到事件，只有有时间触发的时候才会随机画方块
        SDL_PollEvent(&event);
        switch (event.type) {
            case SDL_QUIT:
                quit = 0;
                break;
            default:
                SDL_Log("event type = %d\n", event.type);

        }
        rect.x = rand() % 500;
        rect.y = rand() % 500;
        //SDL_Delay(100);
        SDL_SetRenderTarget(render, texture);//默认的是整个窗口
        SDL_SetRenderDrawColor(render, 0, 100, 100, 0);
        SDL_RenderClear(render);

        SDL_RenderDrawRect(render, &rect);
        SDL_SetRenderDrawColor(render, 255, 0,0,0); //设置render画笔的颜色
        SDL_RenderFillRect(render, &rect);

        SDL_SetRenderTarget(render, NULL); //改变渲染的目标 为窗口，NULL为默认目标
        //纹理复制给渲染目标。（纹理是输入，渲染是输出
        SDL_RenderCopy(render, texture, NULL, NULL);
        SDL_RenderPresent(render);
    } while (quit);

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(render);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return ;
}

#endif //BUILD_FFMPEG_LEARN_SDL_2_CREATE_RECT_HPP
