#include "../unity/src/unity.h"
#include <string.h>

#include "../source/include/exclude.h"

void setUp(void)
{

}

void tearDown(void)
{

}

#define PARSE_TIME_TEST 6
void test_parseTime(void)
{
	char msg[MAX_ROW] = {0};
	int hour[PARSE_TIME_TEST] = {0};
	int expect_hour[PARSE_TIME_TEST] = {16, 1, 23, 0, 0, 0};
	int minute[PARSE_TIME_TEST] = {0};
	int expect_minute[PARSE_TIME_TEST] = {10,1,59,0,0,0};
	char option[PARSE_TIME_TEST][7] = {
		"16:10", "1:1", "23:59", "161:10", "161", "A"
	};
	int result[PARSE_TIME_TEST] = {0};
	int expect[PARSE_TIME_TEST] = {0, 0, 0, -1, -1, -1};

	for(int i = 0 ; i < PARSE_TIME_TEST ; i++) {
		result[i] = parseTime(&hour[i], &minute[i], option[i]);
	}

	for(int i = 0 ; i < PARSE_TIME_TEST ; i++) {
		snprintf(msg, MAX_ROW, "%d. hour expect: %d was %d",i,
				expect_hour[i], hour[i]);
		TEST_ASSERT_EQUAL_INT_MESSAGE(expect_hour[i], hour[i],msg);
		snprintf(msg, MAX_ROW, "%d. minute expect: %d was %d",i,
				expect_minute[i], minute[i]);
		TEST_ASSERT_EQUAL_INT_MESSAGE(expect_minute[i], minute[i],msg);
		snprintf(msg, MAX_ROW, "%d. result expect: %d was %d",i,
				expect_minute[i], minute[i]);
		TEST_ASSERT_EQUAL_INT_MESSAGE(expect[i], result[i],msg);
	}
}

#define DATE_TEST 6
void test_parseDate(void)
{
	struct tm test_date = {0};
	char good[DATE_TEST/2][11] = {{"2019-10-11"}, {"2019-11-01"},
									{"2020-01-01"}};
	char bad[DATE_TEST/2][11] = {{"11-01-2019"},{"2019_01-11"},{"a-b-c"}};
	int result[DATE_TEST] = {0};
	int expected[DATE_TEST] = {0, 0, 0, -1, -1, -1};
	int result_year[DATE_TEST] = {0};
	int result_month[DATE_TEST] = {0};
	int result_day[DATE_TEST] = {0};
	int expected_year[DATE_TEST] = {119, 119, 120, 0, 0, 0};
	int expected_month[DATE_TEST] = {9, 10, 0, 0, 0, 0};
	int expected_day[DATE_TEST] = {11, 1, 1, 0, 0, 0};

	for(int i = 0 ; i < DATE_TEST/2 ; i++) {
		result[i] = parseDate(&test_date, good[i]);
		if(result[i] != 0) {
			result_year[i] = 0;
			result_month[i] = 0;
			result_day[i] = 0;
		}
		else {
			result_year[i] = test_date.tm_year;
			result_month[i] = test_date.tm_mon;
			result_day[i] = test_date.tm_mday;
		}
	}

	for(int i = 0 ; i < DATE_TEST/2 ; i++) {
		result[i+DATE_TEST/2] = parseDate(&test_date, bad[i]);
		if(result[i+DATE_TEST/2] != 0) {
			result_year[i+DATE_TEST/2] = 0;
			result_month[i+DATE_TEST/2] = 0;
			result_day[i+DATE_TEST/2] = 0;
		}
		else {
			result_year[i+DATE_TEST/2] = test_date.tm_year;
			result_month[i+DATE_TEST/2] = test_date.tm_mon;
			result_day[i+DATE_TEST/2] = test_date.tm_mday;
		}
	}

	TEST_ASSERT_EQUAL_INT_ARRAY(expected, result, DATE_TEST);
	TEST_ASSERT_EQUAL_INT_ARRAY(expected_year, result_year, DATE_TEST);
	TEST_ASSERT_EQUAL_INT_ARRAY(expected_month, result_month, DATE_TEST);
	TEST_ASSERT_EQUAL_INT_ARRAY(expected_day, result_day, DATE_TEST);
}

