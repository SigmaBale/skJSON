#ifndef __SK_JSON_H__
#define __SK_JSON_H__

#include "sknode.h"
#include <stddef.h>

/* Marker macro for public functions */
#define PUBLIC(ret) ret

/* Opaque type representing the Json Element */
typedef skJsonNode skJson;
/* Parse the Json tree from 'buff' of size 'bufsize' */
PUBLIC(skJson *) skJson_parse(char *buff, size_t bufsize);
/* Return 'json' element type */
PUBLIC(unsigned int) skJson_type(skJsonNode *json);
/* Return the parent element of 'json' element */
PUBLIC(skJsonNode *) skJson_parent(skJsonNode *json);
/* Return value of 'json' element */
PUBLIC(void *) skJson_value(skJsonNode *json);
/* Drops the 'json' element including its sub-elements. */
PUBLIC(void) skJson_drop(skJson **json);
/* Drops the whole 'json' structure from the root. */
PUBLIC(void) skJson_drop_whole(skJson **json);
/* Transforms the 'json' element into Json Integer element with value 'n' */
PUBLIC(skJsonNode *) skJson_transform_into_int(skJsonNode *json, long int n);
/* Transforms the 'json' element into Json Double element with value 'n' */
PUBLIC(skJsonNode *) skJson_transform_into_double(skJsonNode *json, double n);
/* Transforms the 'json' element into Json Bool element with value 'boolean' */
PUBLIC(skJsonNode *) skJson_transform_into_bool(skJsonNode *json, bool boolean);
/* Transforms the 'json' element into Json String Reference element with value
 * 'string_ref' */
PUBLIC(skJsonNode *)
skJson_transform_into_stringlit(skJsonNode *json, const char *string_ref);
/* Transforms the 'json' element into Json String element with value 'string' */
PUBLIC(skJsonNode *)
skJson_transform_into_string(skJsonNode *json, const char *string);
/* Transforms the 'json' element into empty Json Array element */
PUBLIC(skJsonNode *) skJson_transform_into_empty_array(skJsonNode *json);
/* Transforms the 'json' element into empty Json Object element */
PUBLIC(skJsonNode *) skJson_transform_into_empty_object(skJsonNode *json);

/* Create integer Json element from integer 'n'. */
PUBLIC(skJsonNode *) skJson_integer_new(long int n);
/* Sets the new integer value 'n' for 'json' element, returns true
 * if the 'json' value was changed, false if 'json is not int element. */
PUBLIC(bool) skJson_integer_set(skJsonNode *json, long int n);

/* Create double Json element from double 'n' */
PUBLIC(skJsonNode *) skJson_double_new(double n);
/* Sets the new double value 'n' for 'json' element, returns true
 * if the 'json' value was changed, false if 'json is not double element. */
PUBLIC(bool) skJson_double_set(skJsonNode *json, double n);

/* Create boolean Json element from 'boolean' */
PUBLIC(skJsonNode *) skJson_bool_new(bool boolean);
/* Sets the new bool value 'boolean' for 'json' element, returns true
 * if the 'json' value was changed, false if 'json is not bool element. */
PUBLIC(bool) skJson_bool_set(skJsonNode *json, bool boolean);

/* Create null Json element */
PUBLIC(skJsonNode *) skJson_null_new(void);

/* Create string Json element from 'string' by creating a new allocation */
PUBLIC(skJsonNode *) skJson_string_new(const char *string);
/* Duplicates and sets the new value for 'json' element freeing the old string
 * value. Returns true if 'json' is string element and setting of string was
 * successfull, otherwise false. */
PUBLIC(bool) skJson_string_set(skJsonNode *json, const char *string);

/* Create string Json element from 'string' by holding only a reference to it */
PUBLIC(skJsonNode *) skJson_stringlit_new(const char *string);
/* Sets the new 'string' reference value for 'json' element. */
PUBLIC(bool) skJson_stringlit_set(skJsonNode *json, const char *string);

