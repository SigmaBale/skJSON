#ifndef __SK_TYPES_H__
#define __SK_TYPES_H__

typedef long int skJsonInteger;
typedef char *skJsonString;
typedef double skJsonDouble;
typedef int skJsonBool;

typedef int (*CmpFn)(const void *, const void *);

typedef void (*FreeFn)(void *);

#ifndef __SK_JSON_H__
#ifdef bool
#undef bool
#endif
#define bool skJsonBool

#ifdef true
#undef true
#endif
#define true ((skJsonBool)1)

#ifdef false
#undef false
#endif
#define false ((skJsonBool)0)
#endif

#endif
