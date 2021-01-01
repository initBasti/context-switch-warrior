/**
 * @file types.h
 * @brief different definitions,structs and enums used throughout the software
 * @author Sebastian Fricke
 * @date 2019-12-21
 */

#ifndef TYPES_H
#define TYPES_H

#include <time.h>

#define PATH_MAX 4096
#define MAX_ZONES 10
#define MAX_EXCLUSION 10
#define MAX_ROW 512
#define MAX_MSG 1024
#define MAX_OPTION 128
#define MAX_OPTION_NAME 40
#define VALID_OPTIONS 9
/* 10*zones, 10*exclusion & cancel,notify,delay,intervall + 1 temp delay */
#define MAX_AMOUNT_OPTIONS 25
#define MAX_SUBOPTIONS 4
#define MAX_FIELD 96
#define MAX_COMMAND 35
#define MAX_USER 128
#define MAX_EXCL 12
#define MAX_EXCL_LINE MAX_EXCL*11
#define MAX_CONTEXT 10
#define MAX_ARGUMENTS 12
#define MAX_CRON_JOBS 50
#define MAX_CRON 512
#define MAX_ENV 300 
#define MAX_TERM 100
#define MAX_RE_PATTERN 150
#define MAX_INTERVAL_STR 15
#define MAX_INTERVAL 360
#define OPTIONS 4
#define FIELDS 2
#define WEEKDAYS 7
#define DAY 2
#define DATE 10
#define TYPE_LEN 6
#define DELAY_FORMAT_LEN 17
#define MAX_ARG_LENGTH 20
#define BAD_KEY -1

extern int verbose_flag;

struct zonetime {
	int start_hour;
	int start_minute;
	int	end_hour;
	int end_minute;
};

struct format_type {
	int weekdays[WEEKDAYS];
	struct tm single_days[MAX_EXCLUSION];
	struct tm holiday_start;
	struct tm holiday_end;
	int list_len;
	char sub_type[TYPE_LEN];
};

struct exclusion {
	struct format_type type[MAX_EXCLUSION];
	char type_name[MAX_EXCLUSION][TYPE_LEN];
	int amount;
};

/**
 * @struct config
 * @brief collection of parsed options from struct configcontent
 *
 * every element of the config is paired with it's row index
 * for later retrival and edit
 *
 * @var zone_name	short name for each zone as identifier
 * @var	ztime	collection of start & end time in hour,minute
 * @var	zone_context	context from TaskWarrior used in commands to TW
 * @var zone_amount	the number of zones, is used for later retrieval
 *
 * @var	excl	collection of all excluded time areas for execution
 *
 * @var	delay	exact time when the delay of execution is turned off
 *
 * @var cancel	save user decision from command-line, task cancel on/off
 *
 * @var notify	save user decision from command-line, notification on/off
 *
 * @var	interval	the interval in min, used for the cronjob execution
 *
 * @date	2019-12-27
 */
struct config {
	char zone_name[MAX_ZONES][MAX_FIELD];
	struct zonetime ztime[MAX_ZONES];
	char zone_context[MAX_ZONES][MAX_COMMAND];
	int zone_amount;
	struct exclusion excl;
	struct tm delay;
	int cancel;
	int notify;
	int interval;
};

/**
 * @struct configcontent
 * @brief	structures the rows into their subparts and keeping the index
 *
 * sub_option_amount for a easy retrieval of the different options
 *
 * @var amount	Number of valid entries in the config
 * @var	rowindex	Needed for the delete of options later with deleteFromFile
 * @var	option_name	the title of the option 2-dim array of strings
 * @var	option_value	the value of the option 2-dim arry of strings
 * @var sub_option_amount	integer for recording the amount of suboptions
 * @var valid	integer either 1 for valid or 0 for invalid
 */
struct configcontent {
	int amount;
	int rowindex[MAX_AMOUNT_OPTIONS];
	char option_name[MAX_AMOUNT_OPTIONS][MAX_SUBOPTIONS][MAX_OPTION_NAME];
	char option_value[MAX_AMOUNT_OPTIONS][MAX_SUBOPTIONS][MAX_OPTION];
	int sub_option_amount[MAX_AMOUNT_OPTIONS];
};

struct error {
	int amount;
	int rowindex[MAX_AMOUNT_OPTIONS];
	char error_msg[MAX_AMOUNT_OPTIONS][MAX_ROW];
	int error_code[MAX_AMOUNT_OPTIONS];
};

struct substr {
	char member[MAX_FIELD];
	struct substr *next;
};

struct flags {
	int delay;
	int show;
	int cancel_on;
	int notify_on;
	int cron_interval;
	int *verbose;
	int help;
};

struct context {
	char name[MAX_CONTEXT][MAX_FIELD];
	int amount;
};

struct keyvalue {
	char key[MAX_OPTION_NAME];
	int value;
};

/**
 * @struct loc_env
 * @brief	USER variable and PATH variable + indicator on existance in cronjob
 *
 * @var user	username string of maximum length MAX_USER
 * @var path	path variable content as string of max length PATH_MAX
 * @var	cron_env	indicator if the cron environment is updated
 * @date	2020-01-26
 */
struct loc_env {
	char user[MAX_USER];
	char path[PATH_MAX];
	int cron_env;
};

typedef enum{
	CONFIG_SUCCESS,
	CONFIG_BAD,
	CONFIG_NOTFOUND
}CONFIG_STATE;

typedef enum{
	OPTION_SUCCESS,
	OPTION_NOTFOUND,
	OPTION_NOVALUE,
	OPTION_ERROR
}OPTION_STATE;

typedef enum{
	FILE_GOOD,
	FILE_NOTFOUND,
	FILE_ERROR
}FILE_STATE;

typedef enum{
	PARSER_SUCCESS,
	PARSER_FORMAT,
	PARSER_WRONGSIZE,
	PARSER_ERROR
}PARSER_STATE;

typedef enum{
	SWITCH_SUCCESS,
	SWITCH_NOTNEEDED,
	SWITCH_FAILURE
}SWITCH_STATE;

typedef enum{
	EXCLUSION_MATCH,
	EXCLUSION_NOMATCH,
	EXCLUSION_ERROR
}EXCLUSION_STATE;

typedef enum {
	STR_TO_INT_SUCCESS,
	STR_TO_INT_OVERFLOW,
	STR_TO_INT_UNDERFLOW,
	STR_TO_INT_INCONVERTIBLE
}str_to_int_err;

typedef enum {
	FIND_ZONE,
	FIND_START,
	FIND_END,
	FIND_CONTEXT,
	FIND_DELAY,
	FIND_CANCEL,
	FIND_NOTIFY,
	FIND_INTERVAL,
	FIND_EXCLUDE
}FIND;

typedef enum {
	VALID,
	VALID_PLUS_NEW,
	INVALID_PLUS_NEW,
	INVALID,
	ERROR
}DELAY_CHECK;

typedef enum {
	TIME_EQUAL,
	TIME_SMALLER,
	TIME_BIGGER,
	TIME_ERROR
}TIME_CMP;

typedef enum {
	CRON_ACTIVE,
	CRON_CHANGE,
	CRON_INACTIVE,
	CRON_DELETE
}CRON_STATE;
#endif /* TYPES_H */
