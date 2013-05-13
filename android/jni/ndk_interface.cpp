#include <jni.h>
#include <android/log.h>

#include "common.h"

#include <cstring>

#define  LOG_TAG    "libnativerenderer"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

static JavaVM * javaVM;
static jobject callbackObject;

jint JNI_OnLoad(JavaVM * vm, void * unused) {
    LOGI("JNI_OnLoad called");
    javaVM = vm;
    return JNI_VERSION_1_6;
}

void * resourcecb(const char * fileName) {
    int status;
    JNIEnv *env;
    int isAttached = 0;

    if(!callbackObject)
    	return NULL;

    if((status = javaVM->GetEnv((void**)&env, JNI_VERSION_1_6)) < 0) {
        if((status = javaVM->AttachCurrentThread(&env, NULL)) < 0)
            return NULL;
        isAttached = 1;
    }

    jclass cls = env->FindClass("edu/stanford/nativegraphics/NativeLib");
    if(!cls) {
        if(isAttached)
            javaVM->DetachCurrentThread();
        return NULL;
    }

    jmethodID method = env->GetMethodID(cls, "stringCallback", "(Ljava/lang/String;)Ljava/lang/String;");
    if(!method) {
        if(isAttached)
            javaVM->DetachCurrentThread();
        return NULL;
    }

    jstring jfileName = env->NewStringUTF(fileName);

    LOGI("Pre Java callback.");
    jstring jfile = (jstring) env->CallObjectMethod(callbackObject, method, jfileName);
    LOGI("Post Java callback.");

    const char *file = env->GetStringUTFChars(jfile, 0);
    char * returnFile = strdup(file);
    env->ReleaseStringUTFChars(jfile, file);

    if(isAttached)
        javaVM->DetachCurrentThread();

    return returnFile;
}

extern "C"
JNIEXPORT void JNICALL Java_edu_stanford_nativegraphics_NativeLib_init(JNIEnv * env, jobject obj, jint w, jint h) {
    LOGI("Native Setup() called.");
    callbackObject = env->NewGlobalRef(obj);
    SetResourceCallback(resourcecb);
    Setup(w, h);
}

// TODO: release
//(*env)->DeleteGlobalRef(env, gCallbackObject);
//    gCallbackObject = NULL;
//}

extern "C"
JNIEXPORT void JNICALL Java_edu_stanford_nativegraphics_NativeLib_renderFrame(JNIEnv * env, jobject obj) {
    RenderFrame();
}

extern "C"
JNIEXPORT void JNICALL Java_edu_stanford_nativegraphics_NativeLib_pointerDown(JNIEnv * env, jobject obj, jfloat x, jfloat y) {
    PointerDown(x, y);
}

extern "C"
JNIEXPORT void JNICALL Java_edu_stanford_nativegraphics_NativeLib_pointerMove(JNIEnv * env, jobject obj, jfloat x, jfloat y) {
    PointerMove(x, y);
}

extern "C"
JNIEXPORT void JNICALL Java_edu_stanford_nativegraphics_NativeLib_pointerUp(JNIEnv * env, jobject obj, jfloat x, jfloat y) {
    PointerUp(x, y);
}
