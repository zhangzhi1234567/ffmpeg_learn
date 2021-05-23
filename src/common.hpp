//
// Created by zz on 2021/1/8.
//

#ifndef BUILD_FFMPEG_LEARN_COMMON_HPP
#define BUILD_FFMPEG_LEARN_COMMON_HPP
#include <stdio.h>
#ifdef __cplusplus
extern "C" {
#endif
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavdevice/avdevice.h>
#include <libavcodec/avcodec.h>
#include <libavfilter/avfilter.h>
#include <libswscale/swscale.h>
#include "player/queue.hpp"
#ifdef __cplusplus
};
#endif

#endif //BUILD_FFMPEG_LEARN_COMMON_HPP
