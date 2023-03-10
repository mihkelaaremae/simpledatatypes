/**
 * @file sdhmap.h 	Simple dynamic hash map object implemented for C.
 * @date			04. Feb 2023
 * @author			Mihkel Aaremäe
 */
#ifndef SDHMAP_H
#define SDHMAP_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifndef SDHMAP_DEFAULT_CAPACITY
/**
 *	Default amount of elements to allocate when none are specified. This also 
 *	doubles as the default amount of buckets for a map when none are 
 *	specified.
 */
#define SDHMAP_DEFAULT_CAPACITY 16
#endif

#ifndef SDHMAP_MAX_LOAD_FACTOR
/**
 *	Default maximum ratio of count/bucket before an insert operation will 
 *	trigger the map to be resized.
 */
#define SDHMAP_MAX_LOAD_FACTOR 0.75
#endif

#ifndef SDHMAP_MIN_LOAD_FACTOR
/**
 *	Default minimum ratio of count/bucket before an erase operation will 
 *	trigger the map to be resized.
 */
#define SDHMAP_MIN_LOAD_FACTOR (SDHMAP_MAX_LOAD_FACTOR / 4)
#endif

#ifndef SDHMAP_ENABLE_AUTOSHRINK
/**
 *	Should all maps be shrinked automatically following erase operations.
 *	See @ref SDHMAP_SHRINK_DENOMINATOR for more info.
 */
#define SDHMAP_ENABLE_AUTOSHRINK 1
#endif

#ifndef sdhmap_malloc
#ifndef sdd_malloc
#include <stdlib.h>
/**
 * A user-defineable macro that should have the same prototype and 
 * functionality as malloc, defaults to malloc or sdd_malloc if defined.
 */
#define sdhmap_malloc malloc
#else
#define sdhmap_malloc sdd_malloc
#endif
#endif

#ifndef sdhmap_realloc
#ifndef sdd_realloc
#include <stdlib.h>
/**
 * A user-defineable macro that should have the same prototype and 
 * functionality as realloc, defaults to realloc or sdd_realloc if defined.
 */
#define sdhmap_realloc realloc
#else
#define sdhmap_realloc sdd_realloc
#endif
#endif

#ifndef sdhmap_free
#ifndef sdd_free
#include <stdlib.h>
/**
 * A user-defineable macro that should have the same prototype and
 *  functionality as free, defaults to free or sdd_free if defined.
 */
#define sdhmap_free free
#else
#define sdhmap_free sdd_free
#endif
#endif

#ifndef sdhmap_assert
#ifndef sdd_assert
#include <assert.h>
/**
 * A user-defineable macro that should have the same prototype and 
 * functionality as assert, defaults to assert or sdd_assert if defined.
 */
#define sdhmap_assert assert
#else
#define sdhmap_assert sdd_assert
#endif
#endif

#ifndef sdhmap_index
/**
 * A user-defineable type for indexes in the map, defaults to uint32_t.
 * This type is also used interchangeably for hash types.
 */
#define sdhmap_index uint32_t
#endif

#ifndef sdhmap_typeof
/**
 * A user-defineable macro that should have the same prototype and 
 * functionality as __typeof__
 */
#define sdhmap_typeof __typeof__
#endif

/**
 *	@hideinitializer
 *	@brief		Heap-type simple dynamic hashmap type generator
 *
 *	@param[in]	key_type		Type of the key in the map.
 *	@param[in]	value_type		Type of the value in the map.
 *
 *	@return		Type to sdhmap object that satisfies the input parameters
 */
#define sdhmap(key_type, value_type)\
	struct {\
		struct {\
			key_type key;\
			value_type value;\
			detail_sdhmap_heap_type storage_type;\
			struct {\
				sdhmap_slot slot;\
				key_type key;\
				value_type value;\
			} slot;\
		} *type_data;\
	} *

