#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void *custom_malloc(int n)
{
	void *result = malloc(n);
	printf("custom malloc %p  size %d\n", result, n);
	return result;
}

void *custom_realloc(void *initial, int n)
{
	printf("custom realloc %p", initial);
	void *result = realloc(initial, n);
	printf(" -> %p  size %d\n", result, n);
	return result;
}

void custom_free(void *initial)
{
	printf("custom free %p\n", initial);
	free(initial);
}

#define sdhmap_malloc custom_malloc
#define sdhmap_realloc custom_realloc
#define sdhmap_free custom_free

#include <sdhmap.h>

#define TEST_MAX_SIZE 512

typedef void(*test_fun)(char [TEST_MAX_SIZE]);

#define submit_solution(x) strcat(solution, debug_sdhmap_sanity_checks(x))

typedef struct test_t
{
	const char *solution;
	test_fun test_function;
} test_t;

void strcatf(char *target, const char *format, ...)
{
	char buffer[4096];
	va_list args;
	va_start (args, format);
	vsnprintf(buffer, 4096, format, args);
	strcat(target, buffer);
	va_end(args);
}

const void *dummy;

#define detail_sdhmap_slot(map, index) ((sdhmap_slot *)((char *)(map) + sizeof(sdhmap_header) + index * slot_size))

/*Test null initialization*/
void test_0(char solution[TEST_MAX_SIZE])
{
	(void)solution;
	sdhmap(int, int) a = NULL;
	int count = 20;
	sdhmap_reserve(a, 100);
	for (int i = 0; i < count; i++)
	{
		sdhmap_get(a, i) = i;
	}

	const int *key = sdhmap_first(a);
	while (key)
	{
		printf("%d ", sdhmap_get(a, key));
		key = sdhmap_next(a, key);
	}

	sdhmap_shrink(a);

	int slot_size = sizeof(a[0].type_data->slot);
	sdhmap_header *header = (void *)a;
	for (sdhmap_index i = 0; i < header->slot_count; i++)
	{
		printf("index %d:  slot=%d  next=%d  prev=%d  key=%d  value=%d\n", 
			(int)i, 
			(int)detail_sdhmap_slot(header, i)->slot,
			(int)detail_sdhmap_slot(header, i)->next,
			(int)detail_sdhmap_slot(header, i)->prev,
			*((int *)(detail_sdhmap_slot(header, i)+1)),
			*((int *)(detail_sdhmap_slot(header, i)+1)+1));
	}

	sdhmap_delete(a);
}

const test_t tests[] =
{
	{"good", test_0},
};

void run_test(int i, char solution[TEST_MAX_SIZE])
{
	solution[0] = '\0';
	tests[i].test_function(solution);
}

void run_all_tests(void)
{
	char solution[TEST_MAX_SIZE];
	int i;
	printf("\n---Running all tests for SDHMAP---\n");
	for (i = 0; i < (int)(sizeof(tests) / sizeof(*tests)); i++)
	{
		printf("Running test #%d ...\n", i);
		run_test(i, solution);
		if (strcmp(solution, tests[i].solution) == 0)
		{
			printf("Test #%d -> Success\n", i);
		}
		else
		{
			printf("Test #%d -> Fail. Expected '%s'. Got '%s'.\n", i, tests[i].solution, solution);
		}
	}
	printf("---Done---\n\n");
}

int main(void)
{
	run_all_tests();
	return 0;
}
