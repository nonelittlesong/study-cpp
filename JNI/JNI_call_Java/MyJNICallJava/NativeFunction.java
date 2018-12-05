package com.faceattributes;

import com.faceattributes.monitor.IJavaCallback;

public class NativeFunction {

    // TODO native原型
    //人脸检测模型导入
    public native boolean ModelInit(String path_);

    public native int makeFace(long jrgbaddr);

    public native void setCallback(final IJavaCallback callback);

    static {
        System.loadLibrary("native-lib");
    }

}
