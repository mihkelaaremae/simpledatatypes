/**
 *	@file sdmap.h	Simple dynamic map object implemented with AVL trees for C.
 *	@date			04. Feb 2023
 *	@author			Mihkel Aaremäe
 */
#ifndef SDMAP_H
#define SDMAP_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#ifndef SDMAP_DEFAULT_CAPACITY
/**
 *	Default amount of elements to allocate when none are specified.
 */
#define SDMAP_DEFAULT_CAPACITY 16
#endif

#ifndef SDMAP_SHRINK_DENOMINATOR
/**
 *	Default ratio of minimum capacity/count before an erase operation will
 *	trigger the map to be shrinked.
 */
#define SDMAP_SHRINK_DENOMINATOR 4.0
#endif

#ifndef SDMAP_ENABLE_AUTOSHRINK
/**
 *	Should all maps be shrinked automatically following erase operations.
 *	See @ref SDMAP_SHRINK_DENOMINATOR for more info.
 */
#define SDMAP_ENABLE_AUTOSHRINK 1
#endif

#ifndef sdmap_malloc
#ifndef sdd_malloc
#include <stdlib.h>
/**
 *	A user-defineable macro that should have the same prototype and
 *	functionality as `malloc`, defaults to `malloc` or `sdd_malloc` if
 *	defined.
 */
#define sdmap_malloc malloc
#else
#define sdmap_malloc sdd_malloc
#endif
#endif

#ifndef sdmap_realloc
#ifndef sdd_realloc
#include <stdlib.h>
/**
 *	A user-defineable macro that should have the same prototype and
 *	functionality as `realloc`, defaults to `realloc` or `sdd_realloc` if
 *	defined.
 */
#define sdmap_realloc realloc
#else
#define sdmap_realloc sdd_realloc
#endif
#endif

#ifndef sdmap_free
#ifndef sdd_free
#include <stdlib.h>
/**
 *	A user-defineable macro that should have the same prototype and
 *	functionality as `free`, defaults to `free` or `sdd_free` if defined.
 */
#define sdmap_free free
#else
#define sdmap_free sdd_free
#endif
#endif

#ifndef sdmap_assert
#ifndef sdd_assert
#include <assert.h>
/**
 *	A user-defineable macro that should have the same prototype and
 *	functionality as `assert`, defaults to `assert` or `sdd_assert` if defined.
 */
#define sdmap_assert assert
#else
#define sdmap_assert sdd_assert
#endif
#endif

#ifndef sdmap_index
/**
 *	A user-defineable type for indexes in the map, defaults to `uint32_t`.
 */
#define sdmap_index uint32_t
#endif

#ifndef sdmap_typeof
/**
 *	A user-defineable macro that should have the same prototype and
 *	functionality as `__typeof__`
 */
#define sdmap_typeof __typeof__
#endif

/**
 *	@hideinitializer
 *	@brief		Heap-type simple dynamic map type generator
 *
 *	@param[in]	key_type		Type of the key in the map.
 *	@param[in]	value_type		Type of the value in the map.
 *
 *	@return		Type to sdmap object that satisfies the input parameters
 */
#define sdmap(key_type, value_type)\
	struct {\
		struct {\
			key_type key;\
			value_type value;\
			detail_sdmap_heap_type storage_type;\
			struct {\
				sdmap_slot slot;\
				key_type key;\
				value_type value;\
			} slot;\
		} *type_data;\
	} *

/**
 *	@hideinitializer
 *	@brief		Stack-type simple dynamic map type generator
 *
 *	@param[in]	key_type		Type of the key in the map.
 *	@param[in]	value_type		Type of the value in the map.
 *	@param[in]	element_count	Upper bound of elements in stack-type map.
 *
 *	@return		Type to sdmap object that satisfies the input parameters
 */