/**
 *	@hideinitializer
 *	@brief		Stack-type simple dynamic hashmap type generator
 *
 *	@param[in]	key_type		Type of the key in the map.
 *	@param[in]	value_type		Type of the value in the map.
 *	@param[in]	element_count	Upper bound of elements in stack-type map.
 *
 *	@return		Type to sdhmap object that satisfies the input parameters
 */
#define sdhmap_stack(key_type, value_type, element_count)\
	sdhmap_typeof(struct {\
		struct {\
			key_type key;\
			value_type value;\
			detail_sdhmap_stack_type storage_type;\
			struct {\
				sdhmap_slot slot;\
				key_type key;\
				value_type value;\
			} slot;\
		} *type_data;\
	}[(sizeof(sdhmap_header) + sizeof(struct {\
				sdhmap_slot slot;\
				key_type key;\
				value_type value;\
			}) * element_count + \
		sizeof(void *) - 1) / sizeof(void *)])

/**
 *	@hideinitializer
 *	@brief		Retrieve the amount of elements in the map.
 *	
 *	@details	Average time complexity - `O(1)`
 *
 *	@param[in]	map		Map object to retrieve the count from.
 *	
 *	@return		Amount of elements in map `(sdhmap_index)`.
 */
#define sdhmap_count(map) _Generic(map[0].type_data->storage_type,\
	detail_sdhmap_heap_type :\
		detail_sdhmap_count_impl((void *)map),\
	detail_sdhmap_stack_type :\
		((const sdhmap_index)((sdhmap_header *)((void *)map))->count))

/**
 *	@hideinitializer
 *	@brief		Retrieve the amount of elements the map can store currently.
 *	
 *	@details	Average time complexity - `O(1)`\n
 *				When map is a heap-type map then the capacity will be grown
 *				automatically to make space for more elements.\n
 *				For stack-type maps the capacity is fixed and cannot be
 *				changed. Attempting to do so by adding more elements will
 *				trip an assert.
 *
 *	@param[in]	map		Map object to retrieve the capacity from.
 *	
 *	@return		Capacity of map, counted in elements `(sdhmap_index)`.
 */
#define sdhmap_capacity(map) _Generic(map[0].type_data->storage_type,\
	detail_sdhmap_heap_type :\
		detail_sdhmap_capacity_impl(\
			(void *)map[0].type_data, \
			sizeof(map[0].type_data->slot)),\
	detail_sdhmap_stack_type :\
		((sdhmap_index)(sizeof(map) - sizeof(sdhmap_header)) /\
			sizeof(map[0].type_data->slot)))

/**
 *	@hideinitializer
 *	@brief		Test if a key exists in the map.
 *	
 *	@details	Average time complexity - `O(1)`
 *
 *	@param[in]	map			Map to perform the test on
 *	@param[in]	key_expr	Either a key or a pointer to a key
 *	
 *	@return		Does the element exist in the map `(int)`.
 */
#define sdhmap_contains(map, key_expr)\
	detail_sdhmap_contains_impl(\
		detail_sdhmap_m2h(map),\
		sizeof(map[0].type_data->slot),\
		sizeof(map[0].type_data->key),\
		detail_sdhmap_keyexpr_to_pointer(map, key_expr))


/**
 *	@hideinitializer
 *	@brief		Reserve enough space to fill capacity elements.
 *	
 *	@details	Average time complexity - same as @ref sdhmap_realloc\n
 *				Calling this function invalidates all pointers retrieved with
 *				any function for this object in this library.
 *
 *	@param[in]	map			Map to reserve space for
 *	@param[in]	capacity	Amount of elements to reserve space for
 *	
 */
