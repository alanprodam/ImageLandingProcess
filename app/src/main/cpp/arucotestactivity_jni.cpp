
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

namespace Java_uco_ava_core_ArucoTestActivity {
    aruco::CameraParameters
    cameraCalibrate(std::vector<std::vector<aruco::Marker> > &allMarkers, int imageWidth,
                    int imageHeight, float markerSize, float *currRepjErr);

    static std::vector<aruco::Marker> lastDetectedMarkers;
    static std::vector<std::vector<aruco::Marker> > CalibrationMarkers;
    static aruco::CameraParameters cameraParameters;
    static std::mutex detectionMutex;
    static aruco::MarkerDetector MDetector;
    std::string ConvertJString(JNIEnv* env, jstring str);

    static float markerSize=1;
    void splitView (cv::Mat &io);
    std::stringstream LogStream;

};


using namespace std;
using namespace cv;
using namespace aruco;
using namespace Java_uco_ava_core_ArucoTestActivity;


extern "C" {


JNIEXPORT void JNICALL
Java_uco_ava_core_ArucoTestActivity_jniInitNativeCalib(JNIEnv *env,
                                                       jobject) {
    lastDetectedMarkers.clear();
    CalibrationMarkers.clear();
    cameraParameters.clear();
    LogStream << "Called Java_uco_ava_core_ArucoTestActivity_jniInitNativeCalib" << endl;


}
JNIEXPORT void JNICALL
Java_uco_ava_core_ArucoTestActivity_jniSetMarkerDetectorParameters(JNIEnv *env,
                                                                   jobject, jstring MarkerType,
                                                                   jstring DetectionMode,
                                                                   jstring CornerRefinement,
                                                                   jfloat minmarkersize,
                                                                   jfloat jmarkerSize,
                                                                   jboolean detectEnclosed) {

    std::unique_lock<std::mutex> lock(detectionMutex);
    LogStream << "jniSetMarkerDetectorParameters" << endl;

    std::string mydictionary = "/storage/emulated/0/data/myconnect.dict";
    LogStream << mydictionary << endl;
    std::string myconfig = "/storage/emulated/0/data/arucoConfig.yml";

    std::string mtype = ConvertJString(env, MarkerType);
    std::string dmode = ConvertJString(env, DetectionMode);
    std::string crefmode = ConvertJString(env, CornerRefinement);

    MDetector.setDictionary(mydictionary);
    if (dmode.find("FAST") != std::string::npos)
        MDetector.getParameters().setDetectionMode(aruco::DM_FAST, minmarkersize);
    else if (dmode.find("NORMAL") != std::string::npos)
        MDetector.getParameters().setDetectionMode(aruco::DM_NORMAL, minmarkersize);
    else if (dmode.find("VIDEO_FAST") != std::string::npos)
        MDetector.getParameters().setDetectionMode(aruco::DM_VIDEO_FAST, minmarkersize);

    MDetector.getParameters().detectEnclosedMarkers(detectEnclosed);
    markerSize = jmarkerSize;

}

JNIEXPORT void JNICALL
Java_uco_ava_core_ArucoTestActivity_jniProcessCameraFrame(
        JNIEnv *env,
        jobject, jlong ImageGray, jlong ImageColor,jboolean splitView4Goggles,jboolean showThreshold) {
    cv::Mat &matgray = *(cv::Mat *) ImageGray;
    cv::Mat &matcolor = *(cv::Mat *) ImageColor;

        if (matgray.type() != CV_8UC1) return;

        std::unique_lock<std::mutex> lock(detectionMutex);

        lastDetectedMarkers = MDetector.detect(matgray, cameraParameters, markerSize);



    if (showThreshold)
        cv::cvtColor(MDetector.getThresholdedImage(),matcolor,CV_GRAY2BGRA);


    //LogStream << "jniProcessCameraFrame: " << cameraParameters.isValid() << endl;

    for (auto m:lastDetectedMarkers)
        m.draw(matcolor, cv::Scalar(0, 0, 255), 2, true);


    if (cameraParameters.isValid()) {
        for (auto m:lastDetectedMarkers){
            //aruco::CvDrawingUtils::draw3dAxis(matcolor, m, cameraParameters, 2);
            //aruco::CvDrawingUtils::draw3dCube(matcolor, m, cameraParameters, 2);
            LogStream << " LandMarker [" << m.id << "]: " <<
                 "  Tx: " << m.Tvec.ptr<float>(0)[0] << " m "<<
                 "\tTy: " << m.Tvec.ptr<float>(1)[0] << " m "<<
                 "\tTz: " << m.Tvec.ptr<float>(2)[0] << " m "<<
                 "\tRx: " << m.Rvec.ptr<float>(0)[0] << " rad "<<
                 "\tRy: " << m.Rvec.ptr<float>(1)[0] << " rad "<<
                 "\tRz: " << m.Rvec.ptr<float>(2)[0] << " rad "<< endl;
            putText(matcolor, "X", Point(matcolor.cols / 2, matcolor.rows / 2), 1.2, 1.2, Scalar(0, 0, 255), 2);
            //putText(matcolor, "Tz: " + to_string(m.Tvec.ptr<float>(2)[0]) + " m ", Point(10, 480 / 2), 1.2, 1.2, Scalar(0, 0, 255), 2);
        }
    }

    if(splitView4Goggles) splitView(matcolor);

}


JNIEXPORT jboolean JNICALL
Java_uco_ava_core_ArucoTestActivity_jniSetCalibrationParams(JNIEnv *env,
                                                            jobject, jstring calibparams) {


    stringstream sstr;
    sstr << ConvertJString(env, calibparams);
    sstr >> cameraParameters;
    return cameraParameters.isValid();


}
JNIEXPORT jstring JNICALL
Java_uco_ava_core_ArucoTestActivity_jniGetLog(JNIEnv *env, jobject) {


    string str = LogStream.str();
    LogStream.str(std::string());
    return env->NewStringUTF(str.c_str());
}
}

namespace Java_uco_ava_core_ArucoTestActivity{
    void splitView (cv::Mat &io){
        int w2=io.cols/2;
        cv::Mat row(1,io.cols,CV_8UC4);
        cv::Vec4b *rowPtr=row.ptr<cv::Vec4b>(0);
        for(int y=0;y<io.rows;y++){
            cv::Vec4b *ptr=io.ptr<cv::Vec4b>(y);
            memcpy(rowPtr,ptr,io.cols*4);
            memset(ptr,0,io.cols*4);
            for(int x=0;x<w2;x++){
                ptr[x+w2]=ptr[x]=rowPtr[x*2];
            }

        }
    }
    std::string ConvertJString(JNIEnv* env, jstring str)
    {
        const jsize len = env->GetStringUTFLength(str);
        const char* strChars = env->GetStringUTFChars(str, (jboolean *)0);

        std::string Result(strChars, len);
        env->ReleaseStringUTFChars(str, strChars);

        return Result;
    };

}