#define sdmap_stack(key_type, value_type, element_count)\
	sdmap_typeof(struct {\
		struct {\
			key_type key;\
			value_type value;\
			detail_sdmap_stack_type storage_type;\
			struct {\
				sdmap_slot slot;\
				key_type key;\
				value_type value;\
			} slot;\
		} *type_data;\
	}[(sizeof(sdmap_header) + sizeof(struct {\
				sdmap_slot slot;\
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
 *	@return		Amount of elements in map `(sdmap_index)`.
 */
#define sdmap_count(map) _Generic(map[0].type_data->storage_type,\
	detail_sdmap_heap_type :\
		detail_sdmap_count_impl(detail_sdmap_m2h(map)),\
	detail_sdmap_stack_type :\
		((const sdmap_index)(detail_sdmap_m2h(map)->count))\
	)

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
 *	@return		Capacity of map, counted in elements `(sdmap_index)`.
 */
#define sdmap_capacity(map) _Generic(map[0].type_data->storage_type,\
	detail_sdmap_heap_type :\
		detail_sdmap_capacity_impl(\
			detail_sdmap_m2h(map),\
			sizeof(map[0].type_data->slot)),\
	detail_sdmap_stack_type :\
		((sdmap_index)(sizeof(map) - sizeof(sdmap_header)) /\
			sizeof(map[0].type_data->slot))\
	)

/**
 *	@hideinitializer
 *	@brief		Test if a key exists in the map.
 *	
 *	@details	Average time complexity -\n
 *					`O(log(count))`\n
 *					`O(1)` if `key_expr` is a pointer retrieved by any 
 *					function in this library.
 *
 *	@param[in]	map			Map to perform the test on
 *	@param[in]	key_expr	Either a key or a pointer to a key
 *	
 *	@return		Does the element exist in the map `(int)`.
 */
#define sdmap_contains(map, key_expr)\
	detail_sdmap_contains_impl(\
		detail_sdmap_m2h(map),\
		sizeof(map[0].type_data->slot),\
		detail_sdmap_keyexpr_to_pointer(map, key_expr))\

/**
 *	@hideinitializer
 *	@brief		Reserve enough space to fill capacity elements.
 *	
 *	@details	Average time complexity - same as @ref sdmap_realloc\n
 *				Calling this function invalidates all pointers retrieved with
 *				any function for this object in this library.
 *
 *	@param[in]	map			Map to reserve space for
 *	@param[in]	capacity	Amount of elements to reserve space for
 *	
 */
#define sdmap_reserve(map, capacity) _Generic(map[0].type_data->storage_type,\
	detail_sdmap_heap_type :\
		detail_sdmap_ensure_initialized_impl(\
			detail_sdmap_m2hp(map),\
			sizeof(map[0].type_data->slot) * capacity,\
			detail_sdmap_pick_compare_func(map[0].type_data->key)),\
	detail_sdmap_stack_type : detail_sdmap_dummy_impl()\
	)

/**
 *	@hideinitializer
 *	@brief		Construct a new map.
 *	
 *	@details	Average time complexity - same as @ref sdmap_malloc\n
 *				If map contains a previously used map then it should be freed
 *				with `sdmap_delete`.
 *
 *	@param[in]	map			Map to initialize
 *	@param[in]	function	(OPTIONAL) compare function, should have the same
 *							prototype and functionality as the fourth
 *							parameter of `qsort`. Defaults to a
 *							library-generated function if key type is a basic
 *							type or `char *` (for strings).
 *	@param[in]	capacity	(OPTIONAL, OMITTED) amount of elements to reserve 
 *							space for. Defaults to 
 *							@ref SDMAP_DEFAULT_CAPACITY. This option is 
 *							omitted for stack-type maps.
 *	
 */
#define sdmap_new(...) detail_sdmap_getter_upto_3(\
	__VA_ARGS__, detail_sdmap_new3, detail_sdmap_new2, detail_sdmap_new1,\
	dummy)(__VA_ARGS__)

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
#define sdmap_duplicate(map, source) _Generic(source.type_data->storage_type,\
	detail_sdmap_heap_type : _Generic(map[0].type_data->storage_type,\
		detail_sdmap_heap_type :\
			detail_sdmap_duplicate_heap_heap_impl(\
				detail_sdmap_m2hp(map),\
				detail_sdmap_m2h(source)),\
		detail_sdmap_stack_type :\
			detail_sdmap_duplicate_stack_heap_impl(\
				detail_sdmap_m2h(map),\
				sdmap_capacity(map),\
				sizeof(map[0].type_data->slot),\
				detail_sdmap_m2h(source))\
		),\
	detail_sdmap_stack_type : _Generic(map[0].type_data->storage_type,\
		detail_sdmap_heap_type :\
			detail_sdmap_duplicate_heap_stack_impl(\
				detail_sdmap_m2hp(map),\
				sizeof(map[0].type_data->slot),\
				detail_sdmap_m2h(source),\
				sdmap_capacity(source)\
				),\
		detail_sdmap_stack_type :\
			detail_sdmap_duplicate_stack_stack_impl(\
				detail_sdmap_m2h(map),\
				sdmap_capacity(map),\
				sizeof(map[0].type_data->slot),\
				detail_sdmap_m2h(source))\
		))

/**
 *	@hideinitializer
 *	@brief		Retrieve a value associated with a key, if key doesn't exist
 *				then inserts an element.
 *	
 *	@details	Average time complexity -\n
 *					`O(log(count))`\n
 *					`O(1)` if `key_expr` is a pointer retrieved by any 
 *					function in this library.
 *				Calling this function invalidates all pointers retrieved with
 *				any function for this object in this library.
 *				
 *	@param[in]	map			Map to perform the test on
 *	@param[in]	key_expr	Either a key or a pointer to a key
 *	
 *	@return		lvalue object associated with `key_expr`
 */
#define sdmap_get(map, key_expr) (*((sdmap_typeof(map[0].type_data->key) *)\
	_Generic(map[0].type_data->storage_type,\
	detail_sdmap_heap_type :\
		detail_sdmap_set_heap_impl(\
			detail_sdmap_ensure_initialized(map),\
			sizeof(map[0].type_data->slot),\
			sizeof(map[0].type_data->key),\
			detail_sdmap_keyexpr_to_pointer(map, key_expr)),\
	detail_sdmap_stack_type :\
		detail_sdmap_set_stack_impl(\
			detail_sdmap_m2h(map),\
			sizeof(map[0].type_data->slot),\
			sizeof(map[0].type_data->key),\
			detail_sdmap_keyexpr_to_pointer(map, key_expr),\
			sdmap_capacity(map))\
	)))