#define sdhmap_reserve(map, capacity) _Generic(map[0].type_data->storage_type,\
	detail_sdhmap_heap_type : detail_sdhmap_reserve_heap_impl(\
		detail_sdhmap_ensure_initialized_capacity(map, capacity),\
		sizeof(map[0].type_data->slot),\
		sizeof(map[0].type_data->key),\
		capacity),\
	detail_sdhmap_stack_type : detail_sdhmap_reserve_stack_impl(\
		detail_sdhmap_m2h(map),\
		sizeof(map[0].type_data->slot),\
		sizeof(map[0].type_data->key),\
		sdhmap_capacity(map),\
		capacity)\
	)

/**
 *	@hideinitializer
 *	@brief		Construct a new map.
 *	
 *	@details	Average time complexity - `O(capacity)`\n
 *				If map contains a previously used map then it should be freed
 *				with `sdhmap_delete`.
 *
 *	@param[in]	map			Map to initialize
 *	@param[in]	hash_func	(OPTIONAL) Hash function with the prototype
 *							`sdhmap_index (const void *)`. Defaults to a
 *							library-generated function if key type is a basic
 *							type or `char *` (for strings).
 *	@param[in]	eq_func		(OPTIONAL) Equality function with the prototype 
 *							`int (const void *, const void *)`. This function
 *							should return 0 for equal elements and nonzero for 
 *							elements that are not equal. Defaults to NULL 
 *							which means that `memcmp` is used to test equality 
 *							instead.
 *	@param[in]	capacity	(OPTIONAL, OMITTED) amount of elements to reserve 
 *							space for. Defaults to 
 *							@ref SDHMAP_DEFAULT_CAPACITY. This option is 
 *							omitted for stack-type maps.
 *	
 */
#define sdhmap_new(...) detail_sdhmap_getter_upto_4(\
	__VA_ARGS__, detail_sdhmap_new4, detail_sdhmap_new3, detail_sdhmap_new2,\
	detail_sdhmap_new1, dummy)(__VA_ARGS__)

/**
 *	@hideinitializer
 *	@brief		Duplicate an existing map.
 *	
 *	@details	Average time complexity - `O(count)`
 *
 *	@param		map		Destination map
 *	@param		source	Source map
 *
 */
