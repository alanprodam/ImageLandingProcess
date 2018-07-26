LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#opencv
OPENCVROOT:= OpenCV331
OPENCV_CAMERA_MODULES:=on
OPENCV_INSTALL_MODULES:=on
OPENCV_LIB_TYPE:=SHARED
include ${OPENCVROOT}/sdk/native/jni/OpenCV.mk

#aruco


LOCAL_SRC_FILES := main-ndk.cpp

LOCAL_LDLIBS     += -llog -ldl
LOCAL_C_INCLUDES += $(LOCAL_PATH)
LOCAL_MODULE := MyOpenCVLibs

include $(BUILD_SHARED_LIBRARY)