#ifndef SDMAP_SINGLE_H
#define SDMAP_SINGLE_H

#ifndef SDMAP_API
#define SDMAP_API static inline
#endif

#include "sdmap.h"

#define detail_sdmap_slot(map, index) ((sdmap_slot *)((char *)(map) + sizeof(sdmap_header) + index * slot_size))

#define detail_sdmap_value(map, index) ((void *)((char *)(map) + sizeof(sdmap_header) + index * slot_size + sizeof(sdmap_slot) + key_size))

#define detail_sdmap_heap_from_header(h) ((sdmap_heap *)((char *)((void *)(h)) - offsetof(sdmap_heap, header)))

#define detail_sdmap_inorder_body(...)\
	const uint32_t slot_size = sizeof(sdmap_slot) + key_size + value_size;\
	sdmap_slot *current;\
	sdmap_slot *pre;\
	sdmap_index current_index;\
	sdmap_index pre_index;\
	if (header == NULL ||\
		header->root_slot == (sdmap_index)-1)\
	{\
		return;\
	}\
	current_index = header->root_slot;\
	current = detail_sdmap_slot(header, current_index);\
	while (1)\
	{\
		if (current->left == current_index)\
		{\
			function(__VA_ARGS__);\
			if (current->right == current_index)\
			{\
				break;\
			}\
			else\
			{\
				current_index = current->right;\
				current = detail_sdmap_slot(header, current_index);\
			}\
		}\
		else\
		{\
			pre_index = current->left;\
			pre = detail_sdmap_slot(header, pre_index);\
			while (pre->right != pre_index &&\
				pre->right != current_index)\
			{\
				pre_index = pre->right;\
				pre = detail_sdmap_slot(header, pre_index);\
			}\
			if (pre->right == pre_index)\
			{\
				pre->right = current_index;\
				current_index = current->left;\
				current = detail_sdmap_slot(header, current_index);\
			}\
			else\
			{\
				pre->right = pre_index;\
				function(__VA_ARGS__);\
				current_index = current->right;\
				current = detail_sdmap_slot(header, current_index);\
			}\
		}\
	}

#define detail_sdmap_preorder_body(...)\
	const uint32_t slot_size = sizeof(sdmap_slot) + key_size + value_size;\
	sdmap_slot *current;\
	sdmap_slot *pre;\
	sdmap_index current_index;\
	sdmap_index pre_index;\
	if (header == NULL ||\
		header->root_slot == (sdmap_index)-1)\
	{\
		return;\
	}\
	current_index = header->root_slot;\
	current = detail_sdmap_slot(header, current_index);\
	while (1)\
	{\
		if (current->left == current_index)\
		{\
			function(__VA_ARGS__);\
			if (current->right == current_index)\
			{\
				break;\
			}\
			else\
			{\
				current_index = current->right;\
				current = detail_sdmap_slot(header, current_index);\
			}\
		}\
		else\
		{\
			pre_index = current->left;\
			pre = detail_sdmap_slot(header, pre_index);\
			while (pre->right != pre_index &&\
				pre->right != current_index)\
			{\
				pre_index = pre->right;\
				pre = detail_sdmap_slot(header, pre_index);\
			}\
			if (pre->right == current_index)\
			{\
				pre->right = pre_index;\
				current_index = current->right;\
				current = detail_sdmap_slot(header, current_index);\
			}\
			else\
			{\
				function(__VA_ARGS__);\
				pre->right = current_index;\
				current_index = current->left;\
				current = detail_sdmap_slot(header, current_index);\
			}\
		}\
	}

#define detail_sdmap_define_compare_func(type, postfix) \
SDMAP_API int detail_sdmap_compare_##postfix(const void *a, const void *b)\
{\
	if (*((type *)a) == *((type *)b))\
	{\
		return 0;\
	}\
	if (*((type *)a) < *((type *)b))\
	{\
		return -1;\
	}\
	return 1;\
}

detail_sdmap_define_compare_func(uint8_t, uint8_t)
detail_sdmap_define_compare_func(uint16_t, uint16_t)
detail_sdmap_define_compare_func(uint32_t, uint32_t)
detail_sdmap_define_compare_func(uint64_t, uint64_t)
detail_sdmap_define_compare_func(int8_t, int8_t)
detail_sdmap_define_compare_func(int16_t, int16_t)
detail_sdmap_define_compare_func(int32_t, int32_t)
detail_sdmap_define_compare_func(int64_t, int64_t)
detail_sdmap_define_compare_func(float, float)
detail_sdmap_define_compare_func(double, double)
detail_sdmap_define_compare_func(long double, long_double)

SDMAP_API int detail_sdmap_strcmp(const void *a, const void *b)
{
	return strcmp(*((const char **)a), *((const char **)b));
}

SDMAP_API sdmap_index detail_sdmap_count_impl(sdmap_header *header)
{
	if (header)
	{
		return header->count;
	}
	return 0;
}

SDMAP_API sdmap_index detail_sdmap_capacity_impl(sdmap_header *header, uint32_t slot_size)
{
	if (header)
	{
		return detail_sdmap_heap_from_header(header)->capacity / slot_size;
	}
	return 0;
}

SDMAP_API void detail_sdmap_new_heap_impl(sdmap_header **header, sdmap_index capacity, int (*compare_func)(const void *, const void *))
{
	sdmap_heap *heap;
	heap = sdmap_malloc(sizeof(sdmap_heap) + capacity);
	sdmap_assert(heap != NULL && "sdmap_malloc returned NULL");
	detail_sdmap_new_stack_impl(&heap->header, compare_func);
	heap->capacity = capacity;
	*header = &(heap->header);
}