#define sdhmap_duplicate(map, source)\
	_Generic(source.type_data->storage_type,\
		detail_sdhmap_heap_type : _Generic(map[0].type_data->storage_type,\
			detail_sdhmap_heap_type :\
				detail_sdhmap_duplicate_heap_heap_impl(\
					detail_sdhmap_m2hp(map),\
					detail_sdhmap_m2h(source)),\
			detail_sdhmap_stack_type :\
				detail_sdhmap_duplicate_stack_heap_impl(\
					detail_sdhmap_m2h(map),\
					sdhmap_capacity(map),\
					sizeof(map[0].type_data->slot),\
					detail_sdhmap_m2h(source))\
			),\
		detail_sdhmap_stack_type : _Generic(map[0].type_data->storage_type,\
			detail_sdhmap_heap_type :\
				detail_sdhmap_duplicate_heap_stack_impl(\
					detail_sdhmap_m2hp(map),\
					sizeof(map[0].type_data->slot),\
					detail_sdhmap_m2h(source),\
					sdhmap_capacity(source)\
					),\
			detail_sdhmap_stack_type :\
				detail_sdhmap_duplicate_stack_stack_impl(\
					detail_sdhmap_m2h(map),\
					sdhmap_capacity(map),\
					sizeof(map[0].type_data->slot),\
					detail_sdhmap_m2h(source))\

/**
 *	@hideinitializer
 *	@brief		Retrieve a value associated with a key, if key doesn't exist
 *				then inserts an element.
 *	
 *	@details	Average time complexity - `O(1)`\n
 *				Calling this function invalidates all pointers retrieved with
 *				any function for this object in this library unless `key_expr`
 *				is a valid pointer to a key returned by any function for this
 *				object.
 *				
 *	@param[in]	map			Map to perform the test on
 *	@param[in]	key_expr	Either a key or a pointer to a key
 *	
 *	@return		lvalue object associated with `key_expr`
 */
#define sdhmap_get(map, key_expr) \
	(*((sdhmap_typeof(map[0].type_data->value) *)\
	_Generic(map[0].type_data->storage_type,\
		detail_sdhmap_heap_type : \
			_Generic(key_expr,\
				const sdhmap_typeof(map[0].type_data->key) * :\
					detail_sdhmap_set_heap_optimized_impl,\
				default :\
					detail_sdhmap_set_heap_impl)\
				(\
				detail_sdhmap_ensure_initialized(map),\
				sizeof(map[0].type_data->slot),\
				sizeof(map[0].type_data->key),\
				detail_sdhmap_keyexpr_to_pointer(map, key_expr)),\
		detail_sdhmap_stack_type :\
			_Generic(key_expr,\
				const sdhmap_typeof(map[0].type_data->key) * :\
					detail_sdhmap_set_stack_optimized_impl,\
				default :\
					detail_sdhmap_set_stack_impl)\
				(\
				detail_sdhmap_m2h(map),\
				sizeof(map[0].type_data->slot),\
				sizeof(map[0].type_data->key),\
				detail_sdhmap_keyexpr_to_pointer(map, key_expr),\
				sdhmap_capacity(map)))))

/**
 *	@hideinitializer
 *	@brief		Set a value to a key. If the key already exists in the map,
 *				then it is overwritten.
 *	
 *	@details	Average time complexity - `O(1)`\n
 *				Calling this function invalidates all pointers retrieved with
 *				any function for this object in this library unless `key_expr`
 *				is a valid pointer to a key returned by any function for this
 *				object.
 *				
 *	@param[in]	map			Map to write to
 *	@param[in]	key_expr	Either a key or a pointer to a key
 *	@param[in]	value_expr	Value to associate the key to
 */
#define sdhmap_set(map, key_expr, value_expr)\
	(sdhmap_get(map, key_expr) = value_expr)

/**
 *	@hideinitializer
 *	@brief		Retrieve a pointer to a value associated with a key, if key
 *				doesn't exist returns NULL.
 *	
 *	@details	Average time complexity - `O(1)`
 *				
 *	@param[in]	map			Map to perform the test on
 *	@param[in]	key_expr	Either a key or a pointer to a key
 *	
 *	@return		Pointer to object associated with `key_expr`, NULL if key
 *				isn't found
 */
#define sdhmap_getp(map, key_expr)\
	((sdhmap_typeof(map[0].type_data->value) *)detail_sdhmap_getp_impl(\
		detail_sdhmap_m2h(map),\
		sizeof(map[0].type_data->slot),\
		sizeof(map[0].type_data->key),\
		detail_sdhmap_keyexpr_to_pointer(map, key_expr)))

/**
 *	@hideinitializer
 *	@brief		Erase a key from the map. If the key doesn't exist, then
 *				nothing is done.
 *	
 *	@details	Average time complexity - `O(1)`\n
 *				Calling this function invalidates all pointers retrieved with
 *				any function for this object in this library.\n
 *				This function will automatically shrink any heap-type maps
 *				when @ref SDHMAP_ENABLE_AUTOSHRINK is defined.
 *				
 *	@param[in]	map			Map to erase from
 *	@param[in]	key_expr	Either a key or a pointer to a key
 */
#define sdhmap_erase(map, key_expr)\
	detail_sdhmap_erase_impl(\
		detail_sdhmap_m2h(map),\
		sizeof(map[0].type_data->slot),\
		sizeof(map[0].type_data->key),\
		detail_sdhmap_keyexpr_to_pointer(map, key_expr))

/**
 *	@hideinitializer
 *	@brief		Optimize and shrink the map down as much as possible.
 *	
 *	@details	Average time complexity - `O(capacity)`\n
 *				Calling this function invalidates all pointers retrieved with
 *				any function for this object in this library.
 *				
 *	@param[in]	map		Map to shrink
 */
#define sdhmap_shrink(map) _Generic(map[0].type_data->storage_type,\
	detail_sdhmap_heap_type : detail_sdhmap_shrink_heap_impl(\
		detail_sdhmap_m2hp(map),\
		sizeof(map[0].type_data->slot),\
		sizeof(map[0].type_data->key)),\
	detail_sdhmap_stack_type : detail_sdhmap_shrink_stack_impl(\
		detail_sdhmap_m2h(map),\
		sizeof(map[0].type_data->slot),\
		sizeof(map[0].type_data->key),\
		sdhmap_capacity(map))\
	)

/**
 *	@hideinitializer
 *	@brief		Retrieve a pointer to the "first" key
 *	
 *	@details	Average time complexity - `O(1)`
 *
 *	@param[in]	map		Map to retrieve key from
 *	
 *	@return		Pointer to key, `NULL` if map is empty
 */
#define sdhmap_first(map) ((const sdhmap_typeof(map[0].type_data->key) *)\
	detail_sdhmap_first_impl(\
		detail_sdhmap_m2h(map),\
		sizeof(map[0].type_data->slot)))

