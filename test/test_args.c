#include "../unity/src/unity.h"

#include "../source/include/args.h"
#ifndef TYPES_H
#include "../source/include/types.h"
#endif

int verbose = 0;

void setUp(void)
{

}

void tearDown(void)
{

}

void test_lower_case(void)
{

}

#define ARG_TEST 10
void test_getArgs(void)
{
	struct flags test_flags[ARG_TEST] = {
		{.verbose = &verbose,.cancel_on=-1,.notify_on=-1},
		{.verbose = &verbose,.cancel_on=-1,.notify_on=-1},
		{.verbose = &verbose,.cancel_on=-1,.notify_on=-1},
		{.verbose = &verbose,.cancel_on=-1,.notify_on=-1},
		{.verbose = &verbose,.cancel_on=-1,.notify_on=-1},
		{.verbose = &verbose,.cancel_on=-1,.notify_on=-1},
		{.verbose = &verbose,.cancel_on=-1,.notify_on=-1},
		{.verbose = &verbose,.cancel_on=-1,.notify_on=-1},
		{.verbose = &verbose,.cancel_on=-1,.notify_on=-1},
		{.verbose = &verbose,.cancel_on=-1,.notify_on=-1},
	};
	int test_argc[ARG_TEST] = {3, 3, 5, 3, 1, 3, 5, 10, 5, 3};
	char *test_argv[ARG_TEST][MAX_ARG_LENGTH] = {
		{"csw","-d","30min"},
		{"csw","-v","-s"},
		{"csw","-c","1","-n","0"},
		{"csw","-a","-v"},
		{"csw"},
		{"csw","-d","abc"},
		{"csw","-c","22","-n","3"},
		{"csw","-v","-s","-h","-d","1h","-c","0","-n","1"},
		{"csw","-i","10","-d","15"},
		{"csw","-i","0.5"}
	};
	char opt_string[14] = {"hd:si:c:n:v::"};
	int result[ARG_TEST] = {0};
	int expected[ARG_TEST] = {0, 0, 0, -1, 0, -1, -1, 0, 0, -1};
	int exp_delay[ARG_TEST] = {30, 0, 0, 0, 0, 0, 0, 60, 15, 0};
	int exp_verbose[ARG_TEST] = {0, 1, 0, 0, 0, 0, 0, 1, 0, 0};
	int exp_show[ARG_TEST] = {0, 1, 0, 0, 0, 0, 0, 1, 0, 0};
	int exp_cancel[ARG_TEST] = {-1, -1, 1, -1, -1, -1, -1, 0, -1, -1};
	int exp_notify[ARG_TEST] = {-1, -1, 0, -1, -1, -1, -1, 1, -1, -1};
	int exp_help[ARG_TEST] = {0, 0, 0, 0, 0, 0, 0, 1, 0, 0};
	int exp_interval[ARG_TEST] = {0, 0, 0, 0, 0, 0, 0, 0, 10, 0};

	for(int i = 0 ; i < ARG_TEST ; i++) {
		verbose = 0;
		result[i] = getArgs(&test_flags[i], test_argc[i], test_argv[i],
							opt_string);
		TEST_ASSERT_EQUAL_INT(exp_delay[i], test_flags[i].delay);
		TEST_ASSERT_EQUAL_INT(exp_verbose[i], verbose);
		TEST_ASSERT_EQUAL_INT(exp_cancel[i], test_flags[i].cancel_on);
		TEST_ASSERT_EQUAL_INT(exp_notify[i], test_flags[i].notify_on);
		TEST_ASSERT_EQUAL_INT(exp_help[i], test_flags[i].help);
		TEST_ASSERT_EQUAL_INT(exp_show[i], test_flags[i].show);
		TEST_ASSERT_EQUAL_INT(exp_interval[i], test_flags[i].cron_interval);
	}
	TEST_ASSERT_EQUAL_INT_ARRAY(expected, result, ARG_TEST);
}

void test_strToInt(void)
{
	char test_string[50];
	int number = 0;
	int *output = &number;
	strncpy(test_string, "6", 2);
	TEST_ASSERT_EQUAL_INT(STR_TO_INT_SUCCESS, strToInt(output, &test_string[0]));
	memset(test_string, 0, 50);
	strncpy(test_string, "4123", 5);
	TEST_ASSERT_EQUAL_INT(STR_TO_INT_SUCCESS, strToInt(output, &test_string[0]));
}

void test_strToInt_overflow(void)
{
	char test_string[50];
	int number = 0;
	int *output = &number;
	strncpy(test_string, "1234567891011", 14);
	TEST_ASSERT_EQUAL_INT(STR_TO_INT_OVERFLOW, strToInt(output, &test_string[0]));
}

void test_strToInt_underflow(void)
{
	char test_string[50];
	int number = 0;
	int *output = &number;
	strncpy(test_string, "-1234567891011", 15);
	TEST_ASSERT_EQUAL_INT(STR_TO_INT_UNDERFLOW, strToInt(output, &test_string[0]));
}

void test_strToInt_bad(void)
{
	char test_string[50];
	int number = 0;
	int *output = &number;
	strncpy(test_string, "A", 2);
	TEST_ASSERT_EQUAL_INT(STR_TO_INT_INCONVERTIBLE, strToInt(output, &test_string[0]));
	memset(test_string, 0, 50);
	strncpy(test_string, "&", 5);
	TEST_ASSERT_EQUAL_INT(STR_TO_INT_INCONVERTIBLE, strToInt(output, &test_string[0]));
	memset(test_string, 0, 50);
	strncpy(test_string, "1oo", 5);
	TEST_ASSERT_EQUAL_INT(STR_TO_INT_INCONVERTIBLE, strToInt(output, &test_string[0]));
}

/*=======MAIN=====*/
int main(void)
{
	UnityBegin("test_args.c");
	RUN_TEST(test_getArgs);
	RUN_TEST(test_strToInt);
	RUN_TEST(test_strToInt_underflow);
	RUN_TEST(test_strToInt_bad);

	return UnityEnd();
}