SDMAP_API void detail_sdmap_new_stack_impl(sdmap_header *header, int (*compare_func)(const void *, const void *))
{
	header->count = 0;
	header->slot_count = 0;
	header->root_slot = (sdmap_index)-1;
	header->empty_slot = (sdmap_index)-1;
	header->compare_func = compare_func;
}

SDMAP_API sdmap_header **detail_sdmap_ensure_initialized_impl(sdmap_header **header, sdmap_index capacity, int (*compare_func)(const void *, const void *))
{
	if (*header == NULL)
	{
		detail_sdmap_new_heap_impl(header, capacity, compare_func);
	}
	else
	{
		(*header)->compare_func = compare_func;
	}
	return header;
}

SDMAP_API int detail_sdmap_contains_impl(sdmap_header *header, uint32_t key_size, uint32_t value_size, const char *key)
{
	const uint32_t slot_size = sizeof(sdmap_slot) + key_size + value_size;
	int compare_result;
	sdmap_index slot_index;
	sdmap_slot *slot;
	/*Check if we have an empty map*/
	if (header == NULL ||
		header->count == 0)
	{
		return 0;
	}
	/*Check if key points to inside the object*/
	if ((uintptr_t)key > (uintptr_t)(header + 1) && 
		(uintptr_t)key < (uintptr_t)(((char *)header) + slot_size * header->slot_count))
	{
		if (((uintptr_t)key % (uintptr_t)slot_size) == sizeof(sdmap_slot))
		{
			slot_index = ((uintptr_t)key - (uintptr_t)(header + 1)) / (uintptr_t)slot_size;
			slot = detail_sdmap_slot(header, slot_index);
			if (slot->height >= 0)
			{
				return 1;
			}
		}
	}
	/*Check if we have a compare function*/
	sdmap_assert(header->compare_func != NULL && "sdmap does not have a compare function");
	/*Walk the tree*/
	slot_index = header->root_slot;
	while (1)
	{
		slot = detail_sdmap_slot(header, slot_index);
		compare_result = header->compare_func(slot + 1, key);
		if (compare_result == 0)
		{
			/*Bingo*/
			return 1;
		}
		else if (compare_result > 0)
		{
			/*The element that we are looking for is to our left*/
			if (slot->left == slot_index)
			{
				/*But our slot doesn't have a left branch*/
				return 0;
			}
			else
			{
				slot_index = slot->left;
			}
		}
		else
		{
			/*The element that we are looking for is to our right*/
			if (slot->right == slot_index)
			{
				/*But our slot doesn't have a right branch*/
				return 0;
			}
			else
			{
				slot_index = slot->right;
			}
		}
	}
}

SDMAP_API sdmap_index detail_sdmap_insert_heap_impl(sdmap_header **header, uint32_t key_size, uint32_t value_size, const char *key)
{
	const uint32_t slot_size = sizeof(sdmap_slot) + key_size + value_size;
	sdmap_heap *heap;
	sdmap_index new_capacity;
	sdmap_index slot_index;
	sdmap_slot *slot;
	if ((*header)->empty_slot != (sdmap_index)-1)
	{
		/*We have an empty slot, hooray*/
		slot_index = (*header)->empty_slot;
		slot = detail_sdmap_slot(*header, slot_index);
		(*header)->empty_slot = slot->right;
	}
	else
	{
		(*header)->slot_count ++;
		heap = detail_sdmap_heap_from_header(*header);
		new_capacity = heap->capacity > 0 ? heap->capacity : slot_size * SDMAP_DEFAULT_CAPACITY;
		while (new_capacity < slot_size * heap->header.slot_count)
		{
			new_capacity *= 2;
		}
		if (new_capacity > heap->capacity)
		{
			heap = sdmap_realloc(heap, sizeof(sdmap_heap) + new_capacity);
			sdmap_assert(heap != NULL && "sdmap_realloc returned NULL");
			heap->capacity = new_capacity;
			*header = &(heap->header);
		}
		slot_index = (*header)->count;
		slot = detail_sdmap_slot(*header, slot_index);
	}

	slot->left = slot_index;
	slot->right = slot_index;
	slot->height = 0;
	memcpy(slot + 1, key, key_size);
	(*header)->count ++;
	return slot_index;
}

SDMAP_API sdmap_index detail_sdmap_insert_stack_impl(sdmap_header *header, uint32_t key_size, uint32_t value_size, const char *key, sdmap_index capacity)
{
	const uint32_t slot_size = sizeof(sdmap_slot) + key_size + value_size;
	sdmap_index slot_index;
	sdmap_slot *slot;
	if (header->empty_slot != (sdmap_index)-1)
	{
		/*We have an empty slot, hooray*/
		slot_index = header->empty_slot;
		slot = detail_sdmap_slot(header, slot_index);
		header->empty_slot = slot->right;
	}
	else
	{
		header->slot_count ++;
		sdmap_assert(slot_size * header->slot_count <= capacity && "sdmap_stack capacity exceeded");
		slot_index = header->count;
		slot = detail_sdmap_slot(header, slot_index);
	}
	slot->left = slot_index;
	slot->right = slot_index;
	slot->height = 0;
	memcpy(slot + 1, key, key_size);
	header->count ++;
	return slot_index;
}

