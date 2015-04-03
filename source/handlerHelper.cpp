#include "handlerHelper.h"
#include "common.h"

#include <stdio.h>

void fatalError(char* errstr)
{
    printf("%s: %d\n",errstr,GetLastError());
    perror(errstr);
    Sleep(5000);
    exit(0);
}
