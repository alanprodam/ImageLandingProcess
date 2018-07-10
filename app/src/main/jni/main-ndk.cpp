#include <com_tzutalin_dlibtest_nativeclass_OpencvNativeClass.h>
#include <android/log.h>

#define  LOG_TAG    "someTag"
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)

JNIEXPORT jint JNICALL Java_com_tzutalin_dlibtest_nativeclass_OpencvNativeClass_covertGray
  (JNIEnv *, jclass, jlong addrRgba, jlong addrGray){
    Mat& mRgb = *(Mat*)addrRgba;
    Mat& mGray = *(Mat*)addrGray;

    int conv;
    jint retVal;
    conv = toGray(mRgb,mGray);

    retVal = (jint)conv;

    return retVal;
}

JNIEXPORT void JNICALL Java_com_tzutalin_dlibtest_nativeclass_OpencvNativeClass_FindFeatures
    (JNIEnv *, jclass, jlong addrGray1){
    Mat& imgGray1 = *(Mat*)addrGray1;

    vector<Point2f> points1;   //vectors to store the coordinates of the feature points
    vector<uchar> status;

    featureDetection(imgGray1, points1);    //detect features in imgGray1

    if((points1.size()!=NULL)){

        for(int i=0;i<points1.size();i++){
          circle(imgGray1, Point(points1.at(i).x, points1.at(i).y) ,3, CV_RGB(255,255,255), 1);
        }

        LOGD( "Numero de Features: %d", points1.size());
    }

}

//**********************************************************************************************************
//**********************************************************************************************************

int toGray(Mat img, Mat& gray){
    cvtColor(img, gray, CV_RGBA2GRAY);
    if(gray.rows==img.rows && gray.cols==img.cols){
        return 1;
    }
    else{
        return 0;
    }
}

//**********************************************************************************************************
//**********************************************************************************************************
void featureDetection(Mat& imgGray1, vector<Point2f>& points1)	{

    //uses FAST as of now, modify parameters as necessary
    vector<KeyPoint> keypoints_1;
    int fast_threshold = 30;
    bool nonmaxSuppression = true;

    FAST(imgGray1, keypoints_1, fast_threshold, nonmaxSuppression);
    KeyPoint::convert(keypoints_1, points1, vector<int>());

}
//**********************************************************************************************************
//**********************************************************************************************************