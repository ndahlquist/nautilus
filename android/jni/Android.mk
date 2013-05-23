LOCAL_PATH := $(call my-dir)/
PROJECT_ROOT_PATH := ../..

# Main binary
include $(CLEAR_VARS)

APP_ABI         := all

LOCAL_MODULE    := nativegraphics

LOCAL_C_INCLUDES := $(LOCAL_PATH)$(PROJECT_ROOT_PATH)/common/ \
                    $(LOCAL_PATH)$(PROJECT_ROOT_PATH)/common/libst/include/
# $(LOCAL_PATH)/eigen/include/eigen3 

LIBST_SOURCES   := $(wildcard $(PROJECT_ROOT_PATH)/common/libst/*.cpp)

LOCAL_SRC_FILES := ndk_interface.cpp \
                   $(PROJECT_ROOT_PATH)/common/common.cpp \
                   $(PROJECT_ROOT_PATH)/common/RenderObject.cpp \
                   $(PROJECT_ROOT_PATH)/common/transform.cpp \
                   $(PROJECT_ROOT_PATH)/common/glsl_helper.cpp \
                   $(PROJECT_ROOT_PATH)/common/RenderLight.cpp
                   
LOCAL_LDLIBS    := -llog \
                   -lGLESv2 \
                   -ljnigraphics \
                   -ldl
                   
APP_OPTIM       := release

LOCAL_CFLAGS    := -march=armv7 \
                   -mfloat-abi=softfp \
                   -mfpu=vfp \
                   -ffast-math \
                   -O2 \
                   -DANDROID_NDK \
                   -Werror

include $(BUILD_SHARED_LIBRARY)
