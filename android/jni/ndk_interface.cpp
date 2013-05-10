#include <jni.h>
#include <android/log.h>

#include <cstring>

#include "common.h"

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

void resourcecb(const char * filename) {
    int status;
    JNIEnv *env;
    int isAttached = 0;

    if(!callbackObject)
    	return;

    if((status = javaVM->GetEnv((void**)&env, JNI_VERSION_1_6)) < 0) {
        if((status = javaVM->AttachCurrentThread(&env, NULL)) < 0)
            return;
        isAttached = 1;
    }

    jclass cls = env->FindClass("edu/stanford/nativegraphics/NativeLib");
    if(!cls) {
        if(isAttached)
            javaVM->DetachCurrentThread();
        return;
    }

    jmethodID method = env->GetMethodID(cls, "stringCallback", "(Ljava/lang/String;)Ljava/lang/String;");
    if(!method) {
        if(isAttached)
            javaVM->DetachCurrentThread();
        return;
    }

    jstring fileName = env->NewStringUTF("This string comes from JNI");

    LOGI("Pre-callvoidmethod");
    jstring file = (jstring) env->CallObjectMethod(callbackObject, method, fileName);
    LOGI("Post-callvoidmethod");

    if(isAttached)
        javaVM->DetachCurrentThread();
    /*jclass handlerClass = javaEnv->FindClass("edu/stanford/nativegraphics/NativeLib"); // TODO
    if(!handlerClass) {
        LOGE("Unable to locate java class edu/stanford/nativeagraphics/NativeLib");
        return;
    }

    jmethodID javaMid = javaEnv->GetMethodID(handlerClass, "stringCallback", "()V");
    if(!javaMid) {
        LOGE("Unable to locate java method stringCallback");
        return;
    }
    
    //jstring jstr = javaEnv->NewStringUTF("This string comes from JNI");
    //jbyteArray jbArray = javaEnv->NewByteArray(strlen(filename));
    //javaEnv->SetByteArrayRegion(jbArray, 0, strlen(filename), (jbyte*) filename);

    //jobject result = 
    LOGI("Pre-callvoidmethod");
    javaEnv->CallVoidMethod(savedObj, javaMid);
    LOGI("Post-callvoidmethod");
    
    //const char* str = javaEnv->GetStringUTFChars((jstring) result, NULL);
    
    //LOGI("resourcecb(%s) = %s", filename, str);
    if(javaEnv->ExceptionCheck()) {
        javaEnv->ExceptionDescribe();
        javaEnv->ExceptionClear();
    }*/
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
JNIEXPORT void JNICALL Java_edu_stanford_nativegraphics_NativeLib_step(JNIEnv * env, jobject obj) {
	RenderFrame();
}
