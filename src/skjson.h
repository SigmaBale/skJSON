#ifndef __SK_JSON_H__
#define __SK_JSON_H__
/* clang-format off */

#include "sknode.h"
#include <stddef.h>

/* Marker macro for public functions */
#define PUBLIC(ret) ret

/* Opaque type representing the Json Element */
typedef skJsonNode skJson;
/* Parse the Json from 'buff' of size 'bufsize'.
 * If parsing error occured it returns Error Json element which contains error info aka
 * string describing the error and position where it occured. */
PUBLIC(skJson *) skJson_parse(char *buff, size_t bufsize);
/* Returns null-terminated char array describing the error occured during parsing if 'json' 
 * is of type 'SK_JSERR', otherwise return NULL. */
PUBLIC(char*) skJson_error(skJson *json);
/* Return 'json' element type */
PUBLIC(int) skJson_type(skJson *json);
/* Return the parent element of 'json' element */
PUBLIC(skJson *) skJson_parent(skJson *json);
/* Return value of 'json' element */
PUBLIC(void *) skJson_value(skJson *json);
/* Drops the 'json' element including its sub-elements. */
PUBLIC(void) skJson_drop(skJson **json);
/* Drops the whole 'json' structure from the root. */
PUBLIC(void) skJson_drop_whole(skJson **json);
/* Transforms the 'json' element into Json Integer element with value 'n' */
PUBLIC(skJson *) skJson_transform_into_int(skJson *json, long int n);
/* Transforms the 'json' element into Json Double element with value 'n' */
PUBLIC(skJson *) skJson_transform_into_double(skJson *json, double n);
/* Transforms the 'json' element into Json Bool element with value 'boolean' */
PUBLIC(skJson *) skJson_transform_into_bool(skJson *json, bool boolean);
/* Transforms the 'json' element into Json String Reference element with value 'string_ref' */
PUBLIC(skJson *) skJson_transform_into_stringlit(skJson *json, const char *string_ref);
/* Transforms the 'json' element into Json String element with value 'string' */
PUBLIC(skJson *) skJson_transform_into_string(skJson *json, const char *string);
/* Transforms the 'json' element into empty Json Array element */
PUBLIC(skJson *) skJson_transform_into_empty_array(skJson *json);
/* Transforms the 'json' element into empty Json Object element */
PUBLIC(skJson *) skJson_transform_into_empty_object(skJson *json);
/* Serializes the 'json' element into null terminated string saving as much space as possible.
 * Returns NULL on failure or pointer to the serialized json on success.
 * On failure serialization buffer is destroyed. */
PUBLIC(unsigned char*) skJson_serialize(skJson* json);
/* Serializes the 'json' element into null terminated string using buffer of size 'n',
 * instead of using default buffer size 'BUFSIZ'.
 * If 'expand' is set it will expand the buffer each time it requires more space,
 * otherwise if the flag is not set and buffer tries to expand, it will instead fail serialization.
 * Returns NULL on failure or pointer to the serialized json on success.
 * On failure serialization buffer is destroyed. */
PUBLIC(unsigned char*) skJson_serialize_with_bufsize(skJson* json, size_t n, bool expand);
/* Serializes the 'json' element into null terminated string using already allocated
 * 'buffer' of size 'n'. If 'expand' is set it will expand the buffer each time it
 * requires more space, otherwise if the flag is not set and buffer tries to expand, it
 * will instead fail serialization. Returns NULL on failure or pointer to the serialized 
 * json on success.
 * On failure the passed in 'buffer' won't be destroyed, it is on user to manage that memory. */
PUBLIC(unsigned char*) skJson_serialize_with_buffer(skJson* json, unsigned char* buffer, size_t size, bool expand);

/* Create integer Json element from integer 'n'. */
PUBLIC(skJson *) skJson_integer_new(long int n);
/* Sets the new integer value 'n' for 'json' element, returns true
 * if the 'json' value was changed, false if 'json is not int element. */
PUBLIC(bool) skJson_integer_set(skJson *json, long int n);