/**
 *	@hideinitializer
 *	@brief		Set a value to a key. If the key already exists in the map,
 *				then it is overwritten.
 *	
 *	@details	Average time complexity -\n
 *					`O(log(count))`\n
 *					`O(1)` if `key_expr` is a pointer retrieved by any 
 *					function in this library.
 *				Calling this function invalidates all pointers retrieved with
 *				any function for this object in this library.
 *				
 *	@param[in]	map			Map to write to
 *	@param[in]	key_expr	Either a key or a pointer to a key
 *	@param[in]	value_expr	Value to associate the key to
 */
#define sdmap_set(map, key_expr, value_expr)\
	sdmap_get(map, key_expr) = value_expr

/**
 *	@hideinitializer
 *	@brief		Retrieve a pointer to a value associated with a key, if key
 *				doesn't exist returns NULL.
 *	
 *	@details	Average time complexity -\n
 *					`O(log(count))`\n
 *					`O(1)` if `key_expr` is a pointer retrieved by any 
 *					function in this library.
 *				
 *	@param[in]	map			Map to perform the test on
 *	@param[in]	key_expr	Either a key or a pointer to a key
 *	
 *	@return		Pointer to object associated with `key_expr`, NULL if key
 *				isn't found
 */
#define sdmap_getp(map, key_expr) ((sdmap_typeof(map[0].type_data->key) *)\
	detail_sdmap_getp_impl(\
		detail_sdmap_ensure_initialized(map),\
		sizeof(map[0].type_data->slot),\
		offsetof(sdmap_typeof(map[0].type_data->slot), value),\
		detail_sdmap_keyexpr_to_pointer(map, key_expr)))

#if SDMAP_ENABLE_AUTOSHRINK
/**
 *	@hideinitializer
 *	@brief		Erase a key from the map. If the key doesn't exist, then
 *				nothing is done.
 *	
 *	@details	Average time complexity -\n
 *					`O(log(count))`\n
 *					`O(1)` if `key_expr` is a pointer retrieved by any 
 *					function in this library.
 *				Calling this function invalidates all pointers retrieved with
 *				any function for this object in this library.\n
 *				This function will automatically shrink any heap-type maps
 *				when @ref SDMAP_ENABLE_AUTOSHRINK is defined.
 *				
 *	@param[in]	map			Map to erase from
 *	@param[in]	key_expr	Either a key or a pointer to a key
 *	
 */
#define sdmap_erase(map, key_expr)\
	detail_sdmap_erase_impl(\
		detail_sdmap_m2h(map),\
		sizeof(map[0].type_data->slot),\
		detail_sdmap_keyexpr_to_pointer(map, key_expr));\
	if (detail_sdmap_m2h(map) != NULL &&\
		sdmap_capacity(map) >= sdmap_count(map) * SDMAP_SHRINK_DENOMINATOR)\
	{\
		_Generic(map[0].type_data->storage_type,\
			detail_sdmap_heap_type :\
				detail_sdmap_shrink_impl(\
					detail_sdmap_m2hp(map),\
					sizeof(sdmap_slot) +\
						sizeof(map[0].type_data->key) +\
						sizeof(map[0].type_data->value)),\
			detail_sdmap_stack_type :\
				detail_sdmap_dummy_impl());\
	}
#else
#define sdmap_erase(map, key_expr)\
	detail_sdmap_erase_impl(\
		detail_sdmap_m2h(map),\
		sizeof(map[0].type_data->slot),\
		detail_sdmap_keyexpr_to_pointer(map, key_expr))
#endif

/**
 *	@hideinitializer
 *	@brief		Optimize and shrink the map down as much as possible.
 *	
 *	@details	Average time complexity - `O(capacity)`\n
 *				Calling this function invalidates all pointers retrieved with
 *				any function for this object in this library.
 *				
 *	@param[in]	map		Map to shrink
 *	
 */
#define sdmap_shrink(map) _Generic(map[0].type_data->storage_type,\
	detail_sdmap_heap_type :\
		detail_sdmap_shrink_impl(\
			detail_sdmap_m2h(map),\
			sizeof(map[0].type_data->slot)),\
	detail_sdmap_stack_type :\
		detail_sdmap_dummy_impl()\
	)

/**
 *	@hideinitializer
 *	@brief		Retrieve a pointer to the smallest key
 *	
 *	@details	Average time complexity - `O(log(count))`
 *
 *	@param[in]	map		Map to retrieve key from
 *	
 *	@return		Pointer to key, `NULL` if map is empty
 */
#define sdmap_min(map) ((const sdmap_typeof(map[0].type_data->key) *)\
	detail_sdmap_min_key_impl(\
		detail_sdmap_m2h(map),\
		sizeof(map[0].type_data->slot)))

/**
 *	@hideinitializer
 *	@brief		Retrieve a pointer to the largest key
 *	
 *	@details	Average time complexity - `O(log(count))`
 *
 *	@param[in]	map		Map to retrieve key from
 *	
 *	@return		Pointer to key, `NULL` if map is empty
 */
