extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/log.h>
}
int file_delete() {
    int ret = 0;
    //url 
    ret = avpriv_io_delete("./test.txt");
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "Failed to delete file.\n");
        return -1;
    }
}
int dir_ls() {
    int ret = 0;
    AVIODirContext *ctx = NULL;
    AVIODirEntry *entry = NULL;
    ret = avio_open_dir(&ctx, "./", NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "cant open dir");
        avio_close_dir(&ctx);
        return -1;
    }
    while (1) {
        ret = avio_read_dir(ctx, &entry);
        if (ret < 0) {
            av_log(NULL, AV_LOG_ERROR, "cant read dir:%s\n", av_err2str(ret));
        }
        if (!entry) { //退出循环
            break;
        }
        av_log(NULL, AV_LOG_INFO, "%lu  %s\n", entry->size, entry->name);
        avio_free_directory_entry(&entry); //释放entry
    }
}

int av_dump() {
    AVFormatContext *fmt_ctx = NULL;
    int ret = avformat_open_input(&fmt_ctx, "./test.mp4", NULL, NULL);
    if (ret < 0) {
        av_log(NULL, AV_LOG_ERROR, "cant open file:%s\n", av_err2str(ret));
        return -1;
    }
    av_dump_format(fmt_ctx, 0, "./test.mp4", 0);

    avformat_close_input(&fmt_ctx);
}

