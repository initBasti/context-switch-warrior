#include "../unity/src/unity.h"
#include <string.h>

#include "../source/include/config.h"

#define TESTS 5
extern char *getcwd(char *, size_t);
extern char *realpath(const char*, char*);

char *out_path = NULL;


void setUp(void)
{
	out_path = malloc(sizeof(char)*PATH_MAX);
	if(!out_path)
		return;
}

void tearDown(void)
{
	free(out_path);
}

void test_dirExist_good(void)
{
	TEST_ASSERT_EQUAL_INT(0, dirExist("/home/"));
}

void test_dirExist_bad(void)
{
	TEST_ASSERT_EQUAL_INT(-1, dirExist("/failure/"));
}

void test_dirExist_is_file(void)
{
	TEST_ASSERT_EQUAL_INT(1, dirExist("/usr/include/glib-2.0/glib.h"));
}

void test_findConfig_good(void)
{
	int fd = 0;
	int write_size = 0;
	int test_len = 0;
	char *username = NULL;
	char pathname[12] = {"test_config"};
	char path[PATH_MAX] = {0};
	char buffer[MAX_ROW] = {"test,test,test,test\n"};
	int size_user = 0;

	username = getenv("USER");
	if(!username)
		return;

	size_user = strlen(username);

	strncpy(path, "/home/", PATH_MAX);
	strncat(path, username, size_user);
	strncat(path, "/.task/csw/", 12);
	strncat(path, pathname, 12);

	fd = open(path, O_RDWR|O_CREAT, 0777);
	if(fd < 0)
		perror("file creation failed");

	for(int j = 0 ; j < MAX_ZONES ; j++) {
		test_len = strnlen(buffer, MAX_ROW);
		write_size = write(fd, buffer, test_len);
		if(write_size < 1)
			perror("write fail");
	}
	if(close(fd) < 0) {
		perror("file closing failed");
		return;
	}
	FILE_STATE status = 0;
	status = findConfig(pathname, out_path);
	TEST_ASSERT_EQUAL_INT(FILE_GOOD, status);
	TEST_ASSERT_EQUAL_STRING(path, out_path);
}

void test_findConfig_notfound(void)
{
	char wrong_name[18] = {"wrong_config.conf"};
	memset(out_path, 0, PATH_MAX);
	FILE_STATE status = 0;
	status = findConfig(wrong_name, out_path);
	TEST_ASSERT_EQUAL_INT(FILE_NOTFOUND, status);
	TEST_ASSERT_EQUAL_STRING(out_path, "");
}

void test_readConfig(void)
{
	char buffer [MAX_AMOUNT_OPTIONS][MAX_ROW] = {
		{"ZONE=Test;START=15:30;END=16:30;CONTEXT=work\n"}, /* 0 */
		{"ZONE=Test2;START=16:30;END=17:30;CONTEXT=study\n"}, /* 1 */
		{"ZONE=Test3;START=17:30;END=18:30;CONTEXT=freetime\n"}, /* 2 */
		{"interval=5min\n"}, /* 3 */
		{"permanent_invisibility=yes\n"}, /* 4 */
		{"Exclude=permanent(mo, th)\n"}, /* 5 */
		{"Exclude=temporary(2019-10-15)\n"}, /* 6 */
		{"cancel=on\n"}, /* 7 */
		{"notify=on\n"}, /* 8 */
		{"freedom=0\n"}, /* 9 */
		{"delay=2019-09-14T09:00Z\n"} /* 10 */
	};
	char config_path[PATH_MAX] = {0};
	CONFIG_STATE result = 0;
	CONFIG_STATE expect_result = 0;
	char msg[1100] = {0};
	struct configcontent test = {
		.rowindex={0},
		.option_name={{{0}}},
		.option_value={{{0}}},
		.sub_option_amount={0}
	};

	struct error test_error = {
		.amount = 0,
		.rowindex = {0},
		.error_code = {0},
		.error_msg = {{0}}
	};

	struct configcontent expect = {
		.amount=9,
		.rowindex={0,1,2,3,5,6,7,8,10},
		.option_name={
			{"zone", "start", "end", "context"},
			{"zone", "start", "end", "context"},
			{"zone", "start", "end", "context"},
			{"interval"},
			{"exclude"},
			{"exclude"},
			{"cancel"},
			{"notify"},
			{"delay"}
		},
		.option_value={
			{"Test", "15:30", "16:30", "work"},
			{"Test2", "16:30", "17:30", "study"},
			{"Test3", "17:30", "18:30", "freetime"},
			{"5min"},
			{"permanent(mo, th)"},
			{"temporary(2019-10-15)"},
			{"on"},
			{"on"},
			{"2019-09-14T09:00Z"}
		},
		.sub_option_amount={4,4,4,1,1,1,1,1,1}
	};

	struct error error_expect = {
		.amount=2,
		.rowindex={4,9},
		.error_msg = {
			"Title in permanent_invisibility=yes not valid",
			"No value found in freedom=0"
		},
		.error_code={-2,-1}
	};

	FILE *test_file = NULL;
	char *username = NULL;
	int size_user = 0;

	username = getenv("USER");
	size_user = strlen(username);

	/* Create the test files */
	strncpy(config_path, "/home/", PATH_MAX);
	strncat(config_path, username, size_user);
	strncat(config_path, "/.task/csw/.test_read_config", 30);

	test_file = fopen(config_path, "w");
	if(!test_file)
		perror("file creation failed");

	for(int j = 0 ; j < MAX_AMOUNT_OPTIONS ; j++) {
		if(buffer[j][0] != '\0')
			fprintf(test_file, "%s", buffer[j]);
	}
	rewind(test_file);
	if(fclose(test_file) == EOF) {
		perror("file closing failed");
		return;
	}

	/* Execute commands and test result */
	result = readConfig(&test, &test_error, config_path);

	for(int j = 0 ; j < MAX_AMOUNT_OPTIONS ; j++) {
		snprintf(msg, 100, "[%d] rowindex exp:%d was:%d",
				j, expect.rowindex[j], test.rowindex[j]);
		TEST_ASSERT_EQUAL_INT_MESSAGE(expect.rowindex[j],
										test.rowindex[j],
										msg);
		snprintf(msg, 100, "error rowindex exp:%d was:%d",
				error_expect.rowindex[j], test_error.rowindex[j]);
		TEST_ASSERT_EQUAL_INT_MESSAGE(error_expect.rowindex[j],
										test_error.rowindex[j],
										msg);
		snprintf(msg, 100, "error code exp:%d was:%d",
				error_expect.rowindex[j], test_error.rowindex[j]);
		TEST_ASSERT_EQUAL_INT_MESSAGE(error_expect.error_code[j],
										test_error.error_code[j],
										msg);
		snprintf(msg, 1100, "error msg exp:%s was:%s",
				error_expect.error_msg[j], test_error.error_msg[j]);
		TEST_ASSERT_EQUAL_STRING_MESSAGE(error_expect.error_msg[j],
										test_error.error_msg[j],
										msg);
		snprintf(msg, 100, "[%d] sub_option_amount exp:%d was:%d",
				j, expect.sub_option_amount[j],
				test.sub_option_amount[j]);
		TEST_ASSERT_EQUAL_INT_MESSAGE(expect.sub_option_amount[j],
										test.sub_option_amount[j],
										msg);
		for(int k = 0 ; k < MAX_SUBOPTIONS ; k++) {
			snprintf(msg, 100, "option_name exp:%s was:%s",
					expect.option_name[j][k],
					test.option_name[j][k]);
			TEST_ASSERT_EQUAL_STRING_MESSAGE(expect.option_name[j][k],
											test.option_name[j][k],
											msg);
			snprintf(msg, 300, "option_value exp:%s was:%s",
					expect.option_value[j][k],
					test.option_value[j][k]);
			TEST_ASSERT_EQUAL_STRING_MESSAGE(expect.option_value[j][k],
											test.option_value[j][k],
											msg);
		}
	}
	TEST_ASSERT_EQUAL_INT_ARRAY(expect_result, result, TESTS);
	snprintf(msg, 100, "amount exp:%d was:%d",
			expect.amount, test.amount);
	TEST_ASSERT_EQUAL_INT_MESSAGE(expect.amount,
									test.amount,
									msg);
	snprintf(msg, 100, "error amount exp:%d was:%d",
			error_expect.amount, test_error.amount);
	TEST_ASSERT_EQUAL_INT_MESSAGE(error_expect.amount,
									test_error.amount,
									msg);

	/* remove the test files*/
	remove(config_path);
}

