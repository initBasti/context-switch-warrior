#include "include/parser.h"
#ifndef CONFIG_H
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#endif

extern size_t strnlen (__const char *__string, size_t __maxlen);

/*
 * parseTime:
 * reads in a time string of format HH:MM and splits it up in
 * hours and minutes, parses those to integers and assigns them
 * to the config struct.
 * @param[in]	time_string [HH:MM]
 * @param[out]	hour -> start/end _hour of ztime
 * 					 minute -> start/end _minute of ztime
 * @retval	0 = SUCCESS, 1 = ERROR
 */
int parseTime(int *hour, int *minute, char *time_string)
{
	char *token = NULL;
	char hour_str[3];
	char min_str[3];
	if(!time_string) {
		return 1;
	}
	if(strnlen(time_string, 10) > 5) {
		*hour = 0;
		*minute = 0;
		return 1;
	}

	token = strtok(time_string, ":");
	if(!token) {
		return 1;
	}
	strncpy(hour_str, token, 3);

	token = strtok(NULL, "\0");
	if(!token) {
		return 1;
	}
	strncpy(min_str, token, 3);

	*hour = atoi(hour_str);
	*minute = atoi(min_str);

	return 0;
}

/*
 * parseDate:
 * checks if the string is conform to the exclusion date format
 * YYYY-MM-DD , if that is the case parse the different values
 * year month and day. Parse those to integers and save the date
 * into the exclusion instance.
 * param[in]	input string that was checked for length before
 * param[out]	the exclusion date field of the exclusion instance
 * return:	0 = SUCCESS
 * 			1 = FAILURE
 */
int parseDate(struct tm *date, char *input)
{
	char *token = NULL;
	int token_len = 0;
	if(!input) {
		return -2;
	}

	token = strtok(input, "-");
	if(!token || (exclTokenLength(&token_len, "year", token) != 0)) {
		return -1;
	}
	date->tm_year = atoi(token)-1900;

	token = strtok(NULL, "-");
	if(!token || (exclTokenLength(&token_len, "month", token) != 0)) {
		return -1;
	}
	date->tm_mon = atoi(token)-1;

	token = strtok(NULL, "\n");
	if(!token || (exclTokenLength(&token_len, "day", token) != 0)) {
		return -1;
	}
	date->tm_mday = atoi(token);

	if(date->tm_year == 0 || (date->tm_mon == 0 && date->tm_year == 0) ||
			date->tm_mday == 0) {
		return -1;
	}
	return 0;
}

/*
 * parseExclusion:
 * check the exclusion statement for the keywords 'permanent' or 'temporary'
 *
 * On permanent:
 * get the string inbetween the brackets and get the tokens between ','
 * they have to be in weekday format: mo, tu, we, th, fr, sa, su
 *
 * On temporary:
 * get the string inbetween the brackets, check for a '#'
 *
 * on #:
 * date string before # = start date, date string after # = end date
 *
 * without #:
 * get the tokens between ',' they have to be in date format YYYY-MM-DD
 *
 * param[in]	option string of the exclusion value
 * param[out]	the struct instance of the config exclusions
 * @retval	0=SUCCESS
 * 		   1=FAILURE
 */
PARSER_STATE parseExclusion(struct exclusion *excl, char *option)
{
	char *token = NULL;
	int state = 0;

	if(!option) {
		return PARSER_ERROR;
	}
	stripChar(option, ' ');

	token = strtok(option, "(");
	if(!token) {
		goto wrong_format;
	}
	if(strncmp(token, "permanent", 10) == 0) {
		token = strtok(NULL, ")");
		if(!token) {
			goto parse_error;
		}
		state = parsePermanent(token, excl);
	}
	else if(strncmp(token, "temporary", 10) == 0) {
		token = strtok(NULL, ")");
		if(!token) {
			goto parse_error;
		}
		state = parseTemporary(token, excl);
	}
	else {
		goto parse_error;
	}

	switch(state) {
		case 0:
			goto parse_success;
		case -1:
			goto wrong_format;
		case -2:
			goto wrong_size;
		default:
			goto parse_error;
	}

	wrong_format:
		return PARSER_FORMAT;
	wrong_size:
		return PARSER_WRONGSIZE;
	parse_error:
		return PARSER_ERROR;
	parse_success:
		return PARSER_SUCCESS;
}

