#include <sdhmap.h>

void *sdhmap_malloc(size_t size);
void *sdhmap_realloc(void *ptr, size_t size);
void sdhmap_free(void *ptr);

#define detail_sdhmap_slot(map, index) ((sdhmap_slot *)((char *)(map) + sizeof(sdhmap_header) + index * slot_size))

#define detail_sdhmap_heap_from_header(h) ((sdhmap_heap *)((char *)((void *)(h)) - offsetof(sdhmap_heap, header)))

#define detail_sdhmap_define_hash_func(type, postfix)\
SDHMAP_API sdhmap_index detail_sdhmap_hash_##postfix(const void *a)\
{\
	unsigned int i;\
	int j;\
	sdhmap_index byte, crc, mask;\
	crc = ~0;\
	for (i = 0; i < sizeof(type); i++)\
	{\
		byte = ((const char *)a)[i];\
		crc = crc ^ byte;\
		for (j = 7; j >= 0; j--)\
		{\
			mask = -(crc & 1);\
			crc = (crc >> 1) ^ (0xEDB88320 & mask);\
		}\
	}\
	return ~crc;\
}

detail_sdhmap_define_hash_func(uint8_t, uint8_t)
detail_sdhmap_define_hash_func(uint16_t, uint16_t)
detail_sdhmap_define_hash_func(uint32_t, uint32_t)
detail_sdhmap_define_hash_func(uint64_t, uint64_t)
detail_sdhmap_define_hash_func(int8_t, int8_t)
detail_sdhmap_define_hash_func(int16_t, int16_t)
detail_sdhmap_define_hash_func(int32_t, int32_t)
detail_sdhmap_define_hash_func(int64_t, int64_t)
detail_sdhmap_define_hash_func(float, float)
detail_sdhmap_define_hash_func(double, double)
detail_sdhmap_define_hash_func(long double, long_double)

SDHMAP_API sdhmap_index detail_sdhmap_hash_string(const void *a)
{
	const char *str;
	int i, j;
	sdhmap_index byte, crc, mask;
	str = *((const char **)a);
	if (str == NULL)
	{
		return 0;
	}
	crc = ~0;
	for (i = 0; i < 8 && *str; i++)
	{
		byte = *str;
		crc = crc ^ byte;
		for (j = 7; j >= 0; j--)
		{
			mask = -(crc & 1);
			crc = (crc >> 1) ^ (0xEDB88320 & mask);
		}
		str++;
	}
	return ~crc;
}

SDHMAP_API int detail_sdhmap_eq_string(const void *a, const void *b)
{
	return strcmp(*((const char **)a), *((const char **)b));
}

SDHMAP_API sdhmap_index detail_sdhmap_count_impl(sdhmap_header *header)
{
	if (header)
	{
		return header->count;
	}
	return 0;
}

SDHMAP_API sdhmap_index detail_sdhmap_capacity_impl(
	sdhmap_header *header,
	uint32_t slot_size)
{
	if (header)
	{
		return detail_sdhmap_heap_from_header(header)->capacity / slot_size;
	}
	return 0;
}

SDHMAP_API void detail_sdhmap_init_slots(
	sdhmap_header *header,
	uint32_t slot_size)
{
	sdhmap_index i;
	detail_sdhmap_slot(header, 0)->slot = -1;
	detail_sdhmap_slot(header, 0)->next = 1;
	detail_sdhmap_slot(header, 0)->prev = -1;
	for (i = 1; i + 1 < header->slot_count; i++)
	{
		detail_sdhmap_slot(header, i)->slot = -1;
		detail_sdhmap_slot(header, i)->next = i + 1;
		detail_sdhmap_slot(header, i)->prev = i - 1;
	}
	detail_sdhmap_slot(header, i)->slot = -1;
	detail_sdhmap_slot(header, i)->next = -1;
	detail_sdhmap_slot(header, i)->prev = i - 1;
}

