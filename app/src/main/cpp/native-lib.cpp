#include <jni.h>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <aruco/aruco.h>
#include <mutex>

#include <fstream>
#include <iostream>
#include <opencv2/highgui.hpp>
#include <sstream>
#include <string>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>

using namespace std;
using namespace cv;

extern "C"
{
void JNICALL Java_unicamp_alantavares_nativearucondk_MainActivity_salt
        (JNIEnv *env, jobject instance,
         jlong matAddrGray,
         jint nbrElem) {

    Mat &mGr = *(Mat *) matAddrGray;

    for (int k = 0; k < nbrElem; k++) {
        int i = rand() % mGr.cols;
        int j = rand() % mGr.rows;
        mGr.at<uchar>(j, i) = 255;
    }
}
}