/*
 * parsePermanent:
 * takes a string that was found inside the permanent identifier
 * checks it for 4 possible states:
 * 	- A single element with the correct size (weekday: 2chars)
 * 	- A single element with an incorrect size
 * 	- Multiple elements with correct size and correct delimiter (deli: ',')
 * 	- Multiple elements with either incorrect size or delimiter
 * @parameter(in): the string from the config within permanent()
 * @parameter(out): the exclusion struct
 * @retval	0 = Success,
 *		  -1 = wrong format,
 * 		  -2 = wrong size
*/
int parsePermanent(char *input, struct exclusion *excl)
{
	int input_len = 0;
	int list_len = 0;
	int size_check = 0;
	int *dest = NULL;
	int *length = NULL;
	int set_check = 0;
	int *set = {0};
	struct substr *head = allocate_substring();

	if(strchr(input,',') != NULL) {
		if(get_substring(input, head, &list_len, ',') != 0) {
			return -1;
		}
		if(equalListElements(head) != 0) {
			free_substring(head);
			return -2;
		}
		/* Only 7 weekdays are allowed */
		if(list_len < 1) {
			free_substring(head);
			return -1;
		}
		list_len = 0;
		for(struct substr *ptr = head ; ptr != NULL ; ptr=ptr->next) {
			set = &excl->type[excl->amount].weekdays[0];
			length = &excl->type[excl->amount].list_len;
			size_check = exclTokenLength(&input_len, "perm", ptr->member);
			if(size_check == 0) {
				if(*length >= 7) {
					break;
				}
				set_check = dayToSet(set, *length,
										parseWeekday(ptr->member, DAY));
			}
			else {
				free_substring(head);
				return -2;
			}
			if(set_check == 0) {
				if(excl->type[excl->amount].sub_type[0] == '\0' ||
					strnlen(excl->type[excl->amount].sub_type, MAX_ROW) == 0) {
					strncpy(excl->type[excl->amount].sub_type, "list", 5);
				}
				if(excl->type_name[excl->amount][0] == '\0' ||
					strnlen(excl->type_name[excl->amount], MAX_ROW) == 0) {
					strncpy(excl->type_name[excl->amount], "perm", 5);
				}
				*length += 1;
			}
			if(set_check == 1) {
			}
			if(set_check == -1) {
				free_substring(head);
				return -1;
			}
		}
		free_substring(head);
		excl->amount = excl->amount + 1;
	}
	else {
		free(head);
		size_check = exclTokenLength(&input_len, "perm", input);
		if(size_check == 0) {
			dest = &excl->type[excl->amount]
					.weekdays[excl->type[excl->amount].list_len];
			*dest = parseWeekday(input, DAY);
		}
		else {
			return -2;
		}
		if(*dest == -1) {
			return -1;
		}
		strncpy(excl->type[excl->amount].sub_type, "solo", 5);
		strncpy(excl->type_name[excl->amount], "perm", 5);
		excl->type[excl->amount].list_len += 1;
		excl->amount = excl->amount + 1;
	}
	return 0;
}

/*
 * parseTemporary:
 * takes a string that was found inside the temporary identifier
 * checks it for 6 possible states:
 * 	- A single element with correct size (date: 10chars)
 * 	- A single element with incorrect size
 * 	- 2 dates delimited by a # indicating a range
 * 	- multiple dates delimited by ',' as a list
 * 	- multiple elements with incorrect size or delimiter
 * @parameter(in): the string from the config within temporary()
 * @parameter(out): the exclusion struct
 * @retval	0 = Success,
 * 			-1 = wrong format,
 * 			-2 = wrong size,
 * 			-3 = error
 */
int parseTemporary(char *input, struct exclusion *excl)
{
	int input_len = 0;
	int list_len = 0;
	int size_check = 0;
	int format_check = 0;
	struct tm *dest = NULL;
	struct substr *head = allocate_substring();

	if(strchr(input,'#') != NULL) {
		if(get_substring(input, head, &list_len, '#') != 0) {
			free(head);
			return -1;
		}
		if(equalListElements(head) != 0) {
			free_substring(head);
			return -2;
		}
		/* for the range functionality of temporary only 2 dates are allowed */
		if(list_len > 2) {
			free_substring(head);
			return -1;
		}
		for(struct substr *ptr = head ; ptr != NULL ; ptr=ptr->next) {
			size_check = exclTokenLength(&input_len, "temp", ptr->member);
			if(size_check == 0) {
				if(ptr->next != NULL) {
					dest = &excl->type[excl->amount].holiday_start;
				}
				else {
					dest = &excl->type[excl->amount].holiday_end;
				}
				format_check = parseDate(dest, ptr->member);
			}
			else {
				return -2;
			}
			if(format_check == 0) {
				strncpy(excl->type[excl->amount].sub_type, "range", 6);
				strncpy(excl->type_name[excl->amount], "temp", 5);
			}
			else {
				return -1;
			}
		}
		free_substring(head);
		excl->amount = excl->amount + 1;
	}
	else if(strchr(input,',') != NULL) {
		if(get_substring(input, head, &list_len, ',') != 0) {
			free(head);
			return -1;
		}
		if(list_len > MAX_EXCL) {
			free_substring(head);
			return -1;
		}
		for(struct substr *ptr = head ; ptr != NULL ; ptr=ptr->next) {
			size_check = exclTokenLength(&input_len, "temp", ptr->member);
			if(size_check == 0) {
				dest = &excl->type[excl->amount]
						.single_days[excl->type[excl->amount].list_len++];
				format_check = parseDate(dest, ptr->member);
			}
			else {
				return -2;
			}
			if(format_check != 0) {
				free_substring(head);
				return -1;
			}
			strncpy(excl->type[excl->amount].sub_type, "list", 5);
			strncpy(excl->type_name[excl->amount], "temp", 5);
		}
		free_substring(head);
		excl->amount = excl->amount + 1;
	}
	else {
		free(head);
		size_check = exclTokenLength(&input_len, "temp", input);
		if(size_check == 0) {
			dest = &excl->type[excl->amount]
					.single_days[excl->type[excl->amount].list_len];
			format_check = parseDate(dest, input);
		}
		else {
			return -2;
		}
		if(format_check != 0) {
			return -1;
		}
		strncpy(excl->type[excl->amount].sub_type, "solo", 5);
		strncpy(excl->type_name[excl->amount], "temp", 5);
		excl->type[excl->amount].list_len += 1;
		excl->amount = excl->amount + 1;
	}
	return 0;
}

