#include <jni.h>
#include <string>
#include <pthread.h>
#include "AndroidLog.h"
#include "Listener.h"
#include "FFCallJava.h"
#include "MFFmpeg.h"
#include "Playstatus.h"
extern "C"{
#include "include/libavformat/avformat.h"
#include "include/libavcodec/avcodec.h"
#include "include/libavutil/avutil.h"
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wguet_myplayer_Demo_testFFmpeg(JNIEnv *env, jobject instance) {

    av_register_all();
    AVCodec *c_temp = av_codec_next(NULL);
    while (c_temp != NULL){
        switch (c_temp->type){
            case AVMEDIA_TYPE_VIDEO:
                LOGI("[Video]:%s", c_temp->name);
                break;
            case AVMEDIA_TYPE_AUDIO:
                LOGI("[Audio]:%s", c_temp->name);
                break;
            default:
                LOGI("[Other]:%s", c_temp->name);
                break;
        }
        c_temp = c_temp->next;
    }
}

/*********************************调用Java 方法***************************************/

_JavaVM *jvm = NULL;

Listener *javaListener;
pthread_t chidlThread;

void *childCallback(void *data) {

    Listener *javaListener1 = (Listener *) data;
    javaListener1->onError(0, 101, "c++ call java meid from child thread!");
    pthread_exit(&chidlThread);
}


//调用Java方法，非静态jobject instance ， 静态(JNIEnv *env, jclass type)
extern "C"
JNIEXPORT void JNICALL
Java_com_wguet_myplayer_Demo_callbackFromC(JNIEnv *env, jobject instance) {

    javaListener = new Listener(jvm, env, env->NewGlobalRef(instance));

    //主线程调用
//    javaListener->onError(1, 100, "c++ call java meid from main thread!");

    //子线程调用
    pthread_create(&chidlThread, NULL, childCallback, javaListener);
}


/**
 * 1.获取JVM对象 JavaVM是虚拟机在JNI中的表示，一个虚拟机中只有一个JavaVM对象，这个对象是线程共享的。JNIEnv类型是一个指向全部JNI方法的指针。该指针只在创建它的线程有效，不能跨线程传递。多线程无法共享。
 * 这个方法是在加载相应的.so包的时候，系统主动调用的
 * @param vm
 * @param reserved
 * @return
 */
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void* reserved) {
    JNIEnv *env;
    jvm = vm;
    if(vm->GetEnv((void **) &env, JNI_VERSION_1_6) != JNI_OK) {
        return -1;
    }
    return JNI_VERSION_1_6;
}

/**
 * 2.获取JVM对象
 */
//extern "C"
//JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved){
//    jint result = -1;
//    javaVM = vm;
//    JNIEnv *env;
//    if(vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK)
//    {
//
//        return result;
//    }
//    return JNI_VERSION_1_4;
//
//}



/***********************音频播放*********************************/
FFCallJava *callJava = NULL;
MFFmpeg *mFFmpeg = NULL;

PlayStatus *playStatus = NULL;
bool nexit = true;
pthread_t thread_start;

extern "C"
JNIEXPORT void JNICALL
Java_com_wguet_myplayer_player_FFPlayer_jniPrepared(JNIEnv *env, jobject instance, jstring source_) {

    const char *source = env->GetStringUTFChars(source_, 0);

    if(mFFmpeg == NULL){
        if(callJava == NULL){
            callJava = new FFCallJava(jvm, env, &instance);
        }
        callJava->onCallLoad(MAIN_THREAD, true);
        playStatus = new PlayStatus();
        mFFmpeg = new MFFmpeg(playStatus, callJava, source);
        mFFmpeg->parpared();
    } else{
        LOGI("mFFmpeg != NULL");
    }
//    env->ReleaseStringUTFChars(source_, source);
}

void *startCallBack(void *data){
    MFFmpeg *ffmpeg = (MFFmpeg *) data;
    ffmpeg->start();
//    pthread_exit(&thread_start);
    return 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wguet_myplayer_player_FFPlayer_jniStart(JNIEnv *env, jobject instance) {

    if(mFFmpeg != NULL){
        pthread_create(&thread_start, NULL, startCallBack, mFFmpeg);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wguet_myplayer_player_FFPlayer_jniPause(JNIEnv *env, jobject instance) {

    if(mFFmpeg != NULL){
        mFFmpeg->pause();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wguet_myplayer_player_FFPlayer_jniResume(JNIEnv *env, jobject instance) {

    if(mFFmpeg != NULL){
        mFFmpeg->resume();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wguet_myplayer_player_FFPlayer_jniStop(JNIEnv *env, jobject instance) {

    if (!nexit){
        return;
    }

    jclass jlz = env->GetObjectClass(instance);
    jmethodID jmid_next = env->GetMethodID(jlz, "onCallNext", "()V");

    nexit = false;
    if (mFFmpeg) {
        LOGI("jniStop mFFmpeg->release()");
        mFFmpeg->release();
        pthread_join(thread_start, NULL);
        delete(mFFmpeg);
        mFFmpeg = NULL;
        LOGI("mFFmpeg set NULL");
        if (callJava != NULL) {
            delete(callJava);
            callJava = NULL;
        }

        if (playStatus != NULL) {
            delete(playStatus);
            playStatus = NULL;
        }
    }
    nexit = true;
    env->CallVoidMethod(instance, jmid_next);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wguet_myplayer_player_FFPlayer_jniSeek(JNIEnv *env, jobject instance, jint seconds){

    if (mFFmpeg != NULL){
        mFFmpeg->seek(seconds);
    }
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_wguet_myplayer_player_FFPlayer_jniDuration(JNIEnv *env, jobject instance){

    if (mFFmpeg != NULL){
        return mFFmpeg->duration;
    }
    return 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wguet_myplayer_player_FFPlayer_jniSetVolume(JNIEnv *env, jobject thiz, jint percent) {
    if (mFFmpeg != NULL){
        mFFmpeg->setVolume(percent);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wguet_myplayer_player_FFPlayer_jniSetMute(JNIEnv *env, jobject thiz, jint mute) {
    if (mFFmpeg != NULL){
        mFFmpeg->setMute(mute);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wguet_myplayer_player_FFPlayer_jniSetPitch(JNIEnv *env, jobject thiz, jfloat pitch) {
    if(mFFmpeg != NULL){
        mFFmpeg->setPitch(pitch);
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_wguet_myplayer_player_FFPlayer_jniSetSpeed(JNIEnv *env, jobject thiz, jfloat speed) {
    if(mFFmpeg != NULL){
        mFFmpeg->setSpeed(speed);
    }
}

extern "C"
JNIEXPORT jint JNICALL
Java_com_wguet_myplayer_player_FFPlayer_jniSamplerate(JNIEnv *env, jobject thiz) {
    if(mFFmpeg != NULL){
        return mFFmpeg->getSampleRate();
    }
    return 0;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_wguet_myplayer_player_FFPlayer_jniStartStopRecord(JNIEnv *env, jobject thiz,
                                                           jboolean start) {
    if(mFFmpeg != NULL){
        mFFmpeg->startStopRecord(start);
    }
}