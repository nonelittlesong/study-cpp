参考：  
假设有一个TestHttpClient Solution，这个Solution下分别有TestHttpClient工程（可执行程序）和HttpClient工程（动态库）。  
这样我们需要三个CMakeLists.txt文件。其中HttpClient还依赖了libcurl第三方库。  
下面分别是不同目录下的CMakeLists.txt文件清单  

Solution目录：  
```
project(TestHttpClient)

cmake_minimum_required(VERSION 3.4.1)

# 要显示执行构建过程中的详细信息（比如为了得到更详细的出错信息）
set(CMAKE_VERBOSE_MAKEFILE ON)

# 添加子目录
add_subdirectory(HttpClient)
add_subdirectory(TestHttpClient)
```

HttpClient目录：  
```
message(STATUS "This is TestHttpClient_SOURCE_DIR="${TestHttpClient_SOURCE_DIR})
message(STATUS "This is CMAKE_SOURCE_DIR="${CMAKE_SOURCE_DIR})

# 设置编译出来的库文件（.a/.so）输出到指定目录
# 例如运行 cmake .. 的目录为build，则在build/lib目录下生成
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# 将一个目录下源文件保存到一个变量中
aux_source_directory(. LIB_SRC)

# 添加头文件搜索路径
include_directories(${TestHttpClient_SOURCE_DIR}/HttpClient)
# 设置输出库名
add_library(HttpClient
            SHARED
            ${LIB_SRC})
# 链接库文件
find_package(CURL)
if(CURL_FOUND)
    include_directories(${CURL_INCLUDE_DIR})
    target_link_libraries(HttpClient
                          ${CURL_LIBRARY})
else(CURL_FOUND)
    message(FATAL_ERROR "CURL library not found")
endif(CURL_FOUND)
```


TestHttpClient目录：  
```
# 设置输出可执行文件路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR/bin)

message(${PROJECT_SOURCE_DIR}/HttpClient)

# 添加库文件搜索路径
link_directories(${PROJECT_BINARY_DIR}/lib)

aux_source_directory(. APP_SRC)

# 如果调用"CMake -D DEBUG_MODE=ON .."
# 则为源文件设置_DEBUG宏
if(DEBUG_MODE)
    add_definitions(-D_DEBUG)
endif()

# 添加输出文件
add_executable(TestHttpClient
               ${APP_SRC})
               
#添加编译可执行程序所需要的链接库、如果有多个中间用空格隔开
#第一个参数是可执行程序名称，第二个开始是依赖库
#在这里根据名字XXX去寻找libXXX.a文件
TARGET_LINK_LIBRARIES(TestHttpClient HttpClient)
```
