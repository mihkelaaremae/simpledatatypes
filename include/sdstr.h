/**
 * @file sdstr.h 	Simple dynamic string object implemented for C.
 * @date			23. Feb 2023
 * @author			Mihkel Aaremäe
 */
#ifndef SDSTR_H
#define SDSTR_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifndef SDSTR_DEFAULT_CAPACITY
/**
 *	Default amount of elements to allocate when none are specified.
 */
#define SDSTR_DEFAULT_CAPACITY 16
#endif

#ifndef SDSTR_SHRINK_DENOMINATOR
/**
 *	Default ratio of minimum capacity/count before an erase operation will
 *	trigger the string to be shrinked.
 */
#define SDSTR_SHRINK_DENOMINATOR 4.0
#endif

#ifndef SDSTR_ENABLE_AUTOSHRINK
/**
 *	Should all strs be shrinked automatically following erase operations.
 *	See @ref SDSTR_SHRINK_DENOMINATOR for more info.
 */
#define SDSTR_ENABLE_AUTOSHRINK 1
#endif

#ifndef sdstr_malloc
#ifndef sdd_malloc
#include <stdlib.h>
/**
 *	A user-defineable macro that should have the same prototype and
 *	functionality as `malloc`, defaults to `malloc` or `sdd_malloc` if
 *	defined.
 */
#define sdstr_malloc malloc
#else
#define sdstr_malloc sdd_malloc
#endif
#endif

#ifndef sdstr_realloc
#ifndef sdd_realloc
#include <stdlib.h>
/**
 *	A user-defineable macro that should have the same prototype and
 *	functionality as `realloc`, defaults to `realloc` or `sdd_realloc` if
 *	defined.
 */
#define sdstr_realloc realloc
#else
#define sdstr_realloc sdd_realloc
#endif
#endif

#ifndef sdstr_free
#ifndef sdd_free
#include <stdlib.h>
/**
 *	A user-defineable macro that should have the same prototype and
 *	functionality as `free`, defaults to `free` or `sdd_free` if defined.
 */
#define sdstr_free free
#else
#define sdstr_free sdd_free
#endif
#endif

#ifndef sdstr_assert
#ifndef sdd_assert
#include <assert.h>
/**
 *	A user-defineable macro that should have the same prototype and
 *	functionality as `assert`, defaults to `assert` or `sdd_assert` if defined.
 */
#define sdstr_assert assert
#else
#define sdstr_assert sdd_assert
#endif
#endif

#ifndef sdstr_index
/**
 *	A user-defineable type for indexes in the str, defaults to `uint32_t`.
 */
#define sdstr_index uint32_t
#endif

#define sdstr char *

#define sdstr_utf8 void *

#define sdstr_stack(byte_count) char[\
	(sizeof(sdstr_header) + byte_count) > sizeof(void *) ?\
		(sizeof(sdstr_header) + byte_count) :\
		(sizeof(void *) + 1)]

#define sdstr_size(str) detail_sdstr_size_impl(detail_sdstr_s2h(str))

#define sdstr_count(str) detail_sdstr_switch_type(str,\
	sdstr_size(str),\
	detail_sdstr_count_utf8_impl(detail_sdstr_s2h(str)),\
	sdstr_size(str))

#define sdstr_capacity(str) detail_sdstr_switch_type(str,\
	detail_sdstr_capacity_impl(detail_sdstr_s2heap(str)),\
	detail_sdstr_capacity_impl(detail_sdstr_s2heap(str)),\
	(sizeof(str) - sizoef(sdstr_header)))

#define sdstr_contains(str, value_expr) detail_sdstr_contains_impl(\
	str,\
	detail_sdstr_s2h(str),\
	detail_sdstr_cstr(value_expr))

#define sdstr_find(str, ...) detail_sdstr_getter_upto_2(__VA_ARGS__,\
	detail_sdstr_find3, detail_sdstr_find2, dummy)(str, __VA_ARGS__)

#define sdstr_find_back(str, ...) detail_sdstr_getter_upto_2(__VA_ARGS__,\
	detail_sdstr_find_back3, detail_sdstr_find_back2, dummy)(str, __VA_ARGS__)

#define sdstr_reserve(str, byte_count) detail_sdstr_switch_type(str,\
	detail_sdstr_heap_reserve_impl(detail_sdstr_s2heap(str), byte_count),\
	detail_sdstr_heap_reserve_impl(detail_sdstr_s2heap(str), byte_count),\
	detail_sdstr_dummy_impl())

#define sdstr_new(...) detail_sdstr_getter_upto_3(\
	__VA_ARGS__, detail_sdstr_new3, detail_sdstr_new2, detail_sdstr_new1,\
	dummy)(__VA_ARGS__)

