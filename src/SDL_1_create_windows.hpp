//
// Created by zz on 2021/1/29.
//

#ifndef BUILD_FFMPEG_LEARN_SDL_1_CREATE_WINDOWS_HPP
#define BUILD_FFMPEG_LEARN_SDL_1_CREATE_WINDOWS_HPP

#include <SDL/SDL.h>

int create_sdl_window() {
    int quit = 1;
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow("sdl window", 200, 200, 640, 480,
                                          SDL_WINDOW_SHOWN /*| SDL_WINDOW_BORDERLESS*/); //不注释掉这个，窗口不显示关闭按钮
    if (!window) {
        printf("create sdl window failure.\b");
        return -1;
    }
    //创建渲染器
    SDL_Renderer *render = SDL_CreateRenderer(window, -1, 0);
    SDL_SetRenderDrawColor(render, 255, 100, 0, 255);
    SDL_RenderClear(render);
    SDL_RenderPresent(render);
    //等待事件
    do {
        SDL_Event event;
       // SDL_PollEvent(&event); 轮询事件队列
        SDL_WaitEvent(&event);   //阻塞等待事件
        switch (event.type) {
            case SDL_QUIT:
                quit = 0;
                break;
            default:
                SDL_Log("sdl event type = %d\n", event.type);
        }
    }while(quit);

    SDL_DestroyRenderer(render);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

void create_sdl_rand_rect1() {
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

#endif //BUILD_FFMPEG_LEARN_SDL_1_CREATE_WINDOWS_HPP