SDHMAP_API void detail_sdhmap_new_heap_impl(
	sdhmap_header **header,
	sdhmap_index (*hash_func)(const void *),
	int (*eq_func)(const void *, const void *),
	sdhmap_index count,
	uint32_t slot_size)
{
	sdhmap_heap *heap = sdhmap_malloc(sizeof(sdhmap_heap) + count * slot_size);
	sdhmap_assert((heap != NULL) && "sdhmap_malloc returned NULL");
	heap->capacity = count * slot_size;
	*header = &(heap->header);
	detail_sdhmap_new_stack_impl(
		*header, 
		hash_func, 
		eq_func, 
		count, 
		slot_size);
}

SDHMAP_API void detail_sdhmap_new_stack_impl(
	sdhmap_header *header,
	sdhmap_index (*hash_func)(const void *),
	int (*eq_func)(const void *, const void *),
	sdhmap_index count,
	uint32_t slot_size)
{
	header->count = 0;
	header->slot_count = count;
	header->used_bucket_count = 0;
	header->hash_func = hash_func;
	header->eq_func = eq_func;
	if (count > 0)
	{
		header->empty_slot = 0;
		detail_sdhmap_init_slots(header, slot_size);
	}
	else
	{
		header->empty_slot = (sdhmap_index)-1;
	}
}

SDHMAP_API void detail_sdhmap_duplicate_heap_heap_impl(sdhmap_header **header, sdhmap_header *source)
{
	sdhmap_heap *heap;
	if (source == NULL)
	{
		*header = NULL;
		return;
	}
	heap = sdhmap_malloc(sizeof(sdhmap_heap) + detail_sdhmap_heap_from_header(source)->capacity);
	sdhmap_assert(heap != NULL && "sdhmap_malloc returned NULL");
	memcpy(heap, detail_sdhmap_heap_from_header(source), sizeof(sdhmap_heap) + detail_sdhmap_heap_from_header(source)->capacity);
	*header = &(heap->header);
}

SDHMAP_API void detail_sdhmap_duplicate_stack_heap_impl(sdhmap_header *header, sdhmap_index dest_capacity, uint32_t slot_size, sdhmap_header *source)
{
	(void)dest_capacity;
	if (source == NULL)
	{
		header->count = 0;
		header->slot_count = 0;
		header->used_bucket_count = 0;
		header->empty_slot = (sdhmap_index)-1;
		header->hash_func = NULL;
		header->eq_func = NULL;
		return;
	}
	sdhmap_assert((dest_capacity <= source->slot_count) && "stack-type sdhmap is too small.");
	memcpy(header, source, sizeof(sdhmap_header) + source->slot_count * slot_size);
}

SDHMAP_API void detail_sdhmap_duplicate_heap_stack_impl(sdhmap_header **header, uint32_t slot_size, sdhmap_header *source, sdhmap_index src_capacity)
{
	sdhmap_heap *heap;
	heap = sdhmap_malloc(sizeof(sdhmap_heap) + src_capacity * slot_size);
	heap->capacity = src_capacity * slot_size;
	sdhmap_assert(heap != NULL && "sdhmap_malloc returned NULL");
	memcpy(&heap->header, source, sizeof(sdhmap_header) + heap->capacity);
	*header = &(heap->header);
}

SDHMAP_API void detail_sdhmap_duplicate_stack_stack_impl(sdhmap_header *header, sdhmap_index dest_capacity, uint32_t slot_size, sdhmap_header *source)
{
	(void)dest_capacity;
	sdhmap_assert((dest_capacity <= source->slot_count) && "sdhmap_malloc returned NULL");
	memcpy(header, source, sizeof(sdhmap_header) + source->slot_count * slot_size);
}

SDHMAP_API sdhmap_header **detail_sdhmap_ensure_initialized_impl(
	sdhmap_header **header,
	sdhmap_index (*hash_func)(const void *),
	int (*eq_func)(const void *, const void *),
	sdhmap_index count,
	uint32_t slot_size)
{
	if (!(*header))
	{
		detail_sdhmap_new_heap_impl(
			header, hash_func, eq_func, count, slot_size);
	}
	return header;
}