#define sdstr_dup(str, source) detail_sdstr_switch_type(str,\
	detail_sdstr_dup_to_normal_impl(\
		&str,\
		source,\
		detail_sdstr_s2h(source)),\
	detail_sdstr_dup_to_utf8_impl(\
		&str,\
		source,\
		detail_sdstr_s2h(source)),\
	detail_sdstr_dup_to_stack_impl(\
		str,\
		sdstr_capacity(str),\
		source,\
		detail_sdstr_s2h(source)))

#define sdstr_substr(str, source, ptr, count) detail_sdstr_switch_type(str,\
	detail_sdstr_substr_normal_impl(\
		&str,\
		ptr,\
		source,\
		detail_sdstr_s2h(source),\
		count),\
	detail_sdstr_substr_utf8_impl(\
		&str,\
		ptr,\
		source,\
		detail_sdstr_s2h(str),\
		count),\
	detail_sdstr_substr_stack_impl(\
		str,\
		sdstr_capacity(str),\
		ptr,\
		source,\
		detail_sdstr_s2h(source),\
		count))

#define sdstr_get(str, pos) detail_sdstr_get_impl(\
	str,\
	detail_sdstr_s2h(str),\
	detail_sdstr_pos2ptr(pos))

#define sdstr_set(str, pos, value) detail_sdstr_set_impl(\
	str,\
	detail_sdstr_s2h(str),\
	detail_sdstr_pos2ptr(pos),\
	value)

#define sdstr_getp(str, pos) detail_sdstr_getp_impl(\
	str,\
	detail_sdstr_s2h(str),\
	detail_sdstr_pos2ptr(pos))

#define sdstr_next(str, pos) detail_sdstr_next_impl(\
	str,\
	detail_sdstr_s2h(str),\
	detail_sdstr_pos2ptr(pos))

#define sdstr_prev(str, pos) detail_sdstr_prev_impl(\
	str,\
	detail_sdstr_s2h(str),\
	detail_sdstr_pos2ptr(pos))

#define sdstr_last(str) detail_sdstr_last_impl(\
	str,\
	detail_sdstr_s2h(str))

#define sdstr_insert(str, pos, value_expr) detail_sdstr_insert_impl(\
	&str,\
	detail_sdstr_s2h(str),\
	detail_sdstr_pos2ptr(pos),\
	detail_sdstr_cstr(value_expr))

#define sdstr_insertf(str, pos, format, ...) detail_sdstr_insertf_impl(\
	&str,\
	detail_sdstr_s2h(str),\
	detail_sdstr_pos2ptr(pos),\
	format, __VA_ARGS__)

#define sdstr_push(str, value_expr)\
	sdstr_insert(&str, sdstr_last(str), value_expr)

#define sdstr_pushf(str, format, ...)\
	sdstr_insertf(&str, sdstr_last(str), value_expr, __VA_ARGS__)

#define sdstr_erase(str, pos, count) detail_sdstr_erase_impl(\
	&str,\
	detail_sdstr_s2h(str),\
	detail_sdstr_pos2ptr(pos),\
	count)

#define sdstr_erase_first(str, search_expr) sdstr_replace_first(\
	&str, search_expr, "")

#define sdstr_erase_all(str, search_expr) sdstr_replace_all(\
	&str, search_expr, "")

#define sdstr_pop(str) detail_sdstr_pop_impl(\
	&str,\
	detail_sdstr_s2h(str))

#define sdstr_splice(str, pos, count, value_expr) detail_sdstr_splice_impl(\
	&str,\
	detail_sdstr_s2h(str),\
	detail_sdstr_pos2ptr(pos),\
	count,\
	detail_sdstr_cstr(value_expr))

#define sdstr_splicef(str, pos, count, format, ...) detail_sdstr_splicef_impl(\
	&str,\
	detail_sdstr_s2h(str),\
	detail_sdstr_pos2ptr(pos),\
	count,\
	format,\
	__VA_ARGS__)

#define sdstr_replace_first(str, search_expr, value_expr)\
	detail_sdstr_replace_first_impl(\
		&str,\
		detail_sdstr_s2h(str),\
		detail_sdstr_cstr(search_expr),\
		detail_sdstr_cstr(value_expr))

#define sdstr_replacef_first(str, search_expr, format, ...)\
	detail_sdstr_replacef_first_impl(\
		&str,\
		detail_sdstr_s2h(str),\
		detail_sdstr_cstr(search_expr),\
		detail_sdstr_cstr(format),\
		__VA_ARGS__)

#define sdstr_replace_all(str, search_expr, value_expr)\
	detail_sdstr_replace_all_impl(\
		&str,\
		detail_sdstr_s2h(str),\
		detail_sdstr_cstr(search_expr),\
		detail_sdstr_cstr(value_expr))