SDMAP_API void detail_sdmap_compute_height(sdmap_header *header, uint32_t slot_size, sdmap_index node)
{
	uint8_t temp;
	sdmap_slot *slot = detail_sdmap_slot(header, node);
	slot->height = -1;
	if (slot->left != node)
	{
		temp = detail_sdmap_slot(header, slot->left)->height;
		if (slot->height < temp)
		{
			slot->height = temp;
		}
	}
	if (slot->right != node)
	{
		temp = detail_sdmap_slot(header, slot->right)->height;
		if (slot->height < temp)
		{
			slot->height = temp;
		}
	}
	slot->height++;
}

SDMAP_API int detail_sdmap_compute_balance(sdmap_header *header, uint32_t slot_size, sdmap_index node)
{
	int result = 0;
	sdmap_slot *slot = detail_sdmap_slot(header, node);
	if (slot->left != node)
	{
		result -= 1 + detail_sdmap_slot(header, slot->left)->height;
	}
	if (slot->right != node)
	{
		result += 1 + detail_sdmap_slot(header, slot->right)->height;
	}
	return result;
}

SDMAP_API sdmap_index detail_sdmap_taller_child(sdmap_header *header, uint32_t slot_size, sdmap_index node)
{
	sdmap_slot *slot = detail_sdmap_slot(header, node);
	if (slot->left != node &&
		slot->right == node)
	{
		return slot->left;
	}
	if (slot->right != node &&
		slot->left == node)
	{
		return slot->right;
	}
	if (slot->right == node ||
		slot->left == node)
	{
		return node;
	}
	if (detail_sdmap_slot(header, slot->right)->height > 
		detail_sdmap_slot(header, slot->left)->height)
	{
		return slot->right;
	}
	else
	{
		return slot->left;
	}
}

SDMAP_API void detail_sdmap_rotate_l_impl(sdmap_header *header, uint32_t slot_size, sdmap_index node)
{
	sdmap_slot *p = detail_sdmap_slot(header, node);
	sdmap_index q_index = p->right;
	sdmap_assert(q_index != node && "Parameter of left rotation does not have a right child.");
	sdmap_slot *q = detail_sdmap_slot(header, q_index);
	if (q->left == q_index)
	{
		p->right = node;
	}
	else
	{
		detail_sdmap_slot(header, q->left)->parent = node;
		p->right = q->left;
	}
	q->left = node;
	detail_sdmap_compute_height(header, slot_size, node);
	detail_sdmap_compute_height(header, slot_size, q_index);
	q->parent = p->parent;
	p->parent = q_index;
	if (node == header->root_slot)
	{
		header->root_slot = q_index;
	}
	else
	{
		q = detail_sdmap_slot(header, q->parent);
		if (q->right == node)
		{
			q->right = q_index;
		}
		else
		{
			q->left = q_index;
		}
	}
}

SDMAP_API void detail_sdmap_rotate_r_impl(sdmap_header *header, uint32_t slot_size, sdmap_index node)
{
	sdmap_slot *q = detail_sdmap_slot(header, node);
	sdmap_index p_index = q->left;
	sdmap_assert(p_index != node && "Parameter of right rotation does not have a left child.");
	sdmap_slot *p = detail_sdmap_slot(header, p_index);
	if (p->right == p_index)
	{
		q->left = node;
	}
	else
	{
		detail_sdmap_slot(header, p->right)->parent = node;
		q->left = p->right;
	}
	p->right = node;
	detail_sdmap_compute_height(header, slot_size, node);
	detail_sdmap_compute_height(header, slot_size, p_index);
	p->parent = q->parent;
	q->parent = p_index;
	if (node == header->root_slot)
	{
		header->root_slot = p_index;
	}
	else
	{
		p = detail_sdmap_slot(header, p->parent);
		if (p->right == node)
		{
			p->right = p_index;
		}
		else
		{
			p->left = p_index;
		}
	}
}

SDMAP_API void detail_sdmap_insert_rotate(sdmap_header *header, uint32_t slot_size, sdmap_index at)
{
	int balance;
	int balance_child;
	sdmap_index parent;
	parent = at;
	while (at != header->root_slot)
	{
		parent = detail_sdmap_slot(header, at)->parent;
		detail_sdmap_compute_height(header, slot_size, parent);
		detail_sdmap_compute_height(header, slot_size, at);
		balance = detail_sdmap_compute_balance(header, slot_size, parent);
		balance_child = detail_sdmap_compute_balance(header, slot_size, at);
		if (balance > 1)
		{
			if (balance_child >= 0)
			{
				detail_sdmap_rotate_l_impl(header, slot_size, parent);
			}
			else
			{
				detail_sdmap_rotate_r_impl(header, slot_size, at);
				detail_sdmap_rotate_l_impl(header, slot_size, parent);
			}
			break;
		}
		else if (balance < -1)
		{
			if (balance_child > 0)
			{
				detail_sdmap_rotate_l_impl(header, slot_size, at);
				detail_sdmap_rotate_r_impl(header, slot_size, parent);
			}
			else
			{
				detail_sdmap_rotate_r_impl(header, slot_size, parent);
			}
			break;
		}
		at = parent;
	}
	while (at != header->root_slot)
	{
		detail_sdmap_compute_height(header, slot_size, at);
		at = detail_sdmap_slot(header, at)->parent;
	}
}

