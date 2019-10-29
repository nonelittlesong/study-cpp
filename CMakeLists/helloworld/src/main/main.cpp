#include <iostream>
#include "../../include/helloShared.h"

int main()
{
  HelloShared *hello = new HelloShared("shared");
  std::cout << hello->echo() << std::endl;
  delete hello;
  return 0;
}