#define sdstr_replacef_all(str, search_expr, format, ...)\
	detail_sdstr_replacef_all_impl(\
		&str,\
		detail_sdstr_s2h(str),\
		detail_sdstr_cstr(search_expr),\
		detail_sdstr_cstr(format),\
		__VA_ARGS__)

#define sdstr_reverse(str) detail_sdstr_reverse_impl(&str, )

#define sdstr_delete(str) detail_sdstr_switch_type(str,\
	detail_sdstr_delete_impl((void *)(&str)),\
	detail_sdstr_delete_impl((void *)(&str)),\
	detail_sdstr_dummy_impl())
/*
 *	Detail functions
 *	@cond false
 */
#ifndef SDSTR_API
#define SDSTR_API
#endif

typedef struct
{
	sdstr_index size;
	sdstr_index count;
} sdstr_header;

typedef struct
{
	sdstr_index capacity;
	sdstr_header header;
} sdstr_heap;

#define detail_sdstr_pos2ptr(str, pos) _Generic(pos,\
	default: pos,\
	sdstr_index: detail_sdstr_index_to_pointer_impl(\
		str,\
		detail_sdstr_s2h(str),\
		index))

#define detail_sdstr_cstr(value_expr) _Generic(value_expr,\
	default: detail_sdstr_default(value_expr, char *, NULL),\
	int: (char []){detail_sdstr_default(value_expr, int, '\0'), '\0'})

#define detail_sdstr_size(value_expr) ((sizeof(value_expr) > sizeof(void *)) ?\
	sizeof(value_expr) :\
	strlen(value_expr))\

#define detail_sdstr_getter_upto_3(_1, _2, _3, NAME, ...) NAME

#define detail_sdstr_default(expr, type, value) _Generic(expr,\
	type: expr,\
	default: value)\

#define detail_sdstr_is_utf8(str) _Generic(str, void *:1, char*:0)

#define detail_sdstr_switch_type(str, expr_normal, expr_utf8, expr_stack)\
	_Generic(str,\
		char *: sizeof(str) > sizeof(void *) ? expr_stack : expr_normal,\
		void *: expr_utf8)\

#define detail_sdstr_s2h(str) ((sdstr_header *)\
	((void *)detail_sdstr_switch_type(str,\
		str == NULL ? NULL : (((char *)(str)) - sizeof(sdstr_header)),\
		str == NULL ? NULL : (((char *)(str)) - sizeof(sdstr_header)),\
		(((char *)(str)) + sizeof(str) - sizeof(sdstr_header)))))

#define detail_sdstr_s2heap(str) ((sdstr_heap *)\
	((void *)detail_sdstr_switch_type(str,\
		str == NULL ? NULL : (((char *)(str)) - sizeof(sdstr_heap)),\
		str == NULL ? NULL : (((char *)(str)) - sizeof(sdstr_heap)),\
		NULL)))

#define detail_sdstr_new1(str) detail_sdstr_switch_type(str,\
	detail_sdstr_new_heap_impl(\
		&str, NULL, 0),\
	detail_sdstr_new_heap_utf8_impl(\
		&str, NULL, 0),\
	detail_sdstr_new_stack_impl(\
		str, sdstr_capacity(str), NULL, 0))

#define detail_sdstr_new2(str, value_expr) detail_sdstr_switch_type(str,\
	detail_sdstr_new_heap_impl(\
		&str, detail_sdstr_cstr(value_expr), detail_sdstr_size(value_expr)),\
	detail_sdstr_new_heap_utf8_impl(\
		&str, detail_sdstr_cstr(value_expr), detail_sdstr_size(value_expr)),\
	detail_sdstr_new_stack_impl(\
		str, sdstr_capacity(str), detail_sdstr_cstr(value_expr),\
		detail_sdstr_size(value_expr)))

#define detail_sdstr_new3(str, value_expr, byte_count)\
	detail_sdstr_switch_type(str,\
		detail_sdstr_new_heap_impl(\
			&str, detail_sdstr_cstr(value_expr), byte_count),\
		detail_sdstr_new_heap_utf8_impl(\
			&str, detail_sdstr_cstr(value_expr), byte_count),\
		detail_sdstr_new_stack_impl(\
			str, sdstr_capacity(str), detail_sdstr_cstr(value_expr),\
			byte_count))

#define detail_sdstr_find2(str, value_expr)\

#define detail_sdstr_find3(str, start, value_expr)\

#define detail_sdstr_find_back2(str, value_expr)\

#define detail_sdstr_find_back3(str, start, value_expr)\

SDSTR_API sdstr_index detail_sdstr_size_impl(sdstr_header *header);

SDSTR_API sdstr_index detail_sdstr_count_utf8_impl(sdstr_header *header);