void test_getOption(void)
{
	OPTION_STATE result[TESTS*4] = {0};
	OPTION_STATE expect_result[TESTS*4] = {0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,2,1,0,3,1};
	int test_index[TESTS*4] = {0,0,0,0,1,1,1,1,2,3,4,5,6,7,8,9,10,11,12,13};
	char msg[300] = {0};
	char input[TESTS*4][MAX_ROW] = {
		"zone=test1",
		"start=09:00",
		"end=12:00",
		"context=work",
		"zone=test2",
		"start=13:00",
		"end=15:00",
		"context=study",
		"zone=test3",
		"delay=2019-12-22T05:00Z",
		"freewilly=gethim",
		"totaldomination=ok",
		"cancel=on",
		"notify=off",
		"exclude=temporary(2019-09-12,2019-10-13,2019-10-25)",
		"delay=",
		"dealy=2019-12-21T15:00Z",
		"interval=1min",
		"",
		"endofall=nooo"
	};
	struct configcontent test = {
		.amount=0,
		.rowindex = {0},
		.option_name = {{{0}}},
		.option_value = {{{0}}},
		.sub_option_amount ={0}
	};
	struct configcontent expect = {
		.amount=8,
		.rowindex = {0,1,2,3,6,7,8,11},
		.option_name = {
			{"zone", "start", "end", "context"},
			{"zone", "start", "end", "context"},
			{"zone"},
			{"delay"},
			{"cancel"},
			{"notify"},
			{"exclude"},
			{"interval"}
		},
		.option_value = {
			{"test1", "09:00", "12:00", "work"},
			{"test2", "13:00", "15:00", "study"},
			{"test3"},
			{"2019-12-22T05:00Z"},
			{"on"},
			{"off"},
			{"temporary(2019-09-12,2019-10-13,2019-10-25)"},
			{"1min"}
		},
		.sub_option_amount ={4,4,1,1,1,1,1,1}
	};

	for(int i = 0 ; i < TESTS*4 ; i++) {
		result[i] = getOption(&test, input[i], test_index[i]);
		snprintf(msg, 100, "%d result exp:%d was:%d",
				i, expect_result[i], result[i]);
		TEST_ASSERT_EQUAL_INT_MESSAGE(expect_result[i], result[i], msg);
	}

	for(int i = 0 ; i < MAX_AMOUNT_OPTIONS ; i++) {
		snprintf(msg, 100, "%d rowindex exp:%d was:%d",
				i, expect.rowindex[i], test.rowindex[i]);
		TEST_ASSERT_EQUAL_INT_MESSAGE(expect.rowindex[i],
									test.rowindex[i], msg);
		snprintf(msg, 100, "%d, sub_option_amount (%s) exp:%d was:%d",
				i,
				expect.option_name[i][0],
				expect.sub_option_amount[i],
				test.sub_option_amount[i]);
		TEST_ASSERT_EQUAL_INT_MESSAGE(expect.sub_option_amount[i],
							test.sub_option_amount[i], msg);
		for(int j = 0 ; j < MAX_SUBOPTIONS ; j++) {
			snprintf(msg, 200, "%d.%d option_name exp:%s was:%s",
					i, j,
					expect.option_name[i][j],
					test.option_name[i][j]);
			TEST_ASSERT_EQUAL_STRING_MESSAGE(expect.option_name[i][j],
									test.option_name[i][j], msg);
			snprintf(msg, 300, "%d.%d option_value exp:%s was:%s",
					i,j,
					expect.option_value[i][j],
					test.option_value[i][j]);
			TEST_ASSERT_EQUAL_STRING_MESSAGE(expect.option_value[i][j],
									test.option_value[i][j], msg);
		}
	}
}

#define IN_TEST 4
void test_indexInList(void)
{
	struct configcontent test_struct[IN_TEST] = {
		{
			.amount = 5,
			.option_name = {{{0}}},
			.option_value = {{{0}}},
			.rowindex = {0,1,3,4,5}
		},
		{
			.amount = 10,
			.option_name = {{{0}}},
			.option_value = {{{0}}},
			.rowindex = {0,1,3,4,5,10,12,13,14,15}
		},
		{
			.amount = 3,
			.option_name = {{{0}}},
			.option_value = {{{0}}},
			.rowindex = {0,3,9}
		},
		{
			.amount = 25,
			.option_name = {{{0}}},
			.option_value = {{{0}}},
			.rowindex = {1,3,9,14,15,16,17,20,21,23,25,27,28,29,30,
							36,40,41,42,43,50,60,67,68,69}
		}
	};

	int test_index[IN_TEST*2] = {0,1,3,4,5,6,7,10};
	int result[IN_TEST*8] = {0};
	int expected[IN_TEST*8] = {0,0,0,0,0,-1,-1,-1,
								0,0,0,0,0,-1,-1,0,
								0,-1,0,-1,-1,-1,-1,-1,
								-1,0,0,-1,-1,-1,-1,-1};

	for(int i = 0 ; i < IN_TEST ; i++) {
		for(int j = 0 ; j < IN_TEST*2 ; j++)
			result[i*8+j] = indexInList(&test_struct[i], test_index[j]);
	}
	TEST_ASSERT_EQUAL_INT_ARRAY(expected, result, IN_TEST*8);
}