/*
 * parseWeekday:
 * reads a weekday (mo-su) and parses it to a number
 * in ascending order from su = 0 until sa = 6
 * @parameter(in): string with the weekday, the size of the string
 * return:	returns the appropriate number for the weekday on success
 * 			or -1 on an error
 */
int parseWeekday(char *input, int size)
{
	lowerCase(input, size);
	if(strncmp(input, "mo", size) == 0) {
		return 2;
	}
	else if(strncmp(input, "tu", size) == 0) {
		return 3;
	}
	else if(strncmp(input, "we", size) == 0) {
		return 4;
	}
	else if(strncmp(input, "th", size) == 0) {
		return 5;
	}
	else if(strncmp(input, "fr", size) == 0) {
		return 6;
	}
	else if(strncmp(input, "sa", size) == 0) {
		return 7;
	}
	else if(strncmp(input, "su", size) == 0) {
		return 1;
	}
	else {
		return -1;
	}
}


int exclTokenLength(int *length, char *type, char *input)
{
	*length = snprintf(NULL, 0, "%s", input);
	if(strncmp(type, "perm", 5) == 0) {
		return 2-*length;
	}
	else if(strncmp(type, "temp", 5) == 0) {
		return DATE-*length;
	}
	else if(strncmp(type, "year", 5) == 0) {
		return 4-*length;
	}
	else if(strncmp(type, "month", 6) == 0) {
		return 2-*length;
	}
	else if(strncmp(type, "day", 4) == 0) {
		return 2-*length;
	}
	else {
		return -1;
	}
}

/**
 * @brief	capture the options and translate them to the config structure
 *
 * Parse the different options and fill the error structure whenever a option
 * has a incorrect format.
 *
 * @param[in]	content	configcontent structure pointer from readConfig()
 * @param[out]	error	error structure pointer
 * @param[out]	config	config structure pointer for the parse output
 *
 * @retval	0	SUCCESS
 * @rerval -1	FAILURE
 */
int parseConfig(struct configcontent *content, struct error* error,
		struct config* config)
{
	char msg[MAX_ROW] = {0};
	int value = 0;
	int result = 0;
	int zamount = config->zone_amount;
	struct context *context = NULL;
	char temp_name[MAX_OPTION] = {0};
	char temp_context[MAX_CONTEXT] = {0};
	struct zonetime temp_time = {0};

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

	context = initContext(context);
	if(!context) {
		return -1;
	}

