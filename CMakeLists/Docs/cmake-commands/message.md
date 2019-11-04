语法：  
```
message([<mode>] "message to display" ...)
```

mode：  
```
(none)         = 重要信息
STATUS         = 附加信息
WARNING        = 警告，继续执行
AUTHOR_WARNING = 警告（开发模式），继续执行
SEND_ERROR     = 错误，继续执行，但不生成
FATAL_ERROR    = 错误，停止执行
DEPRECATION    = 如果 CMAKE_ERROR_DEPRECATED 或者 CMAKE_WARN_DEPRECATED 开启，
                 则输出错误或警告。否则，不打印信息。
```
