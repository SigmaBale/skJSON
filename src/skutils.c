#include "skerror.h"
#include "skutils.h"
#include <stdlib.h>
#include <string.h>

char*
strdup_ansi(const char* str)
{
    char*  dup;
    size_t len;

    len = strlen(str) + 1;
    dup = malloc(len);

    if(is_null(dup)) {
        THROW_ERR(OutOfMemory);
        return NULL;
    }

    memcpy(dup, str, len);

    return dup;
}
