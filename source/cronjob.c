/**
 * @file cronjob.c
 * @author Sebastian Fricke
 * @date	2020-01-22
 * @brief	delete/write & check cronjobs for the csw program
 *
 * Search for active cronjobs with the csw command.
 * Delete any entry with csw in the name, on a change or on deletion by the user.
 * Create a new entry on a change or if no entry with csw is found.
 * Check the crontab for environment variables, to find the program and user.
 */

#include "include/cronjob.h"

/**
 * @brief	change the last char of a string if it is a newline
 *
 * @param[in]	buffer	string to be removed from the linebreak
 * @param[in]	len		length of the string
 */
#define REMOVE_NEWLINE(buffer, len) {\
	if(buffer[strnlen(buffer, len)-1] == '\n')\
		buffer[strnlen(buffer, len)-1] = '\0';\
};

int getLocalEnv(struct loc_env*);
void listCrontab(char*, char*);

/**
 * @brief	check the output of crontab to decide for creation, change or delete
 *
 * @param[in]	term	word that has to be in the crontab
 * @param[in]	new_interval	integer value from the interval flag
 *
 * @retval	CRON_ACTIVE	cronjob found and not changed
 * @retval	CRON_CHANGE	cronjob found and changed
 * @retval	CRON_INACTIVE	cronjob not found
 * @retval	CRON_DELETE	cronjob found but deleted by user
 */
CRON_STATE handleCrontab(char *term, int new_interval)
{
	FILE *process = NULL;
	int find_status = 0;
	int interval = 0;
	int index = 0;
	char crontab[MAX_CRON_JOBS][MAX_CRON] = {{0}};
	char cron_command[MAX_CRON] = {0};
	struct loc_env environment = {.user={0}, .path={0}, .cron_env=0};

	if(getLocalEnv(&environment) == -1) {
		fprintf(stderr, "No environment variables");
		return -1;
	}
	listCrontab(environment.user, cron_command);
	process = popen(cron_command, "r");
	if(!process)
		return -1;

	if(readCrontab(process, MAX_CRON, crontab) == -1)
		return -1;

	if(pclose(process) < 0)
		return -1;

	environment.cron_env = checkCronEnv(MAX_CRON, crontab);

	find_status = checkCrontab(term, MAX_CRON, crontab, &index);
	switch(find_status) {
		case 0:
			if(new_interval == -1)
				return CRON_ACTIVE;

			if(new_interval == 0 && deleteCrontab(term, &environment) == 0)
				return CRON_DELETE;

			break;
		case -1:
			if(new_interval == -1) {
				interval = 1;
				if(writeCrontab(term, interval, &environment) == -1)
					return CRON_INACTIVE;
				return CRON_ACTIVE;
			}
			if(new_interval != 0 &&
					writeCrontab(term, new_interval, &environment) == 0) {
				return CRON_ACTIVE;
			}
			return CRON_INACTIVE;
		default:
			return CRON_INACTIVE;
	}

	if(parseCrontab(crontab[index], &interval) == -1)
		return CRON_INACTIVE;

	if(interval != new_interval) {
		if(deleteCrontab(term, &environment) == 0) {
			if(writeCrontab(term, new_interval, &environment) == 0)
				return CRON_CHANGE;
		}
		return CRON_INACTIVE;
	}
	return CRON_ACTIVE;
}

/**
 * @brief	remove each cronjob entry containing the term
 *
 * @param[in]	term	term to be searched for
 * @param[in]	environment	structure with info about the local user env variables	
 *
 * @retval	0	deletion successful
 * @retval	-1	deletion failed
 */
int deleteCrontab(char* term, struct loc_env *environment)
{
	char delete_command[MAX_CRON+44] = {0};
	FILE *process = NULL;

	snprintf(delete_command, MAX_CRON+44,
			"crontab -u %s -l | grep -v \"%s\" | crontab -u %s -",
			environment->user, term, environment->user);

	process = popen(delete_command, "r");
	if(!process)
		return -1;

	if(pclose(process) < 0)
		return -1;

	return 0;
}

/**
 * @brief	read the content of crontab -u $USER -l and save to string array
 *
 * @param[in]	process	FILE pointer to process opened by popen()
 * @param[in]	size	size of the 2nd dimension of the 2D Array
 * @param[out]	output	array of strings with the crontab output
 *
 * @retval	0	SUCCESS
 * @retval -1	FAILURE
 */
int readCrontab(FILE *process, int size, char output[][size])
{
	int index = 0;
	char buffer[MAX_CRON] = {0};

	while(fgets(buffer, MAX_CRON, process) != NULL) {
		if(strnlen(buffer, MAX_CRON) <= 1 || buffer[0] == '#') {
			memset(buffer, 0, MAX_CRON);
			continue;
		}
		REMOVE_NEWLINE(buffer, MAX_CRON);
		strncpy(output[index], buffer, MAX_CRON);
		memset(buffer, 0, MAX_CRON);
		index++;
	}
	return 0;
}

