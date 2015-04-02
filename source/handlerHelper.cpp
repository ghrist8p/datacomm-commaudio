#include "handlerHelper.h"

#include "common.h"

void fatalError(char* errstr)
{
    perror(errstr);
    exit(0);
}
