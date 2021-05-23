//
// Created by zz on 2021/1/6.
//
#include <iostream>
#include "1_ffmpeg_file.hpp"
#include "2_extract_audio.hpp"
#include "3_extract_video.hpp"
#include "4_parser_h264.hpp"
#include "record_video.hpp"
#include "record_audio.hpp"
#include "SDL/SDL_1_create_windows.hpp"
#include "SDL/SDL_2_create_rect.hpp"
#include "SDL/SDL_3_play_audio.hpp"
#include "SDL/SDL_4_play_video.hpp"
#include "player/player1.hpp"
#include "player/player2.hpp"
//#include "simple_avdevice.hpp"
using namespace std;
int main(int argc, char *argv[]) {
    av_log_set_level(AV_LOG_INFO);
    av_register_all();
    if (argc < 2) {
        return 0;
    }
    string flag(argv[1]);
    if (flag == "audio_extract") {
        audio_extract(argv[1], argv[2]);
    }else if (flag == "video_extract") {
        video_extract(argv[1], argv[2]);
    }else if (flag == "record_video") {
        record_video();
    }else if (flag == "record_audio") {
        record_audio();
    }else if (flag == "parser") {
        simplest_h264_parser(argv[2]);
    }else if (flag == "sdl") {
        create_sdl_window();
    }else if (flag == "rect") {
        create_sdl_rand_rect();
    }else if (flag == "audio") {
        sdl_play_audio_pcm();
    }else if (flag == "video") {
        sdl_play_video_yuv();
    }else if (flag == "player1") {
        play_mp4_video(argv[2]);
    }else if (flag == "player2") {
        play_mp4_video_and_audio(argv[2]);
    }
    //av_dump();
    //dir_ls();
    //simple_avdevice();
    printf("ffmpeg_learn quit.\n");
    return 0;
}