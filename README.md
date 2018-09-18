# study_cpp
Some notes recording experence of learning cpp  
## wiki
* [Android.mk](https://github.com/nonelittlesong/study-cpp/wiki/Android.mk)
* [CmakeLists.txt to build .so in AS](https://github.com/nonelittlesong/study-cpp/wiki/CmakeLists.txt-to-build-.so-in-AS)
* [形参对实参的限制](https://github.com/nonelittlesong/study-cpp/wiki/%E5%BD%A2%E5%8F%82%E5%AF%B9%E5%AE%9E%E5%8F%82%E7%9A%84%E9%99%90%E5%88%B6)
* [#pragma once](https://github.com/nonelittlesong/study-cpp/wiki/%23pragma-once)
* [c++ notes](https://github.com/nonelittlesong/study-cpp/wiki/cpp-notes)
* [c notes](https://github.com/nonelittlesong/study-cpp/wiki/c-notes)

## compile and run in ubuntu
https://blog.csdn.net/liuzubing/article/details/78303167  
```c++
g++ -o hello hello.cpp
./hello
```
## compile and run opencv c++
https://www.cnblogs.com/dyufei/p/8205077.html  
## native打印android log
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
## Troubleshotting
### undefined reference to '__kmpc_for_static_init_4'
```
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
```
### '>>' should be '> >' within a nested template argument list
