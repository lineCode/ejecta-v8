APP_ABI := armeabi-v7a x86 x86_64 arm64-v8a
#APP_ABI := arm64-v8a
#APP_STL := gnustl_static
APP_STL := c++_static
APP_PLATFORM := android-16
APP_CPPFLAGS += -std=c++11

NDK_TOOLCHAIN_VERSION=clang

APP_OPTIM := debug