SDHMAP_API int detail_sdhmap_contains_impl(
	sdhmap_header *header,
	uint32_t slot_size,
	uint32_t key_size,
	const void *key)
{
	sdhmap_index hash;
	sdhmap_slot *slot;
	if (header == NULL ||
		header->slot_count == 0)
	{
		return 0;
	}
	assert(header->hash_func && "sdhmap hash function is NULL");
	hash = header->hash_func(key) % header->slot_count;
	slot = detail_sdhmap_slot(header, hash);
	if (slot->slot == (sdhmap_index)-1)
	{
		return 0;
	}
	hash = detail_sdhmap_slot(header, hash)->slot;
	slot = detail_sdhmap_slot(header, hash);
	while (1)
	{
		if (header->eq_func)
		{
			if (header->eq_func(slot + 1, key) == 0)
			{
				return 1;
			}
		}
		else
		{
			if (memcmp(slot + 1, key, key_size) == 0)
			{
				return 1;
			}
		}
		if (slot->next != (sdhmap_index)-1)
		{
			hash = slot->next;
			slot = detail_sdhmap_slot(header, hash);
		}
		else
		{
			return 0;
		}
	}
}

SDHMAP_API sdhmap_index detail_sdhmap_pop_empty(
	sdhmap_header *header,
	uint32_t slot_size)
{
	sdhmap_index index;
	sdhmap_slot *slot;
	assert((header->empty_slot != ((sdhmap_index)-1)) && 
		"sdhmap has invalid slot");
	index = header->empty_slot;
	slot = detail_sdhmap_slot(header, index);
	if (slot->next != (sdhmap_index)-1)
	{
		detail_sdhmap_slot(header, slot->next)->prev = slot->prev;
	}
	header->empty_slot = slot->next;
	slot->next = (sdhmap_index)-1;
	slot->prev = (sdhmap_index)-1;
	return index;
}

SDHMAP_API void *detail_sdhmap_insert_to_empty(
	sdhmap_header *header,
	uint32_t slot_size,
	uint32_t key_size,
	const void *key,
	sdhmap_index empty_index)
{
	sdhmap_index new_index;
	sdhmap_slot *slot;
	new_index = detail_sdhmap_pop_empty(header, slot_size);
	detail_sdhmap_slot(header, empty_index)->slot = new_index;
	slot = detail_sdhmap_slot(header, new_index) + 1;
	memcpy(slot, key, key_size);
	header->count ++;
	header->used_bucket_count ++;
	return ((char *)(slot)) + key_size;
}

SDHMAP_API void *detail_sdhmap_insert_to_list(
	sdhmap_header *header,
	uint32_t slot_size,
	uint32_t key_size,
	const void *key,
	sdhmap_index end_index)
{
	sdhmap_index new_index;
	sdhmap_slot *slot;
	new_index = detail_sdhmap_pop_empty(header, slot_size);
	detail_sdhmap_slot(header, end_index)->next = new_index;
	slot = detail_sdhmap_slot(header, new_index) + 1;
	memcpy(slot, key, key_size);
	header->count ++;
	return ((char *)(slot)) + key_size;
}

SDHMAP_API char *detail_sdhmap_get_kv_pairs(
	sdhmap_header *header,
	uint32_t slot_size)
{
	char *pairs;
	const uint32_t pair_size = slot_size - sizeof(sdhmap_slot);
	sdhmap_index i, j;
	sdhmap_slot *slot;
	if (header->slot_count == 0)
	{
		return NULL;
	}
	pairs = sdhmap_malloc(pair_size * header->count);
	sdhmap_assert((pairs != NULL) && "sdhmap_malloc returned NULL");
	j = 0;
	for (i = 0; i < header->slot_count; i++)
	{
		slot = detail_sdhmap_slot(header, i);
		if (slot->slot != (sdhmap_index)-1)
		{
			slot = detail_sdhmap_slot(header, slot->slot);
			memcpy(pairs + j * pair_size, slot + 1, pair_size);
			j++;
			while (1)
			{
				if (slot->next == (sdhmap_index)-1)
				{
					break;
				}
				slot = detail_sdhmap_slot(header, slot->next);
				memcpy(pairs + j * pair_size, slot + 1, pair_size);
				j++;
			}
		}
	}
	return pairs;
}

