#include "../unity/src/unity.h"
#include <string.h>

#include "../source/include/config.h"
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

void test_stripChar(void)
{
	char test[7][DATE+4] = {
		{" m o "}, {"2019-03 -10"}, {"h a us,"}, {"   "},
		{"2019-10-03"}, {"    ber t    "}, {"mo, tu, we, th"}
	};
	char expect_string[7][DATE+2] = {
		{"mo"}, {"2019-03-10"}, {"haus,"}, {""},
		{"2019-10-03"}, {"bert"}, {"mo,tu,we,th"}
	};
	int result_size[7] = {0};
	int expect_size[7] = {2, 10, 5, 0, 10, 4, 11};
	char sep = ' ';

	for(int i = 0 ; i < 7 ; i++) {
		stripChar(&test[i][0], sep);
		result_size[i] = strnlen(test[i], MAX_ROW); 
	}
	TEST_ASSERT_EQUAL_INT_ARRAY_MESSAGE(expect_size, result_size, 7,
					    "size doesnt match expection");
	for(int j = 0 ; j < 7 ; j++)
		TEST_ASSERT_EQUAL_STRING(expect_string[j], test[j]);
}

void test_contextValidation(void)
{
	struct context option = {.name={{"work"},{"freetime"}},.amount=2};
	char good_option[5] = {"work"};
	char bad_option[4] = {"red"};

	TEST_ASSERT_EQUAL_INT_MESSAGE(0, contextValidation(&option, &good_option[0]),
				      "The context work should be found!");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, contextValidation(&option, &bad_option[0]),
				      "red should not return 1 in (work,free)");
	TEST_ASSERT_EQUAL_INT_MESSAGE(1, contextValidation(&option, ""),
				      "empty input should return 1");
}

#define ZONE_TEST 5
void test_zoneValidation(void)
{
	char *name[ZONE_TEST][MAX_ROW] = {
		{"test1"}, {""}, {"test2"}, {"test3"},
		{"aaaaaaaaaaaabbbbbbbbbbbbbcccccccccccccccdddddddddddddddddeeeeeeeeeffffffffffffffffffffffffffggggggggggggggggggggggggggggggggghhhhhhhhhhhhhhhhhhh"}
	};
	struct zonetime t_t[ZONE_TEST] = {
		{.start_hour=5,.start_minute=30,.end_hour=6,.end_minute=45},
		{.start_hour=0,.start_minute=0,.end_hour=6,.end_minute=45},
		{.start_hour=15,.start_minute=0,.end_hour=0,.end_minute=0},
		{.start_hour=25,.start_minute=00,.end_hour=6,.end_minute=45},
		{.start_hour=5,.start_minute=65,.end_hour=6,.end_minute=45},
	};
	char *context[ZONE_TEST][MAX_CONTEXT] = {
		{"work"}, {""}, {""}, {"study"}, {"freetime"}
	};

	int result[ZONE_TEST*ZONE_TEST*ZONE_TEST] = {0};
	int expect[ZONE_TEST*ZONE_TEST*ZONE_TEST] = {
		0,-1, -1, 0, 0,
		-1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1,
		0,-1, -1, 0, 0,
		-1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1,
		0,-1, -1, 0, 0,
		-1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1,
		-1, -1, -1, -1, -1,
	};

	for(int i = 0 ; i < ZONE_TEST ; i++) {
		for(int j = 0 ; j < ZONE_TEST ; j++) {
			for(int k = 0 ; k < ZONE_TEST ; k++) {
				result[(i*(ZONE_TEST*ZONE_TEST))+(j*ZONE_TEST)+k] =
					zoneValidation(name[i][0], &t_t[j], context[k][0]);
			}
		}
	}

	TEST_ASSERT_EQUAL_INT_ARRAY(expect, result, ZONE_TEST*ZONE_TEST*ZONE_TEST);
}

