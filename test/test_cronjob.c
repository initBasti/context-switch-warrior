#include "../unity/src/unity.h"
#include "../source/include/cronjob.h"

void setUp(void)
{
}
void tearDown(void)
{
}

void createTestCron(void) {
	/* For findCrontab
	* use writeCrontab to create a dummy cronjob
	* find it and then use deleteCrontab to delete it
	* again
	*/
	struct loc_env test_env = {
		.user="basti",
		.path="/usr/bin",
		.cron_env=1
	};
	printf("do set up\n");
	if(writeCrontab("notify-send 'findCrontab test' -t 1000\n", 1, &test_env) == -1)
		return;
}

void deleteTestCron(void) {
	struct loc_env test_env = {
		.user="basti",
		.path="/usr/bin",
		.cron_env=1
	};
	if(deleteCrontab("notify-send 'findCrontab test' -t 1000", &test_env) == -1)
		printf("WARNING dummy cronjob could not be deleted\n");
}

#define CRON_BUILD_TEST 5
void test_buildCronCommand(void)
{
	char term[CRON_BUILD_TEST][MAX_COMMAND] = {
		"whoami", "whoami", "whoami", "whoami", "failer"
	};
	int interval[CRON_BUILD_TEST] = {5, 60, 360, 0, 189};
	struct loc_env environment[CRON_BUILD_TEST] = {
		{.user="user", .path="/usr/bin", .cron_env=-1},
		{.user="user", .path="/usr/bin", .cron_env=0},
		{.user="user", .path="/usr/bin", .cron_env=0},
		{.user="user", .path="/usr/bin", .cron_env=0},
		{.user="user", .path="/usr/bin", .cron_env=0},
	};
	char command[CRON_BUILD_TEST][MAX_CRON] = {{0}};
	char expect_command[CRON_BUILD_TEST][MAX_CRON] = {
		"(echo 'USER=user\nPATH=/usr/bin\n' && crontab -u user -l && echo '*/5 * * * * whoami') | crontab -u user -",
		"(crontab -u user -l && echo '* */1 * * * whoami') | crontab -u user -",
		{0}, {0}, {0}
	};
	int result[CRON_BUILD_TEST] = {0};
	int expect[CRON_BUILD_TEST] = {0, 0, -1, -1, -2};

	for(int i = 0 ; i < CRON_BUILD_TEST ; i++)
		result[i] = buildCronCommand(term[i], interval[i], &environment[i], command[i]);

	for(int i = 0 ; i < CRON_BUILD_TEST ; i++) {
		TEST_ASSERT_EQUAL_INT(expect[i], result[i]);
		TEST_ASSERT_EQUAL_STRING(expect_command[i], command[i]);
	}
}

#define CHECK_TERM_TEST 4
void test_checkTerm(void)
{
	char term[CHECK_TERM_TEST][MAX_COMMAND] = {
		"whoami", "dirname '/usr/bin/'", "xxx", {0}
	};
	int result[CHECK_TERM_TEST] = {0};
	int expect[CHECK_TERM_TEST] = {0, 0, -1, -1};

	for(int i = 0 ; i < CHECK_TERM_TEST ; i++)
		result[i] = checkTerm(term[i]);

	TEST_ASSERT_EQUAL_INT_ARRAY(expect, result, CHECK_TERM_TEST);
}

#define INTERVAL_CRON_TEST 3
void test_buildCronInterval(void)
{
	int interval[INTERVAL_CRON_TEST] = {25, 120, 75};
	char output[INTERVAL_CRON_TEST][MAX_INTERVAL_STR] = {{0}};
	char expect_output[INTERVAL_CRON_TEST][MAX_INTERVAL_STR] = {
		"*/25 * * * *",
		"* */2 * * *",
		"*/15 */1 * * *"
	};

	for(int i = 0 ; i < INTERVAL_CRON_TEST ; i++)
		buildCronInterval(interval[i], output[i]);

	for(int i = 0 ; i < INTERVAL_CRON_TEST ; i++)
		TEST_ASSERT_EQUAL_STRING(expect_output[i], output[i]);
}

