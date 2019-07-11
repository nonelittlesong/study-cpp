* [libcurl](https://curl.haxx.se/libcurl/)
* [curl github](https://github.com/curl/curl)

# 安装 libcurl
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
```