#define sdmap_max(map)((const sdmap_typeof(map[0].type_data->key) *)\
	detail_sdmap_max_key_impl(\
		detail_sdmap_m2h(map),\
		sizeof(map[0].type_data->slot)))

/**
 *	@hideinitializer
 *	@brief		Retrieve a pointer to the root key
 *	
 *	@details	Average time complexity - `O(1)`
 *
 *	@param[in]	map		Map to retrieve key from
 *	
 *	@return		Pointer to key, `NULL` if map is empty
 */
#define sdmap_root(map) ((const sdmap_typeof(map[0].type_data->key) *)\
	detail_sdmap_root_key_impl(\
		detail_sdmap_m2h(map),\
		sizeof(map[0].type_data->slot)))\

/**
 *	@hideinitializer
 *	@brief		Retrieve a pointer to the key that comes after key
 *	
 *	@details	Average time complexity -\n
 *					`O(log(count))`\n
 *					`O(1)` if `key_expr` is a pointer retrieved by any 
 *					function in this library.
 *
 *	@param[in]	map			Map to retrieve key from
 *	@param[in]	key_expr	Either a key or a pointer to a key
 *	
 *	@return		Pointer to key, `NULL` if map is empty or key is largest in
 *				map
 */
#define sdmap_next(map, key_expr)\
	((const sdmap_typeof(map[0].type_data->key) *)\
		(detail_sdmap_next_key_impl(\
			detail_sdmap_m2h(map),\
			sizeof(map[0].type_data->slot),\
			detail_sdmap_keyexpr_to_pointer(map, key_expr))))

/**
 *	@hideinitializer
 *	@brief		Retrieve a pointer to the key that comes before key
 *	
 *	@details	Average time complexity -\n
 *					`O(log(count))`\n
 *					`O(1)` if `key_expr` is a pointer retrieved by any 
 *					function in this library.
 *
 *	@param[in]	map			Map to retrieve key from
 *	@param[in]	key_expr	Either a key or a pointer to a key
 *	
 *	@return		Pointer to key, `NULL` if map is empty or key is largest in
 *				map
 */
#define sdmap_prev(map, key_expr)\
	((const sdmap_typeof(map[0].type_data->key) *)\
		(detail_sdmap_prev_key_impl(\
			detail_sdmap_m2h(map),\
			sizeof(map[0].type_data->slot),\
			detail_sdmap_keyexpr_to_pointer(map, key_expr))))\

/**
 *	@hideinitializer
 *	@brief		Traverse the map in order and look at keys
 *	
 *	@details	Average time complexity - `O(capacity)`\n
 *				Note that this function temporarily modifies the contents of
 *				the map inorder to traverse it effectively. This means that
 *				the entire map is invalid for the duration of the traversal
 *				and should not be looked at/modified.
 *
 *	@param[in]	map			Map to traverse
 *	@param[in]	function	Function to call. Default prototype is
 *							`void (const void *key)`.
 *	@param[in]	user		(OPTIONAL) User pointer `(void *)`, if specified,
 *							then function prototype changes to
 *							`void (const void *key, void *user)`.
 *	
 */
#define sdmap_traverse_inorder_keys(map, ...)\
	detail_sdmap_getter_upto_2(\
		__VA_ARGS__,\
		detail_sdmap_traverse_inorder_keys2,\
		detail_sdmap_traverse_inorder_keys1,\
		dummy)\
	(map, __VA_ARGS__)

/**
 *	@hideinitializer
 *	@brief		Traverse the map in pre order and look at keys
 *	
 *	@details	Average time complexity - `O(capacity)`\n
 *				Note that this function temporarily modifies the contents of
 *				the map inorder to traverse it effectively. This means that
 *				the entire map is invalid for the duration of the traversal
 *				and should not be looked at/modified.
 *
 *	@param[in]	map			Map to traverse
 *	@param[in]	function	Function to call. Default prototype is
 *							`void (const void *key)`.
 *	@param[in]	user		(OPTIONAL) User pointer `(void *)`, if specified,
 *							then function prototype changes to
 *							`void (const void *key, void *user)`.
 *	
 */
#define sdmap_traverse_preorder_keys(map, ...)\
	detail_sdmap_getter_upto_2(\
		__VA_ARGS__,\
		detail_sdmap_traverse_preorder_keys2,\
		detail_sdmap_traverse_preorder_keys1,\
		dummy)\
	(map, __VA_ARGS__)

/**
 *	@hideinitializer
 *	@brief		Traverse the map in order and look at values
 *	
 *	@details	Average time complexity - `O(capacity)`\n
 *				Note that this function temporarily modifies the contents of
 *				the map inorder to traverse it effectively. This means that
 *				the entire map is invalid for the duration of the traversal
 *				and should not be looked at/modified.
 *
 *	@param[in]	map			Map to traverse
 *	@param[in]	function	Function to call. Default prototype is
 *							`void (void *value)`.
 *	@param[in]	user		(OPTIONAL) User pointer `(void *)`, if specified,
 *							then function prototype changes to
 *							`void (void *value, void *user)`.
 *	
 */
