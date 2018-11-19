int UVCPreview::setFrameCallback(JNIEnv *env, jobject frame_callback_obj, int pixel_format) {
    ENTER();
    pthread_mutex_lock(&capture_mutex);
    {
        if (isRunning() && isCapturing()) {
            mIsCapturing = false;
            if (mFrameCallbackObj) {
                pthread_cond_signal(&capture_sync);
                pthread_cond_wait(&capture_sync, &capture_mutex); // waiting finishing capturing
            }
        }
        if (!env->IsSameObject(mFrameCallbackObj, frame_callback_obj)) {
            iframecallback_fields.onFrame = NULL;
            if (mFrameCallbackObj) {
                env->DeleteBlobalRef(mFrameCallbackObj);
            }
            mFrameCallbackObj = frame_callback_obj;
            if (frame_callback_obj) {
                // get method ID of java object for callback
                jclass clazz = env->GetObjectClass(frame_callback_obj);
                if (LIKELY(clazz)) {
                    iframecallback_fields.onFrame = env->GetMethodID(clazz, "onFrame", "(Ljava/nio/ByteBuffer;)V");
                } else {
                    LOGW("failed to get object class");
                }
                env->ExceptionClear();
                if (!iframecallback_fields.onFrame) {
                    LOGE("Can't find IFrameCallback#onFrame");
                    env->DeleteGlobalRef(frame_callback_obj);
                    mFrameCallbackObj = frame_callback_obj = NULL;
                }
            }
        }
        if (frame_callback_obj) {
            mPixelFormat = pixel_format;
            callbackPixelFormatChanged();
        }
    }
    pthread_mutex_unlock(&capture_mutex);
    RETURN(0, int);
}
