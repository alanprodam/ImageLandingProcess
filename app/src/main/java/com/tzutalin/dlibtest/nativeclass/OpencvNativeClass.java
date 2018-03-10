package com.tzutalin.dlibtest.nativeclass;

/**
 * Created by alantavares on 27/04/17.
 */

public class OpencvNativeClass {

    public native static int covertGray(long mRgba,long mGray);

    public native static void FindFeatures(long matAddrRgba);


}