#define sdmap_traverse_inorder_values(map, ...)\
	detail_sdmap_getter_upto_2(\
		__VA_ARGS__,\
		detail_sdmap_traverse_inorder_values2,\
		detail_sdmap_traverse_inorder_values1,\
		dummy)\
	(map, __VA_ARGS__)

/**
 *	@hideinitializer
 *	@brief		Traverse the map in preorder and look at values
 *	
 *	@details	Average time complexity - `O(capacity)`\n
 *				Note that this function temporarily modifies the contents of
 *				the map inorder to traverse it effectively. This means that
 *				the entire map is invalid for the duration of the traversal
 *				and should not be looked at/modified.
 *
 *	@param[in]	map			Map to traverse
 *	@param[in]	function	Function to call. Default prototype is
 *							`void (void *value)`.
 *	@param[in]	user		(OPTIONAL) User pointer `(void *)`, if specified,
 *							then function prototype changes to
 *							`void (void *value, void *user)`.
 *	
 */
#define sdmap_traverse_preorder_values(map, ...)\
	detail_sdmap_getter_upto_2(\
		__VA_ARGS__,\
		detail_sdmap_traverse_preorder_values2,\
		detail_sdmap_traverse_preorder_values1,\
		dummy)\
	(map, __VA_ARGS__)

/**
 *	@hideinitializer
 *	@brief		Traverse the map in order and look at both keys and values
 *	
 *	@details	Average time complexity - `O(capacity)`\n
 *				Note that this function temporarily modifies the contents of
 *				the map inorder to traverse it effectively. This means that
 *				the entire map is invalid for the duration of the traversal
 *				and should not be looked at/modified.
 *
 *	@param[in]	map			Map to traverse
 *	@param[in]	function	Function to call. Default prototype is
 *							`void (const void *key, void *value)`.
 *	@param[in]	user		(OPTIONAL) User pointer `(void *)`, if specified,
 *							then function prototype changes to
 *							`void (const void *key, void *value, void *user)`.
 *	
 */
#define sdmap_traverse_inorder_pairs(map, ...)\
	detail_sdmap_getter_upto_2(\
		__VA_ARGS__,\
		detail_sdmap_traverse_inorder_pairs2,\
		detail_sdmap_traverse_inorder_pairs1,\
		dummy)\
	(map, __VA_ARGS__)

/**
 *	@hideinitializer
 *	@brief		Traverse the map in preorder and look at both keys and values
 *	
 *	@details	Average time complexity - `O(capacity)`\n
 *				Note that this function temporarily modifies the contents of
 *				the map inorder to traverse it effectively. This means that
 *				the entire map is invalid for the duration of the traversal
 *				and should not be looked at/modified.
 *
 *	@param[in]	map			Map to traverse
 *	@param[in]	function	Function to call. Default prototype is
 *							`void (const void *key, void *value)`.
 *	@param[in]	user		(OPTIONAL) User pointer `(void *)`, if specified,
 *							then function prototype changes to
 *							`void (const void *key, void *value, void *user)`.
 *	
 */
#define sdmap_traverse_preorder_pairs(map, ...)\
	detail_sdmap_getter_upto_2(\
		__VA_ARGS__,\
		detail_sdmap_traverse_preorder_pairs2,\
		detail_sdmap_traverse_preorder_pairs1,\
		dummy)\
	(map, __VA_ARGS__)


/**
 *	@hideinitializer
 *	@brief		Free and invalidate map.
 *	
 *	@details	Average time complexity - same as @ref sdmap_free
 *				This function call may be omitted for stack type maps.
 *
 *	@param[in]	map		Map object to free
 *	
 */
#define sdmap_delete(map) _Generic(map[0].type_data->storage_type,\
	detail_sdmap_heap_type: detail_sdmap_delete_impl((void *)(&map)),\
	detail_sdmap_stack_type: detail_sdmap_dummy_impl()\
	)

/*
 *	Detail functions
 *	@cond false
 */

#ifndef SDMAP_API
#define SDMAP_API
#endif

/**
 *	@brief		sdmap header object.
 */
typedef struct sdmap_header
{
	/**
	 *	How many elements exist in the map.
	 */
	sdmap_index count;

	/**
	 *	How many slots exist in the map.
	 */
	sdmap_index slot_count;

	/**
	 *	Index of the root element.
	 */
	sdmap_index root_slot;

	/**
	 *	Index of the topmost empty element.
	 */
	sdmap_index empty_slot;

	/**
	 *	Compare function.
	 */
	int (*compare_func)(const void *, const void *);
} sdmap_header;

/**
 *	@brief		sdmap heap object.
 */
typedef struct sdmap_heap
{
	sdmap_index capacity;
	sdmap_header header;
} sdmap_heap;

/*
 *	@brief		sdmap slot object.
 *
 *	The values in sdmap_slot have special meaning
 *	height < 0 -> slot is an empty slot
 *	height >= 0 -> slot is a node
 *	
 *	if empty slot:
 *		right -> next empty slot
 *		right == -1 -> no more empty slots
 *				
 *	if node:
 *		left == index_of_self -> no child on the left
 *		right == index_of_self -> no child on the right
 *		parent == -1 -> root node
 *			
 */
