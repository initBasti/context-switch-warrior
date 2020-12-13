/**
 * @file exclude.c
 * @brief	Module containing all functions related to exclusions
 *
 * 2020-02-04 (SF)	:	reordered the functions from parser.c(deleted)
 * and helper.c into exclude.c to have a better overview of exclusion
 * related functions
 *
 * @author	Sebastian Fricke
 * @date	2020-02-04
 */

#include "include/exclude.h"

void switchPosExclusion(struct exclusion*, int, int);

/**
 * @brief	check the exclusion for 'permanent' or 'temporary' exclusion
 *
 * On permanent:
 * 		- get the string inbetween the brackets
 * 		  and get the tokens between ','
 * 		- they have to be in weekday format:
 * 		  mo, tu, we, th, fr, sa, su
 *
 * On temporary:
 * 	get the string inbetween the brackets, check for a '#'
 *
 * 		- on #:
 * 			- date string before # = start date,
 * 			- date string after # = end date
 *
 * 		- without #:
 * 			- get the tokens between ','
 * 			  they have to be in date format YYYY-MM-DD
 *
 * @param[in]	option	string of the exclusion value
 * @param[out]	excl	struct instance of the config exclusions
 * @retval	0	SUCCESS
 * @retval	1	FAILURE
 */
PARSER_STATE parseExclusion(struct exclusion *excl, char *option)
{
	char *token = NULL;
	int state = 0;

	if(!option)
		return PARSER_ERROR;

	stripChar(option, ' ');

	token = strtok(option, "(");
	if(!token)
		goto wrong_format;

	if(strncmp(token, "permanent", 10) == 0) {
		token = strtok(NULL, ")");
		if(!token)
			goto parse_error;

		state = parsePermanent(token, excl);
	} else if(strncmp(token, "temporary", 10) == 0) {
		token = strtok(NULL, ")");
		if(!token)
			goto parse_error;

		state = parseTemporary(token, excl);
	} else {
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

/**
 * @brief	parse the config entry for the correct permanent excl format
 *
 * check for 4 possible states:
 * 	- A single element with the correct size (weekday: 2chars)
 * 	- A single element with an incorrect size
 * 	- Multiple elements with correct size and correct delimiter (deli: ',')
 * 	- Multiple elements with either incorrect size or delimiter
 *
 * @param[in]	input	string within permanent()
 * @param[out]	excl	exclusion struct instance pointer
 *
 * @retval	0	Success
 * @retval	-1	wrong format
 * @retval	-2	wrong size
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
	struct substr *head = allocateSubstring();

	if(strchr(input,',') != NULL) {
		if(getSubstring(input, head, &list_len, ',') != 0)
			return -1;

		if(equalListElements(head) != 0) {
			freeSubstring(head);
			return -2;
		}
		if(list_len < 1) {
			freeSubstring(head);
			return -1;
		}
		list_len = 0;
		for(struct substr *ptr = head ; ptr != NULL ; ptr=ptr->next) {
			set = &excl->type[excl->amount].weekdays[0];
			length = &excl->type[excl->amount].list_len;
			size_check = exclTokenLength(&input_len, "perm", ptr->member);
			if(size_check == 0) {
				if(*length >= 7)
					break;

				set_check = dayToSet(set, *length,
										parseWeekday(ptr->member, DAY));
			} else {
				freeSubstring(head);
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
				freeSubstring(head);
				return -1;
			}
		}
		freeSubstring(head);
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

/**
 * @brief	parse config entry for the correct temporary excl format
 *
 * check for 5 possible states:
 * 	- A single element with correct size (date: 10chars)
 * 	- A single element with incorrect size
 * 	- 2 dates delimited by a # indicating a range
 * 	- multiple dates delimited by ',' as a list
 * 	- multiple elements with incorrect size or delimiter
 *
 * @param[in]	input	string within temporary()
 * @param[out]	excl	exclusion struct instance pointer
 *
 * @retval	0	Success
 * @retval	-1	wrong format
 * @retval	-2	wrong size
 * @retval	-3	error
 */
int parseTemporary(char *input, struct exclusion *excl)
{
	int input_len = 0;
	int list_len = 0;
	int size_check = 0;
	int format_check = 0;
	struct tm *dest = NULL;
	struct substr *head = allocateSubstring();

	if(strchr(input,'#') != NULL) {
		if(getSubstring(input, head, &list_len, '#') != 0) {
			free(head);
			return -1;
		}
		if(equalListElements(head) != 0) {
			freeSubstring(head);
			return -2;
		}
		if(list_len > 2) {
			freeSubstring(head);
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
		freeSubstring(head);
		excl->amount = excl->amount + 1;
	}
	else if(strchr(input,',') != NULL) {
		if(getSubstring(input, head, &list_len, ',') != 0) {
			free(head);
			return -1;
		}
		if(list_len > MAX_EXCL) {
			freeSubstring(head);
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
				freeSubstring(head);
				return -1;
			}
			strncpy(excl->type[excl->amount].sub_type, "list", 5);
			strncpy(excl->type_name[excl->amount], "temp", 5);
		}
		freeSubstring(head);
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

/**
 * @brief	check if the string matches the exclusion date format
 * 
 * YYYY-MM-DD, parse to integers and assign in tm struct
 *
 * param[in]	input	string (size check performed before call)
 * param[out]	date	exclusion date field of the exclusion instance
 * @retval	0	SUCCESS
 * @retval	-1	FAILURE
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

/**
 * @brief	check if the strong matches the HH:MM time format
 *
 * @param[in]	time_string	string in [HH:MM] format
 * @param[out]	hour	start/end _hour of ztime
 * @param[out]	minute	start/end _minute of ztime
 *
 * @retval	0	SUCCESS
 * @retval	-1	ERROR
 */
int parseTime(int *hour, int *minute, char *time_string)
{
	char *hour_token = NULL;
	char *min_token = NULL;
	if(!time_string) {
		return -1;
	}
	if(strnlen(time_string, DATE) > 5) {
		return -1;
	}

	hour_token = strtok(time_string, ":");
	if(!hour_token) {
		return -1;
	}

	min_token = strtok(NULL, "\0");
	if(!min_token) {
		return -1;
	}

	*hour = atoi(hour_token);
	*minute = atoi(min_token);

	return 0;
}

/**
 * @brief	return corresponding number for a weekday abbreviation
 *
 * ascending order from su = 0 until sa = 6
 * (similar to tm struct)
 *
 * @param[in]	input	weekday string abbreviation
 * @param[in]	size	len string
 *
 * @retval	0	sunday
 * @retval	1	monday
 * @retval	2	tuesday
 * @retval	3	wednesday
 * @retval	4	thursday
 * @retval	5	friday
 * @retval	6	saturday
 * @retval	-1 	FAILURE
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

/**
 * @brief	initialize all values of a exclusion struct to 0
 *
 * @param[in]	excl	exclusion struct instance pointer
 */
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

/**
 * @brief	draw a box containing the exclusions that are currently active
 *
 * @param[in]	excl	exclusion struct instance pointer
 */
void showExclusions(struct exclusion *excl)
{
	drawLine(79);
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
	drawLine(79);
}

/**
 * @brief	check if any temporary exclusion has expired
 *
 * Used within syncConfig in config.c
 *
 * @param[in]	excl	pointer to the exclusion struct instance
 * @param[in]	time	the current time as tm struct instance
 *
 * @retval	0	content was NOT changed
 * @retval	1	content was changed
 */
int checkExclusion(struct exclusion* excl, struct tm* time)
{
	int compare_result = 0;
	int list_length = 0;
	int list_state = 0;
	int i = excl->amount-1;
	int change = 0;

	while(i > -1) {
		if(strncmp(excl->type_name[i], "perm", 4) == 0) {
			return 0;	
		}
		if(strncmp(excl->type[i].sub_type, "range", 5) == 0) {
			compare_result = compareTime(time, &excl->type[i].holiday_end);
			switch(compare_result) {
				case TIME_EQUAL:
				case TIME_SMALLER:
					resetExclusion(excl, i);
					change = 1;
					break;
				case TIME_BIGGER:
					break;
				default:
					return 0;
			};
		}
		if(strncmp(excl->type[i].sub_type, "list", 4) == 0) {
			list_length = excl->type[i].list_len;	
		}
		for(int j = 0 ; j < list_length ; j++) {
			compare_result = compareTime(time, &excl->type[i].single_days[j]);
			switch(compare_result) {
				case TIME_EQUAL:
				case TIME_SMALLER:
					list_state = 1;
					break;
				case TIME_BIGGER:
					list_state = 0;
					break;
				default:
					return 0; 
			};
			if(compare_result == TIME_BIGGER) {
				break;
			}
		}
		if(list_state == 1) {
			resetExclusion(excl, i);
			change = 1;
		}
		list_length = 0;
		i--;
	}

	return change;
}

/**
 * @brief	set values of a exclusion to 0 & reorder the structure	
 *
 * Unless the specific exclusion is the last in the structure
 * reorder it accordingly.
 *
 * Used within checkExclusion
 *
 * @param[in]	excl	exclusion struct instance pointer
 * @param[in]	index	the specific exclusion within the struct
 */
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

/**
 * @brief	create a config entry out of a exclusion struct entry
 *
 * @param[in]	excl	exclusion type struct instance pointer
 * @param[in]	type	perm or temp, indicating the exclusion type
 * @param[out]	str		config entry string
 */
void buildExclFormat(struct format_type* excl, char* type, char* str)
{
	if(strncmp(type, "perm", 4) == 0) {
		buildPermExclFormat(excl, str);	
	}
	if(strncmp(type, "temp", 4) == 0) {
		buildTempExclFormat(excl, str);
	}
}

#ifndef DOXYGEN_SHOULD_SKIP_THIS
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
			snprintf(day, DAY+1, "%s",wd==1?"su":wd==2?"mo":wd==3?"tu":wd==4?"we":
					wd==5?"th":wd==6?"fr":"sa");
		}
		else {
			snprintf(day, DAY+2, ",%s",wd==1?"su":wd==2?"mo":wd==3?"tu":wd==4?"we":
					wd==5?"th":wd==6?"fr":"sa");
		}
		strncat(buffer, day, DAY+1);
	}
	snprintf(str, MAX_ROW, "Exclude=permanent(%s)\n", buffer);
}
#endif /*DOXYGEN_SHOULD_SKIP_THIS */

/**
 * @brief	switch the position of two exclusions within the array
 *
 * swap a expired exclusion inside the array with the last index
 * then reset the last index of the array, effectively removing
 * the entry from the array
 *
 * @param[in]	excl	exclusion struct instance pointer
 * @param[in]	new		new position of the active element
 * @param[in]	old		old position of the active element
 */
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

/**
 * @brief	check the length of the input for the correct format size
 *
 * @param[out]	length	size of the input
 * @param[in]	type	format type to check
 * @param[in]	input	string for comparision
 *
 * @retval	0	Correct format
 * @retval	!0	Incorrect format
 */
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
			return 1;
		}
	}

	if(element < 1 || element > 7) {
		return -1;
	}

	if(size < WEEKDAYS) {
		arr[size] = element;
	}

	bubbleSort(arr, size+1);

	return 0;
}

/**
 * @brief	simple implementation of the bubble sort algorithm
 *
 * As long as the array is not sorted in a ascending order
 * swap pairs of integers when the 1st is greater than the 2nd
 *
 * @param[in]	arr		pointer to the array of integers
 * @param[in]	size	length of the array
 */
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
