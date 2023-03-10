/**
 * @file sdmap_debug.h Debug functionality for sdmap.h
 * @date       28. Nov 2022
 * @author     Mihkel Aaremäe
 */
#ifndef SDMAP_DEBUG_H
#define SDMAP_DEBUG_H

#define detail_sdmap_get_slot(map, index) ((sdmap_slot *)((char *)(map) + sizeof(sdmap_header) + index * (sizeof(sdmap_slot) + sizeof(map->key) + sizeof(map->value))))

#define detail_sdmap_get_key(map, index) (sdmap_typeof(map[0].type_data->key) *)((char *)(detail_sdmap_m2h(map)) + sizeof(sdmap_header) + index * (sizeof(sdmap_slot) + sizeof(map[0].type_data->key) + sizeof(map[0].type_data->value)) + sizeof(sdmap_slot))

#define detail_sdmap_get_value(map, index) (sdmap_typeof(map[0].type_data->value) *)((char *)(detail_sdmap_m2h(map)) + sizeof(sdmap_header) + index * (sizeof(sdmap_slot) + sizeof(map[0].type_data->key) + sizeof(map[0].type_data->value)) + sizeof(sdmap_slot) + sizeof(map[0].type_data->key))

#define detail_sdmap_slot(map, index) ((sdmap_slot *)((char *)(map) + sizeof(sdmap_header) + index * slot_size))

#define detail_sdmap_key(map, index) ((void *)((char *)(map) + sizeof(sdmap_header) + index * slot_size + sizeof(sdmap_slot)))

#define detail_sdmap_value(map, index) ((void *)((char *)(map) + sizeof(sdmap_header) + index * slot_size + sizeof(sdmap_slot) + key_size))

#define detail_sdmap_heap_from_header(h) ((sdmap_heap *)((char *)((void *)(h)) - offsetof(sdmap_heap, header)))

#define debug_sdmap_pickformat(x) _Generic(x,\
	int32_t: "%d",\
	uint32_t: "%d",\
	char *: "%s",\
	float: "%f",\
	default: "Unknown: "#x)

#define debug_sdmap_print(map)\
while(1)\
{\
	const uint32_t slot_size = sizeof(sdmap_slot) + sizeof(map[0].type_data->key) + sizeof(map[0].type_data->value);\
	sdmap_header *printhead = detail_sdmap_m2h(map);\
	if (printhead == NULL)\
	{\
		break;\
	}\
	for (uint32_t i = 0; i < printhead->slot_count; i++)\
	{\
		if (detail_sdmap_slot(printhead, i)->height >= 0) \
		{\
			int balance;\
			balance = 0xFFFFFFFF;\
			if (detail_sdmap_slot(printhead, i)->left != (sdmap_index)-1 && detail_sdmap_slot(printhead, i)->right != (sdmap_index)-1)\
			{\
				balance = detail_sdmap_compute_balance(printhead, sizeof(sdmap_slot) + sizeof(map[0].type_data->key) + sizeof(map[0].type_data->value), i);\
			}\
			printf("slot:%d  balance:%d  height:%d", i, balance, (int)detail_sdmap_slot(printhead, i)->height);\
			if (detail_sdmap_slot(printhead, i)->left == i)\
			{\
				printf("  left:#");\
			}\
			else\
			{\
				printf("  left:%d", detail_sdmap_slot(printhead, i)->left);\
			}\
			if (detail_sdmap_slot(printhead, i)->right == i)\
			{\
				printf("  right:#");\
			}\
			else\
			{\
				printf("  right:%d", detail_sdmap_slot(printhead, i)->right);\
			}\
			printf("  parent:%d", detail_sdmap_slot(printhead, i)->parent);\
			printf("  key:");\
			printf(debug_sdmap_pickformat(map[0].type_data->key), *detail_sdmap_get_key(map, i));\
			printf("  value:");\
			printf(debug_sdmap_pickformat(map[0].type_data->value), *detail_sdmap_get_value(map, i));\
			printf("\n");\
		}\
		else\
		{\
			printf("slot:%d EMPTY next:%d  height:%d %p\n", i, (int)detail_sdmap_slot(printhead, i)->right, (int)detail_sdmap_slot(printhead, i)->height, (void *)detail_sdmap_slot(printhead, i));\
		}\
	}\
	break;\
}

