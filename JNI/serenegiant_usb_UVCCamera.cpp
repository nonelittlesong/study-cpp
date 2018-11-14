typedef jlong ID_TYPE; // 返回一个long给Java

/**
 * 访问java对象中的字段并赋值
 * GetObjectClass 获得类名
 * GetFieldID 获得类的属性名
 * SetLongField 给属性赋值
 */
static jlong setField_long(JNIEnv *env, jobject java_obj, const char *field_name, jlong val) {
#if LOCAL_DEBUG
    LOGV("set_Field_long:");
#endif
    jclass clazz = env->GetObjectClass(java_obj);
    jfieldID field = env->GetFieldID(clazz, field_name, "J");
    if (LIKELY(field))
        env->SetLongField(java_obj, field, val); // 已经完成赋值
    else {
        LOGE("__setField_long:field '%s' not found", field_name);
    }
#ifdef ANDROID_NDK
    env->DeleteLocalRef(clazz);
#endif
    return val; // 让Java可以访问val的值
}

/**
 * 调用UVCCamera
 * 将camera的地址赋值给mNativePtr
 */
static ID_TYPE nativeCreate(JNIEnv *env, jobject thiz) {
    ENTER();
    UVCCamera *camera = new UVCCamera();
    setField_long(env, thiz, "mNativePtr", reinterpret_cast<ID_TYPE>(camera));
    RETURN(reinterpret_cast<ID_TYPE>(camera), ID_TYPE);
}

jint registerNativeMethods(JNIEnv *env, const char *class_name, JNINativeMethod *methods, int num_methods) {
    int result = 0;
    jclass clazz = env->FindClass(class_name);
    if (LIKELY(clazz)) {
        int result = env->RegisterNatives(clazz, methods, num_methods);
        if (UNLIKELY(result<0)) {
            LOGE("registerNativeMethods failed(class=%s)", class_name);
        }
    } else {
        LOGE("registerNativeMethods: class'%s' not found", class_name);
    }
    return result;
}

static JNINativeMethod methods[] = {
    {"nativeCreate", "()J", (void *)nativeCreate},
    { "nativeSetButtonCallback", "(JLcom/serenegiant/usb/IButtonCallback;)I", (void *) nativeSetButtonCallback },
}

int register_uvccamera(JNIEnv *env) {
    LOGV("register_uvccamera:");
    if (registerNativeMethods(env, "com/serenegiant/usb/UVCCamera", methods, NUM_ARRAY_ELEMENTS(methods)) < 0) {
        return -1;
    }
    return 0;
}