#define SYNC_TEST 3
void test_syncConfig(void)
{
	char msg[MAX_ROW] = {0};
	struct config test_config[SYNC_TEST*2] = {
		//0.1
		{
			.zone_name = {{0}},
			.zone_context = {{0}},
			.ztime = {{0}},
			.zone_amount = 0,
			.excl = {
				.type = {
					{
						.weekdays = {1,2,3,4},
						.list_len = 4,
						.sub_type = {"list"}
					},
					{
						.single_days = {
							{.tm_year=2019-1900,.tm_mon=11,.tm_mday=25,
								.tm_hour=0,.tm_min=0,.tm_sec=0,.tm_isdst=0},
							{.tm_year=2019-1900,.tm_mon=11,.tm_mday=27,
								.tm_hour=0,.tm_min=0,.tm_sec=0,.tm_isdst=0},
							{.tm_year=2019-1900,.tm_mon=11,.tm_mday=30,
								.tm_hour=0,.tm_min=0,.tm_sec=0,.tm_isdst=0},
						},
						.list_len = 3,
						.sub_type = {"list"}
					}
				},
				.type_name = {{"perm"}, {"temp"}},
				.amount = 2
			},
			.delay = {
				.tm_year=2019-1900,.tm_mon=11,.tm_mday=31,
				.tm_hour=12,.tm_min=30,.tm_sec=0,.tm_isdst=0
			},
			.cancel = 0,
			.notify = 1,
			.interval = 5
		},
		//0.2
		{
			.zone_name = {{0}},
			.zone_context = {{0}},
			.ztime = {{0}},
			.zone_amount = 0,
			.excl = {
				.type = {
					{
						.weekdays = {1,2,3,4},
						.list_len = 4,
						.sub_type = {"list"}
					},
					{
						.single_days = {
							{.tm_year=2019-1900,.tm_mon=11,.tm_mday=25,
								.tm_hour=0,.tm_min=0,.tm_sec=0,.tm_isdst=0},
							{.tm_year=2019-1900,.tm_mon=11,.tm_mday=27,
								.tm_hour=0,.tm_min=0,.tm_sec=0,.tm_isdst=0},
							{.tm_year=2019-1900,.tm_mon=11,.tm_mday=30,
								.tm_hour=0,.tm_min=0,.tm_sec=0,.tm_isdst=0},
						},
						.list_len = 3,
						.sub_type = {"list"}
					}
				},
				.type_name = {{"perm"}, {"temp"}},
				.amount = 2
			},
			.delay = {
				.tm_year=2019-1900,.tm_mon=11,.tm_mday=31,
				.tm_hour=12,.tm_min=30,.tm_sec=0,.tm_isdst=0
			},
			.cancel = 0,
			.notify = 1,
			.interval = 5
		},
		//1.1
		{
			.zone_name = {{0}},
			.zone_context = {{0}},
			.ztime = {{0}},
			.zone_amount = 0,
			.excl = {
				.type = {
					{
						.holiday_start = {.tm_year=2019-1900,.tm_mon=11,.tm_mday=20,
								.tm_hour=0,.tm_min=0,.tm_sec=0,.tm_isdst=0},
						.holiday_end = {.tm_year=2019-1900,.tm_mon=11,.tm_mday=26,
								.tm_hour=0,.tm_min=0,.tm_sec=0,.tm_isdst=0},
						.sub_type = {"range"}
					}
				},
				.type_name = {{"temp"}},
				.amount = 1
			},
			.delay = {
				.tm_year=0,.tm_mon=0,.tm_mday=0,
				.tm_hour=0,.tm_min=0,.tm_sec=0,.tm_isdst=0
			},
			.cancel = 1,
			.notify = 0,
			.interval = 10
		},
		//1.2
		{
			.zone_name = {{0}},
			.zone_context = {{0}},
			.ztime = {{0}},
			.zone_amount = 0,
			.excl = {
				.type = {
					{
						.holiday_start = {.tm_year=2019-1900,.tm_mon=11,.tm_mday=20,
								.tm_hour=0,.tm_min=0,.tm_sec=0,.tm_isdst=0},
						.holiday_end = {.tm_year=2019-1900,.tm_mon=11,.tm_mday=26,
								.tm_hour=0,.tm_min=0,.tm_sec=0,.tm_isdst=0},
						.sub_type = {"range"}
					}
				},
				.type_name = {{"temp"}},
				.amount = 1
			},
			.delay = {
				.tm_year=0,.tm_mon=0,.tm_mday=0,
				.tm_hour=0,.tm_min=0,.tm_sec=0,.tm_isdst=0
			},
			.cancel = 1,
			.notify = 0,
			.interval = 10
		},
		//2.1
		{
			.zone_name = {{0}},
			.zone_context = {{0}},
			.ztime = {{0}},
			.zone_amount = 0,
			.excl = {
				.type = {
					{
						.single_days = {
							{.tm_year=2020-1900,.tm_mon=1,.tm_mday=5,
								.tm_hour=0,.tm_min=0,.tm_sec=0,.tm_isdst=0},
						},
						.list_len = 1,
						.sub_type = {"list"}
					}
				},
				.type_name = {{"temp"}},
				.amount = 1
			},
			.delay = {
				.tm_year=2020-1900,.tm_mon=1,.tm_mday=15,
				.tm_hour=15,.tm_min=30,.tm_sec=0,.tm_isdst=0
			},
			.cancel = 0,
			.notify = 0,
			.interval = 0

		},
		//2.2
		{
			.zone_name = {{0}},
			.zone_context = {{0}},
			.ztime = {{0}},
			.zone_amount = 0,
			.excl = {
				.type = {
					{
						.single_days = {
							{.tm_year=2020-1900,.tm_mon=1,.tm_mday=5,
								.tm_hour=0,.tm_min=0,.tm_sec=0,.tm_isdst=0},
						},
						.list_len = 1,
						.sub_type = {"list"}
					}
				},
				.type_name = {{"temp"}},
				.amount = 1
			},
			.delay = {
				.tm_year=2020-1900,.tm_mon=1,.tm_mday=15,
				.tm_hour=15,.tm_min=30,.tm_sec=0,.tm_isdst=0
			},
			.cancel = 0,
			.notify = 0,
			.interval = 0
		}
	};
	struct config expect_config[SYNC_TEST*2] = {
		//0.1
		{
			.delay = {
				.tm_year=2019-1900,.tm_mon=11,.tm_mday=31,
				.tm_hour=13,.tm_min=0,.tm_sec=0,.tm_isdst=0
			},
			.cancel = 1,
			.notify = 0,
			.interval = 10
		},
		//0.2
		{
			.delay = {
				.tm_year=2019-1900,.tm_mon=11,.tm_mday=31,
				.tm_hour=16,.tm_min=0,.tm_sec=0,.tm_isdst=0
			},
			.cancel = 1,
			.notify = 0,
			.interval = 10
		},
		//1.1
		{
			.delay = {
				.tm_year=2019-1900,.tm_mon=11,.tm_mday=20,
				.tm_hour=13,.tm_min=30,.tm_sec=0,.tm_isdst=0
			},
			.cancel = 0,
			.notify = 1,
			.interval = 5
		},
		//1.2
		{
			.delay = {
				.tm_year=2019-1900,.tm_mon=11,.tm_mday=27,
				.tm_hour=15,.tm_min=30,.tm_sec=0,.tm_isdst=0
			},
			.cancel = 0,
			.notify = 1,
			.interval = 5
		},
		//2.1
		{
			.delay = {
				.tm_year=2020-1900,.tm_mon=1,.tm_mday=15,
				.tm_hour=15,.tm_min=30,.tm_sec=0,.tm_isdst=0
			},
			.cancel = 1,
			.notify = 1,
			.interval = 15
		},
		//2.2
		{
			.delay = {
				.tm_year=0,.tm_mon=0,.tm_mday=0,
				.tm_hour=0,.tm_min=0,.tm_sec=0,.tm_isdst=0
			},
			.cancel = 1,
			.notify = 1,
			.interval = 15
		}
	};
	struct flags flag[SYNC_TEST] = {
		//0
		{
			.delay = 30,
			.cancel_on = 1,
			.notify_on = 0,
			.cron_interval = 10
		},
		//1
		{
			.delay = 60,
			.cancel_on = 0,
			.notify_on = 1,
			.cron_interval = 5
		},
		//2
		{
			.delay = 0,
			.cancel_on = 1,
			.notify_on = 1,
			.cron_interval = 15
		}
	};
	struct tm time[SYNC_TEST*2] = {
		//0.1 - before delay and exclude
		{.tm_year=2019-1900,.tm_mon=11,.tm_mday=29,
			.tm_hour=5,.tm_min=30,.tm_sec=0,.tm_isdst=0},
		//0.2 - after delay and exclude
		{.tm_year=2019-1900,.tm_mon=11,.tm_mday=31,
			.tm_hour=15,.tm_min=30,.tm_sec=0,.tm_isdst=0},
		//1.1 - before holiday end
		{.tm_year=2019-1900,.tm_mon=11,.tm_mday=20,
			.tm_hour=12,.tm_min=30,.tm_sec=0,.tm_isdst=0},
		//1.2 - after holiday end
		{.tm_year=2019-1900,.tm_mon=11,.tm_mday=27,
			.tm_hour=14,.tm_min=30,.tm_sec=0,.tm_isdst=0},
		//2.1 - before delay and exclude
		{.tm_year=2020-1900,.tm_mon=1,.tm_mday=1,
			.tm_hour=5,.tm_min=30,.tm_sec=0,.tm_isdst=0},
		//2.2 - after delay and exclude
		{.tm_year=2020-1900,.tm_mon=1,.tm_mday=16,
			.tm_hour=5,.tm_min=30,.tm_sec=0,.tm_isdst=0},
	};
	int result[SYNC_TEST*2] = {0};
	int expect[SYNC_TEST*2] = {1,1,1,1,1,1};

	for(int i = 0 ; i < SYNC_TEST ; i++) {
		result[i*2] = syncConfig(&test_config[i*2], &flag[i],&time[i*2]);
		result[i*2+1] = syncConfig(&test_config[i*2+1], &flag[i],&time[i*2+1]);
	}
	for(int i = 0 ; i < SYNC_TEST*2 ; i++) {
		snprintf(msg, 100, "%d delay year exp:%d was:%d",
				i, expect_config[i].delay.tm_year,
				test_config[i].delay.tm_year);
		TEST_ASSERT_EQUAL_INT_MESSAGE(expect_config[i].delay.tm_year,
				test_config[i].delay.tm_year, msg);
		snprintf(msg, 100, "%d delay mon exp:%d was:%d",
				i, expect_config[i].delay.tm_mon,
				test_config[i].delay.tm_mon);
		TEST_ASSERT_EQUAL_INT_MESSAGE(expect_config[i].delay.tm_mon,
				test_config[i].delay.tm_mon, msg);
		snprintf(msg, 100, "%d delay mday exp:%d was:%d",
				i, expect_config[i].delay.tm_mday,
				test_config[i].delay.tm_mday);
		TEST_ASSERT_EQUAL_INT_MESSAGE(expect_config[i].delay.tm_mday,
				test_config[i].delay.tm_mday, msg);
		snprintf(msg, 100, "%d delay hour exp:%d was:%d",
				i, expect_config[i].delay.tm_hour,
				test_config[i].delay.tm_hour);
		TEST_ASSERT_EQUAL_INT_MESSAGE(expect_config[i].delay.tm_hour,
				test_config[i].delay.tm_hour, msg);
		snprintf(msg, 100, "%d delay min exp:%d was:%d",
				i, expect_config[i].delay.tm_min,
				test_config[i].delay.tm_min);
		TEST_ASSERT_EQUAL_INT_MESSAGE(expect_config[i].delay.tm_min,
				test_config[i].delay.tm_min, msg);
		snprintf(msg, 100, "%d cancel exp:%d was:%d",
				i, expect_config[i].cancel,
				test_config[i].cancel);
		TEST_ASSERT_EQUAL_INT_MESSAGE(expect_config[i].cancel,
				test_config[i].cancel, msg);
		snprintf(msg, 100, "%d notify exp:%d was:%d",
				i, expect_config[i].notify,
				test_config[i].notify);
		TEST_ASSERT_EQUAL_INT_MESSAGE(expect_config[i].notify,
				test_config[i].notify, msg);
		snprintf(msg, 100, "%d interval exp:%d was:%d",
				i, expect_config[i].interval,
				test_config[i].interval);
		TEST_ASSERT_EQUAL_INT_MESSAGE(expect_config[i].interval,
				test_config[i].interval, msg);
		snprintf(msg, 100, "%d result exp:%d was:%d",
				i, expect[i], result[i]);
		TEST_ASSERT_EQUAL_INT_MESSAGE(expect[i], result[i], msg);
	}
}