void test_parseExclusion(void)
{
	char good[5][34] = {
							{"temporary(2019-10-08)"}, 				//1
							{"temporary(2019-10-09,2019-11-02)"},  	//2
							{"temporary(2019-10-08#2019-11-02)"},  	//3
							{"permanent(Mo)"},                     	//4
							{"permanent(Mo, Tu)"} 					//5
						};
	char bad[14][34] = {
							{"temporary(Mo)"}, 						//6
							{"permanent(2019-10-02)"}, 				//7
							{"permanent(M)"}, 						//8
							{"permanent(Mon;Tue)"}, 				//9
							{"permanent(Mo;Tu)"}, 					//10
							{"permanent[Mo,Tu]"}, 					//11
							{"permanent()"}, 						//12
							{"pernanent(Mo)"}, 						//13
							{"temporary(02-10-2019)"}, 				//14
							{"temporary(2019-10-02;2019-11-02)"},  	//15
							{"temporary(2019_10-02)"}, 				//16
							{"temporary( 2019-10-02)"}, 			//17
							{"temporary()"}, 						//18
							{"temponary(2010-10-02#2019-03-02)"}    //19
						};

	struct exclusion test[19];
	for(int i = 0 ; i < 19 ; i++) {
		initExclusionStruct(&test[i]);
	}
	PARSER_STATE result_set[19] = {0};

	PARSER_STATE expected_result[19] = {
		PARSER_SUCCESS, PARSER_SUCCESS, PARSER_SUCCESS, PARSER_SUCCESS,
		PARSER_SUCCESS, PARSER_WRONGSIZE, PARSER_WRONGSIZE, PARSER_WRONGSIZE,
		PARSER_WRONGSIZE, PARSER_WRONGSIZE, PARSER_ERROR, PARSER_ERROR,
		PARSER_ERROR, PARSER_FORMAT, PARSER_WRONGSIZE, PARSER_FORMAT,
		PARSER_SUCCESS, PARSER_ERROR, PARSER_ERROR
	};
	for(int i = 0 ; i < 5 ; i++) {
		result_set[i] = parseExclusion(&test[i], &good[i][0]);
	}
	for(int j = 0 ; j < 14 ; j++) {
		result_set[j+5] = parseExclusion(&test[j+5], &bad[j][0]);
	}
	for(int i = 0 ; i < 19 ; i++) {
		TEST_ASSERT_EQUAL_INT(expected_result[i], result_set[i]);
	}
}

