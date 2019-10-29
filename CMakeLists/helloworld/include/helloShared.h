#ifndef HELLOSHARED_H
#define HELLOSHARED_H

class HelloShared
{
private:
  char *shared;

public:
  HelloShared();
  HelloShared(char *s);
  char *echo();
};

#endif