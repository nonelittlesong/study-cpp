jni 调用 java  

### 1. jmethodID GetMethodID(jclass clazz, const char \*name, const char \*sig);
获取一个Java方法的ID。返回非静态类或接口实例方法的ID。  
这个方法可以是某个clazz的超类中定义，也可从clazz继承。  
