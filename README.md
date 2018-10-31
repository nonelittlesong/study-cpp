# study_cpp
Some notes recording experence of learning cpp  
*wiki*  
* [Android.mk](https://github.com/nonelittlesong/study-cpp/wiki/Android.mk)
* [CmakeLists.txt to build .so in AS](https://github.com/nonelittlesong/study-cpp/wiki/CmakeLists.txt-to-build-.so-in-AS)
* [形参对实参的限制](https://github.com/nonelittlesong/study-cpp/wiki/%E5%BD%A2%E5%8F%82%E5%AF%B9%E5%AE%9E%E5%8F%82%E7%9A%84%E9%99%90%E5%88%B6)
* [#pragma once](https://github.com/nonelittlesong/study-cpp/wiki/%23pragma-once)
* [c++ notes](https://github.com/nonelittlesong/study-cpp/wiki/cpp-notes)
* [c notes](https://github.com/nonelittlesong/study-cpp/wiki/c-notes)

### compile and run in ubuntu
https://blog.csdn.net/liuzubing/article/details/78303167  
```c++
g++ -o hello hello.cpp
./hello
```
常用指令：  
* -std=c++11

### compile and run opencv c++
https://www.cnblogs.com/dyufei/p/8205077.html  

## 1. native打印android log
CmakeLists.txt:
```
target_link_libraries(native-lib
                      
                      log)
```
cpp:
```c++
#include <android/log.h>

#define TAG "MtcnnCpp"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,TAG,__VA_ARGS__)

LOGD("firstBox.size: %d",firstBbox_.size());
```
## 2. CMakeLists.txt打印信息
```
message(STATUS "bulabula")
message(FATAL_ERROR "bulabula")
```
## 3. 获取当前时间
```cpp
static unsigned long get_current_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec*1000000 + tv.tv_usec);
}
```

## Troubleshooting
### undefined reference to '__kmpc_for_static_init_4'
```
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
```
### '>>' should be '> >' within a nested template argument list
