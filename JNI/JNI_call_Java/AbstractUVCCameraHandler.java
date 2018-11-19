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