SDMAP_API void detail_sdmap_erase_rotate(sdmap_header *header, uint32_t slot_size, sdmap_index z)
{
	int balance;
	int balance_child;
	sdmap_index x;
	sdmap_index y;
	while (z != header->root_slot)
	{
		z = detail_sdmap_slot(header, z)->parent;
		if (z == (sdmap_index)-1)
		{
			break;
		}
		y = detail_sdmap_taller_child(header, slot_size, z);
		x = detail_sdmap_taller_child(header, slot_size, y);
		detail_sdmap_compute_height(header, slot_size, z);
		if (z != y && y != x)
		{
			balance = detail_sdmap_compute_balance(header, slot_size, z);
			balance_child = detail_sdmap_compute_balance(header, slot_size, y);
			if (balance > 1)
			{
				if (balance_child >= 0)
				{
					detail_sdmap_rotate_l_impl(header, slot_size, z);
				}
				else
				{
					detail_sdmap_rotate_r_impl(header, slot_size, y);
					detail_sdmap_rotate_l_impl(header, slot_size, z);
				}
				detail_sdmap_compute_height(header, slot_size, y);
			}
			else if (balance < -1)
			{
				if (balance_child > 0)
				{
					detail_sdmap_rotate_l_impl(header, slot_size, y);
					detail_sdmap_rotate_r_impl(header, slot_size, z);
				}
				else
				{
					detail_sdmap_rotate_r_impl(header, slot_size, z);
				}
				detail_sdmap_compute_height(header, slot_size, y);
			}
		}
	}
}


SDMAP_API void detail_sdmap_optimize_empty_slots(sdmap_header *header, uint32_t slot_size)
{
	sdmap_index i;
	sdmap_slot *slot;
	sdmap_slot *parent;
	if (header->empty_slot == (sdmap_index)-1)
	{
		return;
	}
	parent = NULL;
	for (i = 0; i < header->slot_count; i++)
	{
		slot = detail_sdmap_slot(header, i);
		if (slot->height >= 0)
		{
			continue;
		}
		if (parent != NULL)
		{
			parent->right = i;
		}
		else
		{
			header->empty_slot = i;
		}
		parent = slot;
	}
	if (parent != NULL)
	{
		parent->right = (sdmap_index)-1;
	}
}

SDMAP_API void detail_sdmap_optimize_reduce_slots(sdmap_header *header, uint32_t slot_size)
{
	sdmap_index i;
	sdmap_slot *slot;
	sdmap_slot *parent;
	for (i = header->count; i < header->slot_count; i++)
	{
		slot = detail_sdmap_slot(header, i);
		if (slot->height < 0)
		{
			continue;
		}
		if (header->root_slot == i)
		{
			header->root_slot = header->empty_slot;
		}
		else
		{
			parent = detail_sdmap_slot(header, slot->parent);
			if (parent->left == i)
			{
				parent->left = header->empty_slot;
			}
			else
			{
				parent->right = header->empty_slot;
			}
		}
		if (slot->right != i)
		{
			detail_sdmap_slot(header, slot->right)->parent = header->empty_slot;
		}
		else
		{
			slot->right = header->empty_slot;
		}
		if (slot->left != i)
		{
			detail_sdmap_slot(header, slot->left)->parent = header->empty_slot;
		}
		else
		{
			slot->left = header->empty_slot;
		}
		parent = detail_sdmap_slot(header, header->empty_slot);
		header->empty_slot = parent->right;
		memcpy(parent, slot, slot_size);
	}
	header->slot_count = header->count;
	header->empty_slot = (sdmap_index)-1;
}

SDMAP_API void detail_sdmap_optimize_impl(sdmap_header *header, uint32_t slot_size)
{
	detail_sdmap_optimize_empty_slots(header, slot_size);
	detail_sdmap_optimize_reduce_slots(header, slot_size);
}

SDMAP_API void detail_sdmap_shrink_impl(sdmap_header **header, uint32_t slot_size)
{
	sdmap_heap *heap;
	if (*header == NULL)
	{
		return;
	}
	detail_sdmap_optimize_impl(*header, slot_size);
	heap = detail_sdmap_heap_from_header(*header);
	if (heap->capacity > slot_size * (*header)->slot_count)
	{
		heap->capacity = slot_size * (*header)->slot_count;
		heap = sdmap_realloc(heap, sizeof(sdmap_heap) + heap->capacity);
		sdmap_assert(heap != NULL && "sdmap_realloc returned NULL");
		*header = &(heap->header);
	}
}

SDMAP_API void *detail_sdmap_get_min_impl(sdmap_header *header, uint32_t key_size, uint32_t value_size)
{
	const uint32_t slot_size = sizeof(sdmap_slot) + key_size + value_size;
	sdmap_index index;
	sdmap_slot *slot;
	if (header == NULL ||
		header->count == 0)
	{
		return NULL;
	}
	index = header->root_slot;
	while (1)
	{
		slot = detail_sdmap_slot(header, index);
		if (slot->left == index)
		{
			return (char *)(slot + 1) + key_size;
		}
		else
		{
			index = slot->left;
		}
	}
}

SDMAP_API void *detail_sdmap_get_max_impl(sdmap_header *header, uint32_t key_size, uint32_t value_size)
{
	const uint32_t slot_size = sizeof(sdmap_slot) + key_size + value_size;
	sdmap_index index;
	sdmap_slot *slot;
	if (header == NULL ||
		header->count == 0)
	{
		return NULL;
	}
	index = header->root_slot;
	while (1)
	{
		slot = detail_sdmap_slot(header, index);
		if (slot->right == index)
		{
			return (char *)(slot + 1) + key_size;
		}
		else
		{
			index = slot->right;
		}
	}
}