/**
 *	@hideinitializer
 *	@brief		Retrieve a pointer to the "next" key
 *	
 *	@details	Average time complexity - `O(1)`
 *
 *	@param[in]	map			Map to retrieve key from
 *	@param[in]	key_expr	Either a key or a pointer to a key
 *	
 *	@return		Pointer to key, `NULL` if map is empty or key is the "last" 
 *				one
 */
#define sdhmap_next(map, key_expr)\
	((const sdhmap_typeof(map[0].type_data->key) *)\
	detail_sdhmap_next_impl(\
		detail_sdhmap_m2h(map),\
		sizeof(map[0].type_data->slot),\
		sizeof(map[0].type_data->key),\
		detail_sdhmap_keyexpr_to_pointer(map, key_expr)))

/**
 *	@hideinitializer
 *	@brief		Free and invalidate map.
 *	
 *	@details	Average time complexity - same as @ref sdhmap_free\n
 *				This function call may be omitted for stack type maps.
 *
 *	@param[in]	map		Map object to free
 */
#define sdhmap_delete(map) _Generic(map[0].type_data->storage_type,\
	detail_sdhmap_heap_type: detail_sdhmap_delete_impl(\
		detail_sdhmap_m2hp(map)),\
	detail_sdhmap_stack_type: detail_sdhmap_dummy_impl())
	
/*
 *	Detail functions
 *	@cond false
 */
#ifndef SDHMAP_API
#define SDHMAP_API
#endif

/**
 * @brief	sdhmap header object.
 */
typedef struct sdhmap_header
{
	/**
	 * How many elements exist in the map.
	 */
	sdhmap_index count;

	/**
	 * How many slots exist in the map.
	 */
	sdhmap_index slot_count;

	/**
	 * How many buckets are in use.
	 */
	sdhmap_index used_bucket_count;

	/**
	 * The first empty slot
	 */
	sdhmap_index empty_slot;

	/**
	 * Hash function.
	 */
	sdhmap_index (*hash_func)(const void *);

	/**
	 * Equality function. May be NULL.
	 */
	int (*eq_func)(const void *, const void *);
} sdhmap_header;

/**
 *	@brief	sdhmap heap object.
 */
typedef struct sdhmap_heap
{
	sdhmap_index capacity;
	sdhmap_header header;
} sdhmap_heap;

/*
 * slot -> location of key and value
 * slot == -1 -> slot is empty
 */
typedef struct sdhmap_slot
{
	sdhmap_index slot;
	sdhmap_index next;
	sdhmap_index prev;
} sdhmap_slot;

#define detail_sdhmap_heap_type char

#define detail_sdhmap_stack_type short