SDHMAP_API sdhmap_header *detail_sdhmap_heap_realloc(
	sdhmap_header *header,
	sdhmap_index slot_size)
{
	sdhmap_heap *heap;
	sdhmap_index capacity;
	heap = detail_sdhmap_heap_from_header(header);
	capacity = heap->capacity;
	while (capacity < sizeof(sdhmap_heap) + slot_size * header->slot_count)
	{
		capacity *= 2;
	}
	if (capacity > heap->capacity)
	{
		heap = sdhmap_realloc(heap, capacity);
		sdhmap_assert((heap != NULL) && "sdhmap_realloc returned NULL");
		heap->capacity = capacity;
	}
	return &(heap->header);
}

SDHMAP_API void *detail_sdhmap_set_common(
	sdhmap_header *header,
	uint32_t slot_size,
	uint32_t key_size,
	const void *key)
{
	sdhmap_index hash;
	sdhmap_slot *slot;
	assert(header->hash_func && "sdhmap hash function is NULL");
	hash = header->hash_func(key) % header->slot_count;
	slot = detail_sdhmap_slot(header, hash);
	if (slot->slot == (sdhmap_index)-1)
	{
		return detail_sdhmap_insert_to_empty(
			header, slot_size, key_size, key, hash);
	}
	hash = detail_sdhmap_slot(header, hash)->slot;
	slot = detail_sdhmap_slot(header, hash);
	while (1)
	{
		if (header->eq_func)
		{
			if (header->eq_func(slot + 1, key) == 0)
			{
				return ((char *)(slot + 1)) + key_size;
			}
		}
		else
		{
			if (memcmp(slot + 1, key, key_size) == 0)
			{
				return ((char *)(slot + 1)) + key_size;
			}
		}
		if (slot->next != (sdhmap_index)-1)
		{
			hash = slot->next;
			slot = detail_sdhmap_slot(header, hash);
		}
		else
		{
			return detail_sdhmap_insert_to_list(
				header, slot_size, key_size, key, hash);
		}
	}
}

SDHMAP_API void detail_sdhmap_heap_resize(
	sdhmap_header **header,
	uint32_t slot_size,
	uint32_t key_size,
	sdhmap_index target)
{
	char *pairs;
	const uint32_t pair_size = slot_size - sizeof(sdhmap_slot);
	sdhmap_index i, j;
	if ((*header)->slot_count == target)
	{
		return;
	}
	j = (*header)->count;
	pairs = detail_sdhmap_get_kv_pairs(*header, slot_size);
	(*header)->count = 0;
	(*header)->slot_count = target;
	(*header)->used_bucket_count = 0;
	(*header)->empty_slot = 0;
	*header = detail_sdhmap_heap_realloc(*header, slot_size);
	detail_sdhmap_init_slots(*header, slot_size);
	for (i = 0; i < j; i++)
	{
		memcpy(
			detail_sdhmap_set_common(
				*header,
				slot_size,
				key_size,
				pairs + i * pair_size),
			pairs + i * pair_size + key_size,
			slot_size - key_size - sizeof(sdhmap_slot));
	}
	sdhmap_free(pairs);
}

SDHMAP_API void *detail_sdhmap_set_heap_impl(
	sdhmap_header **header,
	uint32_t slot_size,
	uint32_t key_size,
	const void *key)
{
	if (((float)(*header)->slot_count * SDHMAP_MAX_LOAD_FACTOR) < (*header)->count ||
		(*header)->slot_count == 0)
	{
		detail_sdhmap_heap_resize(header, slot_size, key_size,
			(*header)->slot_count == 0 ? 
				SDHMAP_DEFAULT_CAPACITY :
				(*header)->slot_count * 2);
	}
	return detail_sdhmap_set_common(*header, slot_size, key_size, key);
}

