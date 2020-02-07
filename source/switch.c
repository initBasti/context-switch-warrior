/**
 * @file switch.c
 * @author	Sebastian Fricke
 * @date	2019-12-01
 * @brief	Determine if a switch is necessary and validate that there is no exclusion
 *
 * Compare exclusions with the active setting.
 * Compare the zones with current time.
 * Send commands to task warrior to stop tasks, find active tasks or switch a context
 */
#include "include/switch.h"
#ifndef CONFIG_H
#include <string.h>
#include <stdio.h>
#endif

extern FILE* popen(const char* command, const char *type);
extern int pclose(FILE* stream);

/**
 * @brief	Determine if the current date is excluded from switching the context
 *
 * @param[in]	date	tm structure pointer to the current date & time
 * @param[in]	excl	exclusion structure instance pointer
 *
 * @retval	EXLCUSION_MATCH
 * @retval	EXCLUSION_NOMATCH
 * @retval	EXLCUSION_ERROR
 */
EXCLUSION_STATE switchExclusion(struct exclusion* excl, struct tm *date)
{
	if(excl == NULL) {
		return EXCLUSION_ERROR;
	}
	for(int i = 0 ; i < excl->amount ; i++) {
		if(strncmp(excl->type_name[i], "perm", 5) == 0) {
			for(int j = 0 ; j<excl->type[i].list_len ; j++) {
				if(date->tm_wday == excl->type[i].weekdays[j]-1) {
					return EXCLUSION_MATCH;
				}
			}
		}
		else if(strncmp(excl->type_name[i], "temp", 5) == 0) {
			if(strncmp(excl->type[i].sub_type, "list", 5) == 0) {
				for(int j = 0 ; j < excl->type[i].list_len ; j++) {
					if(compareTime(&excl->type[i].single_days[j], date) == TIME_EQUAL){
						return EXCLUSION_MATCH;
					}
				}
			}
			else if(strncmp(excl->type[i].sub_type, "range", 6) == 0) {
				if(rangeMatch(&excl->type[i], date) == 0) {
					return EXCLUSION_MATCH;
				}
			}
		}
	}
	return EXCLUSION_NOMATCH;
}

/**
 * @brief	compare the current date, with the zone settings in the config 
 *
 * On a match return the context from the zone and a signal for further execution
 *
 * @param[in]	conf	config structure instance pointer
 * @param[in]	time	current time in minutes
 * @param[in]	current_context	active context in taskwarrior
 * @param[out]	new_context	context found in the active zone	
 * 
 * @retval	SWITCH_SUCCESS	current & new context differ
 * @retval	SWITCH_NOTNEEDED	current & new context are equal
 * @retval	SWITCH_FAILURE	time is not within any zone
 */
SWITCH_STATE switchContext(struct config* conf, int time, char* new_context,
							char* current_context)
{
	int start_time = 0;
	int end_time = 0;

	if(conf == NULL) {
		return SWITCH_FAILURE;
	}
	for(int i = 0 ; i < conf->zone_amount ; i++) {
		start_time = (conf->ztime[i].start_hour)*60 +
						(conf->ztime[i].start_minute);
		end_time = (conf->ztime[i].end_hour)*60 +
						(conf->ztime[i].end_minute);
		if(time >= start_time && time <= end_time) {
			if(strncmp(conf->zone_context[i], current_context, MAX_FIELD) == 0) {
				strncpy(new_context, "none", 5);
				return SWITCH_NOTNEEDED;
			}
			else {
				strncpy(new_context, conf->zone_context[i], MAX_FIELD);
				return SWITCH_SUCCESS;
			}
		}
	}
	return SWITCH_FAILURE;
}

/**
 * @brief	compare if a date is inside a range of 2 dates start->end
 *
 * @param[in]	range	sub structure of the exclusion struct contain start, end
 * @param[in]	date	tm structure of the current date
 *
 * @retval	1	SUCCESS date in inbetween start and end
 * @retval	0	date is NOT inbetween start and end
 * @retval	-1	FAILURE struct from exclusion struct not found
 */
int rangeMatch(struct format_type *range, struct tm *date)
{
	if(range == NULL) {
		return -1;
	}
	struct tm start = range->holiday_start;
	struct tm end = range->holiday_end;
	if(date->tm_year < start.tm_year || date->tm_year > end.tm_year) {
		return 1;
	}
	if(date->tm_year == end.tm_year && date->tm_mon > end.tm_mon) {
		return 1;
	}
	if(date->tm_year == start.tm_year && date->tm_mon < start.tm_mon) {
		return 1;
	}
	if(date->tm_year == end.tm_year && date->tm_mon == end.tm_mon && date->tm_mday > end.tm_mday) {
		return 1;
	}
	if(date->tm_year == start.tm_year && date->tm_mon == start.tm_mon && date->tm_mday < start.tm_mday) {
		return 1;
	}
	return 0;
}

/**
 * @brief	send a command to taskwarrior to switch to the specified context
 *
 * watch the reaction from taskwarrior for success
 *
 * param[in]	input	specified context
 *
 * @retval	0	SUCCESS
 * @retval	-1	FAILURE
 */
int sendCommand(char *input)
{
	FILE *process = NULL;
	char command[40] = {"task context "};
	char buffer[2][256] = {{0}};
	char *token = NULL;
	int index = 0;

	strncat(command, input, 20);
	strncat(command, " 2>&1", 6);
	process = popen(command, "r");
	while(fgets(buffer[index], 256, process) != NULL) {
		strtok(buffer[index], " ");
		strtok(NULL, " ");
		token = strtok(NULL, " ");
		if(strncmp(token, "not", 4) == 0) {
			goto failure;
		}
		else if(strncmp(token, "set.", 4) == 0) {
			goto success;
		}
		else{
			fprintf(stderr,"Reaction from Taskwarrior:\n\n%s\n", buffer[index]);
			goto failure;
		}
		index++;
	}
	success:
		pclose(process);
		return 0;
	failure:
		pclose(process);
		return -1;
}

/**
 * @brief	send a command to taskwarrior to check for active tasks
 *
 * uses '2<&1' to redirect the stderr output to stdout
 *
 * @retval	0	No active Task
 * @retval	1	Active Task
 */
int activeTask()
{
	char command[MAX_COMMAND] = {"task +ACTIVE 2<&1"};
	char buffer[MAX_FIELD] = {0};
	FILE *process = NULL;

	process = popen(command, "r");

	fgets(buffer, MAX_FIELD, process);
	if(strncmp(buffer, "No matches.\n", 13) == 0) {
		pclose(process);
		return 0;
	}
	pclose(process);
	return 1;
}

/**
 * @brief	send a command to taskwarrior, stop any Active task
 *
 * uses '2<&1' to redirect the stderr output to stdout
 *
 * @retval	0	SUCCESS
 * @retval	-1	FAILURE
 */
int stopTask()
{
	char command[40] = {"task +ACTIVE stop 2<&1"};
	char buffer[6] = {0};
	FILE *process = NULL;

	process = popen(command, "r");

	fgets(buffer, 5, process);
	if(strncmp(buffer, "Stop", 4) == 0) {
		pclose(process);
		return 0;
	}
	return -1;
	pclose(process);
}
