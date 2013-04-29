# The ARMv7 is significanly faster due to the use of the hardware FPU
APP_ABI := armeabi armeabi-v7a
APP_PLATFORM := android-8
APP_STL := stlport_static
#APP_STL := gnustl_static
# NOTE: Needed for OpenMesh, but increases binary size.
#APP_CPPFLAGS += -frtti
