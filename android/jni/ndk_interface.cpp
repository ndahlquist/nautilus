#include <jni.h>
#include <android/log.h>

#include "common.h"

#define  LOG_TAG    "libnativerenderer"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

extern "C"
JNIEXPORT void JNICALL Java_edu_stanford_nativegraphics_NativeLib_init(JNIEnv * env, jobject obj, jint w, jint h) {
    LOGI("Native Setup() called.");
    Setup(w, h);
}

extern "C"
JNIEXPORT void JNICALL Java_edu_stanford_nativegraphics_NativeLib_step(JNIEnv * env, jobject obj) {
    RenderFrame();
}