void test_parseTemporary(void)
{
	char good[4][DATE*4] = {{"2019-09-10"},
							{"2019-09-11,2019-09-12"},
							{"2019-09-13#2019-09-14"},
							{"2019-09-15,2019-09-16,2019-09-17"}};
	char bad[4][DATE*4] = {	{"Mo"},
							{"2019-09-18;2019-09-19"},
							{"20-09-2019,2019-09-21"},
							{"2019-09-22#2019-09-23#2019-09-24"}};
	struct exclusion test;
	initExclusionStruct(&test);
	int index = 0;
	int result[8][3] = {{0},{0},{0},{0},
						{0},{0},{0},{0}};
	int expected[8][3] = {{119,8,10},
							{119,8,11},
							{119,8,12},
							{119,8,13},
							{119,8,14},
							{119,8,15},
							{119,8,16},
							{119,8,17}};
	int result_set[8] = {0};
	int expected_result[8] = {0,0,0,0,-2,-2,-1,-1};

	for(int i = 0 ; i < 4 ; i++) {
		result_set[i] = parseTemporary(&good[i][0], &test);
	}
	/* needs to be seperated for the order of the results */
	for(int i = 0 ; i < 4 ; i++) {
		result_set[i+4] = parseTemporary(&bad[i][0], &test);
	}
	for(int j = 0 ; j < 8 ; j++) {
		if(strncmp(test.type_name[j], "temp", 4) == 0) {
			if(strncmp(test.type[j].sub_type, "solo", 4) == 0) {
				result[index][0] = test.type[j].single_days[0].tm_year;
				result[index][1] = test.type[j].single_days[0].tm_mon;
				result[index][2] = test.type[j].single_days[0].tm_mday;
				index++;
			}
			else if(strncmp(test.type[j].sub_type, "list", 4) == 0) {
				for(int k = 0 ; k < test.type[j].list_len ; k++) {
					result[index][0] = test.type[j].single_days[k].tm_year;
					result[index][1] = test.type[j].single_days[k].tm_mon;
					result[index][2] = test.type[j].single_days[k].tm_mday;
					index++;
				}
			}
			else if(strncmp(test.type[j].sub_type, "range", 5) == 0) {
				result[index][0] = test.type[j].holiday_start.tm_year;
				result[index][1] = test.type[j].holiday_start.tm_mon;
				result[index][2] = test.type[j].holiday_start.tm_mday;
				index++;
				result[index][0] = test.type[j].holiday_end.tm_year;
				result[index][1] = test.type[j].holiday_end.tm_mon;
				result[index][2] = test.type[j].holiday_end.tm_mday;
				if(j < 7) {
					index++;
				}
			}
		}
	}
	for(int i = 0 ; i < 8 ; i++) {
		for(int j = 0 ; j < 3 ; j++) {
			TEST_ASSERT_EQUAL_INT(expected[i][j], result[i][j]);
		}
		TEST_ASSERT_EQUAL_INT(expected_result[i], result_set[i]);
	}
}

void test_parsePermanent(void)
{
	char msg[MAX_ROW] = {0};
	char good[4][DATE*4] = {{"MO"},
							{"Mo,tu"},
							{"Mo,tu,we,th,fr"},
							{"Mo,mo,MO"}};
	char bad[4][DATE*4] = {	{"Mo,tu,we,th,fr,sa,su,mo"},
							{"2019-09-18"},
							{"mo;FR"},
							{"monday"}};
	struct exclusion test;
	initExclusionStruct(&test);
	int index = 0;
	char result[8][WEEKDAYS] = {{0},{0},{0},{0},
								{0},{0},{0},{0}};
	char expected[8][WEEKDAYS] =   {{2},
									{2,3},
									{2,3,4,5,6},
									{2},
									{1,2,3,4,5,6,7},
									{0},
									{0},
									{0}};
	int result_set[8] = {0};
	int expected_result[8] = {0,0,0,0,0,-2,-2,-2};

	for(int i = 0 ; i < 4 ; i++) {
		result_set[i] = parsePermanent(good[i], &test);
		if(result_set[i] == 0) {
			if(strncmp(test.type_name[index], "perm", 4) == 0) {
				if(strncmp(test.type[index].sub_type, "solo", 4) == 0) {
					result[i][0] = test.type[i].weekdays[0];
				}
				else if(strncmp(test.type[index].sub_type, "list", 4) == 0) {
					for(int k = 0 ; k < test.type[index].list_len ; k++) {
						result[i][k] = test.type[index].weekdays[k];
					}
				}
				index++;
			}
		}
		else {
			result[i][0] = 0;
		}
	}

	for(int i = 4 ; i < 8 ; i++) {
		result_set[i] = parsePermanent(bad[i-4], &test);
		if(result_set[i] == 0) {
			if(strncmp(test.type_name[index], "perm", 4) == 0) {
				if(strncmp(test.type[index].sub_type, "solo", 4) == 0) {
					result[i][0] = test.type[i].weekdays[0];
				}
				else if(strncmp(test.type[index].sub_type, "list", 4) == 0) {
					for(int k = 0 ; k < test.type[index].list_len ; k++) {
						result[i][k] = test.type[index].weekdays[k];
					}
				}
				if(index < 7) {
					index++;
				}
			}
		}
		else {
			result[i][0] = 0;
		}
	}

	for(int i = 0 ; i < 8 ; i++) {
		for(int j = 0 ; j < 7 ; j++) {
			snprintf(msg, MAX_ROW, "%d.%d. weekdays: expect: %d was: %d",
					i, j, expected[i][j], result[i][j]);
			TEST_ASSERT_EQUAL_INT_MESSAGE(expected[i][j], result[i][j], msg);
		}
		snprintf(msg, MAX_ROW, "%d. result: expect: %d was: %d",
				i, expected_result[i], result_set[i]);
		TEST_ASSERT_EQUAL_INT_MESSAGE(expected_result[i], result_set[i], msg);
	}
}