SDMAP_API void *detail_sdmap_get_root_impl(sdmap_header *header, uint32_t key_size, uint32_t value_size)
{
	const uint32_t slot_size = sizeof(sdmap_slot) + key_size + value_size;
	if (header == NULL ||
		header->count == 0)
	{
		return NULL;
	}
	return (char *)(detail_sdmap_slot(header, header->root_slot) + 1) + key_size;
}

SDMAP_API void *detail_sdmap_get_min_key_impl(sdmap_header *header, uint32_t key_size, uint32_t value_size)
{
	const uint32_t slot_size = sizeof(sdmap_slot) + key_size + value_size;
	sdmap_index index;
	sdmap_slot *slot;
	if (header == NULL ||
		header->count == 0)
	{
		return NULL;
	}
	index = header->root_slot;
	while (1)
	{
		slot = detail_sdmap_slot(header, index);
		if (slot->left == index)
		{
			return (char *)(slot + 1);
		}
		else
		{
			index = slot->left;
		}
	}
}

SDMAP_API void *detail_sdmap_get_max_key_impl(sdmap_header *header, uint32_t key_size, uint32_t value_size)
{
	const uint32_t slot_size = sizeof(sdmap_slot) + key_size + value_size;
	sdmap_index index;
	sdmap_slot *slot;
	if (header == NULL ||
		header->count == 0)
	{
		return NULL;
	}
	index = header->root_slot;
	while (1)
	{
		slot = detail_sdmap_slot(header, index);
		if (slot->right == index)
		{
			return (char *)(slot + 1);
		}
		else
		{
			index = slot->right;
		}
	}
}

SDMAP_API void *detail_sdmap_get_root_key_impl(sdmap_header *header, uint32_t key_size, uint32_t value_size)
{
	const uint32_t slot_size = sizeof(sdmap_slot) + key_size + value_size;
	if (header == NULL ||
		header->count == 0)
	{
		return NULL;
	}
	return (char *)(detail_sdmap_slot(header, header->root_slot) + 1);
}

SDMAP_API sdmap_index detail_sdmap_get_min_in_subtree(sdmap_header *header, uint32_t slot_size, uint32_t slot_index)
{
	sdmap_slot *slot;
	while (1)
	{
		slot = detail_sdmap_slot(header, slot_index);
		if (slot->left == slot_index)
		{
			return slot_index;
		}
		slot_index = slot->left;
	}
}

SDMAP_API sdmap_index detail_sdmap_get_max_in_subtree(sdmap_header *header, uint32_t slot_size, uint32_t slot_index)
{
	sdmap_slot *slot;
	while (1)
	{
		slot = detail_sdmap_slot(header, slot_index);
		if (slot->right == slot_index)
		{
			return slot_index;
		}
		slot_index = slot->right;
	}
}

SDMAP_API sdmap_index detail_sdmap_get_left_parent(sdmap_header *header, uint32_t slot_size, uint32_t slot_index)
{
	sdmap_slot *slot;
	while (1)
	{
		slot = detail_sdmap_slot(header, slot_index);
		if (slot->parent == (sdmap_index)-1)
		{
			return (sdmap_index)-1;
		}
		if (detail_sdmap_slot(header, slot->parent)->left == slot_index)
		{
			return slot->parent;
		}
		slot_index = slot->parent;
	}
}

SDMAP_API sdmap_index detail_sdmap_get_right_parent(sdmap_header *header, uint32_t slot_size, uint32_t slot_index)
{
	sdmap_slot *slot;
	while (1)
	{
		slot = detail_sdmap_slot(header, slot_index);
		if (slot->parent == (sdmap_index)-1)
		{
			return (sdmap_index)-1;
		}
		if (detail_sdmap_slot(header, slot->parent)->right == slot_index)
		{
			return slot->parent;
		}
		slot_index = slot->parent;
	}
}

SDMAP_API void *detail_sdmap_get_next_impl(sdmap_header *header, uint32_t key_size, uint32_t value_size, const char *key)
{
	const uint32_t slot_size = sizeof(sdmap_slot) + key_size + value_size;
	int compare_result;
	sdmap_index slot_index;
	sdmap_slot *slot;
	if (header == NULL ||
		header->count == 0 ||
		header->compare_func == NULL)
	{
		return NULL;
	}
	if ((uintptr_t)key > (uintptr_t)(header + 1) && 
		(uintptr_t)key < (uintptr_t)(((char *)header) + slot_size * header->slot_count))
	{
		if (((uintptr_t)key % (uintptr_t)slot_size) == sizeof(sdmap_slot))
		{
			slot_index = ((uintptr_t)key - (uintptr_t)(header + 1)) / (uintptr_t)slot_size;
			slot = detail_sdmap_slot(header, slot_index);
			if (slot->height >= 0)
			{
				goto found;
			}
		}
	}
	slot_index = header->root_slot;
	while (1)
	{
		slot = detail_sdmap_slot(header, slot_index);
		compare_result = header->compare_func(slot + 1, key);
		if (compare_result == 0)
		{
			found:;
			if (slot->right != slot_index)
			{
				slot_index = detail_sdmap_get_min_in_subtree(header, slot_size, slot_index);
				return detail_sdmap_value(header, slot_index);
			}
			slot_index = detail_sdmap_get_left_parent(header, slot_size, slot_index);
			if (slot_index != (sdmap_index)-1)
			{
				return detail_sdmap_value(header, slot_index);
			}
			return NULL;
		}
		else if (compare_result > 0)
		{
			slot_index = slot->left;
		}
		else
		{
			slot_index = slot->right;
		}
	}
	return NULL;
}

