jni 调用 java  

### 1. jmethodID GetMethodID(jclass clazz, const char \*name, const char \*sig);
获取一个Java方法的ID。返回非静态类或接口实例方法的ID。  
这个方法可以是某个clazz的超类中定义，也可从clazz继承。  
### 2. ByteBuffer与JNI交互
在Java1.4版本中引入的JNI有三个函数可以用于NIO的直接缓冲器。一个直接字节缓冲器是一个用于字节数据的容器，Java将尽力在它上面执行本机I/O操作。JNI定义了三个用于NIO操作的函数。  
```
/*基于到存储器地址的指针以及存储器长度（容量），函数分配并且返回一个新的Java.nio.ByteBuffer。如果函数没有针对当前Java虚拟机实现，则返回NULL，或者抛出一个异常。如果没有存储器可用，则将会抛出一个OutOfMemoryException。*/
jobject NewDirectByteBuffer(void* address, jlong capacity);
/*GetDirectBufferAddress函数返回一个指向被传入的java.nio.ByteBuffer对象的地址指针。如果函数尚未针对当前虚拟机实现，或者如果buf不是java.nio.ByteBuffer的一个对象，又或者存储器区尚未定义，则都将返回NULL。*/
void* GetDirectBufferAddress(jobject buf);
/*GetDirectBufferCapacity函数返回被传入的java.nio.ByteBuffer对象的容量（以字节计数）。如果函数没有针对当前环境实现，或者如果buf不是java.nio.ByteBuffer类型的对象返回-1。*/
jlong GetDirectBufferCapacity(jobject buf);
```

## Troubleshotting
### 1. use of invalid jobject 0xffcf7e30
JNI每个线程对应一个JNIEnv\*, 当前JNIEnv\*不能访问其他JNIEnv\*。  
