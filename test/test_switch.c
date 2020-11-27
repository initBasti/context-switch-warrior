#include "../unity/src/unity.h"
#include <string.h>

#include "../source/include/switch.h"
#include "../source/include/helper.h"

#define TESTS 4

void setUp(void)
{

}

void tearDown(void)
{

}

void test_switchExclusion(void)
{
	EXCLUSION_STATE result[TESTS*2] = {0};
	EXCLUSION_STATE expected[TESTS*2] = {
										EXCLUSION_NOMATCH,
										EXCLUSION_NOMATCH,
										EXCLUSION_MATCH,
										EXCLUSION_NOMATCH,
										EXCLUSION_NOMATCH,
										EXCLUSION_MATCH,
										EXCLUSION_MATCH,
										EXCLUSION_NOMATCH
									   };
	int index = 0;
	struct tm time[4] = {
		{
			.tm_year=2017-1900, .tm_mon=9-1, .tm_mday=15, .tm_wday=5,
			.tm_hour=0, .tm_min=0, .tm_sec=0, .tm_isdst=0
		},
		{
			.tm_year=2018-1900, .tm_mon=10-1, .tm_mday=16, .tm_wday=2,
			.tm_hour=0, .tm_min=0, .tm_sec=0, .tm_isdst=0
		},
		{
			.tm_year=2019-1900, .tm_mon=9-1, .tm_mday=23, .tm_wday=7,
			.tm_hour=0, .tm_min=0, .tm_sec=0, .tm_isdst=0
		},
		{
			.tm_year=2020-1900, .tm_mon=12-1, .tm_mday=18, .tm_wday=5,
			.tm_hour=0, .tm_min=0, .tm_sec=0, .tm_isdst=0
		}
	};
	struct exclusion test[2] = {
		{
			.type[0]={.weekdays={1,2,3}, .list_len=3, .sub_type={"list"}},
			.type[1]={
				.single_days[0]=
				{
					.tm_year=2019-1900, .tm_mon=9-1, .tm_mday=20, .tm_wday=0,
					.tm_hour=0, .tm_min=0, .tm_sec=0, .tm_isdst=0
				}, 
				.single_days[1]=
				{
					.tm_year=2020-1900, .tm_mon=12-1, .tm_mday=18, .tm_wday=0,
					.tm_hour=0, .tm_min=0, .tm_sec=0, .tm_isdst=0
				}, 
			.list_len=2, .sub_type={"list"}},
		 	.type_name[0]={"perm"},
		 	.type_name[1]={"temp"},
			.amount = 2
		},
		{
			.type[0]={
					.single_days[0]=
					{
						.tm_year=2019-1900, .tm_mon=9-1, .tm_mday=15, .tm_wday=0,
						.tm_hour=0, .tm_min=0, .tm_sec=0, .tm_isdst=0
					}, 
					.single_days[1]=
					{
						.tm_year=2019-1900, .tm_mon=8-1, .tm_mday=25, .tm_wday=0,
						.tm_hour=0, .tm_min=0, .tm_sec=0, .tm_isdst=0
					}, 
					.single_days[2]=
					{
						.tm_year=2019-1900, .tm_mon=7-1, .tm_mday=25, .tm_wday=0,
						.tm_hour=0, .tm_min=0, .tm_sec=0, .tm_isdst=0
					},
					.list_len=3, .sub_type={"list"}
			},
			.type[1]={
					.holiday_start=
					{
						.tm_year=2019-1900, .tm_mon=9-1, .tm_mday=20, .tm_wday=0,
						.tm_hour=0, .tm_min=0, .tm_sec=0, .tm_isdst=0
					}, 
					.holiday_end=
					{
						.tm_year=2019-1900, .tm_mon=9-1, .tm_mday=25, .tm_wday=0,
						.tm_hour=0, .tm_min=0, .tm_sec=0, .tm_isdst=0
					}, 
				.sub_type={"range"}
			},
		 	.type_name[0]={"temp"},
		 	.type_name[1]={"temp"},
			.amount = 2
		}
	};

	for(int i = 0 ; i < TESTS ; i++) {
		result[index] = switchExclusion(&test[0], &time[i]);
		index++;
		result[index] = switchExclusion(&test[1], &time[i]);
		index++;
	}

	TEST_ASSERT_EQUAL_INT_ARRAY(expected, result, TESTS*2);
}