SDHMAP_API void detail_sdhmap_stack_resize(
	sdhmap_header *header,
	uint32_t slot_size,
	uint32_t key_size,
	sdhmap_index capacity,
	sdhmap_index target)
{
	char *pairs;
	const uint32_t pair_size = slot_size - sizeof(sdhmap_slot);
	sdhmap_index i, j;
	j = header->count;
	pairs = detail_sdhmap_get_kv_pairs(header, slot_size);
	header->count = 0;
	header->slot_count = target;
	header->used_bucket_count = 0;
	header->empty_slot = 0;
	if (header->slot_count > capacity)
	{
		header->slot_count = capacity;
	}
	detail_sdhmap_init_slots(header, slot_size);
	for (i = 0; i < j; i++)
	{
		memcpy(
			detail_sdhmap_set_common(
				header,
				slot_size,
				key_size,
				pairs + i * pair_size),
			pairs + i * pair_size + key_size,
			slot_size - key_size - sizeof(sdhmap_slot));
	}
	sdhmap_free(pairs);
}

SDHMAP_API void *detail_sdhmap_set_stack_impl(
	sdhmap_header *header,
	uint32_t slot_size,
	uint32_t key_size,
	const void *key,
	sdhmap_index capacity)
{
	sdhmap_assert((capacity != 0) && "sdhmap has 0 capacity");
	if (((float)header->slot_count * SDHMAP_MAX_LOAD_FACTOR) > header->count &&
		header->slot_count != capacity)
	{
		detail_sdhmap_stack_resize(header, slot_size, key_size, capacity,
			header->slot_count == 0 ? 
				SDHMAP_DEFAULT_CAPACITY :
				header->slot_count * 2);
	}
	return detail_sdhmap_set_common(header, slot_size, key_size, key);
}

SDHMAP_API void *detail_sdhmap_set_heap_optimized_impl(
	sdhmap_header **header,
	uint32_t slot_size,
	uint32_t key_size,
	const void *key)
{
	sdhmap_index i;
	uintptr_t key_offset;
	if ((uintptr_t)key >= (uintptr_t)(*header + 1) &&
		(uintptr_t)key <= (uintptr_t)(((char *)(*header + 1)) + slot_size * (*header)->slot_count))
	{
		key_offset = (uintptr_t)key - (uintptr_t)(*header + 1);
		if (key_offset % (uintptr_t)slot_size == sizeof(sdhmap_slot))
		{
			i = key_offset / (uintptr_t)slot_size;
			return ((char *)(detail_sdhmap_slot(*header, i) + 1)) + key_size;
		}
	}
	return detail_sdhmap_set_heap_impl(header, slot_size, key_size, key);
}

SDHMAP_API void *detail_sdhmap_set_stack_optimized_impl(
	sdhmap_header *header,
	uint32_t slot_size,
	uint32_t key_size,
	const void *key,
	sdhmap_index capacity)
{
	sdhmap_index i;
	uintptr_t key_offset;
	sdhmap_assert((capacity != 0) && "sdhmap has 0 capacity");
	if ((uintptr_t)key >= (uintptr_t)(header + 1) &&
		(uintptr_t)key <= (uintptr_t)(((char *)(header + 1)) + slot_size * header->slot_count))
	{
		key_offset = (uintptr_t)key - (uintptr_t)(header + 1);
		if (key_offset % (uintptr_t)slot_size == sizeof(sdhmap_slot))
		{
			i = key_offset / (uintptr_t)slot_size;
			return ((char *)(detail_sdhmap_slot(header, i) + 1)) + key_size;
		}
	}
	return detail_sdhmap_set_stack_impl(header, slot_size, key_size, key, capacity);
}

SDHMAP_API void detail_sdhmap_reserve_heap_impl(
	sdhmap_header **header,
	uint32_t slot_size,
	uint32_t key_size,
	sdhmap_index target)
{
	if (target > (*header)->count)
	{
		detail_sdhmap_heap_resize(header, slot_size, key_size, target);
	}
}

SDHMAP_API void detail_sdhmap_reserve_stack_impl(
	sdhmap_header *header,
	uint32_t slot_size,
	uint32_t key_size,
	sdhmap_index capacity,
	sdhmap_index target)
{
	if (target > header->count)
	{
		detail_sdhmap_stack_resize(
			header, slot_size, key_size, capacity, target);
	}
}

