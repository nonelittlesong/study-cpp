#include "../include/helloShared.h"

HelloShared::HelloShared()
{
  this->shared = "shared";
}

HelloShared::HelloShared(char *s)
{
  this->shared = s;
}

char *HelloShared::echo()
{
  return this->shared;
}