SDMAP_API void *detail_sdmap_get_next_key_impl(sdmap_header *header, uint32_t key_size, uint32_t value_size, const char *key)
{
	char *result = detail_sdmap_get_next_impl(header, key_size, value_size, key);
	if (result)
	{
		return result - value_size;
	}
	return NULL;
}

SDMAP_API void *detail_sdmap_get_prev_impl(sdmap_header *header, uint32_t key_size, uint32_t value_size, const char *key)
{
	const uint32_t slot_size = sizeof(sdmap_slot) + key_size + value_size;
	int compare_result;
	sdmap_index slot_index;
	sdmap_slot *slot;
	if (header == NULL ||
		header->count == 0 ||
		header->compare_func == NULL)
	{
		return NULL;
	}
	if ((uintptr_t)key > (uintptr_t)(header + 1) && 
		(uintptr_t)key < (uintptr_t)(((char *)header) + slot_size * header->slot_count))
	{
		if (((uintptr_t)key % (uintptr_t)slot_size) == sizeof(sdmap_slot))
		{
			slot_index = ((uintptr_t)key - (uintptr_t)(header + 1)) / (uintptr_t)slot_size;
			slot = detail_sdmap_slot(header, slot_index);
			if (slot->height >= 0)
			{
				goto found;
			}
		}
	}
	slot_index = header->root_slot;
	while (1)
	{
		slot = detail_sdmap_slot(header, slot_index);
		compare_result = header->compare_func(slot + 1, key);
		if (compare_result == 0)
		{
			found:;
			if (slot->left != slot_index)
			{
				slot_index = detail_sdmap_get_max_in_subtree(header, slot_size, slot_index);
				return detail_sdmap_value(header, slot_index);
			}
			slot_index = detail_sdmap_get_right_parent(header, slot_size, slot_index);
			if (slot_index != (sdmap_index)-1)
			{
				return detail_sdmap_value(header, slot_index);
			}
			return NULL;
		}
		else if (compare_result > 0)
		{
			slot_index = slot->left;
		}
		else
		{
			slot_index = slot->right;
		}
	}
	return NULL;
}

SDMAP_API void *detail_sdmap_get_prev_key_impl(sdmap_header *header, uint32_t key_size, uint32_t value_size, const char *key)
{
	char *result = detail_sdmap_get_prev_impl(header, key_size, value_size, key);
	if (result)
	{
		return result - value_size;
	}
	return NULL;
}

SDMAP_API void *detail_sdmap_set_heap_empty(sdmap_header **header, uint32_t key_size, uint32_t value_size, const char *key)
{
	const uint32_t slot_size = sizeof(sdmap_slot) + key_size + value_size;
	sdmap_heap *heap;
	sdmap_slot *slot;
	sdmap_index new_capacity;
	heap = detail_sdmap_heap_from_header(*header);
	new_capacity = heap->capacity > 0 ? heap->capacity : (sizeof(sdmap_slot) + key_size + value_size) * SDMAP_DEFAULT_CAPACITY;
	while (new_capacity < slot_size * ((*header)->slot_count + 1))
	{
		new_capacity *= 2;
	}
	if (new_capacity > heap->capacity)
	{
		heap = sdmap_realloc(heap, sizeof(sdmap_heap) + new_capacity);
		sdmap_assert(heap != NULL && "sdmap_realloc returned NULL");
		heap->capacity = new_capacity;
		*header = &heap->header;
	}
	slot = detail_sdmap_slot((*header), 0);
	slot->left = 0;
	slot->right = 0;
	slot->height = 0;
	slot->parent = (sdmap_index)-1;
	memcpy(((char *)(slot + 1)), key, key_size);
	(*header)->root_slot = 0;
	(*header)->count ++;
	(*header)->slot_count ++;
	(*header)->empty_slot = (sdmap_index)-1;
	return ((char *)(slot + 1)) + key_size;
}

SDMAP_API void *detail_sdmap_set_heap_impl(sdmap_header **header, uint32_t key_size, uint32_t value_size, const char *key)
{
	const uint32_t slot_size = sizeof(sdmap_slot) + key_size + value_size;
	int compare_result;
	sdmap_index new_index;
	sdmap_index slot_index;
	sdmap_slot *slot;

	/*Check if we don't have a compare function*/
	sdmap_assert((*header)->compare_func != NULL && "sdmap does not have a compare function");

	if ((*header)->count == 0 &&
		(*header)->slot_count == 0)
	{
		return detail_sdmap_set_heap_empty(header, key_size, value_size, key);
	}
	slot_index = (*header)->root_slot;
	while (1)
	{
		slot = detail_sdmap_slot((*header), slot_index);
		compare_result = (*header)->compare_func(slot + 1, key);
		if (compare_result == 0)
		{
			return detail_sdmap_value((*header), slot_index);
		}
		else if (
			(compare_result > 0 && slot->left == slot_index) ||
			(compare_result < 0 && slot->right == slot_index))
		{
			new_index = detail_sdmap_insert_heap_impl(header, key_size, value_size, key);
			if (compare_result > 0)
			{
				detail_sdmap_slot((*header), slot_index)->left = new_index;
			}
			else
			{
				detail_sdmap_slot((*header), slot_index)->right = new_index;
			}
			detail_sdmap_slot((*header), new_index)->parent = slot_index;
			detail_sdmap_insert_rotate(*header, slot_size, slot_index);
			return ((char *)detail_sdmap_slot((*header), new_index)) + sizeof(sdmap_slot) + key_size;
		}
		else if (compare_result > 0)
		{
			slot_index = slot->left;
		}
		else
		{
			slot_index = slot->right;
		}
	}
	return NULL;
}