/* Create an empty Json array element */
PUBLIC(skJsonNode *) skJson_array_new(void);
/* Push 'string' into the 'json' array */
PUBLIC(bool) skJson_array_push_str(skJsonNode *json, const char *string);
/* Insert 'string' into the 'json' array at 'index'. */
PUBLIC(bool)
skJson_array_insert_str(skJsonNode *json, const char *string, size_t index);
/* Push 'string' reference into the 'json' array. */
PUBLIC(bool) skJson_array_push_strlit(skJsonNode *json, const char *string);
/* Insert 'string' reference into the 'json' array at 'index'. */
PUBLIC(bool)
skJson_array_insert_strlit(skJsonNode *json, const char *string, size_t index);
/* Push 'n' integer into the 'json' array. */
PUBLIC(bool) skJson_array_push_int(skJsonNode *json, long int n);
/* Insert 'n' integer into the 'json' array at 'index'. */
PUBLIC(bool)
skJson_array_insert_int(skJsonNode *json, long int n, size_t index);
/* Push 'n' double into the 'json' array. */
PUBLIC(bool) skJson_array_push_double(skJsonNode *json, double n);
/* Insert 'n' double into the 'json' array at 'index'. */
PUBLIC(bool)
skJson_array_insert_double(skJsonNode *json, double n, size_t index);
/* Push 'boolean' into the 'json' array. */
PUBLIC(bool) skJson_array_push_bool(skJsonNode *json, bool boolean);
/* Insert 'boolean' into the 'json' array at 'index'. */
PUBLIC(bool)
skJson_array_insert_bool(skJsonNode *json, bool boolean, size_t index);
/* Push NULL element into the 'json' array */
PUBLIC(bool) skJson_array_push_null(skJsonNode *json);
/* Insert NULL element into the 'json' array at 'index'. */
PUBLIC(bool) skJson_array_insert_null(skJsonNode *json, size_t index);
/* Push Json 'element' into the 'json' array. */
PUBLIC(bool) skJson_array_push_element(skJsonNode *json, skJsonNode *element);
/* Insert Json 'element' into the 'json' array at 'index'. */
PUBLIC(bool)
skJson_array_insert_element(skJsonNode *json, skJsonNode *element,
                            size_t index);
/* Create Json array from array 'strings' of 'count' string values */
PUBLIC(skJsonNode *)
skJson_array_from_strings(const char *const *strings, size_t count);
/* Create Json array from array 'strings' of 'count' string reference values */
PUBLIC(skJsonNode *)
skJson_array_from_litstrings(const char *const *strings, size_t count);
/* Create Json array from array 'integers' of 'count' integer values */
PUBLIC(skJsonNode *)
skJson_array_from_integers(const int *integers, size_t count);
/* Create Json array from array 'doubles' of 'count' double values */
PUBLIC(skJsonNode *)
skJson_array_from_doubles(const double *doubles, size_t count);
/* Create Json array from array 'booleans' of 'count' boolean values */
PUBLIC(skJsonNode *)
skJson_array_from_booleans(const bool *booleans, size_t count);
/* Create Json array of 'count' NULL json elements */
PUBLIC(skJsonNode *) skJson_array_from_nulls(size_t count);
/* Create Json array from array 'elements' of 'count' Json elements */
PUBLIC(skJsonNode *)
skJson_array_from_elements(const skJsonNode *const *elements, size_t count);
/* Pop Json element from 'json' array */
PUBLIC(skJsonNode *) skjson_array_pop(skJsonNode *json);
/* Remove Json element from 'json' array at 'index' */
PUBLIC(bool) skjson_array_remove(skJsonNode *json, size_t index);
/* Return 'json' array length */
PUBLIC(size_t) skJson_array_len(skJsonNode *json);
/* Return front Json element from 'json' array */
PUBLIC(skJsonNode *) skJson_array_front(skJsonNode *json);
/* Return Json element at the back of 'json' array */
PUBLIC(skJsonNode *) skJson_array_back(skJsonNode *json);
/* Return Json element at 'index' from 'json' array */
PUBLIC(skJsonNode *) skJson_array_index(skJsonNode *json, size_t index);

/* Create an empty Json object */
PUBLIC(skJsonNode *) skJson_object_new(void);
/* Insert Json 'element' into the 'json' object with 'key' as a hash index.
 * In case there is already 'element' with the same 'key' then the old 'element'
 * will get dropped and be replaced with the new 'element', keys stay unchanged.
 */
PUBLIC(bool)
skJson_object_insert_element(skJsonNode *json, char *key, skJsonNode *element);
/* Removes the Json element associated with the 'key' from the 'json' object */
PUBLIC(bool) skJson_object_remove_element(skJsonNode *json, char *key);
/* Returns the Json element associated with the 'key' from the 'json' object */
PUBLIC(skJsonNode *) skJson_object_get_element(skJsonNode *json, char *key);
/* Returns the number of Json elements in 'json' object */
PUBLIC(size_t) skJson_object_len(skJsonNode *json);
/* Checks if there is a Json element associated with the 'key' in the 'json'
 * object */
PUBLIC(bool) skJson_object_contains(const skJsonNode *json, const char *key);

#endif
