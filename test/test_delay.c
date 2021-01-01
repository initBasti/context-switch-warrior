#define _DEFAULT_SOURCE
#include "../unity/src/unity.h"
#include "../source/include/delay.h"
#include <string.h>
#include <time.h>

void setUp(void)
{
	// pass
}
void tearDown(void)
{
	// pass	
}

#define DELAY_FORM_TEST 6
void test_buildDelayFormat(void)
{
	struct tm delay[DELAY_FORM_TEST] = {
		{
			.tm_year=2019-1900,.tm_mon=12-1,.tm_mday=30,
			.tm_hour=12,.tm_min=30,.tm_sec=0,.tm_isdst=0
		},
		{
			.tm_year=0,.tm_mon=0,.tm_mday=0,
			.tm_hour=0,.tm_min=0,.tm_sec=0,.tm_isdst=0
		},
		{
			.tm_year=2020-1900,.tm_mon=10-1,.tm_mday=15,
			.tm_hour=10,.tm_min=4,.tm_sec=0,.tm_isdst=0
		},
		{
			.tm_year=2020-1900,.tm_mon=3-1,.tm_mday=2,
			.tm_hour=7,.tm_min=4,.tm_sec=0,.tm_isdst=0
		}
	};
	char format[DELAY_FORM_TEST][DELAY_FORMAT_LEN+7] = {{0}};
	char expect_format[DELAY_FORM_TEST][MAX_ROW] = {
		{"Delay=2019-12-30T12:30Z\n"},
		{0},
		{"Delay=2020-10-15T10:04Z\n"},
		{"Delay=2020-03-02T07:04Z\n"}
	};


	for(int i = 0 ; i < DELAY_FORM_TEST ; i++) {
		timegm(&delay[i]);
		buildDelayFormat(&delay[i], format[i]);
		TEST_ASSERT_EQUAL_STRING(expect_format[i], format[i]);
	}
}

#define DELAY_CHECK_TEST 6
void test_checkDelay(void)
{
	DELAY_CHECK result[DELAY_CHECK_TEST] = {0};	
	DELAY_CHECK expect[DELAY_CHECK_TEST] = {
		VALID, VALID_PLUS_NEW, INVALID,
		INVALID_PLUS_NEW, ERROR, VALID
	};

	int flag[DELAY_CHECK_TEST] = {0, 15, 0, 30, 0, 0};
	struct tm delay[DELAY_CHECK_TEST] = {
		{.tm_year=2019,.tm_mon=11,.tm_mday=10,.tm_hour=5,.tm_min=30},
		{.tm_year=2020,.tm_mon=11,.tm_mday=10,.tm_hour=5,.tm_min=30},
		{.tm_year=2020,.tm_mon=11,.tm_mday=10,.tm_hour=5,.tm_min=30},
		{.tm_year=2020,.tm_mon=11,.tm_mday=10,.tm_hour=5,.tm_min=30},
		{.tm_year=2020,.tm_mon=11,.tm_mday=10,.tm_hour=5,.tm_min=30},
		{.tm_year=0,.tm_mon=0,.tm_mday=0,.tm_hour=0,.tm_min=0}
	};
	struct tm time[DELAY_CHECK_TEST] = {
		{.tm_year=2019,.tm_mon=11,.tm_mday=10,.tm_hour=5,.tm_min=0},
		{.tm_year=2020,.tm_mon=11,.tm_mday=9,.tm_hour=5,.tm_min=30},
		{.tm_year=2020,.tm_mon=11,.tm_mday=10,.tm_hour=6,.tm_min=30},
		{.tm_year=2020,.tm_mon=11,.tm_mday=11,.tm_hour=5,.tm_min=30},
		{.tm_year=0,.tm_mon=0,.tm_mday=0,.tm_hour=0,.tm_min=0},
		{.tm_year=2020,.tm_mon=11,.tm_mday=10,.tm_hour=5,.tm_min=30}
	};

	for(int i = 0 ; i < DELAY_CHECK_TEST ; i++)
		result[i] = checkDelay(flag[i], &delay[i], &time[i]);

	TEST_ASSERT_EQUAL_INT_ARRAY(expect, result, DELAY_CHECK_TEST);
}