SDMAP_API void *detail_sdmap_set_stack_impl(sdmap_header *header, uint32_t key_size, uint32_t value_size, const char *key, sdmap_index capacity)
{
	const uint32_t slot_size = sizeof(sdmap_slot) + key_size + value_size;
	int compare_result;
	sdmap_index new_index;
	sdmap_index slot_index;
	sdmap_slot *slot;

	/*Check if we don't have a compare function*/
	sdmap_assert(header->compare_func != NULL && "sdmap does not have a compare function");

	/*Check if we have an empty map*/
	if (header->count == 0 &&
		header->slot_count == 0)
	{
		sdmap_assert(slot_size <= capacity && "sdmap_stack capacity exceeded");
		slot = detail_sdmap_slot(header, 0);
		slot->left = 0;
		slot->right = 0;
		slot->height = 0;
		memcpy(((char *)(slot + 1)), key, key_size);
		header->count ++;
		header->slot_count ++;
		header->empty_slot = (sdmap_index)-1;
		return ((char *)(slot + 1)) + key_size;
	}
	slot_index = header->root_slot;
	while (1)
	{
		slot = detail_sdmap_slot(header, slot_index);
		compare_result = header->compare_func(slot + 1, key);
		if (compare_result == 0)
		{
			return detail_sdmap_value(header, slot_index);
		}
		else if (
			(compare_result > 0 && slot->left == slot_index) ||
			(compare_result < 0 && slot->right == slot_index))
		{
			new_index = detail_sdmap_insert_stack_impl(header, key_size, value_size, key, capacity);
			if (compare_result > 0)
			{
				detail_sdmap_slot(header, slot_index)->left = new_index;
			}
			else
			{
				detail_sdmap_slot(header, slot_index)->right = new_index;
			}
			detail_sdmap_slot(header, new_index)->parent = slot_index;
			detail_sdmap_insert_rotate(header, slot_size, new_index);
			return ((char *)detail_sdmap_slot(header, new_index)) + sizeof(sdmap_slot) + key_size;
		}
		else if (compare_result > 0)
		{
			slot_index = slot->left;
		}
		else
		{
			slot_index = slot->right;
		}
	}
	return NULL;
}

SDMAP_API void detail_sdmap_erase_impl(sdmap_header *header, uint32_t key_size, uint32_t value_size, const char *key)
{
	const uint32_t slot_size = sizeof(sdmap_slot) + key_size + value_size;
	int compare_result;
	sdmap_index slot_index;
	sdmap_slot *slot;
	sdmap_slot *slot_original;
	if (header == NULL ||
		header->count == 0)
	{
		return;
	}
	sdmap_assert(header->compare_func != NULL && "sdmap does not have a compare function");
	slot_index = header->root_slot;
	while (1)
	{
		slot = detail_sdmap_slot(header, slot_index);
		compare_result = header->compare_func(slot + 1, key);
		if (compare_result == 0)
		{
			/*We found our element we are deleting.*/

			/*Take note of where we are going to have our empty slot later on.*/
			slot_original = slot;

			/*If the node we are deletinging has two children*/
			if (slot->left != slot_index &&
				slot->right != slot_index)
			{
				/*Our target node has two children.*/
				slot_index = detail_sdmap_get_min_in_subtree(header, slot_size, slot->right);
				/*Copy key and value from successor.*/
				memcpy(slot + 1, detail_sdmap_slot(header, slot_index) + 1, key_size + value_size);

				slot_original = detail_sdmap_slot(header, slot_index);
				slot = detail_sdmap_slot(header, slot_original->parent);

				if (slot_original->right != slot_index)
				{
					/*In case the successor is not a leaf.*/
					/*This case is a little bit of mind bender. The successor may only have a right child because if it*/
					/*had a left child, then the left child would be the successor.*/
					if (slot->left == slot_index)
					{
						slot->left = slot_original->right;
						detail_sdmap_slot(header, slot_original->right)->parent = slot_original->parent;
					}
					else
					{
						slot->right = slot_original->right;
						detail_sdmap_slot(header, slot_original->right)->parent = slot_original->parent;
					}
				}
				else
				{
					/*In case the successor is a leaf. Reassign pointers.*/
					if (slot->left == slot_index)
					{
						slot->left = slot_original->parent;
					}
					else
					{
						slot->right = slot_original->parent;
					}
				}
			}
			else if (slot->parent == (sdmap_index)-1)
			{
				/*We are deleting the root node.*/
				if (slot->left != slot_index)
				{
					/*Node has one child on the left.*/
					header->root_slot = slot->left;
					detail_sdmap_slot(header, slot->left)->parent = (sdmap_index)-1;
				}
				else if (slot->right != slot_index)
				{
					/*Node has one child on the right.*/
					header->root_slot = slot->right;
					detail_sdmap_slot(header, slot->right)->parent = (sdmap_index)-1;
				}
				else
				{
					/*If the root node has no children then we just need to update the root slot.*/
					header->root_slot = (sdmap_index)-1;
				}
			}
			else
			{
				/*The node we are looking at has up to one child, reassign pointers.*/
				slot = detail_sdmap_slot(header, slot_original->parent);
				if (slot_original->left != slot_index)
				{
					/*The child is on the left.*/
					if (slot->left == slot_index)
					{
						slot->left = slot_original->left;
						detail_sdmap_slot(header, slot->left)->parent = slot_original->parent;
					}
					else
					{
						slot->right = slot_original->left;
						detail_sdmap_slot(header, slot->right)->parent = slot_original->parent;
					}
				}
				else if (slot_original->right != slot_index)
				{
					/*The child is on the right.*/
					if (slot->left == slot_index)
					{
						slot->left = slot_original->right;
						detail_sdmap_slot(header, slot->left)->parent = slot_original->parent;
					}
					else
					{
						slot->right = slot_original->right;
						detail_sdmap_slot(header, slot->right)->parent = slot_original->parent;
					}
				}
				else
				{
					/*The case where our node has two children is above in the first if block*/
					/*The node we are looking at has no children and is not the root node*/
					if (slot->left == slot_index)
					{
						slot->left = slot_original->parent;
					}
					else
					{
						slot->right = slot_original->parent;
					}
				}
			}

			/*We may need to do several rotations to rebalance the tree.*/
			detail_sdmap_erase_rotate(header, slot_size, slot_index);

			/*Pop our next empty slot and adjust element count.*/
			if (header->empty_slot != (sdmap_index)-1 &&
				slot_index > header->empty_slot)
			{
				slot_original->right = detail_sdmap_slot(header, header->empty_slot)->right;
				slot_original->height = -1;
				detail_sdmap_slot(header, header->empty_slot)->right = slot_index;
			} 
			else
			{
				slot_original->right = header->empty_slot;
				slot_original->height = -1;
				header->empty_slot = slot_index;
			}
			header->count--;
			return;
		}
		else if (compare_result > 0)
		{
			/*Keep searching.*/
			if (slot->left == slot_index)
			{
				/*If the element is not present, then give up.*/
				return;
			}
			else
			{
				slot_index = slot->left;
			}
		}
		else
		{
			/*Keep searching.*/
			if (slot->right == slot_index)
			{
				/*If the element is not present, then give up.*/
				return;
			}
			else
			{
				slot_index = slot->right;
			}
		}
	}
}

