//
// Created by workw on 2019/5/23.
//

#ifndef FFMUSIC_MFFMPEG_H
#define FFMUSIC_MFFMPEG_H

#include "FFCallJava.h"
#include "pthread.h"
#include "FFAudio.h"

extern "C"
{
#include "libavformat/avformat.h"
};

class MFFmpeg {

public:
    FFCallJava *callJava = NULL;
    const char* url = NULL;
    pthread_t decodeThread;
    AVFormatContext *pAVFormatCtx = NULL;//整个媒体流的处理流程中都会用到的对象,媒体文件或媒体流的构成和基本信息
    FFAudio *audio = NULL;

public:
    MFFmpeg(FFCallJava *callJava, const char *url);
    ~MFFmpeg();

    void parpared();
    void decodeFFmpegThread();
    void start();
};


#endif //FFMUSIC_MFFMPEG_H