	for(int i = 0 ; i < content->amount ; i++) {
		for(int j = 0 ; j < content->sub_option_amount[i] ; j++) {
			value = valueForKey(&lookuptable[0], content->option_name[i][j]);
			if(zoneValidation(temp_name, &temp_time,
						temp_context) == 0) {
				strncpy(config->zone_name[zamount],
						temp_name,
						MAX_OPTION);
				strncpy(config->zone_context[zamount],
						temp_context,
						MAX_OPTION);
				config->ztime[zamount].start_hour = temp_time.start_hour;
				config->ztime[zamount].start_minute = temp_time.start_minute;
				config->ztime[zamount].end_hour = temp_time.end_hour;
				config->ztime[zamount].end_minute = temp_time.end_minute;
				memset(temp_name, 0, MAX_OPTION);
				memset(temp_context, 0, MAX_CONTEXT);
				temp_time.start_hour = 0;
				temp_time.start_minute = 0;
				temp_time.end_hour = 0;
				temp_time.end_minute = 0;
				config->zone_amount += 1;
				zamount = config->zone_amount;
			}
			switch(value) {
				case FIND_ZONE:
					strncpy(temp_name,
							content->option_value[i][j],
							MAX_OPTION);
					continue;
				case FIND_START:
					result = parseTime(&temp_time.start_hour,
									&temp_time.start_minute,
									content->option_value[i][j]);
					if(result == 1) {
						snprintf(msg, MAX_ROW, "Invalid time format:%s",
								content->option_value[i][j]);
						addError(error, -4, msg, content->rowindex[i]);
					}
					continue;
				case FIND_END:
					result = parseTime(&temp_time.end_hour,
									&temp_time.end_minute,
									content->option_value[i][j]);
					if(result == 1) {
						snprintf(msg, MAX_ROW, "Invalid time format:%s",
								content->option_value[i][j]);
						addError(error, -4, msg, content->rowindex[i]);
					}
					continue;
				case FIND_CONTEXT:
					result = contextValidation(context,
										content->option_value[i][j]);
					if(result == 0) {
						strncpy(temp_context,
								content->option_value[i][j],MAX_CONTEXT);
					}
					if(result == 1) {
						snprintf(msg, MAX_ROW, "Invalid context:%s",
								content->option_value[i][j]);
						addError(error, -5, msg, content->rowindex[i]);
					}
					continue;
				case FIND_DELAY:
					result = parseDelay(&config->delay,
										content->option_value[i][j]);
					if(result == -1) {
						snprintf(msg, MAX_ROW, "Invalid date in delay:%s",
								content->option_value[i][j]);
						addError(error, -6, msg, content->rowindex[i]);
						continue;
					}
					if(result == -2) {
						snprintf(msg, MAX_ROW, "Invalid delay format:%s",
								content->option_value[i][j]);
						addError(error, -6, msg, content->rowindex[i]);
						continue;
					}
					continue;
				case FIND_CANCEL:
					result = strncmp(content->option_value[i][j], "on", 3);
					if(result == 0) {
						config->cancel = 1;
					}
					else {
						config->cancel = 0;
					}
					continue;
				case FIND_NOTIFY:
					result = strncmp(content->option_value[i][j], "on", 3);
					if(result == 0) {
						config->notify = 1;
					}
					else {
						config->notify = 0;
					}
					continue;
				case FIND_INTERVAL:
					result = parseTimeSpan(content->option_value[i][j]);
					if(result == -1) {
						snprintf(msg, MAX_ROW, "Invalid interval: %s",
								content->option_value[i][j]);
						addError(error, -7, msg, content->rowindex[i]);
						continue;
					}
					config->interval = result;
					continue;
				case FIND_EXCLUDE:
					result = parseExclusion(&config->excl,
										content->option_value[i][j]);
					if(result == PARSER_FORMAT) {
						snprintf(msg, MAX_ROW, "Invalid Exclusion format: %s",
								content->option_value[i][j]);
						addError(error, -8, msg, content->rowindex[i]);
						continue;
					}
					if(result == PARSER_WRONGSIZE) {
						snprintf(msg, MAX_ROW, "Exclusion too long: %s",
								content->option_value[i][j]);
						addError(error, -8, msg, content->rowindex[i]);
						continue;
					}
					if(result == PARSER_ERROR) {
						snprintf(msg, MAX_ROW, "Exclusion parse failed: %s",
								content->option_value[i][j]);
						addError(error, -8, msg, content->rowindex[i]);
						continue;
					}
					continue;
			}
		}
	}

	free(context);
	return 0;
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
 * @brief	Check a delay format: for correct format and values
 *
 * Correct format is YYYY-MM-DDTHH:mmZ
 * (Y=Year, M=Month, D=Day, H=Hour, m=Minute)
 *
 * @param[in]	str	the string to be parsed
 * @param[out]	delay	tm struct pointer to save the correct values
 *
 * @retval	0	SUCCESS
 * @retval	-1	Invalid time (13month, 35th day)
 * @retval	-2	Invalid format (10-12-2019T9:00Z)
 */
int parseDelay(struct tm *delay, char *str)
{
	int year = 0;
	int mon = 0;
	int day = 0;
	int hour = 0;
	int min = 0;

	if(strnlen(str, MAX_ROW) > DELAY_FORMAT_LEN) {
		return -2;
	}

	sscanf(str, "%4d-%2d-%2dT%2d:%2dZ",
			&year, &mon, &day, &hour, &min);

	if(year == 0) {
		return -2;
	}
	if(year / 1000 < 1) {
		return -2;
	}
	if(mon > 12 || day > 31 || hour > 24 || min > 60) {
		return -1;
	}
	if(mon < 0 || day < 0 || hour < 0 || min < 0) {
		return -1;
	}

	delay->tm_year = year-1900;
	delay->tm_mon = mon-1;
	delay->tm_mday = day;
	delay->tm_hour = hour;
	delay->tm_min = min;

	return 0;
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
