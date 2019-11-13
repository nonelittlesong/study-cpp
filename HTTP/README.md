* [libcurl](https://curl.haxx.se/libcurl/)
* [curl github](https://github.com/curl/curl)

## 安装 libcurl
查看系统架构：  
```
$ uname -a
$ dpkg --print-architecture
```

至2019.7.11，ubuntu16.04 默认没有 libcurl4。  
由于 ubuntu16.04 中很多程序依赖于 libcurl3， 手动安装 libcurl4 会引起冲突。  
libcurl4-openssl-dev 依赖于 libcurl4，安装可能有问题。  
```
$ sudo apt-get install libcurl4-openssl-dev
# 或者
$ sudo apt-get install libcurl4-gnutls-dev

$ whereis curl
curl: /usr/bin/curl /usr/include/curl /usr/share/man/man1/curl.1.gz

$ pkg-config libcurl --cflags --libs
-lcurl
```

## 堆栈溢出的问题
函数的局部变量（包括局部的静态数组）是存储在堆栈而非内存中，若程序中局部数组过多过大则有堆栈溢出的可能导致错误（比如Windows下大致4MB）。大数组建议使用全局的堆（new 或者 malloc），不仅安全可控，而且可以提高效率。  
比如：  
```cpp
// 效率较低
void Foo(void)
{
    const int N = 100000;
    double buffer[N];
    // ....
}
// Run Foo many times...
```
如果Foo要多次运行，那么上述函数的效率可能远远低于：  
```cpp
// 效率更高
// 全局空间，当然实际应用时这个东西可以封装的更优雅些
int GLOBALARRAYSIZE = 100000;
double* GlobalArray = (double*)malloc(sizeof(double)*GLOBALARRAYSIZE)

void Foo(void)
{
    double* buffer = GlobalArray;
    // ....
}
// Run Foo many times...

free(GlobalArray);
```
参考：  
- https://www.zhihu.com/question/42254123/answer/107678826  
