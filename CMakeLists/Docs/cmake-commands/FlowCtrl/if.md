语法：  
```
if(expression)
  # then section.
  COMMAND1(ARGS ...)
  COMMAND2(ARGS ...)
  ...
elseif(expression2)
  # elseif section.
  COMMAND1(ARGS ...)
  COMMAND2(ARGS ...)
  ...
else(expression)
  # else section.
  COMMAND1(ARGS ...)
  COMMAND2(ARGS ...)
  ...
endif(expression)
```

expression：  

- `if(<constant>)`  
  True： `1`，`ON`，`YES`，`TRUE`，`Y`或者非零数常量。  
  False： `0`，`OFF`，`NO`，`FALSE`，`N`，`IGNORE`，`NOTFOUND`，空字符串或者以`-NOTFOUND`结束的常量。  
- `if(<variable|string>)`  
  True： 变量赋予的值不是 false 常量。  
- `if(NOT <expression>)`  
- `if(<expr1> AND <expr2>)`  
- `if(<expr1> OR <expr2>)`  
- `if(COMMAND command-name)`  
  True： 是一个可被调用的命令，宏或者函数。  
- `if(POLICY policy-id)`
  True： 是一个存在的 policy（(of the form `CMP<NNNN>`）。  
- `if(TARGET target-name)`
  True： 是由[add_executable()](https://cmake.org/cmake/help/v3.5/command/add_executable.html#command:add_executable), [add_library()](https://cmake.org/cmake/help/v3.5/command/add_library.html#command:add_library), 或 [add_custom_target()](https://cmake.org/cmake/help/v3.5/command/add_custom_target.html#command:add_custom_target) 创建的目标。  
- `if(EXISTS path-to-file-or-directory)`  
- `if(IS_DIRECTORY path-to-directory)`
- `if(<variable|string> MATCHES regex)`