#define detail_sdhmap_m2h(map) ((sdhmap_header *)((void *) _Generic(\
		map[0].type_data->storage_type,\
	detail_sdhmap_heap_type : map,\
	detail_sdhmap_stack_type : map))\
	)

#define detail_sdhmap_m2hp(map) ((sdhmap_header **)((void *) _Generic(\
		map[0].type_data->storage_type,\
	detail_sdhmap_heap_type : &map,\
	detail_sdhmap_stack_type : map))\
	)

#define detail_sdhmap_key_to_complit2(map, key_value)\
	_Generic(key_value, \
		sdhmap_typeof(sdhmap_typeof(map[0].type_data->key)) : key_value,\
		default : (sdhmap_typeof(map[0].type_data->key)){0}\
	)

#define detail_sdhmap_key_to_complit(map, key_value)\
	((const void *)(&((sdhmap_typeof(map[0].type_data->key))\
		{detail_sdhmap_key_to_complit2(map, key_value)})))

#define detail_sdhmap_keyexpr_to_pointer(map, key_expr) _Generic(key_expr,\
	sdhmap_typeof(map[0].type_data->key) :\
		detail_sdhmap_key_to_complit(map, key_expr),\
	sdhmap_typeof(map[0].type_data->key) *: key_expr,\
	const sdhmap_typeof(map[0].type_data->key) *: key_expr\
	)

#define detail_sdhmap_getter_upto_4(_1, _2, _3, _4, NAME, ...) NAME

#define detail_sdhmap_new4(map, hash_func, eq_func, capacity)\
	_Generic(map[0].type_data->storage_type,\
		detail_sdhmap_heap_type : \
			detail_sdhmap_new_heap_impl(\
				detail_sdhmap_m2hp(map),\
				hash_func,\
				eq_func,\
				capacity,\
				sizeof(map[0].type_data->slot)),\
		detail_sdhmap_stack_type : \
			sdhmap_assert(\"Stack-type sdhmap_new hashmap called with 4 \
				arguments."))\

#define detail_sdhmap_new3(map, hash_func, eq_func)\
	_Generic(map[0].type_data->storage_type,\
		detail_sdhmap_heap_type : \
			detail_sdhmap_new_heap_impl(\
				detail_sdhmap_m2hp(map),\
				hash_func,\
				eq_func,\
				SDHMAP_DEFAULT_CAPACITY,\
				sizeof(map[0].type_data->slot)),\
		detail_sdhmap_stack_type : \
			detail_sdhmap_new_stack_impl(\
				detail_sdhmap_m2h(map),\
				hash_func,\
				eq_func,\
				sdhmap_capacity(map),\
				sizeof(map[0].type_data->slot)))\

#define detail_sdhmap_new2(map, hash_func)\
	_Generic(map[0].type_data->storage_type,\
		detail_sdhmap_heap_type : \
			detail_sdhmap_new_heap_impl(\
				detail_sdhmap_m2hp(map),\
				hash_func,\
				detail_sdhmap_pick_eq_func(map[0].type_data->key),\
				SDHMAP_DEFAULT_CAPACITY,\
				sizeof(map[0].type_data->slot)),\
		detail_sdhmap_stack_type : \
			detail_sdhmap_new_stack_impl(\
				detail_sdhmap_m2h(map),\
				hash_func,\
				detail_sdhmap_pick_eq_func(map[0].type_data->key),\
				sdhmap_capacity(map),\
				sizeof(map[0].type_data->slot)))\

#define detail_sdhmap_new1(map)\
	_Generic(map[0].type_data->storage_type,\
		detail_sdhmap_heap_type : \
			detail_sdhmap_new_heap_impl(\
				detail_sdhmap_m2hp(map),\
				detail_sdhmap_pick_hash_func(map[0].type_data->key),\
				detail_sdhmap_pick_eq_func(map[0].type_data->key),\
				SDHMAP_DEFAULT_CAPACITY,\
				sizeof(map[0].type_data->slot)),\
		detail_sdhmap_stack_type : \
			detail_sdhmap_new_stack_impl(\
				detail_sdhmap_m2h(map),\
				detail_sdhmap_pick_hash_func(map[0].type_data->key),\
				detail_sdhmap_pick_eq_func(map[0].type_data->key),\
				sdhmap_capacity(map),\
				sizeof(map[0].type_data->slot)))\

