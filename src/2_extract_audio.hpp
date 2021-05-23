#include "common.hpp"
/// @brief 抽取音频信息, 抽取的数据暂时不能播放，需要在每一帧音频数据前添加adts头，后续添加
/// @param src_acc 输入的音频文件
/// @param dst_acc 输出抽取后的音频文件
/// @return
int audio_extract(char *src_acc, char *dst_acc) {
    AVFormatContext *fmt_ctx = NULL;
    int ret = 0, len = 0;
    char *src = NULL, *dst =NULL;
    AVPacket pkt;
    int audio_index;
    av_register_all();
    
    //1 read 输入的多媒体文件
    src = src_acc;
    dst = dst_acc;

    ret = avformat_open_input(&fmt_ctx, src, NULL, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "cant open src file:%s\n", av_err2str(ret));
        return -1;
    }
    FILE *dst_fd = fopen(dst, "wb+");
    if (!dst_fd) {
        av_log(NULL, AV_LOG_ERROR, "cant open dst file %s\n", dst);
    }
    av_dump_format(fmt_ctx, 0, src, 0);
    
    //2 get stream return stream index 
    ret = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "cant find best stream.\n");
    }
    audio_index = ret;
    //3 audio data output aac file
    av_init_packet(&pkt);
    while (av_read_frame(fmt_ctx, &pkt) >= 0) {
        if (pkt.stream_index == audio_index) { //说明是best stream对应的流数据
            len = fwrite(pkt.data, 1, pkt.size, dst_fd);
            if (len != pkt.size) {
                av_log(NULL, AV_LOG_ERROR, "WARNING");
            }
        }
        av_packet_unref(&pkt);
    }

    avformat_close_input(&fmt_ctx);
    if (dst_fd) {
        fclose(dst_fd);
    }

    return 0;
}