typedef struct sdmap_slot
{
	sdmap_index left;
	sdmap_index right;
	sdmap_index parent;
	int8_t height;
} sdmap_slot;

#define detail_sdmap_heap_type char

#define detail_sdmap_stack_type short

#define detail_sdmap_keyexpr_to_pointer(map, key_expr) _Generic(key_expr,\
	sdmap_typeof(map[0].type_data->key) :\
		detail_sdmap_key_to_complit(map, key_expr),\
	sdmap_typeof(map[0].type_data->key) * : key_expr,\
	const sdmap_typeof(map[0].type_data->key) * : key_expr\
	)

#define detail_sdmap_m2h(map) ((sdmap_header *)\
	((void *) _Generic(map[0].type_data->storage_type,\
		detail_sdmap_heap_type : map,\
		detail_sdmap_stack_type : map))\
	)

#define detail_sdmap_m2hp(map) ((sdmap_header **)\
	((void *) _Generic(map[0].type_data->storage_type,\
		detail_sdmap_heap_type : &map,\
		detail_sdmap_stack_type : map))\
	)

#define detail_sdmap_m2h_initialized(map) ((void *)\
	_Generic(map[0].type_data->storage_type,\
		detail_sdmap_heap_type : detail_sdmap_ensure_initialized(map),\
		detail_sdmap_stack_type : map)\
	)

#define detail_sdmap_stack_capacity(map)\
	(sdmap_index)((sizeof(map) - sizeof(sdmap_header)) / sizeof(sdmap_slot))

#define detail_sdmap_getter_upto_3(_1, _2, _3, NAME, ...) NAME

#define detail_sdmap_getter_upto_2(_1, _2, NAME, ...) NAME

#define detail_sdmap_new1(map) _Generic(map[0].type_data->storage_type,\
	detail_sdmap_heap_type : detail_sdmap_new_heap_impl(\
		detail_sdmap_m2hp(map),\
		sizeof(map[0].type_data->slot) * (SDMAP_DEFAULT_CAPACITY),\
		detail_sdmap_pick_compare_func(map[0].type_data->key)),\
	detail_sdmap_stack_type :\
		detail_sdmap_new_stack_impl(\
			detail_sdmap_m2h(map),\
			detail_sdmap_pick_compare_func(map[0].type_data->key))\
)

#define detail_sdmap_new2(map, function) _Generic(\
		map[0].type_data->storage_type,\
	detail_sdmap_heap_type :\
		detail_sdmap_new_heap_impl(\
			detail_sdmap_m2hp(map),\
			sizeof(map[0].type_data->slot) * (SDMAP_DEFAULT_CAPACITY),\
		function),\
	detail_sdmap_stack_type :\
		detail_sdmap_new_stack_impl(\
			detail_sdmap_m2h(map),\
			function)\
)

#define detail_sdmap_new3(map, function, capacity)\
	_Generic(map[0].type_data->storage_type,\
	detail_sdmap_heap_type : detail_sdmap_new_heap_impl(\
		detail_sdmap_m2hp(map),\
		sizeof(map[0].type_data->slot) * (capacity),\
		function),\
	detail_sdmap_stack_type :\
		sdmap_assert(0 && "sdmap_new called with 3 arguments on a stack-type\
map")\
)\

#define detail_sdmap_traverse_inorder_keys1(map, function)\
	detail_sdmap_traverse_inorder_keys_impl(\
		detail_sdmap_m2h(map),\
		sizeof(map[0].type_data->slot),\
		function)

#define detail_sdmap_traverse_inorder_keys2(map, function, user)\
	detail_sdmap_traverse_inorder_keys_ex_impl(\
		detail_sdmap_m2h(map),\
		sizeof(map[0].type_data->slot),\
		function,\
		user)

#define detail_sdmap_traverse_preorder_keys1(map, function)\
	detail_sdmap_traverse_preorder_keys_impl(\
		detail_sdmap_m2h(map),\
		sizeof(map[0].type_data->slot),\
		function)

#define detail_sdmap_traverse_preorder_keys2(map, function, user)\
	detail_sdmap_traverse_preorder_keys_ex_impl(\
		detail_sdmap_m2h(map),\
		sizeof(map[0].type_data->key),\
		sizeof(map[0].type_data->value),\
		function,\
		user)

#define detail_sdmap_traverse_inorder_values1(map, function)\
	detail_sdmap_traverse_inorder_values_impl(\
		detail_sdmap_m2h(map),\
		sizeof(map[0].type_data->slot),\
		offsetof(sdmap_typeof(map[0].type_data->slot), value),\
		function)

#define detail_sdmap_traverse_inorder_values2(map, function, user)\
	detail_sdmap_traverse_inorder_values_ex_impl(\
		detail_sdmap_m2h(map),\
		sizeof(map[0].type_data->slot),\
		offsetof(sdmap_typeof(map[0].type_data->slot), value),\
		function,\
		user)

#define detail_sdmap_traverse_preorder_values1(map, function)\
	detail_sdmap_traverse_preorder_values_impl(\
		detail_sdmap_m2h(map),\
		sizeof(map[0].type_data->slot),\
		offsetof(sdmap_typeof(map[0].type_data->slot), value),\
		function)

