#include <jni.h>
#include <android/log.h>
#include <android/bitmap.h>

#include "common.h"
#include "log.h"

#include <cstring>

static JavaVM * javaVM;
static jobject callbackObject;

jint JNI_OnLoad(JavaVM * vm, void * unused) {
    LOGI("JNI_OnLoad called");
    javaVM = vm;
    return JNI_VERSION_1_6;
}

static bool VerifyBitmap(JNIEnv * env, jobject bitmap, AndroidBitmapInfo & info) {
	int ret;
	if((ret = AndroidBitmap_getInfo(env, bitmap, &info)) < 0) {
		LOGE("AndroidBitmap_getInfo() failed ! error=%d", ret);
		return false;
	}
	if(info.format != ANDROID_BITMAP_FORMAT_RGBA_8888) {
		LOGE("Bitmap format is not RGBA_8888 !");
		return false;
	}
	return true;
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

    jstring jfile = (jstring) env->CallObjectMethod(callbackObject, method, jfileName);
    if(jfile != NULL) {
        const char *file = env->GetStringUTFChars(jfile, 0);
        char * returnFile = strdup(file);
        env->ReleaseStringUTFChars(jfile, file);

        if(isAttached)
            javaVM->DetachCurrentThread();
        return returnFile;
    }
    
    method = env->GetMethodID(cls, "drawableCallback", "(Ljava/lang/String;)Landroid/graphics/Bitmap;");
    if(!method) {
        if(isAttached)
            javaVM->DetachCurrentThread();
        return NULL;
    }
    
    jobject mBitmap = (jstring) env->CallObjectMethod(callbackObject, method, jfileName);
    AndroidBitmapInfo info;
    if(!VerifyBitmap(env, mBitmap, info)) {
        LOGE("Unable to locate resource %s.", fileName);
		return 0;
    }
    void * mPixels;
    if(AndroidBitmap_lockPixels(env, mBitmap, &mPixels) < 0)
		LOGE("AndroidBitmap_lockPixels() failed!");
    return mPixels;
    // TODO: AndroidBitmap_unlockPixels(env, mBitmap);
    
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
JNIEXPORT void JNICALL Java_edu_stanford_nativegraphics_NativeLib_pointerDown(JNIEnv * env, jobject obj, jfloat x, jfloat y, jint pointerID) {
    PointerDown(x, y, pointerID);
}

extern "C"
JNIEXPORT void JNICALL Java_edu_stanford_nativegraphics_NativeLib_pointerMove(JNIEnv * env, jobject obj, jfloat x, jfloat y, jint pointerID) {
    PointerMove(x, y, pointerID);
}

extern "C"
JNIEXPORT void JNICALL Java_edu_stanford_nativegraphics_NativeLib_pointerUp(JNIEnv * env, jobject obj, jfloat x, jfloat y, jint pointerID) {
    PointerUp(x, y, pointerID);
}

extern "C"
JNIEXPORT void JNICALL Java_edu_stanford_nativegraphics_NativeLib_updateOrientation(JNIEnv * env, jobject obj, jfloat roll, jfloat pitch, jfloat yaw) {
    UpdateOrientation(roll, pitch, yaw); 
}
