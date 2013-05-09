#include <jni.h>
#include <android/log.h>

#include <cstring>
#include <vector>

#include "common.h"

#define  LOG_TAG    "libnativerenderer"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

static JavaVM * javaVM;

// TODO: Unholy resource hack
struct resource {
    const char * name;
    const char * contents;
};

std::vector<struct resource> resources;

jint JNI_OnLoad(JavaVM * vm, void * unused) {
    LOGI("JNI_OnLoad called");
    javaVM = vm;
    return JNI_VERSION_1_6;
}

// TODO: Unholy resource hack
void * resourcecb(const char * filename) {
    for(int i = 0; i < resources.size(); i++) {
        if(strcmp(resources[i].name, filename) == 0)
            return strdup(resources[i].contents);
    }
    LOGE("Resource \"%s\" not found", filename);
    return NULL;
}

// TODO: Unholy resource hack
extern "C"
JNIEXPORT void JNICALL Java_edu_stanford_nativegraphics_NativeLib_passResource(JNIEnv * env, jobject obj, jstring jfileName, jstring jfileContents) {
    struct resource mResource;
    mResource.name = env->GetStringUTFChars(jfileName, NULL);
    mResource.contents = env->GetStringUTFChars(jfileContents, NULL);
    resources.push_back(mResource);
}

extern "C"
JNIEXPORT void JNICALL Java_edu_stanford_nativegraphics_NativeLib_init(JNIEnv * env, jobject obj, jint w, jint h) {
    LOGI("Native Setup() called.");
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