#define detail_sdmap_traverse_preorder_values2(map, function, user)\
	detail_sdmap_traverse_preorder_values_ex_impl(\
		detail_sdmap_m2h(map),\
		sizeof(map[0].type_data->slot),\
		offsetof(sdmap_typeof(map[0].type_data->slot), value),\
		function,\
		user)

#define detail_sdmap_traverse_inorder_pairs1(map, function)\
	detail_sdmap_traverse_inorder_pairs_impl(\
		detail_sdmap_m2h(map),\
		sizeof(map[0].type_data->slot),\
		offsetof(sdmap_typeof(map[0].type_data->slot), value),\
		function)

#define detail_sdmap_traverse_inorder_pairs2(map, function, user)\
	detail_sdmap_traverse_inorder_pairs_ex_impl(\
		detail_sdmap_m2h(map),\
		sizeof(map[0].type_data->slot),\
		offsetof(sdmap_typeof(map[0].type_data->slot), value),\
		function,\
		user)

#define detail_sdmap_traverse_preorder_pairs1(map, function)\
	detail_sdmap_traverse_preorder_pairs_impl(\
		detail_sdmap_m2h(map),\
		sizeof(map[0].type_data->slot),\
		offsetof(sdmap_typeof(map[0].type_data->slot), value),\
		function)

#define detail_sdmap_traverse_preorder_pairs2(map, function, user)\
	detail_sdmap_traverse_preorder_pairs_ex_impl(\
		detail_sdmap_m2h(map),\
		sizeof(map[0].type_data->slot),\
		offsetof(sdmap_typeof(map[0].type_data->slot), value),\
		function,\
		user)

#define detail_sdmap_ensure_initialized(map)\
	detail_sdmap_ensure_initialized_impl(\
		(void *)(&map),\
		(sizeof(map[0].type_data->slot)) *	SDMAP_DEFAULT_CAPACITY,\
		detail_sdmap_pick_compare_func(map[0].type_data->key))

#define detail_sdmap_key_to_complit2(map, key_value)\
	_Generic(key_value, \
		sdmap_typeof(sdmap_typeof(map[0].type_data->key)) : key_value,\
		default : (sdmap_typeof(map[0].type_data->key)){0}\
	)

#define detail_sdmap_key_to_complit(map, key_value)\
	((const void *)(&((sdmap_typeof(map[0].type_data->key))\
		{detail_sdmap_key_to_complit2(map, key_value)})))

#define detail_sdmap_pick_compare_func(key) _Generic(key,\
	uint8_t: detail_sdmap_compare_uint8_t,\
	uint16_t: detail_sdmap_compare_uint16_t,\
	uint32_t: detail_sdmap_compare_uint32_t,\
	uint64_t: detail_sdmap_compare_uint64_t,\
	int8_t: detail_sdmap_compare_int8_t,\
	int16_t: detail_sdmap_compare_int16_t,\
	int32_t: detail_sdmap_compare_int32_t,\
	int64_t: detail_sdmap_compare_int64_t,\
	float: detail_sdmap_compare_float,\
	double: detail_sdmap_compare_double,\
	long double: detail_sdmap_compare_long_double,\
	char *: detail_sdmap_strcmp,\
	default: NULL\
	)

#define detail_sdmap_declare_compare_func(type, postfix)\
SDMAP_API int detail_sdmap_compare_##postfix(const void *a, const void *b);

detail_sdmap_declare_compare_func(uint8_t, uint8_t)
detail_sdmap_declare_compare_func(uint16_t, uint16_t)
detail_sdmap_declare_compare_func(uint32_t, uint32_t)
detail_sdmap_declare_compare_func(uint64_t, uint64_t)
detail_sdmap_declare_compare_func(int8_t, int8_t)
detail_sdmap_declare_compare_func(int16_t, int16_t)
detail_sdmap_declare_compare_func(int32_t, int32_t)
detail_sdmap_declare_compare_func(int64_t, int64_t)
detail_sdmap_declare_compare_func(float, float)
detail_sdmap_declare_compare_func(double, double)
detail_sdmap_declare_compare_func(long double, long_double)

SDMAP_API int detail_sdmap_strcmp(const void *a, const void *b);

SDMAP_API sdmap_index detail_sdmap_count_impl(sdmap_header *header);

SDMAP_API sdmap_index detail_sdmap_capacity_impl(
	sdmap_header *header,
	uint32_t slot_size);

SDMAP_API void detail_sdmap_new_heap_impl(
	sdmap_header **header,
	sdmap_index capacity,
	int (*compare_func)(const void *, const void *));

SDMAP_API void detail_sdmap_new_stack_impl(
	sdmap_header *header,
	int (*compare_func)(const void *, const void *));

SDMAP_API void detail_sdmap_duplicate_heap_heap_impl(
	sdmap_header **header,
	sdmap_header *source);

SDMAP_API void detail_sdmap_duplicate_stack_heap_impl(
	sdmap_header *header,
	sdmap_index dest_capacity,
	uint32_t slot_size,
	sdmap_header *source);

