LOCAL_PATH := $(call my-dir)
APP_MODULES := libcubic
#APP_STL := stlport_static
APP_STL := gnustl_static
APP_OPTIM := release
APP_PLATFORM := android-19
#APP_ABI := armeabi armeabi-v7a
APP_ABI := armeabi
APP_CPPFLAGS := -frtti -DCOCOS2D_DEBUG=1 -fexceptions
