/**
 * @file delay.c
 * @brief	implementation of a delay to task cancel and context switch
 * @author	Sebastian Fricke
 * @date	2019-12-19
 */

#include "include/delay.h"

/**
 * @brief	check the input for a delay format.
 *
 * Don't check for valid date or date in the past, those tests
 * are performed in the parser.
 *
 * @param[in]	row	string to be checked
 *
 * @retval	0	valid format
 * @retval	-1	bad size
 * @retval	-2	bad format
 */
int delayFormatValidation(char *row)
{
	int row_length = strnlen(row, MAX_ROW);
	char *ptr = NULL;

	if(row[row_length-1] == '\n') {
		row[row_length-1] = '\0';
		row_length = strnlen(row, MAX_ROW);
	}
	if(row_length < DELAY_FORMAT_LEN-1 || row_length > DELAY_FORMAT_LEN)
		return -1;

	if((ptr=strtok(row, "T")) != NULL) {
		if(strnlen(ptr, MAX_ROW) != 10)
			return -1;

		if((ptr=strtok(NULL, "Z")) != NULL) {
			if(strnlen(ptr, MAX_ROW) < 4 || strnlen(ptr, MAX_ROW) > 5)
				return -1;
			else
				return 0;
		}
	}
	return -2;
}

/**
 * @brief	check if the end of the delay is stated after the current time
 *
 * include the value from the command line flag -d into the calculation
 *
 * @param[in]	flag	timespan of delay of flag (CLI)
 * @param[in]	delay	delay time instance
 * @param[in]	time	current time instance
 *
 * @retval	VALID	delay is active with no further increment
 * @retval	VALID_PLUS_NEW	the delay is still active and increased by the user
 * @retval	INVALID_PLUS_NEW	delay expired but a new one is created
 * @retval	INVALID	delay expired and no new delay to be created
 * @retval	ERROR	the check failed with an error
 */
DELAY_CHECK checkDelay(int flag, struct tm* delay, struct tm* time)
{
	TIME_CMP cmp_result = 0;
	if(!time || time->tm_year + time->tm_mon + time->tm_mday == 0)
		return ERROR;

	if(delay->tm_year+delay->tm_mon+delay->tm_mday == 0) {
		if(flag > 0)
			return VALID_PLUS_NEW;
		else
			return VALID;
	}
	cmp_result = compareTime(time, delay);
	switch(cmp_result) {
		case TIME_EQUAL:
		case TIME_SMALLER:
			if(flag > 0)
				return INVALID_PLUS_NEW;
			else
				return INVALID;

			break;
		case TIME_BIGGER:
			if(flag > 0)
				return VALID_PLUS_NEW;
			else
				return VALID;

			break;
		default:
			return ERROR;
	}
	return 0;
}

/**
 * @brief	increase the given time by the delay value and build the format
 *
 * @param[in]	time	tm struct instance (either current time or delay time)
 * @param[in]	delay	minutes integer of the time to be added
 * @param[out]	format	string of length:DELAY_FORMAT_LEN
 */
void buildDelayFormat(struct tm* time, char* format)
{
	if(time->tm_year+1900 + time->tm_mon + time->tm_mday < 2000)
		return;

	time->tm_isdst = -1;
	snprintf(format, MAX_ROW, "Delay=%4d-%02d-%02dT%02d:%02dZ\n",
			time->tm_year+1900, time->tm_mon+1, time->tm_mday,
			time->tm_hour, time->tm_min);
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

	if(strnlen(str, MAX_ROW) > DELAY_FORMAT_LEN)
		return -2;

	sscanf(str, "%4d-%2d-%2dT%2d:%2dZ",
			&year, &mon, &day, &hour, &min);

	if(year == 0)
		return -2;

	if(year / 1000 < 1)
		return -2;

	if(mon > 12 || day > 31 || hour > 24 || min > 60)
		return -1;

	if(mon < 0 || day < 0 || hour < 0 || min < 0)
		return -1;

	delay->tm_year = year-1900;
	delay->tm_mon = mon-1;
	delay->tm_mday = day;
	delay->tm_hour = hour;
	delay->tm_min = min;

	return 0;
}
