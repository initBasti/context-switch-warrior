#include "../unity/src/unity.h"
#include <string.h>

#include "../source/include/config.h"

#define TESTS 10

void setUp(void)
{

}

void tearDown(void)
{

}

void test_getSubstring(void)
{
	struct substr *string_set[TESTS] = {NULL};
	int mem_index = 0;
	int result_set[TESTS] = {0};
	int amount_substrings_set[TESTS] = {0};
	int expected_result[TESTS] = {0,0,0,0,0,0,0,0, -1, 0};
	int expected_amount[TESTS] = {2,3,4,4,1,2,4,2,0,4};
	char sep[TESTS] = {',', ',', ',', ',', ',', ',', ',', ',', ',', ';'};
	char test[TESTS][46] = {{0}};
	char result_string[26][MAX_FIELD] = {{0}};
	char expected_string[26][MAX_FIELD] = {
		{"2019-09-01"}, {"2019-09-02"}, {"2019-09-03"}, {"2019-09-04"},
		{"2019-09-05"}, {"2019-09-06"}, {"2019-09-07"}, {"2019-09-08"},
		{"2019-09-09"}, {"mo"}, {"tu"}, {"we"}, {"th"}, {"2019-09-10;2019-09-11"},
		{"2019-09-12"}, {"2019-09-13|2019-09-14"}, {"2019-09-15"},
		{"2019-09-16"}, {"2019-09-17"}, {"2019-09-18"}, {"mo"}, {"ads"},
		{"ZONE=Test"}, {"START=15:30"}, {"END=16:30"}, {"CONTEXT=work"}
	};

	for(int i = 0 ; i < TESTS ; i++) {
		string_set[i] = allocateSubstring();
		memset(test[i], 0, 46);
	}
	strncpy(test[0], "2019-09-01,2019-09-02", 22);
	strncpy(test[1], "2019-09-03,2019-09-04,2019-09-05", 33);
	strncpy(test[2], "2019-09-06,2019-09-07,2019-09-08,2019-09-09", 44);
	strncpy(test[3], "mo,tu,we,th", 12);
	strncpy(test[4], "2019-09-10;2019-09-11", 22);
	strncpy(test[5], "2019-09-12,2019-09-13|2019-09-14", 33);
	strncpy(test[6], "2019-09-15,,2019-09-16,2019-09-17,2019-09-18", 45);
	strncpy(test[7], "mo,,ads,", 10);
	strncpy(test[8], "a, b, c", 8);
	strncpy(test[9], "ZONE=Test;START=15:30;END=16:30;CONTEXT=work;", 46);

	for(int i = 0 ; i < TESTS ; i++) {
		result_set[i] = getSubstring(&test[i][0], string_set[i],
					     &amount_substrings_set[i], sep[i]);
		for(struct substr *ptr=string_set[i];ptr!=NULL;ptr=ptr->next) {
			if(amount_substrings_set[i] > 0) {
				strncpy(result_string[mem_index], ptr->member, MAX_FIELD);
				mem_index++;
			}
		}
		freeSubstring(string_set[i]);
	}
	TEST_ASSERT_EQUAL_INT_ARRAY_MESSAGE(expected_result, result_set, TESTS,
					    "result didn't meet expectation");
	TEST_ASSERT_EQUAL_INT_ARRAY_MESSAGE(expected_amount, amount_substrings_set,
					    TESTS, "amount didn't meet expectation");
	for(int i = 0 ; i < 26 ; i++)
		TEST_ASSERT_EQUAL_STRING(expected_string[i], result_string[i]);

}

void test_addSubstring(void)
{
	char good[2][6] = {"house", "smil,"};
	int result = 0;
	struct substr *head[4] = {NULL};
	for(int i = 0 ; i < 2 ; i++)
		head[i] = allocateSubstring();

	strncpy(head[0]->member, "car", 4);
	head[0]->next = NULL;
	strncpy(head[1]->member, "car", 4);
	head[1]->next = NULL;
	head[2] = allocateSubstring();
	head[3] = allocateSubstring();

	result = addSubstring(head[0], good[0], ',');
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, result, "for 'house' should return 0");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("house", head[0]->next->member,
					 "add string should be house");
	result = addSubstring(head[1], "", ',');
	TEST_ASSERT_EQUAL_INT_MESSAGE(-1, result, "for '' should return -1");
	TEST_ASSERT_EQUAL_PTR(NULL, head[1]->next);

	result = addSubstring(head[2], good[0], ',');
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, result,
				      "empty_list: for 'house' should return 0");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("house", head[2]->member,
					 "empty_list: add string should be house");
	result = addSubstring(head[3], good[1], ',');
	TEST_ASSERT_EQUAL_INT_MESSAGE(0, result,
				      "with sep: for 'smil,' should return 0");
	TEST_ASSERT_EQUAL_STRING_MESSAGE("smil", head[3]->member,
					 "with sep: add string should be smil");
	freeSubstring(head[0]);
	freeSubstring(head[1]);
	freeSubstring(head[2]);
	freeSubstring(head[3]);
}

void test_equalListElements(void)
{
	struct substr *test_list[5] = {NULL};
	int result[5] = {0};
	int expected[5] = {-1, 0, 0, 2, -2};
	char errmsg[MAX_FIELD] = {0};
	char words[5][5][30] = {
		{"zone", "start", "end", "context"},
		{"2019-09-11", "2019-10-02", "2019-11-18"},
		{"mo", "tu", "we", "th", "fr"},
		{"", "hallo", "hey", ""},
		{"2019--10--01"}
	};
	for(int i = 0 ; i < 5 ; i++) {
		test_list[i] = allocateSubstring(test_list[i]);
		for(int j = 0 ; j < 5 ; j++)
			addSubstring(test_list[i], words[i][j], ',');

		result[i] = equalListElements(test_list[i]);
		freeSubstring(test_list[i]);
	}
	memset(errmsg, 0, MAX_FIELD);
	for(int i = 0 ; i < 5 ; i++) {
		snprintf(errmsg, MAX_FIELD, "fields: %s, result: %d, expected: %d\n",
				 words[i][1], result[i], expected[i]);
		TEST_ASSERT_EQUAL_INT(expected[i], result[i]);
	}
}
/*=======MAIN=====*/
int main(void)
{
	UnityBegin("test_substring.c");
	RUN_TEST(test_getSubstring);
	RUN_TEST(test_addSubstring);
	RUN_TEST(test_equalListElements);

	return UnityEnd();
}
