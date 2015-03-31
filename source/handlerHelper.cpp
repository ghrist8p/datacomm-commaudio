#include <windows.h>

void fatalError(char* errstr)
{
    perror(errstr);
    exit(0);
}