#define EXCL_CHECK 6
void test_checkExclusion(void)
{
	char msg[MAX_ROW] = {0};
	int result[EXCL_CHECK] = {0};
	int expect[EXCL_CHECK] = {0,0,1,1,0,0};
	struct tm time[EXCL_CHECK] = {
		//0 - empty
		{.tm_year=2019-1900,.tm_mon=10-1,.tm_mday=12,.tm_hour=13,.tm_min=20},
		//1 - permanent
		{.tm_year=2019-1900,.tm_mon=10-1,.tm_mday=12,.tm_hour=13,.tm_min=20},
		//2 - range delete
		{.tm_year=2019-1900,.tm_mon=10-1,.tm_mday=12,.tm_hour=13,.tm_min=20},
		//3 - list delete
		{.tm_year=2019-1900,.tm_mon=10-1,.tm_mday=12,.tm_hour=13,.tm_min=20},
		//4 - range no delete
		{.tm_year=2019-1900,.tm_mon=10-1,.tm_mday=12,.tm_hour=13,.tm_min=20},
		//5 - list no delete
		{.tm_year=2019-1900,.tm_mon=10-1,.tm_mday=12,.tm_hour=13,.tm_min=20},
	};
	struct exclusion excl[EXCL_CHECK] = {
		//0 - empty
		{
			.type = {
				{
					.weekdays={0},
					.single_days={{0}},
					.holiday_start={0},
					.holiday_end={0},
					.list_len=0,
					.sub_type={0}
				}
			},
			.type_name={{0}},
			.amount=0
		},
		//1 - permanent
		{
			.type = {
				{
					.weekdays={1,2},
					.single_days={{0}},
					.holiday_start={0},
					.holiday_end={0},
					.list_len=2,
					.sub_type={"list"}
				}
			},
			.type_name={"perm"},
			.amount=1
		},
		//2 - range temp delete
		{
			.type = {
				{
					.weekdays={0},
					.single_days={{0}},
					.holiday_start={.tm_year=2019-1900,.tm_mon=10-1,.tm_mday=8,
								.tm_hour=0,.tm_min=0,.tm_sec=0,.tm_isdst=0},
					.holiday_end={.tm_year=2019-1900,.tm_mon=10-1,.tm_mday=9,
								.tm_hour=0,.tm_min=0,.tm_sec=0,.tm_isdst=0},
					.list_len=0,
					.sub_type={"range"}
				}
			},
			.type_name={"temp"},
			.amount=1
		},
		//3 - list temp delete
		{
			.type = {
				{
					.weekdays={0},
					.single_days={
						{.tm_year=2019-1900,.tm_mon=10-1,.tm_mday=7,
								.tm_hour=0,.tm_min=0,.tm_sec=0,.tm_isdst=0},
						{.tm_year=2019-1900,.tm_mon=10-1,.tm_mday=8,
								.tm_hour=0,.tm_min=0,.tm_sec=0,.tm_isdst=0},
					},
					.holiday_start={0},
					.holiday_end={0},
					.list_len=2,
					.sub_type={"list"}
				},
				{
					.weekdays={0},
					.single_days={
						{.tm_year=2019-1900,.tm_mon=10-1,.tm_mday=4,
							.tm_hour=0,.tm_min=0,.tm_sec=0,.tm_isdst=0},
					},
					.holiday_start={0},
					.holiday_end={0},
					.list_len=1,
					.sub_type={"list"}
				}
			},
			.type_name={"temp", "temp"},
			.amount=2
		},
		//4 - range temp no delete
		{
			.type = {
				{
					.weekdays={0},
					.single_days={{0}},
					.holiday_start={.tm_year=2019-1900,.tm_mon=10-1,.tm_mday=15,
							.tm_hour=0,.tm_min=0,.tm_sec=0,.tm_isdst=0},
					.holiday_end={.tm_year=2019-1900,.tm_mon=10-1,.tm_mday=17,
							.tm_hour=0,.tm_min=0,.tm_sec=0,.tm_isdst=0},
					.list_len=0,
					.sub_type={"range"}
				}
			},
			.type_name={"temp"},
			.amount=1
		},
		//5 - list temp no delete
		{
			.type = {
				{
					.weekdays={0},
					.single_days={
						{.tm_year=2019-1900,.tm_mon=10-1,.tm_mday=15,
							.tm_hour=0,.tm_min=0,.tm_sec=0,.tm_isdst=0},
					},
					.holiday_start={0},
					.holiday_end={0},
					.list_len=1,
					.sub_type={"list"}
				}
			},
			.type_name={"temp"},
			.amount=1
		}
	};

	struct exclusion expect_excl[EXCL_CHECK] = {
		//0 - empty
		{
			.type = {
				{
					.weekdays={0},
					.single_days={{0}},
					.holiday_start={0},
					.holiday_end={0},
					.list_len=0,
					.sub_type={0}
				}
			},
			.type_name={{0}},
			.amount=0
		},
		//1 - permanent
		{
			.type = {
				{
					.weekdays={1,2},
					.single_days={{0}},
					.holiday_start={0},
					.holiday_end={0},
					.list_len=2,
					.sub_type={"list"}
				}
			},
			.type_name={"perm"},
			.amount=1
		},
		//2 - range temp delete
		{
			.type = {
				{
					.weekdays={0},
					.single_days={{0}},
					.holiday_start={.tm_year=0,.tm_mon=0,.tm_mday=0,
								.tm_hour=0,.tm_min=0,.tm_sec=0,.tm_isdst=0},
					.holiday_end={.tm_year=0,.tm_mon=0,.tm_mday=0,
								.tm_hour=0,.tm_min=0,.tm_sec=0,.tm_isdst=0},
					.list_len=0,
					.sub_type={0}
				}
			},
			.type_name={{0}},
			.amount=0
		},
		//3 - list temp delete
		{
			.type = {
				{
					.weekdays={0},
					.single_days={
						{.tm_year=0,.tm_mon=0,.tm_mday=0,
								.tm_hour=0,.tm_min=0,.tm_sec=0,.tm_isdst=0},
					},
					.holiday_start={0},
					.holiday_end={0},
					.list_len=0,
					.sub_type={0}
				}
			},
			.type_name={{0}},
			.amount=0
		},
		//4 - range temp no delete
		{
			.type = {
				{
					.weekdays={0},
					.single_days={{0}},
					.holiday_start={.tm_year=2019-1900,.tm_mon=10-1,.tm_mday=15,
							.tm_hour=0,.tm_min=0,.tm_sec=0,.tm_isdst=0},
					.holiday_end={.tm_year=2019-1900,.tm_mon=10-1,.tm_mday=17,
							.tm_hour=0,.tm_min=0,.tm_sec=0,.tm_isdst=0},
					.list_len=0,
					.sub_type={"range"}
				}
			},
			.type_name={"temp"},
			.amount=1
		},
		//5 - list temp no delete
		{
			.type = {
				{
					.weekdays={0},
					.single_days={
						{.tm_year=2019-1900,.tm_mon=10-1,.tm_mday=15,
							.tm_hour=0,.tm_min=0,.tm_sec=0,.tm_isdst=0},
					},
					.holiday_start={0},
					.holiday_end={0},
					.list_len=1,
					.sub_type={"list"}
				}
			},
			.type_name={"temp"},
			.amount=1
		}
	};

	for(int i = 0 ; i < EXCL_CHECK ; i++)
		result[i] = checkExclusion(&excl[i], &time[i]);

	for(int i = 0 ; i < EXCL_CHECK ; i++) {
		snprintf(msg, MAX_ROW, "%d result expect: %d was %d",
				i, expect[i], result[i]);
		TEST_ASSERT_EQUAL_INT_MESSAGE(expect[i], result[i], msg);
		snprintf(msg, MAX_ROW, "%d amount expect: %d was %d",
				i, expect_excl[i].amount, excl[i].amount);
		TEST_ASSERT_EQUAL_INT_MESSAGE(expect_excl[i].amount,
				excl[i].amount,msg);
		for(int j = 0 ; j < excl[i].amount ; j++) {
			snprintf(msg, MAX_ROW, "%d.%d type_name expect: %s was %s",
					i, j, expect_excl[i].type_name[j], excl[i].type_name[j]);
			TEST_ASSERT_EQUAL_STRING_MESSAGE(expect_excl[i].type_name[j],
					excl[i].type_name[j],msg);
			snprintf(msg, MAX_ROW, "%d.%d sub_type expect: %s was %s",
					i, j, expect_excl[i].type[j].sub_type,
					excl[i].type[j].sub_type);
			TEST_ASSERT_EQUAL_STRING_MESSAGE(expect_excl[i].type[j].sub_type,
					excl[i].type[j].sub_type, msg);
			snprintf(msg, MAX_ROW, "%d.%d list_len expect: %d was %d",
					i, j, expect_excl[i].type[j].list_len,
					excl[i].type[j].list_len);
			TEST_ASSERT_EQUAL_INT_MESSAGE(expect_excl[i].type[j].list_len,
					excl[i].type[j].list_len, msg);
			snprintf(msg, MAX_ROW, "%d.%d holiday_start.tm_year expect: %d was %d",
					i, j, expect_excl[i].type[j].holiday_start.tm_year,
					excl[i].type[j].holiday_start.tm_year);
			TEST_ASSERT_EQUAL_INT_MESSAGE(
					expect_excl[i].type[j].holiday_start.tm_year,
					excl[i].type[j].holiday_start.tm_year, msg);
			snprintf(msg, MAX_ROW, "%d.%d holiday_start.tm_mon expect: %d was %d",
					i, j, expect_excl[i].type[j].holiday_start.tm_mon,
					excl[i].type[j].holiday_start.tm_mon);
			TEST_ASSERT_EQUAL_INT_MESSAGE(expect_excl[i].type[j].holiday_start.tm_mon,
					excl[i].type[j].holiday_start.tm_mon, msg);
			snprintf(msg, MAX_ROW, "%d.%d holiday_start.tm_mday expect: %d was %d",
					i, j, expect_excl[i].type[j].holiday_start.tm_mday,
					excl[i].type[j].holiday_start.tm_mday);
			TEST_ASSERT_EQUAL_INT_MESSAGE(expect_excl[i].type[j].holiday_start.tm_mday,
					excl[i].type[j].holiday_start.tm_mday, msg);
			snprintf(msg, MAX_ROW, "%d.%d holiday_end.tm_year expect: %d was %d",
					i, j, expect_excl[i].type[j].holiday_end.tm_year,
					excl[i].type[j].holiday_end.tm_year);
			TEST_ASSERT_EQUAL_INT_MESSAGE(
					expect_excl[i].type[j].holiday_end.tm_year,
					excl[i].type[j].holiday_end.tm_year, msg);
			snprintf(msg, MAX_ROW, "%d.%d holiday_end.tm_mon expect: %d was %d",
					i, j, expect_excl[i].type[j].holiday_end.tm_mon,
					excl[i].type[j].holiday_end.tm_mon);
			TEST_ASSERT_EQUAL_INT_MESSAGE(
					expect_excl[i].type[j].holiday_end.tm_mon,
					excl[i].type[j].holiday_end.tm_mon, msg);
			snprintf(msg, MAX_ROW, "%d.%d holiday_end.tm_mday expect: %d was %d",
					i, j, expect_excl[i].type[j].holiday_end.tm_mday,
					excl[i].type[j].holiday_end.tm_mday);
			TEST_ASSERT_EQUAL_INT_MESSAGE(
					expect_excl[i].type[j].holiday_end.tm_mday,
					excl[i].type[j].holiday_end.tm_mday, msg);
			for(int k = 0 ; k < excl[i].type[j].list_len ; k++) {
				snprintf(msg, MAX_ROW,
						"%d.%d single_days[%d].tm_year expect: %d was %d",
						i, j, k, expect_excl[i].type[j].single_days[k].tm_year,
						excl[i].type[j].single_days[k].tm_year);
				TEST_ASSERT_EQUAL_INT(
						expect_excl[i].type[j].single_days[k].tm_year,
						excl[i].type[j].single_days[k].tm_year);
				snprintf(msg, MAX_ROW,
						"%d.%d single_days[%d].tm_mon expect: %d was %d",
						i, j, k, expect_excl[i].type[j].single_days[k].tm_mon,
						excl[i].type[j].single_days[k].tm_mon);
				TEST_ASSERT_EQUAL_INT(
						expect_excl[i].type[j].single_days[k].tm_mon,
						excl[i].type[j].single_days[k].tm_mon);
				snprintf(msg, MAX_ROW,
						"%d.%d single_days[%d].tm_mday expect: %d was %d",
						i, j, k, expect_excl[i].type[j].single_days[k].tm_mday,
						excl[i].type[j].single_days[k].tm_mday);
				TEST_ASSERT_EQUAL_INT(
						expect_excl[i].type[j].single_days[k].tm_mday,
						excl[i].type[j].single_days[k].tm_mday);
			}
		}
	}
}