#define INC_TEST 6
void test_increaseTime(void)
{
	struct tm time[INC_TEST] = {
		{.tm_year=2019,.tm_mon=11,.tm_mday=13,.tm_hour=22,.tm_min=10},
		{.tm_year=2020,.tm_mon=1,.tm_mday=13,.tm_hour=0,.tm_min=10},
		{.tm_year=2020,.tm_mon=7,.tm_mday=18,.tm_hour=15,.tm_min=10},
		{.tm_year=2020,.tm_mon=1,.tm_mday=4,.tm_hour=14,.tm_min=22},
		{.tm_year=2019,.tm_mon=12,.tm_mday=31,.tm_hour=20,.tm_min=10},
		{.tm_year=2019,.tm_mon=12,.tm_mday=31,.tm_hour=20,.tm_min=10}
	};
	int minute[INC_TEST] = {5,10,30,60,360,6000};
	struct tm expect_time[INC_TEST] = {
		{.tm_year=2019,.tm_mon=11,.tm_mday=13,.tm_hour=22,.tm_min=15},
		{.tm_year=2020,.tm_mon=1,.tm_mday=13,.tm_hour=0,.tm_min=20},
		{.tm_year=2020,.tm_mon=7,.tm_mday=18,.tm_hour=15,.tm_min=40},
		{.tm_year=2020,.tm_mon=1,.tm_mday=04,.tm_hour=15,.tm_min=22},
		{.tm_year=2020,.tm_mon=1,.tm_mday=1,.tm_hour=2,.tm_min=10},
		{.tm_year=2020,.tm_mon=1,.tm_mday=5,.tm_hour=0,.tm_min=10}
	};

	for(int i = 0 ; i < INC_TEST ; i++) {
		increaseTime(minute[i], &time[i]);
		TEST_ASSERT_EQUAL_INT(expect_time[i].tm_year,
				time[i].tm_year);
		TEST_ASSERT_EQUAL_INT(expect_time[i].tm_mon,
				time[i].tm_mon);
		TEST_ASSERT_EQUAL_INT(expect_time[i].tm_mday,
				time[i].tm_mday);
		TEST_ASSERT_EQUAL_INT(expect_time[i].tm_hour,
				time[i].tm_hour);
		TEST_ASSERT_EQUAL_INT(expect_time[i].tm_min,
				time[i].tm_min);
	}
}

void test_compareTime(void)
{
	struct tm curr_time[8] = {
		{
			.tm_year=2019-1900, .tm_mon=10, .tm_mday=20,
			.tm_hour=9, .tm_min=30, .tm_sec=0, .tm_isdst=0
		},
		{
			.tm_year=2019-1900, .tm_mon=10, .tm_mday=20,
			.tm_hour=9, .tm_min=30, .tm_sec=0, .tm_isdst=0
		},
		{
			.tm_year=2019-1900, .tm_mon=10, .tm_mday=20,
			.tm_hour=9, .tm_min=30, .tm_sec=0, .tm_isdst=0
		},
		{
			.tm_year=2020-1900, .tm_mon=11, .tm_mday=5,
			.tm_hour=13, .tm_min=30, .tm_sec=0, .tm_isdst=0
		},
		{
			.tm_year=2020-1900, .tm_mon=11, .tm_mday=5,
			.tm_hour=13, .tm_min=30, .tm_sec=0, .tm_isdst=0
		},
		{
			.tm_year=2020-1900, .tm_mon=11, .tm_mday=5,
			.tm_hour=13, .tm_min=30, .tm_sec=0, .tm_isdst=0
		},
		{
			.tm_year=2019-1900, .tm_mon=3, .tm_mday=25,
			.tm_hour=22, .tm_min=10, .tm_sec=0, .tm_isdst=0
		},
		{
			.tm_year=0, .tm_mon=0, .tm_mday=0,
			.tm_hour=0, .tm_min=0, .tm_sec=0, .tm_isdst=0
		}
	};
	struct tm spec_time[8] = {
		// bigger hour
		{
			.tm_year=2019-1900, .tm_mon=10, .tm_mday=20,
			.tm_hour=10, .tm_min=30, .tm_sec=0, .tm_isdst=0
		},
		// bigger day
		{
			.tm_year=2019-1900, .tm_mon=10, .tm_mday=21,
			.tm_hour=9, .tm_min=30, .tm_sec=0, .tm_isdst=0
		},
		// bigger month
		{
			.tm_year=2019-1900, .tm_mon=11, .tm_mday=20,
			.tm_hour=9, .tm_min=30, .tm_sec=0, .tm_isdst=0
		},
		// smaller minute
		{
			.tm_year=2020-1900, .tm_mon=11, .tm_mday=5,
			.tm_hour=13, .tm_min=15, .tm_sec=0, .tm_isdst=0
		},
		// smaller hour
		{
			.tm_year=2020-1900, .tm_mon=11, .tm_mday=5,
			.tm_hour=12, .tm_min=30, .tm_sec=0, .tm_isdst=0
		},
		// equal
		{
			.tm_year=2020-1900, .tm_mon=11, .tm_mday=5,
			.tm_hour=13, .tm_min=30, .tm_sec=0, .tm_isdst=0
		},
		// no specified time
		{
			.tm_year=0, .tm_mon=0, .tm_mday=0,
			.tm_hour=0, .tm_min=0, .tm_sec=0, .tm_isdst=0
		},
		// no current time
		{
			.tm_year=2019-1900, .tm_mon=3, .tm_mday=25,
			.tm_hour=22, .tm_min=10, .tm_sec=0, .tm_isdst=0
		}
	};
	TIME_CMP result[8] = {0};
	TIME_CMP expect[8] = {
		TIME_BIGGER,
		TIME_BIGGER,
		TIME_BIGGER,
		TIME_SMALLER,
		TIME_SMALLER,
		TIME_EQUAL,
		TIME_SMALLER,
		TIME_ERROR
	};

	for(int i = 0 ; i < 8 ; i++)
		result[i] = compareTime(&curr_time[i], &spec_time[i]);

	TEST_ASSERT_EQUAL_INT_ARRAY(expect, result, 8);
}


