#ifndef __SK_ERROR_H__
#define __SK_ERROR_H__

#include <stdio.h>

/* Errors description */
#define TABLE_SIZE_LIMIT_ERR " table size limit exceeded"
#define INVALID_KEY_ERR " invalid key value"
#define ZST_ERR " zero sized types not allowed"
#define OOM_ERR " out of memory"
#define INDEX_OOB_ERR " index out of bounds"
#define ALLOC_ERR " allocation too large"
#define CMP_FN_ERR " comparison function can't be NULL"
#define WRONG_NT_ERR " wrong node type"
#define INVALID_STR_ERR " invalid json string"
#define INVALID_VAL_ERR " invalid value provided"
#define SERIALIZER_NUMBER_ERR " errored while serializing number"
#define SERIALIZER_INVALID_JSON_ERR " trying to serialize invalid json element"
#define UNREACHABLE_ERR " unreachable code!"

/* Warnings text */
#define OVERFLOW_WARN " detected overflow"

/* Possible of Errors */
enum {
  TableSizeLimit = 1,
  InvalidKey = 2,
  ZeroSizedType = 3,
  OutOfMemory = 4,
  IndexOutOfBounds = 5,
  AllocationTooLarge = 6,
  MissingComparisonFn = 7,
  WrongNodeType = 8,
  InvalidString = 9,
  InvalidValue = 10,
  SerializerNumberError = 11,
  SerializerInvalidJson = 12,
  UnreachableCode = 13
};

/* Possible Warnings */
enum { OverflowDetected = 128 };

/* Indirection for correctly stringifying literal integers */
#define STRINGIFY(x) #x

/* Print err_msg to stderr */
#define SK_PRINT_ERR(msg) fprintf(stderr, "%s", msg)

/* Errors format */
#define SK_ERROR_FORMAT(filename, line, err)                                   \
  do {                                                                         \
    const char *errmsg;                                                        \
    switch (err) {                                                             \
    case TableSizeLimit:                                                       \
      errmsg = filename ":" STRINGIFY(line) TABLE_SIZE_LIMIT_ERR "\n";         \
      break;                                                                   \
    case InvalidKey:                                                           \
      errmsg = filename ":" STRINGIFY(line) INVALID_KEY_ERR "\n";              \
      break;                                                                   \
    case ZeroSizedType:                                                        \
      errmsg = filename ":" STRINGIFY(line) ZST_ERR "\n";                      \
      break;                                                                   \
    case OutOfMemory:                                                          \
      errmsg = filename ":" STRINGIFY(line) OOM_ERR "\n";                      \
      break;                                                                   \
    case IndexOutOfBounds:                                                     \
      errmsg = filename ":" STRINGIFY(line) INDEX_OOB_ERR "\n";                \
      break;                                                                   \
    case AllocationTooLarge:                                                   \
      errmsg = filename ":" STRINGIFY(line) ALLOC_ERR "\n";                    \
      break;                                                                   \
    case MissingComparisonFn:                                                  \
      errmsg = filename ":" STRINGIFY(line) CMP_FN_ERR "\n";                   \
      break;                                                                   \
    case WrongNodeType:                                                        \
      errmsg = filename ":" STRINGIFY(line) WRONG_NT_ERR "\n";                 \
      break;                                                                   \
    case InvalidString:                                                        \
      errmsg = filename ":" STRINGIFY(line) INVALID_STR_ERR "\n";              \
      break;                                                                   \
    case InvalidValue:                                                         \
      errmsg = filename ":" STRINGIFY(line) INVALID_VAL_ERR "\n";              \
      break;                                                                   \
    case SerializerNumberError:                                                \
      errmsg = filename ":" STRINGIFY(line) SERIALIZER_NUMBER_ERR "\n";        \
      break;                                                                   \
    case SerializerInvalidJson:                                                \
      errmsg = filename ":" STRINGIFY(line) SERIALIZER_INVALID_JSON_ERR "\n";  \
      break;                                                                   \
    case UnreachableCode:                                                      \
      errmsg = filename ":" STRINGIFY(line) UNREACHABLE_ERR "\n";              \
      break;                                                                   \
    }                                                                          \
    SK_PRINT_ERR(errmsg);                                                      \
  } while (0)

/* Throw error to stderr */
#define THROW_ERR(err) SK_ERROR_FORMAT(__FILE__, __LINE__, err)

/* Warnings format */
#define SK_WARNING_FORMAT(warn, ln, col)                                       \
  do {                                                                         \
    const char *warnmsg;                                                       \
    switch (warn) {                                                            \
    case OverflowDetected:                                                     \
      warnmsg = STRINGIFY(ln) ":" STRINGIFY(col) OVERFLOW_WARN;                \
      break;                                                                   \
    }                                                                          \
    SK_PRINT_ERR(warnmsg);                                                     \
  } while (0)

/* Throw warning to stderr */
#define THROW_WARN(warn, scanner)                                              \
  SK_WARNING_FORMAT(warn, scanner->iter.state.ln, scanner->iter.state.col)

#endif