SDMAP_API void detail_sdmap_duplicate_heap_stack_impl(
	sdmap_header **header,
	uint32_t slot_size,
	sdmap_header *source,
	sdmap_index src_capacity);

SDMAP_API void detail_sdmap_duplicate_stack_stack_impl(
	sdmap_header *header,
	sdmap_index dest_capacity,
	uint32_t slot_size,
	sdmap_header *source);

SDMAP_API sdmap_header **detail_sdmap_ensure_initialized_impl(
	sdmap_header **header,
	sdmap_index capacity,
	int (*compare_func)(const void *, const void *));

SDMAP_API int detail_sdmap_contains_impl(
	sdmap_header *header,
	uint32_t slot_size,
	const void *key);

SDMAP_API void detail_sdmap_optimize_empty_slots(
	sdmap_header *header,
	uint32_t slot_size);

SDMAP_API void detail_sdmap_optimize_reduce_slots(
	sdmap_header *header,
	uint32_t slot_size);

SDMAP_API void detail_sdmap_optimize_impl(
	sdmap_header *header,
	uint32_t slot_size);

SDMAP_API void detail_sdmap_shrink_impl(
	sdmap_header **header,
	uint32_t slot_size);

SDMAP_API void *detail_sdmap_min_key_impl(
	sdmap_header *header,
	uint32_t slot_size);

SDMAP_API void *detail_sdmap_max_key_impl(
	sdmap_header *header,
	uint32_t slot_size);

SDMAP_API void *detail_sdmap_root_key_impl(
	sdmap_header *header,
	uint32_t slot_size);

SDMAP_API void *detail_sdmap_next_key_impl(
	sdmap_header *header,
	uint32_t slot_size,
	const void *key);

SDMAP_API void *detail_sdmap_prev_key_impl(
	sdmap_header *header,
	uint32_t slot_size,
	const void *key);

SDMAP_API void *detail_sdmap_getp_impl(
	sdmap_header *header,
	uint32_t slot_size,
	uint32_t value_offset,
	const void *key);

SDMAP_API void *detail_sdmap_set_heap_impl(
	sdmap_header **header,
	uint32_t slot_size,
	uint32_t key_size,
	const void *key);

SDMAP_API void *detail_sdmap_set_stack_impl(
	sdmap_header *header,
	uint32_t slot_size,
	uint32_t key_size,
	const void *key,
	sdmap_index capacity);

SDMAP_API void detail_sdmap_erase_impl(
	sdmap_header *header,
	uint32_t slot_size,
	const void *key);

SDMAP_API void detail_sdmap_delete_impl(sdmap_header **header);

SDMAP_API void detail_sdmap_dummy_impl(void);

SDMAP_API void detail_sdmap_traverse_inorder_keys_impl(
	sdmap_header *header,
	uint32_t slot_size,
	void (*function)(const void *key));

SDMAP_API void detail_sdmap_traverse_preorder_keys_impl(
	sdmap_header *header,
	uint32_t slot_size,
	void (*function)(const void *key));

SDMAP_API void detail_sdmap_traverse_inorder_values_impl(
	sdmap_header *header,
	uint32_t slot_size,
	uint32_t value_offset,
	void (*function)(void *value));

SDMAP_API void detail_sdmap_traverse_preorder_values_impl(
	sdmap_header *header,
	uint32_t slot_size,
	uint32_t value_offset,
	void (*function)(void *value));

SDMAP_API void detail_sdmap_traverse_inorder_pairs_impl(
	sdmap_header *header,
	uint32_t slot_size,
	uint32_t value_offset,
	void (*function)(const void *key, void *value));

SDMAP_API void detail_sdmap_traverse_preorder_pairs_impl(
	sdmap_header *header,
	uint32_t slot_size,
	uint32_t value_offset,
	void (*function)(const void *key, void *value));

SDMAP_API void detail_sdmap_traverse_inorder_keys_ex_impl(
	sdmap_header *header,
	uint32_t slot_size,
	void (*function)(const void *key, void *user),
	void *user);

SDMAP_API void detail_sdmap_traverse_preorder_keys_ex_impl(
	sdmap_header *header,
	uint32_t slot_size,
	void (*function)(const void *key, void *user),
	void *user);

SDMAP_API void detail_sdmap_traverse_inorder_values_ex_impl(
	sdmap_header *header,
	uint32_t slot_size,
	uint32_t value_offset,
	void (*function)(void *value, void *user),
	void *user);

SDMAP_API void detail_sdmap_traverse_preorder_values_ex_impl(
	sdmap_header *header,
	uint32_t slot_size,
	uint32_t value_offset,
	void (*function)(void *value, void *user),
	void *user);

SDMAP_API void detail_sdmap_traverse_inorder_pairs_ex_impl(
	sdmap_header *header,
	uint32_t slot_size,
	uint32_t value_offset,
	void (*function)(const void *key, void *value, void *user),
	void *user);

SDMAP_API void detail_sdmap_traverse_preorder_pairs_ex_impl(
	sdmap_header *header,
	uint32_t slot_size,
	uint32_t value_offset,
	void (*function)(const void *key, void *value, void *user),
	void *user);

/*
 *	End of detail functions
 *	@endcond
 */

#endif
