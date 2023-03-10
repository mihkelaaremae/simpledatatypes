#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*
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

#define sdmap_malloc custom_malloc
#define sdmap_realloc custom_realloc
#define sdmap_free custom_free
*/

#include <sdmap.h>
#include <sdmap_debug.h>

#define TEST_MAX_SIZE 512

typedef void(*test_fun)(char [TEST_MAX_SIZE]);

#define submit_solution(x) strcat(solution, debug_sdmap_sanity_checks(x))

typedef struct test_t
{
	const char *solution;
	test_fun test_function;
} test_t;

void test_0_helper(const void *key)
{
	(void)key;
}

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

/*Test null initialization*/
void test_0(char solution[TEST_MAX_SIZE])
{
	sdmap(int, int) x = NULL;
	submit_solution(x);

	sdmap_count(x); submit_solution(x); sdmap_delete(x);

	if (x == NULL)
	{
		strcat(solution, "good");
	}

	sdmap_capacity(x); submit_solution(x); sdmap_delete(x);

	sdmap_contains(x, 5); submit_solution(x); sdmap_delete(x);

	sdmap_reserve(x, 5); submit_solution(x); sdmap_delete(x);

	sdmap_new(x); submit_solution(x); sdmap_delete(x);

	sdmap_new(x, detail_sdmap_strcmp); submit_solution(x); sdmap_delete(x);

	sdmap_new(x, detail_sdmap_strcmp, 5); submit_solution(x); sdmap_delete(x);

	dummy = &sdmap_get(x, 5); submit_solution(x); sdmap_delete(x);

	dummy = sdmap_min(x); submit_solution(x); sdmap_delete(x);

	dummy = sdmap_max(x); submit_solution(x); sdmap_delete(x);

	dummy = sdmap_root(x); submit_solution(x); sdmap_delete(x);

	dummy = sdmap_next(x, 0); submit_solution(x); sdmap_delete(x);

	dummy = sdmap_prev(x, 0); submit_solution(x); sdmap_delete(x);

	sdmap_set(x, 5, 5); submit_solution(x); sdmap_delete(x);

	sdmap_erase(x, 5); submit_solution(x); sdmap_delete(x);

	sdmap_traverse_inorder_keys(x, test_0_helper); submit_solution(x); sdmap_delete(x);

	sdmap_delete(x); submit_solution(x); sdmap_delete(x);
}

/*Test empty initialized maps*/
void test_1(char solution[TEST_MAX_SIZE])
{
	sdmap(int, int) x = NULL;

	sdmap_reserve(x, 5);

	sdmap_count(x); submit_solution(x); sdmap_delete(x); sdmap_reserve(x, 5);

	sdmap_capacity(x); submit_solution(x); sdmap_delete(x); sdmap_reserve(x, 5);

	sdmap_contains(x, 5); submit_solution(x); sdmap_delete(x); sdmap_reserve(x, 5);

	sdmap_reserve(x, 5); submit_solution(x); sdmap_delete(x); sdmap_reserve(x, 5);

	dummy = &sdmap_get(x, 5); submit_solution(x); sdmap_delete(x); sdmap_reserve(x, 5);

	dummy = sdmap_min(x); submit_solution(x); sdmap_delete(x); sdmap_reserve(x, 5);

	dummy = sdmap_max(x); submit_solution(x); sdmap_delete(x); sdmap_reserve(x, 5);

	dummy = sdmap_root(x); submit_solution(x); sdmap_delete(x); sdmap_reserve(x, 5);

	dummy = sdmap_next(x, 0); submit_solution(x); sdmap_delete(x); sdmap_reserve(x, 5);

	dummy = sdmap_prev(x, 0); submit_solution(x); sdmap_delete(x); sdmap_reserve(x, 5);

	sdmap_set(x, 5, 5); submit_solution(x); sdmap_delete(x); sdmap_reserve(x, 5);

	sdmap_erase(x, 5); submit_solution(x); sdmap_delete(x); sdmap_reserve(x, 5);

	sdmap_traverse_inorder_keys(x, test_0_helper); submit_solution(x); sdmap_delete(x); sdmap_reserve(x, 5);

	sdmap_delete(x); submit_solution(x); sdmap_delete(x); sdmap_reserve(x, 5);
}