#define DELAY_TEST 10
void test_parseDelay(void)
{
	char msg[MAX_ROW] = {0};

	char test_str[DELAY_TEST][DELAY_FORMAT_LEN+2] = {
		"2019-12-10T15:00Z", "2020-01-01T01:00Z",
		"2020-01-02T00:00Z", "2019-14-12T13:23Z",
		"2019--01-02T10:00Z", "2019-2-3T22:00Z",
		"2019-12-24T25:00Z", "12-14-2019T12:30Z",
		"correctforTmmeepZ", ""
	};

	int result[DELAY_TEST] = {0};
	int expected[DELAY_TEST] = {0,0,0,-1,-2,0,-1,-2,-2,-2};

	struct tm test_delay[DELAY_TEST] = {
		{
			.tm_year=0, .tm_mon=0, .tm_mday=0, .tm_hour=0, .tm_min=0
		}
	};

	struct tm expect_delay[DELAY_TEST] = {
		//0
		{
			.tm_year=119, .tm_mon=11, .tm_mday=10, .tm_hour=15, .tm_min=0
		},
		//1
		{
			.tm_year=120, .tm_mon=0, .tm_mday=1, .tm_hour=1, .tm_min=0
		},
		//2
		{
			.tm_year=120, .tm_mon=0, .tm_mday=2, .tm_hour=0, .tm_min=0
		},
		//3
		{
			.tm_year=0, .tm_mon=0, .tm_mday=0, .tm_hour=0, .tm_min=0
		},
		//4
		{
			.tm_year=0, .tm_mon=0, .tm_mday=0, .tm_hour=0, .tm_min=0
		},
		//5
		{
			.tm_year=119, .tm_mon=1, .tm_mday=3, .tm_hour=22, .tm_min=0
		},
		//6
		{
			.tm_year=0, .tm_mon=0, .tm_mday=0, .tm_hour=0, .tm_min=0
		},
		//7
		{
			.tm_year=0, .tm_mon=0, .tm_mday=0, .tm_hour=0, .tm_min=0
		},
		//8
		{
			.tm_year=0, .tm_mon=0, .tm_mday=0, .tm_hour=0, .tm_min=0
		},
		//9
		{
			.tm_year=0, .tm_mon=0, .tm_mday=0, .tm_hour=0, .tm_min=0
		}
	};

	for(int i = 0 ; i < DELAY_TEST ; i++)
		result[i] = parseDelay(&test_delay[i], test_str[i]);

	for(int i = 0 ; i < DELAY_TEST ; i++) {
		snprintf(msg, MAX_ROW, "%s result exp: %d was: %d",
				test_str[i], expected[i], result[i]);
		TEST_ASSERT_EQUAL_INT_MESSAGE(expected[i], result[i], msg);
		snprintf(msg, MAX_ROW, "%d year exp: %d was: %d",
				i, expect_delay[i].tm_year, test_delay[i].tm_year);
		TEST_ASSERT_EQUAL_INT_MESSAGE(expect_delay[i].tm_year,
				test_delay[i].tm_year, msg);
		snprintf(msg, MAX_ROW, "%d mon exp: %d was: %d",
				i, expect_delay[i].tm_mon, test_delay[i].tm_mon);
		TEST_ASSERT_EQUAL_INT_MESSAGE(expect_delay[i].tm_mon,
				test_delay[i].tm_mon, msg);
		snprintf(msg, MAX_ROW, "%d mday exp: %d was: %d",
				i, expect_delay[i].tm_mday, test_delay[i].tm_mday);
		TEST_ASSERT_EQUAL_INT_MESSAGE(expect_delay[i].tm_mday,
				test_delay[i].tm_mday, msg);
		snprintf(msg, MAX_ROW, "%d hour exp: %d was: %d",
				i, expect_delay[i].tm_hour, test_delay[i].tm_hour);
		TEST_ASSERT_EQUAL_INT_MESSAGE(expect_delay[i].tm_hour,
				test_delay[i].tm_hour, msg);
		snprintf(msg, MAX_ROW, "%d min exp: %d was: %d",
				i, expect_delay[i].tm_min, test_delay[i].tm_min);
		TEST_ASSERT_EQUAL_INT_MESSAGE(expect_delay[i].tm_min,
				test_delay[i].tm_min, msg);
	}
}

/*=======MAIN=====*/
int main(void)
{
  UnityBegin("test_delay.c");
  RUN_TEST(test_checkDelay);
  RUN_TEST(test_buildDelayFormat);

  return UnityEnd();
}
