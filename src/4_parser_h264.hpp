//
// Created by zz on 2021/1/27.
//

#ifndef BUILD_FFMPEG_LEARN_4_PARSER_H264_HPP
#define BUILD_FFMPEG_LEARN_4_PARSER_H264_HPP
/**
 * 最简单的视音频数据处理示例
 * Simplest MediaData Test
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * 本项目包含如下几种视音频测试示例：
 *  (1)像素数据处理程序。包含RGB和YUV像素格式处理的函数。
 *  (2)音频采样数据处理程序。包含PCM音频采样格式处理的函数。
 *  (3)H.264码流分析程序。可以分离并解析NALU。
 *  (4)AAC码流分析程序。可以分离并解析ADTS帧。
 *  (5)FLV封装格式分析程序。可以将FLV中的MP3音频码流分离出来。
 *  (6)UDP-RTP协议分析程序。可以将分析UDP/RTP/MPEG-TS数据包。
 *
 * This project contains following samples to handling multimedia data:
 *  (1) Video pixel data handling program. It contains several examples to handle RGB and YUV data.
 *  (2) Audio sample data handling program. It contains several examples to handle PCM data.
 *  (3) H.264 stream analysis program. It can parse H.264 bitstream and analysis NALU of stream.
 *  (4) AAC stream analysis program. It can parse AAC bitstream and analysis ADTS frame of stream.
 *  (5) FLV format analysis program. It can analysis FLV file and extract MP3 audio stream.
 *  (6) UDP-RTP protocol analysis program. It can analysis UDP/RTP/MPEG-TS Packet.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    NALU_TYPE_SLICE    = 1,
    NALU_TYPE_DPA      = 2,
    NALU_TYPE_DPB      = 3,
    NALU_TYPE_DPC      = 4,
    NALU_TYPE_IDR      = 5,
    NALU_TYPE_SEI      = 6,
    NALU_TYPE_SPS      = 7,
    NALU_TYPE_PPS      = 8,
    NALU_TYPE_AUD      = 9,
    NALU_TYPE_EOSEQ    = 10,
    NALU_TYPE_EOSTREAM = 11,
    NALU_TYPE_FILL     = 12,
} NaluType;

typedef enum {
    NALU_PRIORITY_DISPOSABLE = 0,
    NALU_PRIRITY_LOW         = 1,
    NALU_PRIORITY_HIGH       = 2,
    NALU_PRIORITY_HIGHEST    = 3
} NaluPriority;


typedef struct
{
    int startcodeprefix_len;      //! 4 for parameter sets and first slice in picture, 3 for everything else (suggested)
    unsigned len;                 //! Length of the NAL unit (Excluding the start code, which does not belong to the NALU)
    unsigned max_size;            //! Nal Unit Buffer size
    int forbidden_bit;            //! should be always FALSE
    int nal_reference_idc;        //! NALU_PRIORITY_xxxx
    int nal_unit_type;            //! NALU_TYPE_xxxx
    char *buf;                    //! contains the first byte followed by the EBSP
} NALU_t;

FILE *f_h264bit_stream = NULL;                //!< the bit stream file

int info2 = 0, info3 = 0;

static int FindStartCode2 (unsigned char *Buf){
    if(Buf[0] != 0 || Buf[1] != 0 || Buf[2] != 1) return 0; //0x000001?
    else return 1;
}

static int FindStartCode3 (unsigned char *Buf){
    if(Buf[0] != 0 || Buf[1] != 0 || Buf[2] != 0 || Buf[3] != 1) return 0;//0x00000001?
    else return 1;
}


int GetNALUFromAnnexb(NALU_t *nalu) {
    int pos = 0;
    int StartCodeFound, rewind;
    unsigned char *Buf;

    if ((Buf = (unsigned char*)calloc (nalu->max_size , sizeof(char))) == NULL)
        printf("GetAnnexbNALU: Could not allocate Buf memory\n");

    nalu->startcodeprefix_len = 3;

    if (3 != fread(Buf, 1, 3, f_h264bit_stream)){
        free(Buf);
        return 0;
    }
    info2 = FindStartCode2(Buf);
    if (info2 != 1) {
        //if prev 3 is not 000001, read 1 byte , b is 00000001
        if (1 != fread(Buf + 3, 1, 1, f_h264bit_stream)) {
            free(Buf);
            return 0;
        }
        info3 = FindStartCode3 (Buf);
        if (info3 != 1) {
            free(Buf);
            return -1;
        }else {
            pos = 4;
            nalu->startcodeprefix_len = 4;
        }
    }else {
        nalu->startcodeprefix_len = 3;
        pos = 3;
    }
    StartCodeFound = 0;
    info2 = 0;
    info3 = 0;
    //find next start code
    while (!StartCodeFound) {
        if (feof(f_h264bit_stream)) { ///file end
            nalu->len = (pos - 1) - nalu->startcodeprefix_len;
            memcpy (nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len);
            nalu->forbidden_bit = nalu->buf[0] & 0x80; //1 bit  a & b : 与的话，有0代表不取该位，值为0，有1代表取该位，该位是1则是1，是0则是0
            nalu->nal_reference_idc = nalu->buf[0] & 0x60; // 2 bit 取第二位
            nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;// 5 bit   后5位都是1，取后5位
            free(Buf);
            return pos - 1;
        }

        ///读取一字节
        Buf[pos++] = fgetc(f_h264bit_stream);
        ///一个字节一个字节的查找 第二个 start code
        info3 = FindStartCode3(&Buf[pos - 4]);
        if (info3 != 1)
            info2 = FindStartCode2(&Buf[pos - 3]);
        StartCodeFound = (info2 == 1 || info3 == 1);
    }

    // Here, we have found another start code (and read length of startcode bytes more than we should
    // have.  Hence, go back in the file
    rewind = (info3 == 1) ? -4 : -3;

    ///找到了第二个start code，将文件指针向前移start code的字节数，此时文件指针指向的地址就是一个nalu主体的最后地址
    if (0 != fseek (f_h264bit_stream, rewind, SEEK_CUR)) {
        free(Buf);
        printf("GetAnnexbNALU: Cannot fseek in the bit stream file");
    }

    // Here the Start code, the complete NALU, and the next start code is in the Buf.
    // The size of Buf is pos, pos+rewind are the number of bytes excluding the next start code
    // and (pos+rewind) - startcodeprefix_len is the size of the NALU excluding the start code

    nalu->len = (pos + rewind) - nalu->startcodeprefix_len;
    memcpy (nalu->buf, &Buf[nalu->startcodeprefix_len], nalu->len);//
    nalu->forbidden_bit = nalu->buf[0] & 0x80; //1 bit
    nalu->nal_reference_idc = nalu->buf[0] & 0x60; // 2 bit
    nalu->nal_unit_type = (nalu->buf[0]) & 0x1f;// 5 bit
    free(Buf);

    return (pos + rewind);
}

/**
 * Analysis H.264 Bitstream
 * @param url    Location of input H.264 bitstream file.
 */