void test_exclTokenLength(void)
{
	char good[5][11] = {{"MO"}, {"2010-11-01"}, {"2019"}, {"10"}, {"01"}};
	char bad_1[5][11] = {{"mon"}, {"2010-11-1"}, {"219"}, {"1"}, {"013"}};
	char bad_2[5][12] = {{"d"}, {"2010-011-01"}, {"20019"}, {"001"}, {"0"}};
	char types[5][6] = {{"perm"}, {"temp"}, {"year"}, {"month"}, {"day"}};
	int result_set[20] = {0};
	int len_set[20] = {0};
	int expected_result[20] = {	0, 0, 0, 0, 0,
								-1, 1, 1, 1, -1,
								1, -1, -1, -1, 1,
								2, 10, 4, 2, 2};
	int expected_len[20] =	{	2, 10, 4, 2, 2,
								3, 9, 3, 1, 3,
								1, 11, 5, 3, 1,
								0, 0, 0, 0, 0};

	for(int i = 0 ; i < 5 ; i++) {
		result_set[i] = exclTokenLength(&len_set[i], types[i], good[i]);
		result_set[i+5] = exclTokenLength(&len_set[i+5], types[i], bad_1[i]);
		result_set[i+10] = exclTokenLength(&len_set[i+10], types[i], bad_2[i]);
		result_set[i+15] = exclTokenLength(&len_set[i+15], types[i], "");
	}
	TEST_ASSERT_EQUAL_INT_ARRAY_MESSAGE(expected_result, result_set, 20,
										"result set did not match expected");
	TEST_ASSERT_EQUAL_INT_ARRAY_MESSAGE(expected_len, len_set, 20,
										"len set did not match expected");
}

void test_parseWeekday(void)
{
	char good[5][DAY] = {{"mo"},{"TH"},{"fR"},{"WE"},{"Su"}};
	char bad[5][DAY] = {{"xx"},{"ma"},{"12"},{"  "},{".."}};
	int result[10] = {0};
	int expected[10] = {2,5,6,4,1,-1,-1,-1,-1,-1};
	for(int i = 0 ; i < 5 ; i++) {
		result[i] = parseWeekday(good[i], DAY);
		result[i+5] = parseWeekday(bad[i], DAY);
	}
	TEST_ASSERT_EQUAL_INT_ARRAY(expected, result, 10);
}