void test_switchContext(void)
{
	struct config config[TESTS/2] = {
	{.zone_name[0] = {"Test1"},
	 .ztime[0] = {.start_hour = 5, .start_minute = 15,
		 		  .end_hour = 7, .end_minute = 30},
	 .zone_context[0] = {"work"},
	 .zone_name[1] = {"Test2"},
	 .ztime[1] = {.start_hour = 8, .start_minute = 0,
	 			  .end_hour = 9, .end_minute = 0},
	 .zone_context[1] = {"freetime"},
	 .zone_name[2] = {"Test3"},
	 .ztime[2] = {.start_hour = 10, .start_minute = 0,
	 			  .end_hour = 11, .end_minute = 30},
	 .zone_context[2] = {"study"},
	 .zone_name[3] = {"Test4"},
	 .ztime[3] = {.start_hour = 12, .start_minute = 0,
	 			  .end_hour = 12, .end_minute = 45},
	 .zone_context[3] = {"work"},
	 .zone_amount = 4
	},
	{.zone_name[0] = {"Test1"},
	 .ztime[0] = {.start_hour = 5, .start_minute = 15,
		 		  .end_hour = 8, .end_minute = 30},
	 .zone_context[0] = {"work"},
	 .zone_name[1] = {"Test2"},
	 .ztime[1] = {.start_hour = 10, .start_minute = 0,
	 			  .end_hour = 10, .end_minute = 0},
	 .zone_context[1] = {"freetime"},
	 .zone_name[2] = {"Test3"},
	 .ztime[2] = {.start_hour = 14, .start_minute = 0,
	 			  .end_hour = 16, .end_minute = 30},
	 .zone_context[2] = {"study"},
	 .zone_name[3] = {"Test4"},
	 .ztime[3] = {.start_hour = 10, .start_minute = 0,
	 			  .end_hour = 10, .end_minute = 45},
	 .zone_context[3] = {"work"},
	 .zone_amount = 4
	}
	};
	int index = 0;
	int min[TESTS*2] = {315, 605, 903, 150, 366, 480, 550, 735};
	char example_context[TESTS/2][MAX_COMMAND] = {"work", "study"};
	char command[TESTS*8][MAX_COMMAND] = {{0}};
	char expected_command[TESTS*8][MAX_COMMAND] = {{"none"},{"none"},
												   {"work"},{"work"},
										  		   {"study"},{"none"},
												   {"none"},{"work"},
										  		   {"none"},{"none"},
												   {"none"},{"study"},
												   {"none"},{"none"},
												   {"none"},{"none"},
										  		   {"none"},{"none"},
												   {"work"},{"study"},
										  		   {"freetime"},{"none"},
												   {"freetime"},{"study"},
										  		   {"none"},{"none"},
												   {"none"},{"none"},
										  		   {"none"},{"none"},
												   {"work"},{"none"}};
	SWITCH_STATE result[TESTS*8] = {0};
	SWITCH_STATE expected[TESTS*8] = {SWITCH_NOTNEEDED, SWITCH_NOTNEEDED,
									  SWITCH_SUCCESS, SWITCH_SUCCESS,
									  SWITCH_SUCCESS, SWITCH_NOTNEEDED,
									  SWITCH_NOTNEEDED, SWITCH_SUCCESS,
									  SWITCH_FAILURE, SWITCH_SUCCESS,
									  SWITCH_FAILURE, SWITCH_NOTNEEDED,
									  SWITCH_FAILURE, SWITCH_FAILURE,
									  SWITCH_FAILURE, SWITCH_FAILURE,
									  SWITCH_NOTNEEDED, SWITCH_NOTNEEDED,
									  SWITCH_SUCCESS, SWITCH_SUCCESS,
									  SWITCH_SUCCESS, SWITCH_NOTNEEDED,
									  SWITCH_SUCCESS, SWITCH_SUCCESS,
									  SWITCH_FAILURE, SWITCH_FAILURE,
									  SWITCH_FAILURE, SWITCH_FAILURE,
									  SWITCH_NOTNEEDED, SWITCH_FAILURE,
									  SWITCH_SUCCESS, SWITCH_FAILURE};
	for(int i = 0 ; i < TESTS*2 ; i++) {
		for(int j = 0 ; j < 2; j++) {
			result[index] = switchContext(&config[0], min[i],
										  command[index],
										  example_context[j]);
			index++;
			result[index] = switchContext(&config[1], min[i],
										  command[index],
										  example_context[j]);
			index++;
		}
	}
	TEST_ASSERT_EQUAL_INT_ARRAY(expected, result, TESTS*8);
	for(int i = 0 ; i<TESTS*8 ; i++) {
		TEST_ASSERT_EQUAL_STRING(expected_command, command);
	}
}

void test_rangeMatch(void)
{
	int index = 0;
	int result[TESTS*3] = {0};
	int expected[TESTS*3] = {1, 1, -1, 0, 1, -1, 1, 1, -1, 1, 0, -1};
	struct tm date[TESTS] = {
		{.tm_year=2019-1900, .tm_mon=10-1, .tm_mday=20},
		{.tm_year=2019-1900, .tm_mon=10-1, .tm_mday=21},
		{.tm_year=2018-1900, .tm_mon=10-1, .tm_mday=22},
		{.tm_year=2020-1900, .tm_mon=8-1, .tm_mday=16}
	};
	struct format_type test[2] = {
		{
			.holiday_start = {.tm_year = 2019-1900, .tm_mon = 10-1, .tm_mday = 21},
			.holiday_end = {.tm_year = 2019-1900, .tm_mon = 10-1, .tm_mday = 24}
		},
		{
			.holiday_start = {.tm_year = 2020-1900, .tm_mon = 8-1, .tm_mday = 16},
			.holiday_end = {.tm_year = 2020-1900, .tm_mon = 8-1, .tm_mday = 16}
		}
	};

	for(int i = 0 ; i < TESTS ; i++) {
		result[index++] = rangeMatch(&test[0], &date[i]);
		result[index++] = rangeMatch(&test[1], &date[i]);
		result[index++] = rangeMatch(NULL, &date[i]);
	}
	TEST_ASSERT_EQUAL_INT_ARRAY(expected, result, TESTS*3);
}

/*=======MAIN=====*/
int main(void)
{
	UnityBegin("test_switch.c");
	RUN_TEST(test_switchExclusion);
	RUN_TEST(test_switchContext);
	RUN_TEST(test_rangeMatch);

	return UnityEnd();
}