#define WRITE_TEST 9
void test_writeConfig(void)
{
	char expected_content[WRITE_TEST][MAX_ROW] = {
		"Zone=Test1;Start=09:00;End=11:00;Context=work\n",
		"Zone=Test2;Start=11:00;End=13:00;Context=study\n",
		"Zone=Test3;Start=13:00;End=15:00;Context=freetime\n",
		"Exclude=permanent(su,mo,tu)\n",
		"Exclude=temporary(2019-11-20#2019-12-03)\n",
		"Delay=2019-12-05T15:00Z\n",
		"Cancel=on\n",
		"Notify=off\n",
		"Interval=10min\n"
	};
	char content[WRITE_TEST][MAX_ROW] = {{0}};
	int result = 0;
	int expect = 0;
	struct config test_config = {
		.zone_amount = 3,
		.zone_name = {"Test1", "Test2", "Test3"},
		.ztime = {
			{
			.start_hour=9,.start_minute=0,.end_hour=11,.end_minute=0
			},
			{
			.start_hour=11,.start_minute=0,.end_hour=13,.end_minute=0
			},
			{
			.start_hour=13,.start_minute=0,.end_hour=15,.end_minute=0
			}
		},
		.zone_context={"work", "study", "freetime"},
		.excl = {
			.amount = 2,
			.type_name = {"perm", "temp"},
			.type = {
				{
					.weekdays = {1,2,3},
					.single_days = {{0}},
					.holiday_start={0},
					.holiday_end={0},
					.list_len=3,
					.sub_type={"list"}
				},
				{
					.weekdays = {0},
					.single_days = {{0}},
					.holiday_start={.tm_year=2019-1900,.tm_mon=11-1,.tm_mday=20,
					.tm_hour=0, .tm_min=0, .tm_sec=0, .tm_isdst=0},
					.holiday_end={.tm_year=2019-1900,.tm_mon=12-1,.tm_mday=3,
					.tm_hour=0, .tm_min=0, .tm_sec=0, .tm_isdst=0},
					.list_len=0,
					.sub_type={"range"}
				}
			}
		},
		.delay = {
			.tm_year=2019-1900,.tm_mon=12-1,.tm_mday=5,
			.tm_hour=15, .tm_min=0, .tm_sec=0, .tm_isdst=0
		},
		.cancel = 1,
		.notify = 0,
		.interval = 10
	};

	// set path
	int fd = 0;
	FILE *read_file = NULL;
	int index = 0;
	int write_size = 0;
	int test_len = 0;
	char write_buffer[4][MAX_ROW] = {{0}};
	char *username = NULL;
	char pathname[22] = {"/.task/csw/.write_test"};
	char config_path[PATH_MAX] = {0};
	int size_user = 0;
	char msg[MAX_MSG+35] = {0};

	username = getenv("USER");
	if(!username)
		return;

	size_user = strlen(username);

	strncpy(config_path, "/home/", PATH_MAX);
	strncat(config_path, username, size_user);
	strncat(config_path, pathname, 22);

	if(dirExist(config_path) != 1) {
		// create file
		strncpy(write_buffer[0],
				"Zone=Test2;Start=08:00;End=08:30;Context=study",MAX_ROW);
		strncpy(write_buffer[1],"Exclude=temporary(2019-09-12)",MAX_ROW);
		strncpy(write_buffer[2],"Delay=2019-09-13T15:30Z",MAX_ROW);
		strncpy(write_buffer[3],"Cancel=off",MAX_ROW);
		fd = open(config_path, O_RDWR|O_CREAT, 0777);
		if(fd < 0)
			perror("file creation failed");

		for(int i = 0 ; i < 4 ; i++) {
			test_len = strnlen(write_buffer[i], MAX_ROW);
			write_size = write(fd, write_buffer[i], test_len);
			if(write_size < 1)
				perror("write failed");
		}
		if(close(fd) < 0) {
			perror("file closing failed");
			return;
		}
	}
	// use function
	result = writeConfig(&test_config, config_path);
	// read content
	read_file = fopen(config_path, "r");
	if(!read_file)
		return;

	while(fgets(content[index], MAX_ROW, read_file) != NULL) {
		index++;
	}
	// compare content
	for(int i = 0 ; i < WRITE_TEST ; i++) {
		snprintf(msg, MAX_MSG+35, "%d. read row expect: %s was: %s",
				i, expected_content[i], content[i]);
		TEST_ASSERT_EQUAL_STRING_MESSAGE(expected_content[i], content[i], msg);
	}
	TEST_ASSERT_EQUAL_INT_MESSAGE(expect, result, "result didn't match");
	fclose(read_file);
}