/**
 * @brief	combines the username with the crontab -u xxx -l command
 *
 * @param[in]	user	name of the user logged in
 * @param[in]	output	string containing the finished command
 */
void listCrontab(char* user, char* output)
{
	snprintf(output, MAX_CRON, "crontab -u %s -l", user);
}

/**
 * @brief	change the interval amount in the crontab entry to a new value
 *
 * @param[in]	term	command for the cronjob
 * @param[in]	interval	minute amount to replace the current
 * @param[in]	local_environment	PATH and USER of local environment
 *
 * @retval	0	successful edit, new entry in str
 * @retval	-1	edit failed, str unchanged
 */
int writeCrontab(char *term, int interval, struct loc_env *local_environment)
{
	char cron[MAX_CRON] = {0};
	char command[MAX_CRON] = {0};
	FILE *command_output = NULL;
	int build_state = 0;

	if(interval > MAX_INTERVAL) {
		fprintf(stderr, "The maximum interval is %d\n", MAX_INTERVAL);
		return -1;
	}

	build_state = buildCronCommand(term, interval, local_environment, command);
	switch(build_state) {
		case -1:
			return -1;
		case -2:
			fprintf(stderr, "ERROR: Command %s not found\n", term);
			return -1;
	};

	errno = 0;
	command_output = popen(command, "r");
	if(!command_output) {
		fprintf(stderr, "ERROR: write failed: %s | %s\n",
				cron, strerror(errno));
		return -1;
	}
	errno = 0;
	if(pclose(command_output) ==  -1) {
		fprintf(stderr, "ERROR: command failed: %s | %s\n",
				command, strerror(errno));
	}
	return 0;
}

/**
 * @brief	split the arguments of the crontab output to get the minutes/hour interval
 *
 * @param[in]	crontab	the row out of the crontab with the cronjob 
 * @param[out]	interval	the parsed minutes interval as integer
 *
 * @retval	0	SUCCESS
 * @retval -1	FAILURE
 */
int parseCrontab(char crontab[], int *interval)
{
	int minute = 0;
	int hour = 0;
	int occ = 0;
	char new[MAX_CRON] = {0};
	char *temp = NULL;
	char *curr = NULL;
	size_t cmd_part = 0;

	if((temp=strrchr(crontab, '*')) != NULL)
		cmd_part = strnlen(temp, MAX_TERM)-1;

	strncpy(new, crontab, strnlen(crontab, MAX_CRON)-cmd_part);
	curr = new;
	while((temp = strchr(curr, '*')) != NULL) {
		curr = temp+1;
		if(curr[0] == '/' && occ == 0) {
			if(sscanf(curr, "/%d ", &minute) == 0) {
				fprintf(stderr, "ERROR: Minute interval is not a number from 2-60\n");
				*interval = 0;
				return -1;
			}
		}
		if(curr[0] == '/' && occ == 1) {
			if(sscanf(curr, "/%d ", &hour) == 0) {
				fprintf(stderr, "ERROR: Hour interval is not a number from 2-24\n");
				*interval = 0;
				return -1;
			}
		}
		if(curr[0] == '/' && occ > 1)
			return -1;

		occ++;
	}
	if(occ != 5) {
		*interval = 0;
		return -1;
	}
	if(minute+hour == 0) {
		*interval = 1;
		return 0;
	}
	*interval = hour*60 + minute;
	return 0;
}

/**
 * @brief	retrieve values from the PATH and USER variable and parse them to a string
 *
 * @param[out]	environment	structure with PATH and USER vars + cron environment indicator
 *
 * @retval	0	SUCCESS
 * @retval -1	FAILURE
 */
int getLocalEnv(struct loc_env *environment)
{
	char *username = getenv("USER");
	char *path = getenv("PATH");

	if(username == NULL || path == NULL)
		return -1;

	strncpy(environment->user, username, MAX_USER);
	strncpy(environment->path, path, PATH_MAX);
	if(strnlen(environment->user, MAX_USER) <= 1 ||
			strnlen(environment->path, PATH_MAX) <= 1) {
		return -1;
	}

	return 0;
}

/**
 * @brief	Combine command, cron string and environment variables to one string
 *
 * @param[in]	term	command term used in the cronjob
 * @param[in]	interval	minutes integer of the interval for cron execution
 * @param[in]	environment	environment variables structure from getLocalEnv()
 * @param[out]	str	output string on success, 0 on failure
 *
 * @retval	0	successful string build
 * @retval	-1	FAILURE
 * @retval	-2	Command not found
 */