SDMAP_API void detail_sdmap_delete_impl(sdmap_header **header)
{
	if (*header != NULL)
	{
		sdmap_free(detail_sdmap_heap_from_header(*header));
		*header = NULL;
	}
}

SDMAP_API void detail_sdmap_dummy(void)
{

}

SDMAP_API void detail_sdmap_traverse_inorder_keys_impl(sdmap_header *header, uint32_t key_size, uint32_t value_size, void (*function)(const void *key))
{
	detail_sdmap_inorder_body(current + 1)
}

SDMAP_API void detail_sdmap_traverse_preorder_keys_impl(sdmap_header *header, uint32_t key_size, uint32_t value_size, void (*function)(const void *key))
{
	detail_sdmap_preorder_body(current + 1)
}

SDMAP_API void detail_sdmap_traverse_inorder_values_impl(sdmap_header *header, uint32_t key_size, uint32_t value_size, void (*function)(void *value))
{
	detail_sdmap_inorder_body(((char *)(current + 1)) + key_size)
}

SDMAP_API void detail_sdmap_traverse_preorder_values_impl(sdmap_header *header, uint32_t key_size, uint32_t value_size, void (*function)(void *value))
{
	detail_sdmap_inorder_body(((char *)(current + 1)) + key_size)
}

SDMAP_API void detail_sdmap_traverse_inorder_pairs_impl(sdmap_header *header, uint32_t key_size, uint32_t value_size, void (*function)(const void *key, void *value))
{
	detail_sdmap_inorder_body(current + 1, ((char *)(current + 1)) + key_size)
}

SDMAP_API void detail_sdmap_traverse_preorder_pairs_impl(sdmap_header *header, uint32_t key_size, uint32_t value_size, void (*function)(const void *key, void *value))
{
	detail_sdmap_inorder_body(current + 1, ((char *)(current + 1)) + key_size)
}

SDMAP_API void detail_sdmap_traverse_inorder_keys_ex_impl(sdmap_header *header, uint32_t key_size, uint32_t value_size, void (*function)(const void *key, void *user), void *user)
{
	detail_sdmap_inorder_body(current + 1, user)
}

SDMAP_API void detail_sdmap_traverse_preorder_keys_ex_impl(sdmap_header *header, uint32_t key_size, uint32_t value_size, void (*function)(const void *key, void *user), void *user)
{
	detail_sdmap_preorder_body(current + 1, user)
}

SDMAP_API void detail_sdmap_traverse_inorder_values_ex_impl(sdmap_header *header, uint32_t key_size, uint32_t value_size, void (*function)(void *value, void *user), void *user)
{
	detail_sdmap_inorder_body(((char *)current + 1) + key_size, user)
}

SDMAP_API void detail_sdmap_traverse_preorder_values_ex_impl(sdmap_header *header, uint32_t key_size, uint32_t value_size, void (*function)(void *value, void *user), void *user)
{
	detail_sdmap_inorder_body(((char *)current + 1) + key_size, user)
}

SDMAP_API void detail_sdmap_traverse_inorder_pairs_ex_impl(sdmap_header *header, uint32_t key_size, uint32_t value_size, void (*function)(const void *key, void *value, void *user), void *user)
{
	detail_sdmap_inorder_body(current + 1, ((char *)current + 1) + key_size, user)
}

SDMAP_API void detail_sdmap_traverse_preorder_pairs_ex_impl(sdmap_header *header, uint32_t key_size, uint32_t value_size, void (*function)(const void *key, void *value, void *user), void *user)
{
	detail_sdmap_inorder_body(current + 1, ((char *)current + 1) + key_size, user)
}

#endif