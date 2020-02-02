/**
 * @file helper.c
 * @author Sebastian Fricke
 * @license GNU Public License
 * @brief	various functions used within different modules
 */

#include "include/helper.h"
#include <getopt.h>


long encode(struct tm*);
void switchPosExclusion(struct exclusion*, int, int);
#define DRAW_LINE(x) {\
					printf("*");\
					for(int i = 0 ; i < x ; i++) {\
						printf("-");\
					}\
					printf("*\n");\
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
extern FILE *popen( const char *command, const char *modes);
extern int pclose(FILE *stream);
extern int ftruncate(int fd, off_t length);
extern int fileno(FILE *stream);
extern size_t strnlen(const char *s, size_t maxlen);
extern int opterr, optind, optopt;
extern char *optarg;
extern time_t timegm(struct tm *tm);
#endif /* DOXYGEN_SHOULD_SKIP_THIS */


/*
 * @brief	Check if the path points to a directory
 *
 * @param[in]	path	The path to location of the directory
 * @retval	0	The directory exists
 * @retval	1	path is valid but doesn't point to a directory
 * @retval	-1	path not valid
 * @retval	-2	stat malfunctions
 */
int dirExist(char *path)
{
	struct stat s;
	int errno;
	int err = stat(path, &s);
	if(err == -1) {
		if(errno == ENOENT) {
			perror("ENOENT");
			return -1;
		}
		else {
			perror("stat");
			return -2;
		}
	}
	else {
		if(S_ISDIR(s.st_mode)) {
			return 0;
		}
		else {
			return 1;
		}
	}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
void lowerCase(char *option, int length)
{
	for(int i = 0 ; i < length ; i++) {
		if(option[i] >= 65 && option[i] <= 90)
			option[i] = option[i] + 32;
	}
}
#endif /* DOXYGEN_SHOULD_SKIP_THIS */


void stripChar(char *input, char c)
{
	int i = 0;
	int x = 0;
	char current = 0;

	while((current=input[i++]) != '\0') {
		if(current != c) {
			input[x++] = current;
		}
	}
	input[x] = '\0';
}

/**
 * @brief Array as a set implementation, compare the new value with the array
 *
 * search an array of integers for equality with the given number
 * on a match or on an invalid value, don't attach it.
 * In any other case attach the number at the end and sort the array.
 *
 * @param[in]	size	number of elements in array
 * @param[in]	element	new value to attach
 * @param[out]	arr	the array of integers
 * @retval	0	SUCCESS
 * @retval	1	Number already in set
 * @retval	-1	Invalid value
 */
int dayToSet(int *arr, int size, int element) {
	for(int i = 0 ; i < size ; i++) {
		if(arr[i] == element) {
			/* day is in set allready */
			return 1;
		}
	}

	if(element < 1 || element > 7) {
		/* element contains invalid value */
		return -1;
	}

	if(size < WEEKDAYS) {
		arr[size] = element;
	}

	bubbleSort(arr, size+1);

	return 0;
}

void bubbleSort(int* arr, int size) {
	int unsorted = 1;
	int temp = 0;
	while(unsorted) {
		unsorted = 0;
		for(int i = 0 ; i < size-1 ; i++) {
			if((arr[i] > arr[i+1] && arr[i+1] != 0) || (arr[i] == 0 && arr[i+1] != 0)) {
				temp = arr[i];
				arr[i] = arr[i+1];
				arr[i+1] = temp;
				unsorted = 1;
			}
		}
	}
}

void initExclusionStruct(struct exclusion *excl)
{
	for(int i = 0 ; i < MAX_EXCLUSION ; i++) {
		excl->type[i].holiday_start.tm_year = 0;
		excl->type[i].holiday_start.tm_mon = 0;
		excl->type[i].holiday_start.tm_mday = 0;
		excl->type[i].holiday_end.tm_year = 0;
		excl->type[i].holiday_end.tm_mon = 0;
		excl->type[i].holiday_end.tm_mday = 0;
		for(int j = 0 ; j < MAX_EXCLUSION ; j++) {
			excl->type[i].single_days[j].tm_year = 0;
			excl->type[i].single_days[j].tm_mon = 0;
			excl->type[i].single_days[j].tm_mday = 0;
		}
		for(int k = 0 ; k < WEEKDAYS ; k++) {
			excl->type[i].weekdays[k] = 0;
		}
		memset(excl->type[i].sub_type, 0, TYPE_LEN);
		excl->type[i].list_len = 0;
		strncpy(excl->type_name[i], "", TYPE_LEN);
		excl->amount = 0;
	}
}

void showZones(struct config *conf)
{
	DRAW_LINE(79);
	for(int i = 0 ; i < conf->zone_amount ; i++) {
		printf("| ");
		printf("Zone:%25s Start:%02d:%02d End:%02d:%02d Context:%16s |\n",
				conf->zone_name[i], conf->ztime[i].start_hour,
				conf->ztime[i].start_minute, conf->ztime[i].end_hour,
				conf->ztime[i].end_minute, conf->zone_context[i]);
	}
	DRAW_LINE(79);
}

void debugShowContent(struct configcontent *cont)
{
	printf("DEBUG: Content\n");
	for(int i = 0 ; i < cont->amount ; i++) {
		for(int j = 0 ; j < cont->sub_option_amount[i] ; j++) {
			printf("%d.%d: %s -> %s [@%d]\n",i , j, cont->option_name[i][j],
					cont->option_value[i][j], cont->rowindex[i]);
		}
	}
}

void debugShowError(struct error* err)
{
	printf("DEBUG: Error\n");
	for(int i = 0 ; i < err->amount ; i++) {
		printf("ERROR: index:%d, code:%d, msg:%s\n",
				err->rowindex[i], err->error_code[i],
				err->error_msg[i]);
	}
}

void showExclusions(struct exclusion *excl)
{
	DRAW_LINE(79);
	int filler = 0;
	for(int i = 0 ; i < excl->amount ; i++) {
		filler = 5;
		printf("| ");
		printf("Type:%8s ", excl->type_name[i]);
		if(strncmp(excl->type_name[i], "temp", 5) == 0) {
			if(strncmp(excl->type[i].sub_type, "list", 5) == 0 ||
				strncmp(excl->type[i].sub_type, "solo", 5) == 0) {
				printf("(");
				for(int j = 0 ; j < excl->type[i].list_len ; j++) {
					if(j != 0) {
						printf(",");
					}
					printf("%4d-%02d-%02d",
							excl->type[i].single_days[j].tm_year+1900,
							excl->type[i].single_days[j].tm_mon+1,
							excl->type[i].single_days[j].tm_mday);
					if((j+1) % 5 == 0 && j+1 < excl->type[i].list_len) {
						printf("%9s|\n|%15s", " ", " ");
					}
					filler -= 1;
					if(j == 5) {
						filler = 4;
					}
				}
				printf(")");
				for(int i = 0 ; i < filler ; i++) {
					printf("%11s", " ");
				}
				printf("%8s|\n", " ");
			}
			else if(strncmp(excl->type[i].sub_type, "range", 6) == 0) {
				printf("from %4d-%02d-%02d until %4d-%02d-%02d%32s|\n",
						excl->type[i].holiday_start.tm_year+1900,
						excl->type[i].holiday_start.tm_mon+1,
						excl->type[i].holiday_start.tm_mday,
						excl->type[i].holiday_end.tm_year+1900,
						excl->type[i].holiday_end.tm_mon+1,
						excl->type[i].holiday_end.tm_mday,
						" ");
			}
		}
		else if(strncmp(excl->type_name[i], "perm", 5) == 0) {
			if(strncmp(excl->type[i].sub_type, "list", 5) == 0 ||
				strncmp(excl->type[i].sub_type, "solo", 5) == 0) {
				printf("(");
				for(int j = 0 ; j < excl->type[i].list_len ; j++) {
					if(j != 0) {
						printf(",");
					}
					printf("%5s",
							excl->type[i].weekdays[j]==2?"Mon":
							excl->type[i].weekdays[j]==3?"Tue":
							excl->type[i].weekdays[j]==4?"Wed":
							excl->type[i].weekdays[j]==5?"Thu":
							excl->type[i].weekdays[j]==6?"Fri":
							excl->type[i].weekdays[j]==7?"Sat":"Sun"
							);
					filler -= 1;
				}
				printf(")");
				for(int i = 0 ; i < filler+2 ; i++) {
					printf("%6s", " ");
				}
				printf("%22s\n", "|");
			}
		}
	}
	DRAW_LINE(79);
}

void buildExclFormat(struct format_type* excl, char* type, char* str)
{
	if(strncmp(type, "perm", 4) == 0) {
		buildPermExclFormat(excl, str);	
	}
	if(strncmp(type, "temp", 4) == 0) {
		buildTempExclFormat(excl, str);
	}
}

void buildTempExclFormat(struct format_type* excl, char* str)
{
	char buffer[MAX_FIELD] = {0};
	char date[DATE+3] = {0};

	if(strncmp(excl->sub_type, "list", 4) == 0) {
		for(int i = 0 ; i < excl->list_len ; i++) {
			if(i == 0) {
				snprintf(date, DATE+2, "%4d-%02d-%02d",
						excl->single_days[i].tm_year+1900,
						excl->single_days[i].tm_mon+1,
						excl->single_days[i].tm_mday);
			}
			else {
				snprintf(date, DATE+3, ",%4d-%02d-%02d",
						excl->single_days[i].tm_year+1900,
						excl->single_days[i].tm_mon+1,
						excl->single_days[i].tm_mday);
			}
			strncat(buffer, date, DATE+2);
		}
		snprintf(str, MAX_ROW, "Exclude=temporary(%s)\n",buffer);
	}
	if(strncmp(excl->sub_type, "range", 5) == 0) {
		snprintf(str, MAX_ROW, "Exclude=temporary(%4d-%02d-%02d#%4d-%02d-%02d)\n",
				excl->holiday_start.tm_year+1900,
				excl->holiday_start.tm_mon+1,
				excl->holiday_start.tm_mday,
				excl->holiday_end.tm_year+1900,
				excl->holiday_end.tm_mon+1,
				excl->holiday_end.tm_mday);
	}
}

void buildPermExclFormat(struct format_type* excl, char* str)
{
	char buffer[MAX_FIELD] = {0};
	char day[DAY+2] = {0};
	int wd = 0;

	for(int i = 0 ; i < excl->list_len ; i++) {
		wd = excl->weekdays[i];
		if(i == 0) {
			snprintf(day, DAY+1, "%s",wd==1?"mo":wd==2?"tu":wd==3?"we":wd==4?"th":
					wd==5?"fr":wd==6?"sa":"su");
		}
		else {
			snprintf(day, DAY+2, ",%s",wd==1?"mo":wd==2?"tu":wd==3?"we":wd==4?"th":
					wd==5?"fr":wd==6?"sa":"su");
		}
		strncat(buffer, day, DAY+1);
	}
	snprintf(str, MAX_ROW, "Exclude=permanent(%s)\n", buffer);
}

/**
 * @brief	generate the format for either the cancel or the notify option
 *
 * transforms a 1 to a 'on' and a 0 to a 'off', if the value has changed to any
 * other value enter ERROR as value
 *
 * @param[in]	value	the boolean either 0 or 1
 * @param[in]	type	the option type either 'Cancel' or 'Notify'
 * @param[out]	str	the format string used in the writeConfig function
 */
void buildBoolFormat(int value, char *type, char *str)
{
	snprintf(str, MAX_ROW, "%s=%s\n",type, value==0?"off":value==1?"on":"ERROR");
}

struct context* initContext(struct context* ptr)
{
	ptr = malloc(sizeof(struct context));
	if(!ptr) {
		return NULL;
	}
	for(int i = 0 ; i < MAX_CONTEXT ; i++) {
		for(int j = 0 ; j < MAX_FIELD ; j++) {
			ptr->name[i][j] = 0;
		}
	}
	ptr->amount = 0;

	return ptr;
}

/**
 * @brief	execute the task _context command to retrieve the used contexts
 *
 * open a file stream with the data from the task _context command
 * which shows all used contexts in the taskwarrior config
 * parse it for the different values and save them into a heap allocated
 * array.
 *
 * @param[out]	options	heap-allocated struct with an array of strings
 */
void getContext(struct context *options)
{
	FILE *command_output = NULL;
	char command[MAX_COMMAND] = {0};
	char full_output[MAX_FIELD*MAX_ZONES] = {0};
	char *token = NULL;

	strncpy(command, "task _context | tr '\\n' ';'", MAX_COMMAND);

	command_output = popen(command, "r");
	if(!command_output) {
		perror("No context options defined");
		return;
	}
	if(fgets(full_output, MAX_ZONES*MAX_FIELD, command_output) != NULL) {
		token = strtok(full_output, ";");
		strncpy(options->name[0], token, MAX_FIELD);
		options->amount += 1;
		for(int index = 1 ; index < MAX_ZONES ; index++) {
			if((token = strtok(NULL, ";"))!= NULL) {
				strncpy(options->name[index], token, MAX_FIELD);
				options->amount += 1;
			}
		}
	}
	pclose(command_output);
}

/**
 * @brief	compare the context string with the available options
 *
 * Check if the context options are allready assigned if not
 * get the data from taskwarrior, then check if the context string
 * is found inside the options
 *
 * @param[in]	context	the context string from the config
 * @param[out]	options	the heap allocated structure of of valid values
 * @retval	0	SUCCESS
 * @retval	1	ERROR
 */
int contextValidation(struct context *options, char* context)
{
	if(options->name[0][0] == '\0' || strnlen(options->name[0], MAX_ROW) == 0) {
		getContext(options);
	}

	for(int index = 0 ; index < options->amount ; index++) {
		if(options->name[index]) {
			if(strncmp(options->name[index],
						context,
						MAX_FIELD) == 0) {
				return 0;
			}
		}
	}
	return 1;
}

/**
 * @brief	Check if the elements of a zone exist and are properly formated
 *
 * @param[in]	name	zone name string
 * @param[in]	zone_t	zone start & end time as zonetime struct
 * @param[in]	context	zone context string
 *
 * @retval	0	complete and valid zone
 * @retval	-1	incomplete or invalid zone
 */
int zoneValidation(char *name, struct zonetime *zone_t, char *context)
{
	if(strnlen(name, MAX_ROW) > MAX_OPTION || strnlen(name, MAX_ROW) < 1) {
		return -1;
	}
	if(strnlen(context, MAX_ROW) > MAX_CONTEXT ||
			strnlen(context, MAX_ROW) < 1) {
		return -1;
	}
	if(zone_t->start_hour + zone_t->start_minute == 0 ||
			zone_t->end_hour + zone_t->end_minute == 0) {
		return -1;
	}
	if(zone_t->start_hour < 0 || zone_t->start_hour > 24 ||
			zone_t->end_hour < 0 || zone_t->end_hour > 24) {
		return -1;
	}
	if(zone_t->start_minute < 0 || zone_t->start_minute > 60 ||
			zone_t->end_minute < 0 || zone_t->end_minute > 60) {
		return -1;
	}
	return 0;
}

int currentContext(char* context)
{
	char command[MAX_COMMAND] = {0};
	char full_output[MAX_COMMAND] = {0};
	FILE* command_output = NULL;

	strncpy(command, "task _get rc.context", MAX_COMMAND);
	command_output = popen(command, "r");
	if(!command_output) {
		return -1;
	}
	if(fgets(full_output, MAX_CONTEXT, command_output) != NULL) {
		strncpy(context, full_output, MAX_CONTEXT);
		pclose(command_output);
		if(context[0] == '\0') {
			return -1;
		}
		return 0;
	}
	return -1;
}

int getArgs(struct flags *flag, int argc, char **argv, char *opt_string)
{
	int arg_int = 0;
	int delay = 0;
	int c = 0;
	opterr = 0;
	optind = 0;
	optopt = 0;
	optarg = NULL;

	if(argv == NULL || opt_string == NULL) {
		return -1;
	}

	while((c=getopt (argc, argv, opt_string)) != -1) {
		switch(c) {
			case 'd':
				if(optarg ==  NULL) {
					fprintf(stderr, "Option -d: requires an argument\n");
					return -1;
				}
				if((delay = parseTimeSpan(optarg)) != -1) {
					flag->delay = delay;
				}
				else {
					fprintf(stderr, "Valid for -d: int/float + min/h/d\n");
					return -1;
				}
				break;
			case 'h':
				flag->help = 1;
				break;
			case 's':
				flag->show = 1;
				break;
			case 'v':
				if(optarg == NULL) {
					if(flag->verbose != NULL) {
						*flag->verbose = 1;
					}
				}
				else {
					if(strToInt(&arg_int, optarg) == STR_TO_INT_SUCCESS) {
						if( arg_int == 1 || arg_int == 2 ) {
							*flag->verbose = arg_int;
						}
						else {
							fprintf(stderr, "Valid for -v: 1 | 2\n");
							return -1;
						}
					}
				}
				break;
			case 'n':
				if(optarg ==  NULL) {
					fprintf(stderr, "Option -n: requires an argument\n");
					return -1;
				}
				if(strToInt(&arg_int, optarg) == STR_TO_INT_SUCCESS) {
					if( arg_int == 0 || arg_int == 1 ) {
						flag->notify_on = arg_int;
					}
					else {
						fprintf(stderr, "Valid for -n: 0 | 1\n");
						return -1;
					}
				}
				break;
			case 'c':
				if(optarg ==  NULL) {
					fprintf(stderr, "Option -c: requires an argument\n");
					return -1;
				}
				if(strToInt(&arg_int, optarg) == STR_TO_INT_SUCCESS) {
					if( arg_int == 0 || arg_int == 1 ) {
						flag->cancel_on = arg_int;
					}
					else {
						fprintf(stderr, "Valid for -c: 0 | 1\n");
						return -1;
					}
				}
				break;
			case 'i':
				if(optarg == NULL) {
					fprintf(stderr, "Option -i: requires an argument\n");
					return -1;
				}
				if(strToInt(&arg_int, optarg) == STR_TO_INT_SUCCESS) {
					if(arg_int >= 0) {
						flag->cron_interval = arg_int;
					}
					else {
						fprintf(stderr, "Valid for -i: integer > 0\n");
						return -1;
					}
				}
				else {
					return -1;
				}
				break;
			case '?':
				fprintf(stderr, "unknown option %c\n", optopt);
				return -1;
				break;
			default:
				printf("DEBUG hit return -1; @ default \n");
				return -1;
		}
	}
	for (int index = optind; index < argc; index++) {
		printf ("Non-option argument %s\n", argv[index]);
	}
	return 0;
}

/*
 * showHelp:
 * prints usage information about the tool.
 */
void showHelp()
{
	printf("[C]ontext [S]witch [W]arrior\n");
	printf("Usage information:\n");
	printf("Command line options:\n");
	printf("-h - help (show usage information)\n");
	printf("-v - verbose (show messages about the details of a run)\n");
	printf("-s - show (show the options from the config)\n");
	printf("-d - delay (add a delay to the switch of a context)\n");
	printf("     requires an argument, valid values:\n");
	printf("     integer/float number & m|min|minute or h|hour or d|day)\n");
	printf("-c - cancel (Set if csw should cancel active tasks)\n");
	printf("     requires an argument, valid values:\n");
	printf("     1(yes) or 0(no) (change the behavior permanently)\n");
	printf("-n - notify (Set if csw should send notifications)\n");
	printf("     requires an argument, valid values:\n");
	printf("     1(yes) or 0(no) (change the behavior permanently)\n");
}

/*
 * onlyDigits:
 * checks if the input string contains a letter or if it is a pure number
 * @parameter(in): the string input, the length of the string
 * return:			1 on only digits
 * 					0 on contains letters
 */
int onlyDigits(char *input, size_t len)
{
	for(size_t i = 0 ; i < len ; i++) {
		if(!isdigit(input[i])) {
			return 0;
		}
	}
	return 1;
}


/*
 * str_to_int:
 * A wrapper around strtol, that tries to implement a robust
 * error handling for the specific use case.
 * @param (out) output is the number after the operation
 * @param (in) input is the string that is send by the function
 * @return the error or success code
 */
str_to_int_err strToInt(int *output, char* input)
{
	char *end = NULL;
	long number = 0;
	if(input[0] == '\0') {
		printf("Error @ str_to_int: first character is a string terminator\n");
		return STR_TO_INT_INCONVERTIBLE;
	}

	number = strtol(input, &end, 10);
	errno = 0;

	if(number > INT_MAX || (errno == ERANGE && number == LONG_MAX)) {
		printf("Error @ str_to_int: Input overflows int type size\n");
		return STR_TO_INT_OVERFLOW;
	}

	if(number < INT_MIN || (errno == ERANGE && number == LONG_MIN)) {
		printf("Error @ str_to_int: Input underflows int type size\n");
		return STR_TO_INT_UNDERFLOW;
	}

	if(*end != '\0') {
		return STR_TO_INT_INCONVERTIBLE;
	}

	*output = number;

	return STR_TO_INT_SUCCESS;
}

/**
 * @brief compare string with valid keys and return the enum(FIND) int value
 *
 * @param[in]	table	array of valid keys with values
 * @param[in]	str	the comparison string
 * @retval	FIND	one of the enum values of the FIND enumeration
 * @retval	BAD_KEY	-1 on a invalid key
 */
int valueForKey(struct keyvalue* table, char* str)
{
	struct keyvalue *temp = NULL;

	for(int i = 0 ; i < VALID_OPTIONS ; i++) {
		temp = &table[i];
		if(strncmp(str, temp->key, MAX_OPTION_NAME) == 0) {
			return temp->value;
		}
	}
	return BAD_KEY;
}

/**
 * @brief add a new member to the struct error
 *
 * Used for interfunction communication about a wrong syntax in the config
 * of the user.
 *
 * @param[out]	error	pointer to structure error
 * @param[in]	error_code	a integer (-1|-2|-3), points out the error type
 * @param[in]	error_msg	a string of max. length MAX_ROW with error details
 * @param[in]	index	index of the row where the error is located
 *
 */
void addError(struct error *error, int error_code, char *error_msg, int index)
{
	int current = error->amount;

	error->error_code[current] = error_code;
	strncpy(error->error_msg[current], error_msg, MAX_ROW);
	error->rowindex[current] = index;
	error->amount += 1;
}

/**
 * @brief add the minutes to the time
 *
 * @param[out]	time	struct tm pointer with the time to be increased
 * @param[in]	min	minute integer
 */
void increaseTime(int min, struct tm* time)
{
	time->tm_min += min;
	time->tm_isdst = -1;
	timegm(time);
}

/**
 * @brief compare 2 dates as struct tm with each other
 *
 * @param[in]	time	the current date and time
 * @param[in]	spec	the specified date and time to be compared
 *
 * @retval	TIME_EQUAL	both have the exact same date and time
 * @retval	TIME_SMALLER	specific date &/| time is smaller than the current
 * @retval	TIME_BIGGER	specific date &/| time is bigger than the current
 * @retval	TIME_ERROR	no current time provided
 */
TIME_CMP compareTime(struct tm* time, struct tm* spec)
{
	if(!time || time->tm_year+time->tm_mon+time->tm_mday == 0) {
		return TIME_ERROR;
	}
	if(encode(time) > encode(spec)) {
		return TIME_SMALLER;
	}
	if(encode(time) < encode(spec)) {
		return TIME_BIGGER;
	}
	if(encode(time) == encode(spec)) {
		return TIME_EQUAL;
	}
	return TIME_ERROR;
}

long encode(struct tm* time)
{
	return time->tm_year*10000000000 + time->tm_mon*100000000 +
		time->tm_mday*1000000 + time->tm_hour*10000 +
		time->tm_min*100;
}

void resetExclusion(struct exclusion* excl, int index)
{
	int switch_index = 0;
	if(index >= excl->amount) {
		return;
	}
	if(strncmp(excl->type_name[index], "perm", TYPE_LEN) == 0) {
		return;
	}
	if(index != excl->amount-1) {
		switch_index = excl->amount-1;
		switchPosExclusion(excl, index, switch_index);
		excl->amount -= 1;
		return;
	}

	if(strncmp(excl->type[index].sub_type, "list", TYPE_LEN) == 0) {
		for(int i = 0 ; i < excl->type[index].list_len ; i++) {
			resetTm(&excl->type[index].single_days[i]);
		}
		excl->type[index].list_len = 0;
	}
	if(strncmp(excl->type[index].sub_type, "range", TYPE_LEN) == 0) {
		resetTm(&excl->type[index].holiday_start);
		resetTm(&excl->type[index].holiday_start);
	}
	memset(excl->type[index].sub_type, 0, TYPE_LEN);
	memset(excl->type_name[index], 0, TYPE_LEN);
	excl->amount -= 1;
}

void switchPosExclusion(struct exclusion* excl, int new, int old) {
	if(strncmp(excl->type_name[old], "perm", TYPE_LEN) == 0) {
		for(int i = 0 ; WEEKDAYS ; i++) {
			excl->type[new].weekdays[i] = excl->type[old].weekdays[i];
		}
		strncpy(excl->type[new].sub_type, excl->type[old].sub_type, TYPE_LEN);
		memset(excl->type[old].sub_type, 0, TYPE_LEN);
		resetTm(&excl->type[new].holiday_start);
		resetTm(&excl->type[new].holiday_end);
		resetTm(&excl->type[old].holiday_start);
		resetTm(&excl->type[old].holiday_end);
		for(int i = 0 ; i < MAX_EXCLUSION ; i++) {
			resetTm(&excl->type[new].single_days[i]);
			resetTm(&excl->type[old].single_days[i]);
		}
		excl->type[new].list_len = excl->type[old].list_len;
		excl->type[old].list_len = 0;
	}
	else if(strncmp(excl->type_name[old], "temp", TYPE_LEN) == 0) {
		if(strncmp(excl->type[old].sub_type, "range", TYPE_LEN) == 0) {
			copyTm(&excl->type[new].holiday_start,
					&excl->type[old].holiday_start);
			copyTm(&excl->type[new].holiday_end,
					&excl->type[old].holiday_end);
			resetTm(&excl->type[old].holiday_start);
			resetTm(&excl->type[old].holiday_end);
			for(int i = 0 ; i < MAX_EXCLUSION ; i++) {
				resetTm(&excl->type[new].single_days[i]);
				resetTm(&excl->type[old].single_days[i]);
			}
			for(int i = 0 ; i < WEEKDAYS ; i++) {
				excl->type[new].weekdays[i] = excl->type[old].weekdays[i] = 0;
			}
			excl->type[new].list_len = excl->type[old].list_len = 0;
			strncpy(excl->type[new].sub_type,
					excl->type[old].sub_type, TYPE_LEN);
			memset(excl->type[old].sub_type, 0, TYPE_LEN);
		}
		else if(strncmp(excl->type[old].sub_type, "list", TYPE_LEN) == 0) {
			resetTm(&excl->type[new].holiday_start);
			resetTm(&excl->type[new].holiday_end);
			resetTm(&excl->type[old].holiday_start);
			resetTm(&excl->type[old].holiday_end);
			for(int i = 0 ; i < MAX_EXCLUSION ; i++) {
				copyTm(&excl->type[new].single_days[i],
						&excl->type[old].single_days[i]);
				resetTm(&excl->type[old].single_days[i]);
			}
			for(int i = 0 ; i < WEEKDAYS ; i++) {
				excl->type[new].weekdays[i] = excl->type[old].weekdays[i] = 0;
			}
			excl->type[new].list_len = excl->type[old].list_len;
			excl->type[old].list_len = 0;
			strncpy(excl->type[new].sub_type,
					excl->type[old].sub_type, TYPE_LEN);
			memset(excl->type[old].sub_type, 0, TYPE_LEN);
		}
	}
	strncpy(excl->type_name[new], excl->type_name[old], TYPE_LEN);
	memset(excl->type_name[old], 0, TYPE_LEN);
}

void resetTm(struct tm* target)
{
	target->tm_year = 0;
	target->tm_mon = 0;
	target->tm_mday = 0;
	target->tm_hour = 0;
	target->tm_min = 0;
	target->tm_sec = 0;
	target->tm_wday = 0;
	target->tm_yday = 0;
	target->tm_isdst = 0;
}

void copyTm(struct tm* target, struct tm* source)
{
	target->tm_year = source->tm_year;
	target->tm_mon = source->tm_mon;
	target->tm_mday = source->tm_mday;
	target->tm_hour = source->tm_hour;
	target->tm_min = source->tm_min;
	target->tm_sec = source->tm_sec;
	target->tm_wday = source->tm_wday;
	target->tm_yday = source->tm_yday;
	target->tm_isdst = source->tm_isdst;
}

/**
 * @brief	use <localtime>"()" to build a time structure
 *
 * @param[in]	rawtime	unix timestamp
 * @param[out]	date	pointer to a struct tm
 * @retval	0	SUCCESS
 * @retval	-1	FAILURE
 */
int getDate(struct tm *date, time_t rawtime)
{
	/**
	 * temp variable necessary because the addresses of the instance variables
	 * change after localtime.
	 */
	struct tm *temp = NULL;
	temp = localtime(&rawtime);
	if(temp == NULL) {
		return -1;
	}
	date->tm_year = temp->tm_year;
	date->tm_mon = temp->tm_mon;
	date->tm_mday = temp->tm_mday;
	date->tm_wday = temp->tm_wday;
	date->tm_hour = temp->tm_hour;
	date->tm_min = temp->tm_min;
	date->tm_sec = temp->tm_sec;
	date->tm_isdst = 0;

	return 0;
}

/**
 * @brief	use sendNotification to send a error notification to the user
 *
 * @param[in]	pointer to the error structure
 *
 * @retval	0	notification send successful
 * @retval	-1	sending failed
 */
int notifyError(struct error* error)
{
	char command[MAX_MSG] = {0};

	for(int i = 0 ; i < error->amount ; i++) {
		snprintf(command, MAX_MSG,
				"notify-send 'CSW Error-code:%d at line:%d' '%s' --icon=dialog-error",
				error->error_code[i], error->rowindex[i], error->error_msg[i]);
		if(sendNotification(command) != 0) {
			return -1;
		}
	}
	return 0;
}

/**
 * @brief send a string to notify-send to communicate with the user
 *
 * @param[in]	str	containing warning:config errors or info about following actions
 * @retval	0	successful send
 * @retval	-1	sending failed
 */
int sendNotification(char* str)
{
	FILE *command_output = NULL;
	char string[10][MAX_ROW] = {{0}};
	int index = 0;

	errno = 0;
	command_output = popen(str, "r");
	if(!command_output) {
		return -1;
	}
	if(errno != 0) {
		return -1;
	}

	while(fgets(string[index], MAX_ROW, command_output) != NULL) {
		errno = 0;
		if(errno != 0) {
			return -1;
		}
		index++;
	}
	pclose(command_output);
	return 0;
}

/**
 * @brief	check the system if the required tools are installed for notification
 *
 * @retval	0	SUCCESS
 * @retval	-1	FAILURE
 */
int checkNotificationSetup()
{
	const char *args[64] = {"/usr/bin/notify-send" , "notify-send" ,
							"notification enabled", "--expire-time=2000"};
	pid_t my_pid;

	errno = 0;
	if((my_pid = fork()) == 0) {
		if(execl(args[0], args[1], args[2], args[3], (char*)0) == -1) {
			perror("test execution of notify-send failed");
			return -1;
		}
	}

	return 0;
}
