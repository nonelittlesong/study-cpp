// 导入本地的so库
private static boolean isLoaded;
static {
	if (!isLoaded) {
		System.loadLibrary("jpeg-turbo1500");
		System.loadLibrary("usb100");
		System.loadLibrary("uvc");
		System.loadLibrary("UVCCamera");
		isLoaded = true;
	}
}

protected long mNativePtr;

public UVCCamera() {
    mNativePtr = nativeCreate(); // 传递c++实例的地址，用于在多个cpp中使用同一个对象
    mSupportedSize = null;
}

private final native long nativeCreate(); // Java调用native方法的接口
