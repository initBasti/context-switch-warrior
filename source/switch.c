#include "include/switch.h"
#ifndef CONFIG_H
#include <string.h>
#include <stdio.h>
#endif

extern FILE* popen(const char* command, const char *type);
extern int pclose(FILE* stream);

/*
 * switchExclusion:
 * Move through the different exclusion objects if any of them matches
 * either the weekday(wd) or the date(y,m,d) stop the program from
 * switching the context
 * @parameter(in): y(year), m(month), d(day), wd(weekday) and the
 * 				   exclusion struct
 * return 		EXLCUSION_MATCH on exclusion
 * 				EXCLUSION_NOMATCH if nothing matches
 * 			    EXLCUSION_ERROR on failure
 */
EXCLUSION_STATE switchExclusion(struct exclusion* excl, struct tm *date)
{
	int y = date->tm_year;
	int m = date->tm_mon;
	int d = date->tm_mday;
	int wd = date->tm_wday;

	if(excl == NULL) {
		return EXCLUSION_ERROR;
	}
	for(int i = 0 ; i < excl->amount ; i++) {
		if(strncmp(excl->type_name[i], "perm", 5) == 0) {
			for(int j = 0 ; j<excl->type[i].list_len ; j++) {
				if(wd == excl->type[i].weekdays[j]-1) {
					return EXCLUSION_MATCH;
				}
			}
		}
		else if(strncmp(excl->type_name[i], "temp", 5) == 0) {
			if(strncmp(excl->type[i].sub_type, "list", 5) == 0) {
				for(int j = 0 ; j < excl->type[i].list_len ; j++) {
					if(dateMatch(&excl->type[i].single_days[j], y, m, d) == 0){
						return EXCLUSION_MATCH;
					}
				}
			}
			else if(strncmp(excl->type[i].sub_type, "range", 6) == 0) {
				if(rangeMatch(&excl->type[i], y, m, d) == 0) {
					return EXCLUSION_MATCH;
				}
			}
		}
	}
	return EXCLUSION_NOMATCH;
}

/*
 * switchContext:
 * compare the different zones with the current time and return the
 * context command from the matching zone
 * @parameter(in): the config, the time in minutes, the current context
 * @parameter(out): the string containing the command
 * return:	SWITCH_SUCCESS: match & context isn't already set
 * 			SWITCH_NOTNEEDED: match & context is already set
 * 			SWITCH_FAILURE: no match
 */
SWITCH_STATE switchContext(struct config* conf, int time, char* command,
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
				strncpy(command, "none", 5);
				return SWITCH_NOTNEEDED;
			}
			else {
				strncpy(command, conf->zone_context[i], MAX_FIELD);
				return SWITCH_SUCCESS;
			}
		}
	}
	return SWITCH_FAILURE;
}

int dateMatch(struct tm *date, int year, int month, int day)
{
	if(!date) {
		return -1;
	}
	return !(date->tm_year==year && date->tm_mon==month && date->tm_mday==day);
}

int rangeMatch(struct format_type* range, int year, int month, int day)
{
	if(range == NULL) {
		return -1;
	}
	struct tm start = range->holiday_start;
	struct tm end = range->holiday_end;
	if(year < start.tm_year || year > end.tm_year) {
		return 1;
	}
	if(year == end.tm_year && month > end.tm_mon) {
		return 1;
	}
	if(year == start.tm_year && month < start.tm_mon) {
		return 1;
	}
	if(year == end.tm_year && month == end.tm_mon && day > end.tm_mday) {
		return 1;
	}
	if(year == start.tm_year && month == start.tm_mon && day < start.tm_mday) {
		return 1;
	}
	return 0;
}

/*
 * sendCommand:
 * uses the context string from switchContext and sends it to taskwarrior
 * watch the reaction from taskwarrior for success
 * parameter(in): The string containing the CONTEXT
 * return:		  0 on SUCCESS
 * 			     -1 on FAILURE
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
			printf("Reaction from Taskwarrior:\n\n%s\n", buffer[index]);
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

/*
 * activeTask:
 * sends a command to check if there is any active taskwarrior task.
 * uses '2<&1' to redirect the stderr output to stdout
 * return:		0 on No active Task
 * 				1 on Active Task
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

/*
 * stopTask:
 * sends a command to stop any Active task on taskwarrior
 * uses '2<&1' to redirect the stderr output to stdout
 * return:		0 on SUCCESS
 * 			   -1 on FAILURE
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
