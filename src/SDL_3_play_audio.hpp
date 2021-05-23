//
// Created by zz on 2021/3/11.
//

#ifndef BUILD_FFMPEG_LEARN_3_SDL_PLAY_AUDIO_HPP
#define BUILD_FFMPEG_LEARN_3_SDL_PLAY_AUDIO_HPP
#include <SDL/SDL.h>

#define BUFFER_SIZE 40960000
int audio_read_len = 0;
Uint8 *audio_read_pos = NULL;

void fill_audio_data(void *userdata, Uint8 *stream, int len) {
    SDL_memset(stream, 0, len);
    if (len == 0) return;
    len = (len > audio_read_len) ? audio_read_len : len;

    SDL_MixAudio(stream, audio_read_pos, len, SDL_MIX_MAXVOLUME);
    audio_read_pos += len;
    audio_read_len -= len;
}
/// @brief   SDL_Init, SDL_OpenAduio, SDL_PauseAudio, SDL_memset, SDL_MixAudio
/// @return
int sdl_play_audio_pcm() {

    SDL_Init(SDL_INIT_AUDIO);

    FILE *fd_audio = fopen("./audio_44.1k_s16le_ch1.pcm", "r");
    if (!fd_audio) {
        SDL_Log("open audio file failure.\n");
        return 0;
    }

    SDL_AudioSpec spec;
    spec.freq = 44100;
    spec.format = AUDIO_S16SYS;
    spec.channels = 1;
    spec.callback = fill_audio_data;
    spec.userdata = NULL;
    if (SDL_OpenAudio(&spec, NULL) < 0) {
        SDL_Log("open audio device failure");
        return -1;
    }
    SDL_PauseAudio(0);//让SDL内存准备调fill_audio_data这个回调函数

    int buf_len = 0;
    Uint8 *buf_audio = (Uint8 *)malloc(BUFFER_SIZE);
    do{
        buf_len = fread(buf_audio, 1, BUFFER_SIZE, fd_audio);
        audio_read_len = buf_len;
        audio_read_pos = buf_audio;
        while (audio_read_len > 0) { //当还没有读完这个4M的buf时，要等SDL读完在去读取下一个4M
            SDL_Delay(1);
        }

    }while(buf_len != 0);

    free(buf_audio);
    SDL_CloseAudio();
    SDL_Quit();
    return 0;
}





#endif //BUILD_FFMPEG_LEARN_3_SDL_PLAY_AUDIO_HPP