#define detail_sdhmap_ensure_initialized(map)\
	detail_sdhmap_ensure_initialized_impl(\
		detail_sdhmap_m2hp(map),\
		detail_sdhmap_pick_hash_func(map[0].type_data->key),\
		detail_sdhmap_pick_eq_func(map[0].type_data->key),\
		SDHMAP_DEFAULT_CAPACITY,\
		sizeof(map[0].type_data->slot))

#define detail_sdhmap_ensure_initialized_capacity(map, capacity)\
	detail_sdhmap_ensure_initialized_impl(\
		detail_sdhmap_m2hp(map),\
		detail_sdhmap_pick_hash_func(map[0].type_data->key),\
		detail_sdhmap_pick_eq_func(map[0].type_data->key),\
		capacity,\
		sizeof(map[0].type_data->slot))

#define detail_sdhmap_pick_hash_func(key) _Generic(key,\
	uint8_t: detail_sdhmap_hash_uint8_t,\
	uint16_t: detail_sdhmap_hash_uint16_t,\
	uint32_t: detail_sdhmap_hash_uint32_t,\
	uint64_t: detail_sdhmap_hash_uint64_t,\
	int8_t: detail_sdhmap_hash_int8_t,\
	int16_t: detail_sdhmap_hash_int16_t,\
	int32_t: detail_sdhmap_hash_int32_t,\
	int64_t: detail_sdhmap_hash_int64_t,\
	float: detail_sdhmap_hash_float,\
	double: detail_sdhmap_hash_double,\
	long double: detail_sdhmap_hash_long_double,\
	char *: detail_sdhmap_hash_string,\
	default: NULL\
	)

#define detail_sdhmap_pick_eq_func(key) _Generic(key,\
	char *: detail_sdhmap_eq_string,\
	default: NULL\
	)

#define detail_sdhmap_declare_hash_func(type, postfix)\
SDHMAP_API sdhmap_index detail_sdhmap_hash_##postfix(const void *a);

detail_sdhmap_declare_hash_func(uint8_t, uint8_t)
detail_sdhmap_declare_hash_func(uint16_t, uint16_t)
detail_sdhmap_declare_hash_func(uint32_t, uint32_t)
detail_sdhmap_declare_hash_func(uint64_t, uint64_t)
detail_sdhmap_declare_hash_func(int8_t, int8_t)
detail_sdhmap_declare_hash_func(int16_t, int16_t)
detail_sdhmap_declare_hash_func(int32_t, int32_t)
detail_sdhmap_declare_hash_func(int64_t, int64_t)
detail_sdhmap_declare_hash_func(float, float)
detail_sdhmap_declare_hash_func(double, double)
detail_sdhmap_declare_hash_func(long double, long_double)

SDHMAP_API sdhmap_index detail_sdhmap_hash_string(const void *a);

SDHMAP_API int detail_sdhmap_eq_string(const void *a, const void *b);

SDHMAP_API sdhmap_index detail_sdhmap_count_impl(sdhmap_header *header);

SDHMAP_API sdhmap_index detail_sdhmap_capacity_impl(
	sdhmap_header *header,
	uint32_t slot_size);

SDHMAP_API void detail_sdhmap_new_heap_impl(
	sdhmap_header **header,
	sdhmap_index (*hash_func)(const void *),
	int (*eq_func)(const void *, const void *),
	sdhmap_index count,
	uint32_t slot_size);