#define debug_sdmap_printtree(map) debug_sdmap_printtree_impl(detail_sdmap_m2h(map), sizeof(map[0].type_data->key), sizeof(map[0].type_data->value), ((sdmap_header *)detail_sdmap_m2h(map))->root_slot, 0)

static inline int detail_sdmap_compute_balance(sdmap_header *header, uint32_t slot_size, sdmap_index node)
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

static inline void debug_sdmap_printtree_impl(void *header, uint32_t key_size, uint32_t value_size, sdmap_index root, int space)
{
	if (((sdmap_header *)header)->count == 0)
	{
		return;
	}
	const int COUNT = 10;
	space += COUNT;
	sdmap_slot *slot = (void *)((char *)header + sizeof(sdmap_header) + (sizeof(sdmap_slot) + key_size + value_size) * root);
	if (slot->right != root)
	{
		debug_sdmap_printtree_impl(header, key_size, value_size, slot->right, space);
	}
	printf("\n");
	for (int i = COUNT; i < space; i++)
	{
		printf(" ");
	}
	printf("%d\n", *(int *)((char *)slot + sizeof(sdmap_slot)));
	if (slot->left != root)
	{
		debug_sdmap_printtree_impl(header, key_size, value_size, slot->left, space);
	}
}

#define debug_sdmap_sanity_checks(map) debug_sdmap_sanity_checks_impl(detail_sdmap_m2h(map), sizeof(map[0].type_data->key), sizeof(map[0].type_data->value))

static inline int debug_sdmap_sanity_header(sdmap_header *header, uint32_t key_size, uint32_t value_size)
{
	(void)key_size;
	(void)value_size;
	if (header == NULL)
	{
		return 0;
	}
	if (header->count > header->slot_count ||
		(header->root_slot > header->slot_count && header->root_slot != (sdmap_index)-1) ||
		(header->empty_slot > header->slot_count && header->empty_slot != (sdmap_index)-1))
	{
		printf("counts %d %d %d %d\n", (int)header->slot_count, (int)header->count, (int)header->root_slot, (int)header->empty_slot);
		return 1;
	}
	if (header->compare_func == NULL)
	{
		printf("no compare function\n");
		return 1;
	}
	return 0;
}

static inline sdmap_index debug_sdmap_sanity_count_helper(sdmap_header *header, uint32_t slot_size, uint32_t index)
{
	sdmap_index count = 1;
	if (detail_sdmap_slot(header, index)->left != index)
	{
		count += debug_sdmap_sanity_count_helper(header, slot_size, detail_sdmap_slot(header, index)->left);
	}
	if (detail_sdmap_slot(header, index)->right != index)
	{
		count += debug_sdmap_sanity_count_helper(header, slot_size, detail_sdmap_slot(header, index)->right);
	}
	return count;
}

static inline int debug_sdmap_sanity_count(sdmap_header *header, uint32_t key_size, uint32_t value_size)
{
	if (header == NULL)
	{
		return 0;
	}
	if (header->root_slot == (sdmap_index)-1)
	{
		return 0;
	}
	sdmap_index read_count = debug_sdmap_sanity_count_helper(header, sizeof(sdmap_slot) + key_size + value_size, header->root_slot);
	if (read_count != header->count)
	{
		printf("read_count=%d  header->count=%d\n", (int)read_count, (int)header->count);
		return 1;
	}
	return 0;
}

static inline int debug_sdmap_sanity_empty_count(sdmap_header *header, uint32_t key_size, uint32_t value_size)
{
	const uint32_t slot_size = sizeof(sdmap_slot) + key_size + value_size;
	sdmap_index slot_index;
	sdmap_index count;
	if (header == NULL)
	{
		return 0;
	}
	if (header->empty_slot == (sdmap_index)-1)
	{
		if (header->count != header->slot_count)
		{
			printf("no empty slots but header->count(%d) != header->slot_count(%d)\n", (int)header->count, (int)header->slot_count);
			return 1;
		}
	}
	slot_index = header->empty_slot;
	count = 0;
	while (slot_index != (sdmap_index)-1)
	{
		count ++;
		if (detail_sdmap_slot(header, slot_index)->height >= 0)
		{
			printf("positive height=%d empty slot=%d  %p\n", (int)detail_sdmap_slot(header, slot_index)->height, (int)slot_index, (void *)detail_sdmap_slot(header, slot_index));
			return 1;
		}
		slot_index = detail_sdmap_slot(header, slot_index)->right;
	}
	return count + header->count != header->slot_count;
}

