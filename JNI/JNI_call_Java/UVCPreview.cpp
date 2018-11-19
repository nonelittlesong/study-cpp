typedef struct {
    jmethodID onFrame;
} Fields_iframecallback;

Fields_iframecallback iframecallback_fields; // 保存回调函数的ID

jobject mFrameCallbackObj; // 回调函数的接口实例


// 设置回调函数
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
// 调用回调函数
void UVCPreview::do_capture_callback(JNIEnv *env, uvc_frame_t *frame) {
    ENTER();
    if (LIKELY(frame)) {
        uvc_frame_t *callback_frame = frame;
        if (mFrameCallbackObj) {
            if (mFrameCallbackFunc) {
                callback_frame = get_frame(callbackPixelBytes);
                if (LIKELY(callback_frame)) {
                    int b = mFrameCallbackFunc(frame, callback_frame);
                    recycle_frame(frame);
                    if (UNLIKELY(b)) {
                        LOGW("failed to convert for callback frame");
                        goto SKIP;
                    }
                } else {
                    LOGW("failed to allocate for callback frame");
                    callback_frame = frame;
                    goto SKIP;
                }
            }
            jobject buf = env->NewDirectByteBuffer(callback_frame->data, callbackPixelBytes);
            env->CallVoidMethod(mFrameCallbackObj, iframecallback_fields.onFrame, buf);
            env->ExceptionClear();
            env->DeleteLocalRef(buf);
        }
SKIP:
        recycle_frame(callback_frame);
    }
    EXIT();
}