int simplest_h264_parser(char *url){

    int buffersize = 100000;

    //FILE *myout=fopen("output_log.txt","wb+");
    FILE *myout = stdout;

    f_h264bit_stream = fopen(url, "rb+");
    if (f_h264bit_stream == NULL){
        printf("Open file error\n");
        return 0;
    }

    NALU_t *nalu = (NALU_t*)calloc(1, sizeof (NALU_t));//calloc申请一个内存块，且会初始化内存为0，count可以为其他值，内存大小为n*sizeof (NALU_t)
    if (nalu == NULL){
        printf("Alloc NALU Error\n");
        return 0;
    }

    nalu->max_size = buffersize;
    nalu->buf = (char*)calloc (buffersize, sizeof (char));
    if (nalu->buf == NULL){
        free (nalu);
        printf ("AllocNALU: n->buf");
        return 0;
    }

    int data_offset=0;
    int nal_num=0;
    printf("-----+-------- NALU Table ------+---------+\n");
    printf(" NUM |    POS  |    IDC |  TYPE |   LEN   |\n");
    printf("-----+---------+--------+-------+---------+\n");

    while(!feof(f_h264bit_stream))
    {
        int data_lenth = GetNALUFromAnnexb(nalu); ///data_lenth == one Annexb length(start code + nalu)

        char type_str[20] = { 0 };
        switch(nalu->nal_unit_type) {
            case NALU_TYPE_SLICE:sprintf(type_str,"SLICE");break;
            case NALU_TYPE_DPA:sprintf(type_str,"DPA");break;
            case NALU_TYPE_DPB:sprintf(type_str,"DPB");break;
            case NALU_TYPE_DPC:sprintf(type_str,"DPC");break;
            case NALU_TYPE_IDR:sprintf(type_str,"IDR");break;
            case NALU_TYPE_SEI:sprintf(type_str,"SEI");break;
            case NALU_TYPE_SPS:sprintf(type_str,"SPS");break;
            case NALU_TYPE_PPS:sprintf(type_str,"PPS");break;
            case NALU_TYPE_AUD:sprintf(type_str,"AUD");break;
            case NALU_TYPE_EOSEQ:sprintf(type_str,"EOSEQ");break;
            case NALU_TYPE_EOSTREAM:sprintf(type_str,"EOSTREAM");break;
            case NALU_TYPE_FILL:sprintf(type_str,"FILL");break;
        }
        char idc_str[20] = { 0 };
        switch(nalu->nal_reference_idc >> 5) {
            case NALU_PRIORITY_DISPOSABLE:sprintf(idc_str,"DISPOS");break;
            case NALU_PRIRITY_LOW:sprintf(idc_str,"LOW");break;
            case NALU_PRIORITY_HIGH:sprintf(idc_str,"HIGH");break;
            case NALU_PRIORITY_HIGHEST:sprintf(idc_str,"HIGHEST");break;
        }

        fprintf(myout, "%5d| %8d| %7s| %6s| %8d|\n", nal_num, data_offset, idc_str, type_str, nalu->len);

        data_offset = data_offset + data_lenth;

        nal_num++;
    }

    //Free
    if (nalu) {
        if (nalu->buf) {
            free(nalu->buf);
            nalu->buf=NULL;
        }
        free (nalu);
    }
    return 0;
}

#endif //BUILD_FFMPEG_LEARN_4_PARSER_H264_HPP