SDHMAP_API void detail_sdhmap_new_stack_impl(
	sdhmap_header *header,
	sdhmap_index (*hash_func)(const void *),
	int (*eq_func)(const void *, const void *),
	sdhmap_index count,
	uint32_t slot_size);

SDHMAP_API void detail_sdhmap_duplicate_heap_heap_impl(
	sdhmap_header **header,
	sdhmap_header *source);

SDHMAP_API void detail_sdhmap_duplicate_stack_heap_impl(
	sdhmap_header *header,
	sdhmap_index dest_capacity,
	uint32_t slot_size,
	sdhmap_header *source);

SDHMAP_API void detail_sdhmap_duplicate_heap_stack_impl(
	sdhmap_header **header,
	uint32_t slot_size,
	sdhmap_header *source,
	sdhmap_index src_capacity);

SDHMAP_API void detail_sdhmap_duplicate_stack_stack_impl(
	sdhmap_header *header,
	sdhmap_index dest_capacity,
	uint32_t slot_size,
	sdhmap_header *source);

SDHMAP_API sdhmap_header **detail_sdhmap_ensure_initialized_impl(
	sdhmap_header **header,
	sdhmap_index (*hash_func)(const void *),
	int (*eq_func)(const void *, const void *),
	sdhmap_index count,
	uint32_t slot_size);

SDHMAP_API int detail_sdhmap_contains_impl(
	sdhmap_header *header,
	uint32_t slot_size,
	uint32_t key_size,
	const void *key);

SDHMAP_API void *detail_sdhmap_set_heap_impl(
	sdhmap_header **header,
	uint32_t slot_size,
	uint32_t key_size,
	const void *key);

SDHMAP_API void *detail_sdhmap_set_stack_impl(
	sdhmap_header *header,
	uint32_t slot_size,
	uint32_t key_size,
	const void *key,
	sdhmap_index capacity);

SDHMAP_API void *detail_sdhmap_set_heap_optimized_impl(
	sdhmap_header **header,
	uint32_t slot_size,
	uint32_t key_size,
	const void *key);

SDHMAP_API void *detail_sdhmap_set_stack_optimized_impl(
	sdhmap_header *header,
	uint32_t slot_size,
	uint32_t key_size,
	const void *key,
	sdhmap_index capacity);

SDHMAP_API void detail_sdhmap_reserve_heap_impl(
	sdhmap_header **header,
	uint32_t slot_size,
	uint32_t key_size,
	sdhmap_index target);

SDHMAP_API void detail_sdhmap_reserve_stack_impl(
	sdhmap_header *header,
	uint32_t slot_size,
	uint32_t key_size,
	sdhmap_index capacity,
	sdhmap_index target);

SDHMAP_API void *detail_sdhmap_getp_impl(
	sdhmap_header *header,
	uint32_t slot_size,
	uint32_t key_size,
	const void *key);

SDHMAP_API void detail_sdhmap_erase_impl(
	sdhmap_header *header,
	uint32_t slot_size,
	uint32_t key_size,
	const void *key);

SDHMAP_API void detail_sdhmap_shrink_heap_impl(
	sdhmap_header **header,
	uint32_t slot_size,
	uint32_t key_size);

SDHMAP_API void detail_sdhmap_shrink_stack_impl(
	sdhmap_header *header,
	uint32_t slot_size,
	uint32_t key_size,
	sdhmap_index capacity);

SDHMAP_API void *detail_sdhmap_first_impl(
	sdhmap_header *header,
	uint32_t slot_size);

SDHMAP_API void *detail_sdhmap_next_impl(
	sdhmap_header *header,
	uint32_t slot_size,
	uint32_t key_size,
	const void *key);

SDHMAP_API void detail_sdhmap_delete_impl(sdhmap_header **header);

SDHMAP_API void detail_sdhmap_dummy_impl(void);

/*
 *	End of detail functions
 *	@endcond
 */

#endif