void test_readCrontab(void)
{
	FILE *test_file = NULL;
	char path[PATH_MAX] = {0};
	char test_buffer[MAX_CRON_JOBS][MAX_CRON] = {
		"#this is a comment\n",
		"USER=user\n",
		"PATH=path/to/file\n",
		"* */1 * * * command\n",
		"\n",
		"*/25 * * * * command\n",
		"\n"	
	};
	char buffer[MAX_CRON_JOBS][MAX_CRON] = {{0}};
	char expect_buffer[MAX_CRON_JOBS][MAX_CRON] = {
		"USER=user",
		"PATH=path/to/file",
		"* */1 * * * command",
		"*/25 * * * * command"
	};
	int result = 0;
	int expect = 0;

	char *username = NULL;
	int size_user = 0;

	username = getenv("USER");
	if(!username)
		return;

	size_user = strlen(username);

	/* Create the test files */
	strncpy(path, "/home/", PATH_MAX);
	strncat(path, username, size_user);
	strncat(path, "/.task/csw/.cronjob_test", 30);

	test_file = fopen(path, "w");
	if(!test_file) {
		perror("file creation failed");
		return;
	}

	for(int j = 0 ; j < MAX_AMOUNT_OPTIONS ; j++) {
		if(test_buffer[j][0] != '\0')
			fprintf(test_file, "%s", test_buffer[j]);
	}
	rewind(test_file);
	if(fclose(test_file) == EOF) {
		perror("file closing failed");
		return;
	}
	test_file = fopen(path, "r");
	if(!test_file) {
		perror("file open for read failed");
		return;
	}

	result = readCrontab(test_file, MAX_CRON, buffer);

	if(fclose(test_file) == EOF) {
		perror("file closing failed");
		return;
	}
	remove(path);

	TEST_ASSERT_EQUAL_INT(expect, result);

	for(int i = 0 ; i < MAX_CRON_JOBS ; i++)
		TEST_ASSERT_EQUAL_STRING(expect_buffer[i], buffer[i]);
}

#define CRONTAB_CHECK_TEST 8
void test_checkCrontab(void)
{
	char msg[MAX_ROW] = {0};
	char term[CRONTAB_CHECK_TEST][MAX_COMMAND] = {
		"USER", "PATH", "dirname", "csw", "csw", {0}, "csw", "csw"
	};
	char input[CRONTAB_CHECK_TEST][MAX_CRON_JOBS][MAX_CRON] = {
		/* 0 - found */
		{
			"USER=user", "PATH=/usr/bin",
			"* * * * * whoami",
			"*/5 * * * * notify-send 'test'"
		},
		/* 1 - not found */
		{
			"USER=user",
			"*/25 * * * * whoami", "PUTH=/usr/bin"
		},
		/* 2 - term in other case */
		{
			"USER=user", "PATH=/usr/bin",
			"* */3 * * * Dirname '/usr/bin'"
		},
		/* 3 - term within other term */
		{
			"* */5 * * * cswman"
		},
		/* 4 - multiple hits */
		{
			"* * * * * csw -v -s",
			"*/25 * * * * csw -d 30min",
			"* * * * * csw -n 1",
			"* * * * * csw= -n 1",
			"* * * * * csw=x -n 1"
		},
		/* 5 - empty term */
		{
			"* * * * * whoami"
		},
		/* 6 - empty crontab */
		{{0}},
		/* 7 - term at end of line */
		{
			"* * * * * csw"
		}
	};
	int index[CRONTAB_CHECK_TEST] = {0};
	int expect_index[CRONTAB_CHECK_TEST] = {0, -1, -1, -1, 0, -1, -1, 0};
	int result[CRONTAB_CHECK_TEST] = {0};
	int expect[CRONTAB_CHECK_TEST] = {0, -1, -1, -1, 0, -1, -1, 0};

	for(int i = 0 ; i < CRONTAB_CHECK_TEST ; i++)
		result[i] = checkCrontab(term[i], MAX_CRON, input[i], &index[i]);

	for(int i = 0 ; i < CRONTAB_CHECK_TEST ; i++) {
		snprintf(msg, MAX_ROW, "[%d. result expect: %d was %d]",
				i, expect[i], result[i]);
		TEST_ASSERT_EQUAL_INT_MESSAGE(expect[i], result[i], msg);
		snprintf(msg, MAX_ROW, "[%d. index expect: %d was %d]",
				i, expect_index[i], index[i]);
		TEST_ASSERT_EQUAL_INT_MESSAGE(expect_index[i], index[i], msg);
	}
}