#define CONF_TEST 4
#define MSG_LEN 1100
void test_parseConfig(void)
{
	char msg[MSG_LEN] = {0};
	int result[CONF_TEST] = {0};
	int expected[CONF_TEST] = {0};
	struct configcontent test_content[CONF_TEST] = {
		//0
		{
			.amount=4,
			.rowindex = {0,1,3,4},
			.option_name={
				{"zone", "start", "end", "context"},
				{"delay"},
				{"cancel"},
				{"exclude"}
			},
			.option_value={
				{"morning", "5:00", "6:30", "study"},
				{"2019-10-12T14:00Z"},
				{"active"},
				{"permanent(mo,tu,we)"}
			},
			.sub_option_amount={4,1,1,1}
		},
		//1
		{
			.amount=2,
			.rowindex = {0,1},
			.option_name={
				{"delay"},
				{"delay"}
			},
			.option_value={
				{"2019-10-12T14:00Z"},
				{"10-12-2019T14:00Z"}
			},
			.sub_option_amount={1,1}
		},
		//2
		{
			.amount=8,
			.rowindex = {0,1,2,3,4,5,6,7},
			.option_name={
				{"zone", "start", "end", "context"},
				{"zone", "start", "end", "context"},
				{"delay"},
				{"cancel"},
				{"notify"},
				{"interval"},
				{"exclude"},
				{"exclude"}
			},
			.option_value={
				{"morning", "5:00", "6:30", "study"},
				{"brunch", "5,00", "6:30", "eating"},
				{"2019-10-12T14:00Z"},
				{"active"},
				{"inaction"},
				{"5min"},
				{"permanent(mo,tu,we)"},
				{"temporary(2019-12-10#2019-12-14#2019-12-16)"}
			},
			.sub_option_amount={4,4,1,1,1,1,1,1}
		},
		//3
		{
			.amount=8,
			.rowindex = {0,1,2,3,4,5,6,7},
			.option_name={
				{"zone", "start", "end", "context"},
				{"zone", "start", "end", "context"},
				{"zone", "start", "end", "context"},
				{"exclude"},
				{"exclude"},
				{"exclude"},
				{"exclude"},
				{"exclude"}
			},
			.option_value={
				{"morning", "5:00", "6:30", "sturdy"},
				{"lunch", "12:00", "13:30", "freetime"},
				{"dinner", "18:00", "19:30", "work"},
				{"permanent(mond,tues,wedn,thur)"},
				{"temporary(2019-12-10#2019-12-14)"},
				{"temporary(2019-12-10,2019-12-14)"},
				{"temporary(2019-12-10;2019-12-14)"},
				{"temporary(12-10-2019)"},
			},
			.sub_option_amount={4,4,4,1,1,1,1,1}
		}
	};
	struct error test_error[CONF_TEST] = {
		//0
		{
			.amount=1,
			.rowindex={2},
			.error_msg={{0}},
			.error_code={-1}
		},
		//1
		{
			.amount=0,
			.rowindex={0},
			.error_msg={{0}},
			.error_code={0}
		},
		//2
		{
			.amount=0,
			.rowindex={0},
			.error_msg={{0}},
			.error_code={0}
		},
		//3
		{
			.amount=0,
			.rowindex={0},
			.error_msg={{0}},
			.error_code={0}
		}
	};
	struct error expect_error[CONF_TEST] = {
		//0
		{
			.amount=1,
			.rowindex={2},
			.error_msg={{0}},
			.error_code={-1}
		},
		//1
		{
			.amount=1,
			.rowindex={1},
			.error_msg={{0}},
			.error_code={-6}
		},
		//2
		{
			.amount=3,
			.rowindex={1,1,7},
			.error_msg={{0}},
			.error_code={-4,-5,-8}
		},
		//3
		{
			.amount=4,
			.rowindex={0,3,6,7},
			.error_msg={{0}},
			.error_code={-5,-8,-8,-8}
		}
	};
	struct config test_config[CONF_TEST] = {
		{
			.zone_name = {{0}},
			.ztime = {{0}},
			.zone_context = {{0}},
			.zone_amount=0,
			.excl = {
				.amount=0,
				.type={
					{
						.sub_type={0},
						.weekdays={0},
						.single_days={{0}},
						.holiday_start={0},
						.holiday_end={0},
						.list_len=0
					}
				},
				.type_name={{0}}
			},
			.delay = {0},
			.cancel=0,
			.notify=0,
			.interval=0
		},
	};
	struct config expect_config[CONF_TEST] = {
		//0
		{
			.zone_name = {
				"morning"
			},
			.ztime = {
				{.start_hour=5,
				.start_minute=0,
				.end_hour=6,
				.end_minute=30}
			},
			.zone_context={
				"study"
			},
			.zone_amount=1,
			.excl = {
				.type= {
					{
						.weekdays = {2,3,4},
						.list_len = 3,
						.sub_type = {"list"}
					}
				},
				.type_name= {
					"perm"
				},
				.amount=1
			},
			.delay = {
				.tm_year = 119,
				.tm_mon = 9,
				.tm_mday = 12,
				.tm_hour = 14,
				.tm_min = 0,
				.tm_sec = 0,
				.tm_isdst = 0
			},
			.cancel=1,
			.notify=0,
			.interval=0
		},
		//1
		{
			.zone_name = {{0}},
			.ztime = {{0}},
			.zone_context={{0}},
			.zone_amount=0,
			.excl = {
				.amount=0,
				.type={
					{
						.sub_type={0},
						.weekdays={0},
						.single_days={{0}},
						.holiday_start={0},
						.holiday_end={0},
						.list_len=0
					}
				},
				.type_name={{0}}
			},
			.delay = {
				.tm_year = 119,
				.tm_mon = 9,
				.tm_mday = 12,
				.tm_hour = 14,
				.tm_min = 0,
				.tm_sec = 0,
				.tm_isdst = 0
			},
			.cancel=0,
			.notify=0,
			.interval=0
		},
		//2
		{
			.zone_name = {
				"morning"
			},
			.ztime = {
				{.start_hour=5,
				.start_minute=0,
				.end_hour=6,
				.end_minute=30}
			},
			.zone_context={
				"study"
			},
			.zone_amount=1,
			.excl = {
				.type= {
					{
						.weekdays = {2,3,4},
						.list_len = 3,
						.sub_type = {"list"}
					}
				},
				.type_name= {
					"perm"
				},
				.amount=1
			},
			.delay = {
				.tm_year = 119,
				.tm_mon = 9,
				.tm_mday = 12,
				.tm_hour = 14,
				.tm_min = 0,
				.tm_sec = 0,
				.tm_isdst = 0
			},
			.cancel=1,
			.notify=0,
			.interval=5
		},
		//3
		{
			.zone_name = {"lunch","dinner"},
			.ztime = {
				{.start_hour=12, .start_minute=0,
				.end_hour=13, .end_minute=30},
				{.start_hour=18, .start_minute=0,
				.end_hour=19, .end_minute=30}
			},
			.zone_context={"freetime","work"},
			.zone_amount=2,
			.excl = {
				.type= {
					{
						.weekdays = {0},
						.holiday_start = {
							.tm_year = 119,
							.tm_mon = 11,
							.tm_mday = 10,
							.tm_hour = 0,
							.tm_min = 0,
							.tm_sec = 0,
							.tm_isdst = 0
						},
						.holiday_end = {
							.tm_year = 119,
							.tm_mon = 11,
							.tm_mday = 14,
							.tm_hour = 0,
							.tm_min = 0,
							.tm_sec = 0,
							.tm_isdst = 0
						},
						.list_len = 0,
						.sub_type = {"range"}
					},
					{
						.weekdays = {0},
						.single_days = {
							{
								.tm_year=119, .tm_mon=11, .tm_mday=10,
								.tm_hour = 0, .tm_min = 0, .tm_sec = 0,
								.tm_isdst = 0
							},
							{
								.tm_year=119, .tm_mon=11, .tm_mday=14,
								.tm_hour = 0, .tm_min = 0, .tm_sec = 0,
								.tm_isdst = 0
							}
						},
						.list_len = 2,
						.sub_type = {"list"}
					},
				},
				.type_name= {
					"temp","temp"
				},
				.amount=2
			},
			.delay = {0},
			.cancel=0,
			.notify=0,
			.interval=0
		}
	};

	for(int i = 0 ; i < CONF_TEST ; i++) {
		result[i] = parseConfig(&test_content[i], &test_error[i],
								&test_config[i]);
		TEST_ASSERT_EQUAL_INT_ARRAY(expected, result, CONF_TEST);
		snprintf(msg, MSG_LEN, "%d zone_amount exp:%d was:%d",
				i, expect_config[i].zone_amount,
				test_config[i].zone_amount);
		TEST_ASSERT_EQUAL_INT_MESSAGE(expect_config[i].zone_amount,
									test_config[i].zone_amount, msg);
		for(int j = 0 ; j < test_config[i].zone_amount ; j++) {
			snprintf(msg, MSG_LEN, "%d zone_name exp:[%s] was:[%s]",
					i, expect_config[i].zone_name[j],
					test_config[i].zone_name[j]);
			TEST_ASSERT_EQUAL_STRING_MESSAGE(expect_config[i].zone_name[j],
										test_config[i].zone_name[j], msg);
			snprintf(msg, MSG_LEN, "%d zone_context exp:%s was:%s",
					i, expect_config[i].zone_context[j],
					test_config[i].zone_context[j]);
			TEST_ASSERT_EQUAL_STRING_MESSAGE(expect_config[i].zone_context[j],
										test_config[i].zone_context[j], msg);
			snprintf(msg, MSG_LEN, "%d start_hour exp:%d was:%d",
					i, expect_config[i].ztime[j].start_hour,
					test_config[i].ztime[j].start_hour);
			TEST_ASSERT_EQUAL_INT_MESSAGE(expect_config[i].ztime[j].start_hour,
										test_config[i].ztime[j].start_hour,
										msg);
			snprintf(msg, MSG_LEN, "%d start_minute exp:%d was:%d",
					i, expect_config[i].ztime[j].start_minute,
					test_config[i].ztime[j].start_minute);
			TEST_ASSERT_EQUAL_INT_MESSAGE(expect_config[i].ztime[j].start_minute,
										test_config[i].ztime[j].start_minute,
										msg);
			snprintf(msg, MSG_LEN, "%d end_hour exp:%d was:%d",
					i, expect_config[i].ztime[j].end_hour,
					test_config[i].ztime[j].end_hour);
			TEST_ASSERT_EQUAL_INT_MESSAGE(expect_config[i].ztime[j].end_hour,
										test_config[i].ztime[j].end_hour,
										msg);
			snprintf(msg, MSG_LEN, "%d end_minute exp:%d was:%d",
					i, expect_config[i].ztime[j].end_minute,
					test_config[i].ztime[j].end_minute);
			TEST_ASSERT_EQUAL_INT_MESSAGE(expect_config[i].ztime[j].end_minute,
										test_config[i].ztime[j].end_minute,
										msg);
		}
		snprintf(msg, MSG_LEN, "%d excl_amount exp:%d was:%d",
				i, expect_config[i].excl.amount,
				test_config[i].excl.amount);
		TEST_ASSERT_EQUAL_INT_MESSAGE(expect_config[i].excl.amount,
									test_config[i].excl.amount, msg);
		for(int j = 0 ; j < test_config[i].excl.amount ; j++) {
			snprintf(msg, MSG_LEN, "%d type_name exp:%s was:%s",
					i, expect_config[i].excl.type_name[j],
					test_config[i].excl.type_name[j]);
			TEST_ASSERT_EQUAL_STRING_MESSAGE(expect_config[i].excl.type_name[j],
									test_config[i].excl.type_name[j], msg);
			snprintf(msg, MSG_LEN, "%d type.list_len exp:%d was:%d",
					i, expect_config[i].excl.type[j].list_len,
					test_config[i].excl.type[j].list_len);
			TEST_ASSERT_EQUAL_INT_MESSAGE(
					expect_config[i].excl.type[j].list_len,
					test_config[i].excl.type[j].list_len, msg);
			snprintf(msg, MSG_LEN, "%d sub_type exp:%s was:%s",
					i, expect_config[i].excl.type[j].sub_type,
					test_config[i].excl.type[j].sub_type);
			TEST_ASSERT_EQUAL_STRING_MESSAGE(
					expect_config[i].excl.type[j].sub_type,
					test_config[i].excl.type[j].sub_type, msg);
			for(int k = 0 ; k < test_config[i].excl.type[j].list_len ; k++) {
				snprintf(msg, MSG_LEN, "%d.%d.%d weekdays exp:%d was:%d",
						i, j, k,
						expect_config[i].excl.type[j].weekdays[k],
						test_config[i].excl.type[j].weekdays[k]);
				TEST_ASSERT_EQUAL_INT_MESSAGE(
						expect_config[i].excl.type[j].weekdays[k],
						test_config[i].excl.type[j].weekdays[k],
						msg);
				snprintf(msg, MSG_LEN, "%d.%d.%d days:year exp:%d was:%d",
						i,j,k,
						expect_config[i].excl.type[j].single_days[k].tm_year,
						test_config[i].excl.type[j].single_days[k].tm_year);
				TEST_ASSERT_EQUAL_INT_MESSAGE(
						expect_config[i].excl.type[j].single_days[k].tm_year,
						test_config[i].excl.type[j].single_days[k].tm_year,
						msg);
				snprintf(msg, MSG_LEN, "%d.%d.%d days:month exp:%d was:%d",
						i,j,k,
						expect_config[i].excl.type[j].single_days[k].tm_mon,
						test_config[i].excl.type[j].single_days[k].tm_mon);
				TEST_ASSERT_EQUAL_INT_MESSAGE(
						expect_config[i].excl.type[j].single_days[k].tm_mon,
						test_config[i].excl.type[j].single_days[k].tm_mon,
						msg);
				snprintf(msg, MSG_LEN, "%d.%d.%d days:day exp:%d was:%d",
						i,j,k,
						expect_config[i].excl.type[j].single_days[k].tm_mday,
						test_config[i].excl.type[j].single_days[k].tm_mday);
				TEST_ASSERT_EQUAL_INT_MESSAGE(
						expect_config[i].excl.type[j].single_days[k].tm_mday,
						test_config[i].excl.type[j].single_days[k].tm_mday,
						msg);
			}
			snprintf(msg, MSG_LEN, "%d.%d holi_start:year exp:%d was:%d",
					i, j,
					expect_config[i].excl.type[j].holiday_start.tm_year,
					test_config[i].excl.type[j].holiday_start.tm_year);
			TEST_ASSERT_EQUAL_INT_MESSAGE(
					expect_config[i].excl.type[j].holiday_start.tm_year,
					test_config[i].excl.type[j].holiday_start.tm_year,
					msg);
			snprintf(msg, MSG_LEN, "%d.%d holi_start:mon exp:%d was:%d",
					i, j,
					expect_config[i].excl.type[j].holiday_start.tm_mon,
					test_config[i].excl.type[j].holiday_start.tm_mon);
			TEST_ASSERT_EQUAL_INT_MESSAGE(
					expect_config[i].excl.type[j].holiday_start.tm_mon,
					test_config[i].excl.type[j].holiday_start.tm_mon,
					msg);
			snprintf(msg, MSG_LEN, "%d.%d holi_start:day exp:%d was:%d",
					i, j,
					expect_config[i].excl.type[j].holiday_start.tm_mday,
					test_config[i].excl.type[j].holiday_start.tm_mday);
			TEST_ASSERT_EQUAL_INT_MESSAGE(
					expect_config[i].excl.type[j].holiday_start.tm_mday,
					test_config[i].excl.type[j].holiday_start.tm_mday,
					msg);
			snprintf(msg, MSG_LEN, "%d.%d holi_end:year exp:%d was:%d",
					i, j,
					expect_config[i].excl.type[j].holiday_end.tm_year,
					test_config[i].excl.type[j].holiday_end.tm_year);
			TEST_ASSERT_EQUAL_INT_MESSAGE(
					expect_config[i].excl.type[j].holiday_end.tm_year,
					test_config[i].excl.type[j].holiday_end.tm_year,
					msg);
			snprintf(msg, MSG_LEN, "%d.%d holi_end:mon exp:%d was:%d",
					i, j,
					expect_config[i].excl.type[j].holiday_end.tm_mon,
					test_config[i].excl.type[j].holiday_end.tm_mon);
			TEST_ASSERT_EQUAL_INT_MESSAGE(
					expect_config[i].excl.type[j].holiday_end.tm_mon,
					test_config[i].excl.type[j].holiday_end.tm_mon,
					msg);
			snprintf(msg, MSG_LEN, "%d.%d holi_end:day exp:%d was:%d",
					i, j,
					expect_config[i].excl.type[j].holiday_end.tm_mday,
					test_config[i].excl.type[j].holiday_end.tm_mday);
			TEST_ASSERT_EQUAL_INT_MESSAGE(
					expect_config[i].excl.type[j].holiday_end.tm_mday,
					test_config[i].excl.type[j].holiday_end.tm_mday,
					msg);
		}
		snprintf(msg, MSG_LEN, "%d error:amount exp:%d was:%d",
				i,
				expect_error[i].amount,
				test_error[i].amount);
		TEST_ASSERT_EQUAL_INT_MESSAGE(
				expect_error[i].amount,
				test_error[i].amount,
				msg);
		for(int j = 0 ; j < test_error[i].amount ; j++) {
			snprintf(msg, MSG_LEN, "%d.%d error:rowindex exp:%d was:%d",
					i, j,
					expect_error[i].rowindex[j],
					test_error[i].rowindex[j]);
			TEST_ASSERT_EQUAL_INT_MESSAGE(
					expect_error[i].rowindex[j],
					test_error[i].rowindex[j],
					msg);
			snprintf(msg, MSG_LEN, "%d.%d error:error_code exp:%d was:%d",
					i, j,
					expect_error[i].error_code[j],
					test_error[i].error_code[j]);
			TEST_ASSERT_EQUAL_INT_MESSAGE(
					test_error[i].error_code[j],
					expect_error[i].error_code[j],
					msg);
		}
	}
}

