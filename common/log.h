
#ifdef ANDROID_NDK
    #include <android/log.h>
    #define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
    #define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#else // linux & ios
    // TODO: does this work for ios?
    #define  LOGI(...);  printf("INFO: "); printf(__VA_ARGS__); printf("\n");
    #define  LOGE(...);  printf("ERROR: "); printf(__VA_ARGS__); printf("\n");
#endif
