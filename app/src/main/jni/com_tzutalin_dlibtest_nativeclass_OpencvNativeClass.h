#include <jni.h>
#include "opencv2/opencv.hpp"
#include <string>
#include <vector>
#include <ctime>
#include <ctype.h>
#include <android/log.h>
#include <iostream>
#include <fstream>
#include <android/bitmap.h>

#include <stdio.h>
#include <stdlib.h>

using namespace std;
using namespace cv;

#ifndef _Included_com_tzutalin_dlibtest_nativeclass_OpencvNativeClass
#define _Included_com_tzutalin_dlibtest_nativeclass_OpencvNativeClass
#ifdef __cplusplus
extern "C" {
#endif

int toGray(Mat img, Mat& gray);

void featureDetection(Mat& imgGray1, vector<Point2f>& points1);

/*
 * Class:     com_tzutalin_dlibtest_nativeclass_OpencvNativeClass
 * Method:    covertGray
 * Signature: (JJ)I
 */
JNIEXPORT jint JNICALL Java_com_tzutalin_dlibtest_nativeclass_OpencvNativeClass_covertGray
  (JNIEnv *, jclass, jlong, jlong);

/*
 * Class:     com_tzutalin_dlibtest_nativeclass_OpencvNativeClass
 * Method:    FindFeatures
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_com_tzutalin_dlibtest_nativeclass_OpencvNativeClass_FindFeatures
  (JNIEnv *, jclass, jlong);

#ifdef __cplusplus
}
#endif
#endif