void test_buildTempExclFormat(void)
{
	struct exclusion excl = {
		.amount = 2,	
		.type = {
			{
				.weekdays={0},
				.single_days = {
					{.tm_year=2019-1900,.tm_mon=10-1,.tm_mday=13,.tm_isdst=0},
					{.tm_year=2019-1900,.tm_mon=10-1,.tm_mday=15,.tm_isdst=0},
					{.tm_year=2019-1900,.tm_mon=10-1,.tm_mday=17,.tm_isdst=0}
				},
				.holiday_start = {0},
				.holiday_end = {0},
				.list_len=3,
				.sub_type = {"list"}
			},
			{
				.weekdays={0},
				.single_days = {{0}},
				.holiday_start = {.tm_year=2019-1900,.tm_mon=10-1,.tm_mday=12,.tm_isdst=0},
				.holiday_end = {.tm_year=2019-1900,.tm_mon=10-1,.tm_mday=18,.tm_isdst=0},
				.list_len=0,
				.sub_type = {"range"}
			}
		},
		.type_name = {"temp", "temp"}
	};	

	char format[2][MAX_ROW] = {{0}}; 
	char expect_format[2][MAX_ROW] = {
		"Exclude=temporary(2019-10-13,2019-10-15,2019-10-17)\n",
		"Exclude=temporary(2019-10-12#2019-10-18)\n"
	};

	for(int i = 0 ; i < 2 ; i++) {
		buildTempExclFormat(&excl.type[i], format[i]);
		TEST_ASSERT_EQUAL_STRING(expect_format[i], format[i]);
	}
}

void test_buildPermExclFormat(void)
{
	
}

void test_bubbleSort(void)
{
	int test[5][7] = {{2,3,4,7,1,5,6},
					  {7,6,5,4,3,2,1},
					  {7,2,3,4,5,6,1},
					  {7,2,3,4},
					  {2,1,4,3,6,5,7}};
	int expected[5][7] = {{1,2,3,4,5,6,7},
					  	  {1,2,3,4,5,6,7},
					  	  {1,2,3,4,5,6,7},
						  {2,3,4,7},
					  	  {1,2,3,4,5,6,7}};
	for(int i = 0 ; i < 5 ; i++) {
		bubbleSort(test[i], 7);
	}
	TEST_ASSERT_EQUAL_INT_ARRAY(expected, test, 5);
}

void test_dayToSet(void)
{
	int test[6][7] = {{3,4,7,1,5,6},
					  {1,2,3,4},
					  {1,2,3,4,5,6,7},
					  {1,2,3,4,5},
					  {1,2,3,4,5},
					  {0}};
	int expected[6][7] = {{1,2,3,4,5,6,7},
					  	  {1,2,3,4,5},
					  	  {1,2,3,4,5,6,7},
						  {1,2,3,4,5},
						  {1,2,3,4,5},
					  	  {3}};
	int element[6] = {2, 5, 6, 3, 8, 3};
	int length[6] = {6, 4, 7, 5, 5, 0};
	int result[6] = {0};
	int expected_result[6] = {0, 0, 1, 1, -1, 0};
	char message[MAX_ROW] = {0};

	for(int i = 0 ; i < 6 ; i++) {
		result[i] = dayToSet(test[i], length[i], element[i]);
	}
	for(int i = 0 ; i < 6 ; i++) {
		for(int j = 0 ; j < 7 ; j++) {
			snprintf(message, MAX_ROW, "%d.%d expect:%d, got:%d",
					 i, j, expected[i][j], test[i][j]);
			TEST_ASSERT_EQUAL_INT_MESSAGE(expected[i][j], test[i][j], message);
		}
		TEST_ASSERT_EQUAL_INT(expected_result[i], result[i]);
	}

}

/*=======MAIN=====*/
int main(void)
{
	UnityBegin("test_exclude.c");
	RUN_TEST(test_parseExclusion);
	RUN_TEST(test_parseTemporary);
	RUN_TEST(test_parsePermanent);
	RUN_TEST(test_parseDate);
	RUN_TEST(test_parseTime);
	RUN_TEST(test_parseWeekday);
	RUN_TEST(test_exclTokenLength);
	RUN_TEST(test_buildTempExclFormat);
	RUN_TEST(test_bubbleSort);
	RUN_TEST(test_dayToSet);

	return UnityEnd();
}
