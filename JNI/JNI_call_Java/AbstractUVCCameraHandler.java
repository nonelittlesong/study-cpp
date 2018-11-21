private final IFrameCallback mIFrameCallback = new IFrameCallback() {
    @Override
    public void onFrame(final Bytebuffer frame) {
        final MediaVideoBufferEncoder videoEncoder;
        synchronized (mSync) {
            videoEncoder = mVideoEncoder;
        }
        if (videoEncoder != null) {
            videoEncoder.frameAvailableSoon();
            videoEncoder.encode(frame);
        }
    }
}

private static final native int nativeSetFrameCallback(final long mNativePtr, final IFrameCallback callback, final int pixelFormat);