SDSTR_API sdstr_index detail_sdstr_capacity_impl(sdstr_heap *heap);

SDSTR_API int detail_sdstr_contains_impl(
	void *str,
	sdstr_header *header,
	const char *expression);

SDSTR_API void *detail_sdstr_find_impl(
	void *str,
	void *beg,
	sdstr_header *header,
	const char *expression);

SDSTR_API void *detail_sdstr_find_back_impl(
	void *str,
	void *beg,
	sdstr_header *header,
	const char *expression);

SDSTR_API void detail_sdstr_heap_reserve_impl(
	void **str,
	sdstr_index capacity);

SDSTR_API void detail_sdstr_new_heap_impl(
	void **str,
	char *source,
	sdstr_index source_len);

SDSTR_API void detail_sdstr_new_heap_utf8_impl(
	void **str,
	char *source,
	sdstr_index source_len);

SDSTR_API void detail_sdstr_new_stack_impl(
	void **str,
	sdstr_index capacity,
	char *source,
	sdstr_index source_len);

SDSTR_API void detail_sdstr_dup_to_normal_impl(
	void **str, 
	void *src_str, 
	sdstr_header *src_header);

SDSTR_API void detail_sdstr_dup_to_utf8_impl(
	void **str, 
	void *src_str, 
	sdstr_header *src_header);

SDSTR_API void detail_sdstr_dup_to_stack_impl(
	void *str, 
	sdstr_index capacity,
	void *src_str, 
	sdstr_header *src_header);

SDSTR_API void detail_sdstr_substr_normal_impl(
	void **str,
	void *ptr,
	void *source,
	sdstr_header *header,
	sdstr_index count);

SDSTR_API void detail_sdstr_substr_utf8_impl(
	void **str,
	void *ptr,
	void *source,
	sdstr_header *header,
	sdstr_index count);

SDSTR_API void detail_sdstr_substr_stack_impl(
	void *str,
	sdstr_index capacity,
	void *ptr,
	void *source,
	sdstr_header *header,
	sdstr_index count);

SDSTR_API void *detail_sdstr_index_to_pointer_impl(
	void *str,
	sdstr_header *header,
	sdstr_index index);

SDSTR_API uint32_t detail_sdstr_get_impl(
	void *str,
	sdstr_header *header,
	void *index);

SDSTR_API uint32_t detail_sdstr_set_impl(
	void *str,
	sdstr_header *header,
	void *index,
	int value);

SDSTR_API void *detail_sdstr_getp_impl(
	void *str,
	sdstr_header *header,
	void *index);

SDSTR_API void *detail_sdstr_next_impl(
	void *str,
	sdstr_header *header,
	void *index);

SDSTR_API void *detail_sdstr_prev_impl(
	void *str,
	sdstr_header *header,
	void *ptr);

SDSTR_API void *detail_sdstr_last_impl(
	void *str,
	sdstr_header *header);

SDSTR_API void *detail_sdstr_insert_impl(
	void **str,
	sdstr_header *header,
	void *index,
	const char *expression);

SDSTR_API void *detail_sdstr_insertf_impl(
	void **str,
	sdstr_header *header,
	void *index,
	const char *format,
	...);

SDSTR_API void *detail_sdstr_erase_impl(
	void **str,
	sdstr_header *header,
	void *index,
	sdstr_index count);

SDSTR_API uint32_t detail_sdstr_pop_impl(
	void **str,
	sdstr_header *header);

SDSTR_API void *detail_sdstr_splice_impl(
	void **str,
	sdstr_header *header,
	void *index,
	sdstr_index count,
	const char *expression);

SDSTR_API void *detail_sdstr_splicef_impl(
	void **str,
	sdstr_header *header,
	void *index,
	sdstr_index count,
	const char *format,
	...);

SDSTR_API void *detail_sdstr_replace_first_impl(
	void **str,
	sdstr_header *header,
	const char *serach,
	const char *expression);

SDSTR_API void *detail_sdstr_replacef_fitst_impl(
	void **str,
	sdstr_header *header,
	const char *serach,
	const char *format,
	...);

SDSTR_API void *detail_sdstr_replace_all_impl(
	void **str,
	sdstr_header *header,
	const char *serach,
	const char *expression);

SDSTR_API void *detail_sdstr_replacef_all_impl(
	void **str,
	sdstr_header *header,
	const char *serach,
	const char *format,
	...);

SDSTR_API void detail_sdstr_reverse_impl(
	void *str,
	sdstr_header *header);

SDSTR_API void detail_sdstr_delete_impl(void **str);

SDSTR_API void detail_sdstr_dummy_impl(void);

/*
 *	End of detail functions
 *	@endcond
 */

#endif
