/**
 * @file helper.c
 * @author Sebastian Fricke
 * @license GNU Public License
 * @brief	various functions used within different modules
 */

#include "include/helper.h"


long encodeDay(struct tm*);
long encode(struct tm*);

#ifndef DOXYGEN_SHOULD_SKIP_THIS
extern FILE *popen( const char *command, const char *modes);
extern int pclose(FILE *stream);
extern size_t strnlen(const char *s, size_t maxlen);
extern time_t timegm(struct tm *tm);

void drawLine(int x)
{
	printf("*");
	for(int i = 0 ; i < x ; i++) {
		printf("-");
	}
	printf("*\n");
	
}

void lowerCase(char *option, int length)
{
	for(int i = 0 ; i < length ; i++) {
		if(option[i] >= 65 && option[i] <= 90)
			option[i] = option[i] + 32;
	}
}
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

/**
 * @brief	remove any occurence of the given char from the string
 *
 * @param	input	string
 * @param[in]	c	character to be stripped
 */
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

#ifndef DOXYGEN_SHOULD_SKIP_THIS
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
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

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

/**
 * @brief	use _get rc.context command from taskwarrior to get the current context
 *
 * @param[out]	context		string for the result
 *
 * @retval	0	SUCCESS
 * @retval	-1	FAILURE
 */
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

/**
 * @brief	Check the string for a timespan format, return a minute integer.
 *
 * 	- an integer and a legal type (example: 1min, 2hour, 1day)
 * 	- a floting point number and a legal type (example: 2.5min 1,2h, 0.03d)
 * 	- just an integer is parsed as minutes
 * 	- legal types:
 * 		- min / m / minute
 * 		- hour / h
 * 		- day / d
 * multiply the number with the correct multiplier to return time in minutes.
 * use: <multiplierForType>"("char* type")"
 *
 * @param[in]	str	the time span string, number and type without space
 *
 * @retval	positive integer	the time in minutes on SUCCESS
 * @retval	-1	FAILURE
 */
int parseTimeSpan(char *str)
{
	char type[10] = {0};
	char *temp = NULL;
	int number = 0;
	float float_number = 0;
	size_t len = 0;

	if(str == NULL) {
		return -1;
	}
	len = strnlen(str, DELAY_FORMAT_LEN);

	if(strchr(str, '.') != NULL) {
		sscanf(str, "%f%s", &float_number, type);
	}
	else if((temp=strchr(str, ',')) != NULL) {
		*temp = '.';
		sscanf(str, "%f%s", &float_number, type);
	}
	else if(onlyDigits(str, len)){
		sscanf(str, "%d", &number);
		if(number>0) {
			return number;
		}
	}
	else {
		sscanf(str, "%d%s", &number, type);
	}

	if(type[0] == '\0' || (number == 0 && float_number == 0)) {
		return -1;
	}
	if(float_number > 0) {
		return (int)(float_number*multiplierForType(type));
	}
	else if(number > 0) {
		return number * multiplierForType(type);
	}
	return -1;
}

/**
 * @brief	Find the multiplier to calculate time into minutes from type
 *
 * Recognized types are:
 * 	- min/minute/m(1)
 * 	- h/hour(60)
 * 	- d/day(1440)
 *
 * used in: <parseDelay>"("char *str")"
 * @param[in]	type	string of the type
 * @retval	multiplier integer	SUCCESS
 * @retval	-1	FAILURE
 */
int multiplierForType(char* type)
{
	size_t len = strnlen(type, DELAY_FORMAT_LEN);
	if(len < 1) {
		return -1;
	}

	lowerCase(type, len);

	if(strncmp(type, "minute", len) == 0) {
		return 1;
	}
	else if(strncmp(type, "hour", len) == 0) {
		return 60;
	}
	else if(strncmp(type, "day", len) == 0) {
		return 1440;
	}
	else {
		return -1;
	}
}

/**
 * @brief	check if a string contains only numbers
 *
 * @param[in]	input	string
 * @param[in]	len		string size
 *
 * @retval		1	string contains only digits
 * @retval		0 	string contains letters
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
 * @brief compare 2 dates as struct tm with each other without hour/minute/sec
 *
 * @param[in]	time	current date and time
 * @param[in]	spec	specified date to be compared
 *
 * @retval	TIME_EQUAL	both have the exact same date
 * @retval	TIME_SMALLER	specific date smaller than the current
 * @retval	TIME_BIGGER	specific date bigger than the current
 * @retval	TIME_ERROR	no current time provided
 */
TIME_CMP compareDate(struct tm* time, struct tm* spec)
{
	if(!time || time->tm_year+time->tm_mon+time->tm_mday == 0) {
		return TIME_ERROR;
	}
	if(encodeDay(time) > encodeDay(spec)) {
		return TIME_SMALLER;
	}
	if(encodeDay(time) < encodeDay(spec)) {
		return TIME_BIGGER;
	}
	if(encodeDay(time) == encodeDay(spec)) {
		return TIME_EQUAL;
	}
	return TIME_ERROR;
}

/**
 * @brief compare 2 dates as struct tm with each other
 *
 * @param[in]	time	current date and time
 * @param[in]	spec	specified date and time to be compared
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

/**
 * @brief	encode a date by multiplying the the members with a value 10^x
 *
 * without hour/minute/sec
 * 
 * @param[in]	tm structure instance pointer
 *
 * @retval	encoded date
 */
long encodeDay(struct tm* time)
{
	return time->tm_year*10000000000 + time->tm_mon*100000000 +
		time->tm_mday*1000000;
}

/**
 * @brief	encode a date by multiplying the the members with a value 10^x
 * 
 * @param[in]	tm structure instance pointer
 *
 * @retval	encoded date
 */
long encode(struct tm* time)
{
	return time->tm_year*10000000000 + time->tm_mon*100000000 +
		time->tm_mday*1000000 + time->tm_hour*10000 +
		time->tm_min*100;
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
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
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

/**
 * @brief	use <localtime>"()" to build a time structure
 *
 * temp variable necessary because the addresses of the instance variables
 * change after localtime.
 *
 * @param[in]	rawtime	unix timestamp
 * @param[out]	date	pointer to a struct tm
 *
 * @retval	0	SUCCESS
 * @retval	-1	FAILURE
 */
int getDate(struct tm *date, time_t rawtime)
{
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
 * @brief	wrapper for sendNotification to send a error notification
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