void test_getDate(void)
{
	time_t test_time[4] = {406747800, 915196200, 1602538200, 698770800};
	int result[4] = {0};
	int expected[4] = {0};
	char msg[100] = {0};
	struct tm test_tm[4] = {{.tm_year=0, .tm_mon=0, .tm_mday=0}};
	struct tm expect_tm[4] = {
		{
			.tm_year=1982-1900,
			.tm_mon=11-1,
			.tm_mday=21
		},
		{
			.tm_year=1999-1900,
			.tm_mon=1-1,
			.tm_mday=1
		},
		{
			.tm_year=2020-1900,
			.tm_mon=10-1,
			.tm_mday=12
		},
		{
			.tm_year=1992-1900,
			.tm_mon=2-1,
			.tm_mday=22
		}
	};

	for(int i = 0 ; i < 4 ; i++)
		result[i] = getDate(&test_tm[i], test_time[i]);

	TEST_ASSERT_EQUAL_INT_ARRAY(expected, result, 4);
	for(int i = 0 ; i < 4 ; i++) {
		snprintf(msg, 100, "expect: %4d-%02d-%02d was %04d-%02d-%02d",
				expect_tm[i].tm_year, expect_tm[i].tm_mon,
				expect_tm[i].tm_mday, test_tm[i].tm_year, test_tm[i].tm_mon,
				test_tm[i].tm_mday); 
		TEST_ASSERT_EQUAL_INT_MESSAGE(expect_tm[i].tm_year,
									  test_tm[i].tm_year,
									  msg);	
		TEST_ASSERT_EQUAL_INT_MESSAGE(expect_tm[i].tm_mon,
									  test_tm[i].tm_mon,
									  msg);	
		TEST_ASSERT_EQUAL_INT_MESSAGE(expect_tm[i].tm_mday,
									  test_tm[i].tm_mday,
									  msg);	
	}
}

#define PAR_TEST 9
void test_parseTimeSpan(void)
{
	char test_string[PAR_TEST][DELAY_FORMAT_LEN] = {
		"30min", "1h", "abc", "0.5h", "1,2h",
		"-10min", "0", "0.5d", "10"
	};
	int result[PAR_TEST] =  {0};
	int expected[PAR_TEST] = {30, 60, -1, 30, 72, -1, -1, 720, 10};

	for(int i = 0 ; i < PAR_TEST ; i++)
		result[i] = parseTimeSpan(test_string[i]);

	TEST_ASSERT_EQUAL_INT_ARRAY(expected, result, PAR_TEST);
}

#define MUL_TEST 11
void test_multiplierForType(void)
{
	char test_input[MUL_TEST][DELAY_FORMAT_LEN] = {
		"h", "hour", "m", "min", "minute",
		"d", "day", "week", "haur", "123", "\0"
	};
	int result[MUL_TEST] = {0};
	int expected[MUL_TEST] = {60, 60, 1, 1, 1, 1440, 1440, -1, -1, -1, -1};

	for(int i = 0 ; i < MUL_TEST ; i++)
		result[i] = multiplierForType(test_input[i]);

	TEST_ASSERT_EQUAL_INT_ARRAY(expected, result, MUL_TEST);
}


/*=======MAIN=====*/
int main(void)
{
	UnityBegin("test_helper.c");
	RUN_TEST(test_stripChar);
	RUN_TEST(test_contextValidation);
	RUN_TEST(test_zoneValidation);
	RUN_TEST(test_increaseTime);
	RUN_TEST(test_compareTime);
	RUN_TEST(test_multiplierForType);
	RUN_TEST(test_parseTimeSpan);

	return UnityEnd();
}