/*Count is working properly*/
void test_2(char solution[TEST_MAX_SIZE])
{
	sdmap(int, int) x = NULL;
	strcatf(solution, "%d ", sdmap_count(x));
	sdmap_set(x, 0, 0);
	strcatf(solution, "%d ", sdmap_count(x));
	sdmap_set(x, 1, 0);
	strcatf(solution, "%d ", sdmap_count(x));
	sdmap_set(x, 2, 0);
	strcatf(solution, "%d ", sdmap_count(x));
	sdmap_set(x, 2, 0);
	strcatf(solution, "%d ", sdmap_count(x));
	sdmap_set(x, 2, 1);
	strcatf(solution, "%d ", sdmap_count(x));
	submit_solution(x); 
	sdmap_delete(x);
}

/*Getting is working properly*/
void test_3(char solution[TEST_MAX_SIZE])
{
	sdmap(int, int) x = NULL;
	sdmap_set(x, 0, 0);
	strcatf(solution, "%d ", sdmap_get(x, 0));
	sdmap_set(x, 1, 0);
	strcatf(solution, "%d ", sdmap_get(x, 1));
	sdmap_set(x, 2, 0);
	strcatf(solution, "%d ", sdmap_get(x, 2));
	sdmap_set(x, 0, 5);
	strcatf(solution, "%d ", sdmap_get(x, 0));
	sdmap_set(x, 1, 5);
	strcatf(solution, "%d ", sdmap_get(x, 1));
	sdmap_set(x, 2, 5);
	strcatf(solution, "%d ", sdmap_get(x, 2));
	submit_solution(x); 
	sdmap_delete(x);
}

/*Simple stress test*/
void test_4(char solution[TEST_MAX_SIZE])
{
	int i;
	sdmap(int, int) x = NULL;
	for (i = 0; i < 1000; i++)
	{
		sdmap_set(x, i, i);
	}
	submit_solution(x);
	sdmap_delete(x);
}

/*Larger stress test*/
void test_5(char solution[TEST_MAX_SIZE])
{
	int i;
	sdmap(int, int) x = NULL;
	for (i = 0; i < 10000; i++)
	{
		sdmap_set(x, i, i);
	}
	for (i = 0; i < 10000; i++)
	{
		sdmap_erase(x, i);
	}
	debug_sdmap_print(x);
	debug_sdmap_printtree(x);
	submit_solution(x);
	sdmap_delete(x);
}

/*But now its backwards*/
void test_6(char solution[TEST_MAX_SIZE])
{
	int i;
	sdmap(int, int) x = NULL;
	for (i = 0; i < 10000; i++)
	{
		sdmap_set(x, i, i);
	}
	for (i = 9999; i >= 0; i--)
	{
		sdmap_erase(x, i);
	}
	submit_solution(x);
	sdmap_delete(x);
}

/*Stress test with random ints*/
void test_7(char solution[TEST_MAX_SIZE])
{
	int i;
	sdmap(int, int) x = NULL;
	srand(time(NULL));
	for (i = 0; i < 100000; i++)
	{
		sdmap_set(x, rand() % 100000, i);
	}
	submit_solution(x);
	for (i = 0; i < 100000; i++)
	{
		sdmap_erase(x, rand() % 100000);
	}
	submit_solution(x);
	for (i = 0; i < 100000; i++)
	{
		sdmap_set(x, rand() % 100000, i);
	}
	submit_solution(x);
	for (i = 0; i < 100000; i++)
	{
		sdmap_erase(x, rand() % 100000);
	}
	submit_solution(x);
	sdmap_delete(x);

	int count = 100;

	sdmap_stack(int, int, count) y;
	sdmap_new(y);
	for (i = 0; i < 100; i++)
	{
		sdmap_set(y, rand() % 100000, i);
	}
	sdmap_delete(y);
}

/*Iterate just like in example*/
void test_8(char solution[TEST_MAX_SIZE])
{
	sdmap(int, int) x = NULL;
	sdmap_get(x, 0) = 0;
	sdmap_get(x, -4) = -4;
	sdmap_get(x, 6) = 6;
	sdmap_get(x, 8) = 8;
	sdmap_get(x, 14) = 14;
	sdmap_get(x, 11) = 11;
	const int *key = sdmap_min(x);
	while (key)
	{
		strcatf(solution, "%d ", sdmap_get(x, key));
		key = sdmap_next(x, key);
	}

	key = sdmap_max(x);
	while (key)
	{
		strcatf(solution, "%d ", sdmap_get(x, key));
		key = sdmap_prev(x, key);
	}
}

const test_t tests[] =
{
	{"good", test_0},
	{"", test_1},
	{"0 1 2 3 3 3 ", test_2},
	{"0 0 0 5 5 5 ", test_3},
	{"", test_4},
	{"", test_5},
	{"", test_6},
	{"", test_7},
	{"-4 0 6 8 11 14 14 11 8 6 0 -4 ", test_8},
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
	printf("\n---Running all tests for SDMAP---\n");
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