/* Create double Json element from double 'n' */
PUBLIC(skJson *) skJson_double_new(double n);
/* Sets the new double value 'n' for 'json' element, returns true
 * if the 'json' value was changed, false if 'json is not double element. */
PUBLIC(bool) skJson_double_set(skJson *json, double n);

/* Create boolean Json element from 'boolean' */
PUBLIC(skJson *) skJson_bool_new(bool boolean);
/* Sets the new bool value 'boolean' for 'json' element, returns true
 * if the 'json' value was changed, false if 'json is not bool element. */
PUBLIC(bool) skJson_bool_set(skJson *json, bool boolean);

/* Create null Json element */
PUBLIC(skJson *) skJson_null_new(void);

/* Create string Json element from 'string' by creating a new allocation */
PUBLIC(skJson *) skJson_string_new(const char *string);
/* Duplicates and sets the new value for 'json' element freeing the old string
 * value. Returns true if 'json' is string element and setting of string was
 * successfull, otherwise false. */
PUBLIC(bool) skJson_string_set(skJson *json, const char *string);

/* Create string Json element from 'string' by holding only a reference to it */
PUBLIC(skJson *) skJson_stringlit_new(const char *string);
/* Sets the new 'string' reference value for 'json' element. */
PUBLIC(bool) skJson_stringlit_set(skJson *json, const char *string);

/* Create an empty Json array element */
PUBLIC(skJson *) skJson_array_new(void);
/* Push 'string' into the 'json' array */
PUBLIC(bool) skJson_array_push_str(skJson *json, const char *string);
/* Insert 'string' into the 'json' array at 'index'. */
PUBLIC(bool) skJson_array_insert_str(skJson *json, const char *string, size_t index);
/* Push 'string' reference into the 'json' array. */
PUBLIC(bool) skJson_array_push_strlit(skJson *json, const char *string);
/* Insert 'string' reference into the 'json' array at 'index'. */
PUBLIC(bool) skJson_array_insert_strlit(skJson *json, const char *string, size_t index);
/* Push 'n' integer into the 'json' array. */
PUBLIC(bool) skJson_array_push_int(skJson *json, long int n);
/* Insert 'n' integer into the 'json' array at 'index'. */
PUBLIC(bool) skJson_array_insert_int(skJson *json, long int n, size_t index);
/* Push 'n' double into the 'json' array. */
PUBLIC(bool) skJson_array_push_double(skJson *json, double n);
/* Insert 'n' double into the 'json' array at 'index'. */
PUBLIC(bool) skJson_array_insert_double(skJson *json, double n, size_t index);
/* Push 'boolean' into the 'json' array. */
PUBLIC(bool) skJson_array_push_bool(skJson *json, bool boolean);
/* Insert 'boolean' into the 'json' array at 'index'. */
PUBLIC(bool) skJson_array_insert_bool(skJson *json, bool boolean, size_t index);
/* Push NULL element into the 'json' array */
PUBLIC(bool) skJson_array_push_null(skJson *json);
/* Insert NULL element into the 'json' array at 'index'. */
PUBLIC(bool) skJson_array_insert_null(skJson *json, size_t index);
/* Push Json 'element' into the 'json' array. */
PUBLIC(bool) skJson_array_push_element(skJson *json, skJson *element);
/* Insert Json 'element' into the 'json' array at 'index'. */
PUBLIC(bool) skJson_array_insert_element(skJson *json, skJson *element, size_t index);
/* Create Json array from array 'strings' of 'count' string values */
PUBLIC(skJson *) skJson_array_from_strings(const char *const *strings, size_t count);
/* Create Json array from array 'strings' of 'count' string reference values */
PUBLIC(skJson *) skJson_array_from_litstrings(const char *const *strings, size_t count);
/* Create Json array from array 'integers' of 'count' integer values */
PUBLIC(skJson *) skJson_array_from_integers(const int *integers, size_t count);
/* Create Json array from array 'doubles' of 'count' double values */
PUBLIC(skJson *) skJson_array_from_doubles(const double *doubles, size_t count);
/* Create Json array from array 'booleans' of 'count' boolean values */
PUBLIC(skJson *) skJson_array_from_booleans(const bool *booleans, size_t count);
/* Create Json array of 'count' NULL json elements */
PUBLIC(skJson *) skJson_array_from_nulls(size_t count);
/* Create Json array from array 'elements' of 'count' Json elements */
PUBLIC(skJson *) skJson_array_from_elements(const skJson *const *elements, size_t count);
/* Pop Json element from 'json' array */
PUBLIC(skJson *) skjson_array_pop(skJson *json);
/* Remove Json element from 'json' array at 'index' */
PUBLIC(bool) skjson_array_remove(skJson *json, size_t index);
/* Return 'json' array length */
PUBLIC(size_t) skJson_array_len(skJson *json);
/* Return front Json element from 'json' array */
PUBLIC(skJson *) skJson_array_front(skJson *json);
/* Return Json element at the back of 'json' array */
PUBLIC(skJson *) skJson_array_back(skJson *json);
/* Return Json element at 'index' from 'json' array */
PUBLIC(skJson *) skJson_array_index(skJson *json, size_t index);
/* Clears the 'json' array, destroying its sub elements but preserving the wrapper allocation. */
PUBLIC(void) skJson_array_clear(skJson* json);