SDHMAP_API void *detail_sdhmap_getp_impl(
	sdhmap_header *header,
	uint32_t slot_size,
	uint32_t key_size,
	const void *key)
{
	sdhmap_index hash;
	sdhmap_slot *slot;
	if (header == NULL ||
		header->slot_count == 0)
	{
		return NULL;
	}
	assert(header->hash_func && "sdhmap hash function is NULL");
	hash = header->hash_func(key) % header->slot_count;
	slot = detail_sdhmap_slot(header, hash);
	if (slot->slot == (sdhmap_index)-1)
	{
		return NULL;
	}
	hash = detail_sdhmap_slot(header, hash)->slot;
	slot = detail_sdhmap_slot(header, hash);
	while (1)
	{
		if (header->eq_func)
		{
			if (header->eq_func(slot + 1, key) == 0)
			{
				return ((char *)(slot + 1)) + key_size;
			}
		}
		else
		{
			if (memcmp(slot + 1, key, key_size) == 0)
			{
				return ((char *)(slot + 1)) + key_size;
			}
		}
		if (slot->next != (sdhmap_index)-1)
		{
			hash = slot->next;
			slot = detail_sdhmap_slot(header, hash);
		}
		else
		{
			return NULL;
		}
	}
}

SDHMAP_API void detail_sdhmap_erase_at(
	sdhmap_header *header,
	uint32_t slot_size,
	sdhmap_index bucket,
	sdhmap_index hash)
{
	sdhmap_slot *h_slot;
	sdhmap_slot *b_slot;
	h_slot = detail_sdhmap_slot(header, hash);
	if (h_slot->prev == (sdhmap_index)-1)
	{
		b_slot = detail_sdhmap_slot(header, bucket);
		if (h_slot->next == (sdhmap_index)-1)
		{
			b_slot->slot = (sdhmap_index)-1;
			header->used_bucket_count --;
		}
		else
		{
			b_slot->slot = h_slot->next;
			detail_sdhmap_slot(header, h_slot->next)->prev = 
				(sdhmap_index)-1;
		}
		detail_sdhmap_slot(header, header->empty_slot)->prev = hash;
		h_slot->prev = (sdhmap_index)-1;
		h_slot->next = header->empty_slot;
		header->empty_slot = hash;
	}
	else
	{
		b_slot = detail_sdhmap_slot(header, h_slot->prev);
		if (h_slot->next == (sdhmap_index)-1)
		{
			b_slot->next = (sdhmap_index)-1;
		}
		else
		{
			b_slot->next = h_slot->next;
			detail_sdhmap_slot(header, h_slot->next)->prev = h_slot->prev;
		}
		detail_sdhmap_slot(header, header->empty_slot)->prev = hash;
		h_slot->prev = (sdhmap_index)-1;
		h_slot->next = header->empty_slot;
		header->empty_slot = hash;
	}
	header->count--;
}

SDHMAP_API void detail_sdhmap_erase_impl(
	sdhmap_header *header,
	uint32_t slot_size,
	uint32_t key_size,
	const void *key)
{
	sdhmap_index bucket;
	sdhmap_index hash;
	sdhmap_slot *slot;
	if (header == NULL ||
		header->slot_count == 0)
	{
		return;
	}
	assert(header->hash_func && "sdhmap hash function is NULL");
	hash = header->hash_func(key) % header->slot_count;
	slot = detail_sdhmap_slot(header, hash);
	if (slot->slot == (sdhmap_index)-1)
	{
		return;
	}
	bucket = hash;
	hash = detail_sdhmap_slot(header, hash)->slot;
	slot = detail_sdhmap_slot(header, hash);
	while (1)
	{
		if (header->eq_func)
		{
			if (header->eq_func(slot + 1, key) == 0)
			{
				detail_sdhmap_erase_at(header, slot_size, bucket, hash);
				return;
			}
		}
		else
		{
			if (memcmp(slot + 1, key, key_size) == 0)
			{
				detail_sdhmap_erase_at(header, slot_size, bucket, hash);
				return;
			}
		}
		if (slot->next != (sdhmap_index)-1)
		{
			hash = slot->next;
			slot = detail_sdhmap_slot(header, hash);
		}
		else
		{
			return;
		}
	}
}

