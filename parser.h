#ifndef __SK_JSON_H__
#define __SK_JSON_H__

#include <stddef.h>

typedef struct skJson skJson;

skJson *sk_json_new(void *buff, size_t bufsize);

#endif