/* Create an empty Json object */
PUBLIC(skJson *) skJson_object_new(void);
/* Sorts the 'json' object elements by its keys in ascending order using qsort.
 * This marks the json object as sorted, further insertions/removals mark the object as unsorted. */
PUBLIC(bool) skJson_object_sort_ascending(skJson* json);
/* Sorts the 'json' object elements by its keys in descending order using qsort.
 * This marks the json object as sorted, further insertions/removals mark the object as unsorted. */
PUBLIC(bool) skJson_object_sort_descending(skJson* json);
/* Insert 'key' and json element into 'json' object at 'index', element behind the
 * 'elementp' is immediatly nulled and destroyed upon successfull insertion, this is by
 * the design because it is not part of the 'json' object, if inserting fails element is
 * not destroyed and pointer is not nulled. */
PUBLIC(bool) skJson_object_insert_element(skJson *json, const char *key, const skJson *elementp, size_t index);
/* Remove json element from 'json' object at 'index'. Return true upon success otherwise false. */
PUBLIC(bool) skJson_object_remove_element(skJson *json, size_t index);
/* Remove element by 'key', if the 'json' object is sorted and 'sorted' is set, search is done using binary
 * search, otherwise key comparison is done using linear search. If keys are sorted in
 * descnedning order then 'descending' must be set. If the object is not sorted and calle
 * sets the 'sorted' then the search is undefined. */
PUBLIC(bool) skJson_object_remove_element_by_key(skJson* json, const char* key, bool sorted, bool descending);
/* Get element from 'json' object at 'index'. Returns NULL if element was not found, or 
 * isnput arguments are invalid. */ 
PUBLIC(skJson *) skJson_object_get_element(const skJson *json, size_t index);
/* Get element associated with the 'key' from the 'json' object.
 * Searching is done using binary search if object is sorted, otherwise linear search is used.
 * Return NULL if element was not found or input arguments are invalid. */
PUBLIC(skJson*) skJson_object_get_element_by_key(
        const skJson* json,
        const char* key,
        bool sorted,
        bool descending);
/* Returns the number of Json elements in 'json' object */
PUBLIC(size_t) skJson_object_len(const skJson *json);
/* Checks if there is a Json element associated with the 'key' in the 'json' object.
 * If object is sorted user can set 'sorted' to utilize binary search, if keys are sorted
 * in descending order user must set 'descending', if object is not sorted and 'sorted' is
 * set, then the search in undefined and result is undefined. If 'sorted' is not set then
 * the linear search is performed. */
PUBLIC(bool) skJson_object_contains(const skJson* json,
        const char* key,
        bool sorted,
        bool descending);
/* Clears the 'json' object, destroying its sub elements but preserving the wrapper allocation. */
PUBLIC(void) skJson_object_clear(skJson* json);

#endif