#define CRONENV_CHECK_TEST 3
void test_checkCronEnv(void)
{
	char msg[MAX_ROW] = {0};
	char input[CRONENV_CHECK_TEST][MAX_CRON_JOBS][MAX_CRON] = {
		/* 0 - found */
		{
			"USER=user", "PATH=/usr/bin",
			"* * * * * whoami",
			"*/5 * * * * notify-send 'test'"
		},
		/* 1 - single */
		{
			"USER=user",
			"*/25 * * * * whoami", "PUTH=/usr/bin"
		},
		/* 2 - none */
		{
			"* */3 * * * Dirname '/usr/bin'"
		},
	};
	int result[CRONENV_CHECK_TEST] = {0};
	int expect[CRONENV_CHECK_TEST] = {0, -1, -1};

	for(int i = 0 ; i < CRONENV_CHECK_TEST ; i++) {
		result[i] = checkCronEnv(MAX_CRON, input[i]);
		snprintf(msg, MAX_ROW, "[%d. result expect: %d was %d]",
				i, expect[i], result[i]);
		TEST_ASSERT_EQUAL_INT_MESSAGE(expect[i], result[i], msg);
	}

}

#define CRONPARSE_TEST 8
void test_parseCrontab(void)
{
	char msg[MAX_ROW] = {0};
	char input[CRONPARSE_TEST][MAX_CRON] = {
		"*/5 * * * * whoami",
		"* */2 * * * whoami",
		"*/25 */3 * * * whoami",
		"* * * * * whoami",
		"whoami",
		"*/5 * * whoami",
		"*/A * * * * whoami",
		"*/5 * * * * * whoami"
	};
	int result[CRONPARSE_TEST] = {0};
	int expect[CRONPARSE_TEST] = {0, 0, 0, 0, -1, -1, -1, -1};
	int interval[CRONPARSE_TEST] = {0};
	int expect_interval[CRONPARSE_TEST] = {5, 120, 205, 1, 0, 0, 0, 0};

	for(int i = 0 ; i < CRONPARSE_TEST ; i++) {
		result[i] = parseCrontab(input[i], &interval[i]);
		snprintf(msg, MAX_ROW, "[%d. result expect: %d was %d]",
				i, expect[i], result[i]);
		TEST_ASSERT_EQUAL_INT_MESSAGE(expect[i], result[i], msg);
		snprintf(msg, MAX_ROW, "[%d. interval expect: %d was %d]",
				i, expect_interval[i], interval[i]);
		TEST_ASSERT_EQUAL_INT_MESSAGE(expect_interval[i], interval[i], msg);
	}
}
/*=======MAIN=====*/
int main(void)
{
  UnityBegin("test_cronjob.c");
  RUN_TEST(test_buildCronCommand);
  RUN_TEST(test_checkTerm);
  RUN_TEST(test_buildCronInterval);
  RUN_TEST(test_readCrontab);
  RUN_TEST(test_parseCrontab);
  RUN_TEST(test_checkCrontab);
  RUN_TEST(test_checkCronEnv);

  return UnityEnd();
}
