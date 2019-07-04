//
// Created by userwang on 2019/5/22.
//

#ifndef FFMUSIC_FFCALLJAVA_H
#define FFMUSIC_FFCALLJAVA_H

#include <linux/stddef.h>
#include "jni.h"
#include "AndroidLog.h"

#define MAIN_THREAD 0
#define CHILD_THREAD 1

class FFCallJava {
public:
    _JavaVM *javaVM;
    JNIEnv *jniEnV;
    jobject jobj;

    jmethodID jmid_prepared;

public:
    FFCallJava(_JavaVM *javaVM, JNIEnv *env, jobject *job);
    ~FFCallJava();

    void onCallPrepared(int type);
};


#endif //FFMUSIC_FFCALLJAVA_H