int buildCronCommand(char *term, int interval, struct loc_env *environment, char *str)
{
	char cron_string[MAX_INTERVAL_STR] = {0};
	char env_string[MAX_ENV] = {0};
	char *user = environment->user;


	if(term == NULL || interval == 0 || interval >= MAX_INTERVAL)
		return -1;

	REMOVE_NEWLINE(term, MAX_TERM);
	if(checkTerm(term) == -1)
		return -2;

	buildCronInterval(interval, cron_string);
	if(environment->cron_env == 0) {
		if(snprintf(str, MAX_CRON,
					"(crontab -u %s -l && echo '%s %s') | crontab -u %s -",
					user, cron_string, term, user) < 0) {
			return -1;
		}
		return 0;
	}
	if(snprintf(env_string, MAX_ENV, "echo 'USER=%s\nPATH=%s\n'",
				environment->user, environment->path) < 0) {
		return -1;
	}
	if(snprintf(str, MAX_CRON,
				"(%s && crontab -u %s -l && echo '%s %s') | crontab -u %s -",
				env_string, user, cron_string, term, user) < 0) {
		return -1;
	}
	return 0;
}

/**
 * @brief	search for a term in the crontab
 *
 * Uses the POSIX regular expression for matching the term with"
 * 	-	a following space
 * 	-	a following =
 * 	-	at the end of the line
 *
 * @param[in]	term	term to be located
 * @param[in]	size	size of the 2nd dimension of the crontab array
 * @param[in]	crontab	pointer to 2-dimensional array of read lines in crontab
 * @param[out]	index	index of the row with the first match
 *
 * @retval	0	SUCCESS term found in crontab
 * @retval	-1	FAILURE
 */
int checkCrontab(char *term, int size,  char crontab[][size], int *index) {
	regex_t regex;
	int value = 0;
	char pattern[MAX_RE_PATTERN] = {0};
	char msg[MAX_ROW] = {0};
	int match = -1;
	int row = -1;
	size_t nmatch = 2;
	regmatch_t pmatch[2] = {{.rm_so=0, .rm_eo=0}};

	if(strnlen(term, MAX_TERM) < 1) {
		*index = row;
		return -1;
	}
	snprintf(pattern, MAX_RE_PATTERN,
			"((%s)[[:space:]=]|(%s$))",
			term, term);
	value = regcomp(&regex, pattern, REG_EXTENDED);
	if(value != 0) {
		if(value == REG_ESPACE) {
			fprintf(stderr, "%s\n", strerror(ENOMEM));
		} else {
			fprintf(stderr, "Syntax-Error for the regex compilation with: %s",
					pattern);
			return -1;
		}
	}

	for(int i = 0 ; i < MAX_CRON_JOBS ; i++) {
		if(strnlen(crontab[i], MAX_CRON) <= 1)
			continue;

		value = regexec(&regex, crontab[i], nmatch, pmatch, REG_EXTENDED);
		if(value == 0 && match == -1) {
			match = 0;
			row = i;
		}
		if(value != 0 && value != REG_NOMATCH) {
			regerror(value, &regex, msg, MAX_ROW);
			fprintf(stderr, "Regex match failed: %s\n", msg);
		}
	}
	*index = row;
	regfree(&regex);
	return match;
}

/** 
 * @brief	wrapper function around checkCrontab to check if PATH and USER are declared
 *
 * @param[in]	size	size of the 2nd dimension of the 2D input array
 * @param[in]	input	2D array of the read content from the crontab 
 *
 * @retval	0	SUCCESS both found
 * @retval	-1	FAILURE only one or none found
 */
int checkCronEnv(int size, char input[][size])
{
	char term[2][MAX_COMMAND] = {"USER", "PATH"};
	int index = 0;

	if(checkCrontab(term[0], size, input, &index) == 0 &&
			checkCrontab(term[1], size, input, &index) == 0) {
		return 0;
	}
	return -1;	
}

/** 
 * @brief	check if program used for the program exists within the declared PATH
 *
 * @param[in]	term	string of the program with or without parameters
 *
 * @retval	0	SUCCESS
 * @retval	-1	FAILURE
 */
int checkTerm(char* term)
{
	FILE *process = NULL;
	char command[MAX_COMMAND+11] = {0};
	char buffer[MAX_ROW] = {0};
	int found = 0;

	snprintf(command, MAX_COMMAND+11, "command -v %s", term);

	process = popen(command, "r");
	if(!process)
		return -1;

	if(fgets(buffer, MAX_ROW, process) != NULL)
		found = 0;
	else
		found = -1;

	if(pclose(process) < 0)
		return -1;

	return found;
}

/**
 * @brief	create the interval for a cronjob by applying the interval minutes
 *
 * @param[in]	interval	interval minutes integer
 * @param[out]	output	cron interval string on success, empty on failure
 */
void buildCronInterval(int interval, char* output)
{
	int hour = (int)interval/60;
	int minute = interval%60;

	if(hour > 0 && minute > 0) {
		if(snprintf(output, MAX_INTERVAL_STR, "*/%d */%d * * *", minute, hour) < 0)
			goto reset_return;
	}
	if(hour > 0 && minute == 0) {
		if(snprintf(output, MAX_INTERVAL_STR, "* */%d * * *", hour) < 0)
			goto reset_return;
	}
	if(hour == 0 && minute > 0) {
		if(snprintf(output, MAX_INTERVAL_STR, "*/%d * * * *", minute) < 0)
			goto reset_return;
	}
	return;

reset_return:
	memset(output, 0, MAX_INTERVAL_STR);
	return;
}
