#include "sknode.h"
#include <stddef.h>

skJsonNode *skJson_new(void *buff, size_t bufsize);
void skJson_drop(skJsonNode **json);
void skJson_drop_whole(skJsonNode **json);