#define KEY_TEST 12
void test_valueForKey(void)
{
	struct keyvalue lookuptable[VALID_OPTIONS] = {
		{"zone", FIND_ZONE},
		{"start", FIND_START},
		{"end", FIND_END},
		{"context", FIND_CONTEXT},
		{"delay", FIND_DELAY},
		{"cancel", FIND_CANCEL},
		{"notify", FIND_NOTIFY},
		{"interval", FIND_INTERVAL},
		{"exclude", FIND_EXCLUDE}
	};

	char test_key[KEY_TEST][MAX_OPTION_NAME] = {
		"zone", "start", "end", "context",
		"delay", "cancel", "notify", "interval",
		"exclude", "rubbish", "", "123"
	};
	int result[KEY_TEST] = {0};
	int expect[KEY_TEST] = {0, 1, 2, 3, 4, 5, 6, 7, 8, -1, -1, -1};

	for(int i = 0 ; i < KEY_TEST ; i++) {
		result[i] = valueForKey(&lookuptable[0], test_key[i]);
	}
	TEST_ASSERT_EQUAL_INT_ARRAY(expect, result, KEY_TEST);
}

/*=======MAIN=====*/
int main(void)
{
	UnityBegin("test_config.c");
	RUN_TEST(test_dirExist_good);
	RUN_TEST(test_dirExist_bad);
	RUN_TEST(test_dirExist_is_file);
	RUN_TEST(test_findConfig_good);
	RUN_TEST(test_findConfig_notfound);
	RUN_TEST(test_readConfig);
	RUN_TEST(test_getOption);
	RUN_TEST(test_indexInList);
	RUN_TEST(test_syncConfig);
	RUN_TEST(test_checkExclusion);
	RUN_TEST(test_writeConfig);
	RUN_TEST(test_valueForKey);

	return UnityEnd();
}