SDHMAP_API void detail_sdhmap_shrink_heap_impl(
	sdhmap_header **header,
	uint32_t slot_size,
	uint32_t key_size)
{
	sdhmap_heap *heap;
	sdhmap_index capacity;
	if (*header == NULL)
	{
		return;
	}
	if ((*header)->slot_count > (*header)->count)
	{
		detail_sdhmap_heap_resize(header, slot_size, key_size, (*header)->count);
		heap = detail_sdhmap_heap_from_header(*header);
		capacity = sizeof(sdhmap_heap) + slot_size * (*header)->slot_count;
		heap = sdhmap_realloc(heap, capacity);
		sdhmap_assert((heap != NULL) && "sdhmap_realloc returned NULL");
		heap->capacity = capacity;
		*header = &heap->header;
	}
}

SDHMAP_API void detail_sdhmap_shrink_stack_impl(
	sdhmap_header *header,
	uint32_t slot_size,
	uint32_t key_size,
	sdhmap_index capacity)
{
	if (header->slot_count > header->count)
	{
		detail_sdhmap_stack_resize(
			header, slot_size, key_size, capacity, header->count);
	}
}

SDHMAP_API void *detail_sdhmap_first_impl(
	sdhmap_header *header,
	uint32_t slot_size)
{
	sdhmap_index hash;
	sdhmap_slot *slot;
	if (header == NULL ||
		header->slot_count == 0)
	{
		return NULL;
	}
	hash = 0;
	while (1)
	{
		slot = detail_sdhmap_slot(header, hash);
		if (slot->slot != (sdhmap_index)-1)
		{
			return detail_sdhmap_slot(header, slot->slot) + 1;
		}
		hash ++;
		if (hash == header->slot_count)
		{
			return NULL;
		}
	}
}

SDHMAP_API void *detail_sdhmap_next_impl(
	sdhmap_header *header,
	uint32_t slot_size,
	uint32_t key_size,
	const void *key)
{
	sdhmap_index index;
	sdhmap_index hash;
	sdhmap_slot *slot;
	if (header == NULL ||
		header->slot_count == 0)
	{
		return NULL;
	}
	assert(header->hash_func && "sdhmap hash function is NULL");
	hash = header->hash_func(key) % header->slot_count;
	slot = detail_sdhmap_slot(header, hash);
	if (slot->slot == (sdhmap_index)-1)
	{
		return NULL;
	}
	index = slot->slot;
	slot = detail_sdhmap_slot(header, index);
	while (1)
	{
		if (header->eq_func)
		{
			if (header->eq_func(slot + 1, key) == 0)
			{
				goto match;
			}
		}
		else
		{
			if (memcmp(slot + 1, key, key_size) == 0)
			{
				goto match;
			}
		}
		if (slot->next != (sdhmap_index)-1)
		{
			index = slot->next;
			slot = detail_sdhmap_slot(header, index);
		}
		else
		{
			return NULL;
		}
	}
	match:;
	if (slot->next != (sdhmap_index)-1)
	{
		return detail_sdhmap_slot(header, slot->next) + 1;
	}
	hash ++;
	if (hash == header->slot_count)
	{
		return NULL;
	}
	while (1)
	{
		slot = detail_sdhmap_slot(header, hash);
		if (slot->slot != (sdhmap_index)-1)
		{
			return detail_sdhmap_slot(header, slot->slot) + 1;
		}
		hash ++;
		if (hash == header->slot_count)
		{
			return NULL;
		}
	}
}

SDHMAP_API void detail_sdhmap_delete_impl(sdhmap_header **header)
{
	if (*header)
	{
		sdhmap_free(detail_sdhmap_heap_from_header(*header));
		*header = NULL;
	}
}

SDHMAP_API void detail_sdhmap_dummy_impl(void)
{

}
