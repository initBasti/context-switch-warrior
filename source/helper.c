/**
 * @file helper.c
 * @author Sebastian Fricke
 * @license GNU Public License
 * @brief	various functions used within different modules
 */

#include "include/helper.h"
#include <getopt.h>


long encode(struct tm*);

#ifndef DOXYGEN_SHOULD_SKIP_THIS
extern FILE *popen( const char *command, const char *modes);
extern int pclose(FILE *stream);
extern int ftruncate(int fd, off_t length);
extern int fileno(FILE *stream);
extern size_t strnlen(const char *s, size_t maxlen);
extern int opterr, optind, optopt;
extern char *optarg;
extern time_t timegm(struct tm *tm);

void drawLine(int x)
{
	printf("*");
	for(int i = 0 ; i < x ; i++) {
		printf("-");
	}
	printf("*\n");
	
}
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

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

void showZones(struct config *conf)
{
	drawLine(79);
	for(int i = 0 ; i < conf->zone_amount ; i++) {
		printf("| ");
		printf("Zone:%25s Start:%02d:%02d End:%02d:%02d Context:%16s |\n",
				conf->zone_name[i], conf->ztime[i].start_hour,
				conf->ztime[i].start_minute, conf->ztime[i].end_hour,
				conf->ztime[i].end_minute, conf->zone_context[i]);
	}
	drawLine(79);
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

/* Argument Parser */

/**
 * @brief	use the gnu function getopt to read arguments into the flag struct
 *
 * @param[in]	argc	number of arguments
 * @param[in]	argv	2D array of argument strings
 * @param[in]	opt_string	arg parse options
 * @param[out]	flag	pointer to the flag structure containg the parsed options
 *
 * @retval	0	SUCCESS
 * @retval	-1	FAILURE
 */
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
 * @brief	wrapper around strtol, with robust error handling
 *
 * @param[in] input		input string 
 * @param[out] output	integer of the parsed number 
 *
 * @retval	STR_TO_INT_INCONVERTIBLE	string empty	
 * @retval	STR_TO_INT_OVERFLOW			exceed integer data type boundary
 * @retval	STR_TO_INT_UNDERFLOW		exceed integer data type negative boundary
 * @retval	STR_TO_INT_SUCCESS			parse SUCCESSful
 */
str_to_int_err strToInt(int *output, char* input)
{
	char *end = NULL;
	long number = 0;
	if(input[0] == '\0') {
		return STR_TO_INT_INCONVERTIBLE;
	}

	number = strtol(input, &end, 10);
	errno = 0;

	if(number > INT_MAX || (errno == ERANGE && number == LONG_MAX)) {
		return STR_TO_INT_OVERFLOW;
	}

	if(number < INT_MIN || (errno == ERANGE && number == LONG_MIN)) {
		return STR_TO_INT_UNDERFLOW;
	}

	if(*end != '\0') {
		return STR_TO_INT_INCONVERTIBLE;
	}

	*output = number;

	return STR_TO_INT_SUCCESS;
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