static inline int debug_sdmap_sanity_height(sdmap_header *header, uint32_t slot_size, uint32_t index)
{
	int result = 0;
	if (detail_sdmap_slot(header, index)->left != index)
	{
		int h = debug_sdmap_sanity_height(header, slot_size, detail_sdmap_slot(header, index)->left);
		if (h + 1 > result)
		{
			result = h + 1;
		}
	}
	if (detail_sdmap_slot(header, index)->right != index)
	{
		int h = debug_sdmap_sanity_height(header, slot_size, detail_sdmap_slot(header, index)->right);
		if (h + 1 > result)
		{
			result = h + 1;
		}
	}
	return result;
}

static inline int debug_sdmap_sanity_nodes_helper(sdmap_header *header, uint32_t slot_size, uint32_t index)
{
	int balance;
	sdmap_index parent;
	if (detail_sdmap_slot(header, index)->right >= header->slot_count ||
		detail_sdmap_slot(header, index)->left >= header->slot_count ||
		detail_sdmap_slot(header, index)->height < 0 ||
		detail_sdmap_slot(header, index)->height != debug_sdmap_sanity_height(header, slot_size, index))
	{
		printf("node %d has incorrect pointers right=%d left=%d height%d header->slot_count=%d\n", (int)index, 
			(int)detail_sdmap_slot(header, index)->right,
			(int)detail_sdmap_slot(header, index)->left,
			(int)detail_sdmap_slot(header, index)->height,
			(int)header->slot_count);
		return 1;
	}
	balance = detail_sdmap_compute_balance(header, slot_size, index);
	if (balance <= -2 || balance >= 2)
	{
		printf("balance of node %d is %d\n", (int)index, (int)balance);
		return 1;
	}
	if (detail_sdmap_slot(header, index)->parent == (sdmap_index)-1)
	{
		if (header->root_slot != index)
		{
			printf("node %d, parent is -1 but is not root node, true root node=%d\n", (int)index, (int)header->root_slot);
			return 1;
		}
	}
	else
	{
		parent = detail_sdmap_slot(header, index)->parent;
		if (parent >= header->slot_count)
		{
			printf("node %d, parent=%d is out of range=%d\n", (int)index, (int)parent, (int)header->slot_count);
			return 1;
		}
		if (detail_sdmap_slot(header, parent)->left != index && 
			detail_sdmap_slot(header, parent)->right != index)
		{
			printf("node %d, not pointed back to by parent (left=%d  right=%d)\n", (int)index,
				(int)(detail_sdmap_slot(header, parent)->left),
				(int)(detail_sdmap_slot(header, parent)->right));
			return 1;
		}
	}
	if (detail_sdmap_slot(header, index)->left != index)
	{
		if (debug_sdmap_sanity_nodes_helper(header, slot_size, detail_sdmap_slot(header, index)->left))
		{
			return 1;
		}
	}
	if (detail_sdmap_slot(header, index)->right != index)
	{
		if (debug_sdmap_sanity_nodes_helper(header, slot_size, detail_sdmap_slot(header, index)->right))
		{
			return 1;
		}
	}
	return 0;
}

static inline int debug_sdmap_sanity_nodes(sdmap_header *header, uint32_t key_size, uint32_t value_size)
{
	if (header == NULL)
	{
		return 0;
	}
	if (header->root_slot == (sdmap_index)-1)
	{
		return 0;
	}
	return debug_sdmap_sanity_nodes_helper(header, sizeof(sdmap_slot) + key_size + value_size, header->root_slot);
}

static inline const char *debug_sdmap_sanity_checks_impl(sdmap_header *header, uint32_t key_size, uint32_t value_size)
{
	if (debug_sdmap_sanity_header(header, key_size, value_size))
	{
		return "header";
	}
	if (debug_sdmap_sanity_count(header, key_size, value_size))
	{
		return "count";
	}
	if (debug_sdmap_sanity_empty_count(header, key_size, value_size))
	{
		return "empty";
	}
	if (debug_sdmap_sanity_nodes(header, key_size, value_size))
	{
		return "nodes";
	}
	return "";
}

#endif
