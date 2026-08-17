#include <string.h>
#include "demangle.h"

void main(int argc, char *argv[])
{
  char *rest;
  Name *name = Demangle(argv[1], rest);
  char buf[255];
  strcpy(buf, name->Text());
  buf[0]='\0';
}


 