/**
 * @file config.c
 * @brief	Implementation of a config reader for csw project
 * @author	Sebastian Fricke
 * @license GNU Public License
 */

#include "include/config.h"

extern int mkstemp(char *template);
extern int unlink(const char *pathname);

#define REWIND_RETURN(x, file)  {\
	rewind(file);\
	return x;\
	}

/**
 * @brief find the file within the expected location
 *
 * locate the .task folder in the directory of the user,
 * check if a csw folder exists, create one if necessary,
 * check it for size and extension, then allocate the path
 * on the heap and return it as the parameter path.
 *
 * @param[in]	name	the name of the file to be found
 * @param[out]	path	string that contains the realpath
 * @retval FILE_GOOD	file found & without problems
 * @retval FILE_ERROR	Name doesn't exist/Folder creation malfunctioned
 * @retval FILE_NOTFOUND	file was not found in .task/csw/
 * @date 2019-10-10
 */
FILE_STATE findConfig(char* name, char* path)
{
	struct stat s;
	int empty_config = 0;
	int status = 0;
	int name_len = 0;
	int user_len = 0;
	char folder_path[PATH_MAX] = {"/home/"};
	char config_path[PATH_MAX];
	char *username = getenv("USER");
	if(username == NULL) {
		fprintf(stderr, "No user found in environment\n");
		return FILE_ERROR;
	}

	strncpy(path, "\0", PATH_MAX);

	if(!name) {
		printf("name doesn't exist\n");
		return FILE_ERROR;
	}
	name_len = strnlen(name, PATH_MAX);
	user_len = strnlen(username, MAX_USER);
	if(!username) {
		return FILE_ERROR;
	}
	strncat(folder_path, username, user_len);
	strncat(folder_path, "/.task/csw/", 12);
	strncpy(config_path, folder_path, PATH_MAX);
	if(!config_path[0]) {
		perror("strncpy failed");
		return FILE_ERROR;
	}
	strncat(config_path, name, name_len);

	/* the default location is .task in the user home */
	if(dirExist(folder_path) != 0) {
		mkdir(folder_path, 0777);
		status = dirExist(folder_path);
		switch(status) {
			case -2:
				return FILE_ERROR;
			case -1:
				printf("ERROR: creation of folder csw failed!\n");
				return FILE_ERROR;
			case 1:
				printf("ERROR: created data is not a folder!\n");
				return FILE_ERROR;
			case 0:
				empty_config = open(config_path, 0777);
				if(!empty_config) { return FILE_ERROR; }
				if(ftruncate(empty_config, 1000) != 0) { 
					close(empty_config);
					return FILE_ERROR;
				}
				printf("INFO: Folder csw in /.task and config created\n");
				close(empty_config);
				break; 
		}
	}

	if(stat(config_path ,&s) == 0) {
		strncpy(path, config_path, PATH_MAX);
		return FILE_GOOD;
	}
	else {
		return FILE_NOTFOUND;
	}
}

/**
 * @brief	read the config file in .task/csw and fill the config struct
 *
 * ZONE={NAME};START={Start_t};END={End_t};CONTEXT={context option from tw};
 * ';' the option separator , '=' the value separator
 *
 * @param[in]	path	the path to the config file
 * @param[out]	config	the pointer to heap allocated struct
 * @retval	CONFIG_SUCCESS	config read success, no errors
 * @retval	CONFIG_BAD	config has a bad format
 * @retval	CONFIG_NOTFOUND	config file was not found in .task/csw
 * @date 2019-10-12
 */
CONFIG_STATE readConfig(struct configcontent* config, struct error* error,
						char* path)
{
	FILE *config_file = NULL;
	struct substr *option[MAX_ZONES+MAX_EXCLUSION+4] = {NULL};
	char row[MAX_ZONES+MAX_EXCLUSION+4][MAX_ROW] = {{0}};
	char err_msg[MAX_ROW] = {0};
	int eof = 0;
	int ferr = 0;
	int amount = 0;
	OPTION_STATE state = 0;
	int substr_state = 0;
	int rows = 0;
	errno = 0;

	config_file = fopen(path, "r");
	if(!config_file) {
		return CONFIG_NOTFOUND;
	}

	for(int i = 0 ; i < MAX_ZONES+MAX_EXCLUSION+4 ; i++) {
		memset(row[i], 0, MAX_ROW);
	}

	while(fgets(row[rows], MAX_ROW, config_file) != NULL) {
		eof=feof(config_file);
		if(eof != 0) {
			goto read_success;
		}
		ferr=ferror(config_file);
		if(ferr != 0) {
			goto read_failed;
		}


		/* replace newline with empty string */
		if(row[rows][(int)strnlen(row[rows], MAX_ROW)-1] == '\n') {
			row[rows][(int)strnlen(row[rows], MAX_ROW)-1] = '\0';
		}

		if(strnlen(row[rows], MAX_ROW) > 0) {
			rows++;
		}
		if(rows == MAX_ZONES+MAX_EXCLUSION+3) {
			break;
		}
		errno = 0;
	}

	for(int i = 0 ; i < rows ; i++) {
		amount = 0;
		if(strnlen(row[i], MAX_ROW) > MAX_FIELD) {
			fprintf(stderr,"Row %d of Config, longer than the limit of: %d\n",
					i+1, MAX_FIELD);
			continue;
		}
		option[i] = allocate_substring(option[i]);
		if((substr_state=get_substring(row[i], option[i],
										&amount, ';')) != 0) {
			fprintf(stderr,"ERROR: reading substrings failed %d!\n",
					substr_state);
			goto read_failed;
		}

		for(struct substr *ptr = option[i] ; ptr != NULL ; ptr=ptr->next) {
			/* Check the different possible config values */
			state = getOption(config, ptr->member, i);
			switch(state){
				case OPTION_SUCCESS:
					break;
				case OPTION_NOVALUE:
					snprintf(err_msg, MAX_ROW, "No value found in %s",
							ptr->member);
					addError(error, -1, err_msg, i);
					break;
				case OPTION_NOTFOUND:
					snprintf(err_msg, MAX_ROW, "Title in %s not valid",
							ptr->member);
					addError(error, -2, err_msg, i);
					break;
				case OPTION_ERROR:
					snprintf(err_msg, MAX_ROW, "No option or memory full(%s)",
							ptr->member);
					addError(error, -3, err_msg, i);
					break;
			}
		}
	}

	read_success:
		for(int i = 0 ; i < rows ; i++) {
			free_substring(option[i]);
		}
		fclose(config_file);
		return CONFIG_SUCCESS;
	read_failed:
		fclose(config_file);
		return CONFIG_BAD;
}

/**
 * @brief	compares the option title with the valid titles and saves the
 * value with the title in the configcontent struct.
 *
 * Valid Titles are:
 * @li	zone, start, end, context
 * @li	exclude
 * @li	delay, cancel, notify
 *
 * @param[in]	option	the string to parse
 * @param[in]	row_index	the current line in the config
 * @param[out]	config	the pointer to the config struct
 * @retval OPTION_SUCCESS	option found and value assigned to struct instance
 * @retval OPTION_NOTFOUND	option not found in valid titles
 * @retval OPTION_NOVALUE	option found but no value assigned
 * @retval OPTION_ERROR	return on failure of function
 * @date 2019-10-11
 */
OPTION_STATE getOption(struct configcontent *config, char *option, int index)
{
	struct substr *sub_option = NULL;
	int current = 0;
	int sub = 0;
	int amount = 0;
	char valid_titles[VALID_OPTIONS][MAX_OPTION_NAME] = {
		"zone", "start", "end", "context", "delay", "cancel",
		"exclude", "notify", "interval"
	};

	sub_option = allocate_substring(sub_option);
	if(!sub_option || !option || strnlen(option,MAX_ROW) < 2) {
		free(sub_option);
		return OPTION_ERROR;
	}

	if(get_substring(option, sub_option, &amount, '=') != 0) {
		free(sub_option);
		return OPTION_ERROR;
	}

	/* Convert the string to lowercase */
	lowerCase(sub_option->member, strnlen(sub_option->member, MAX_ROW));

	if(amount <= 1) {
		goto option_novalue;
	}
	if(amount > 2) {
		free_substring(sub_option);
		return OPTION_ERROR;
	}

	for(int i = 0 ; i < VALID_OPTIONS ; i++) {
		if(strncmp(sub_option->member, valid_titles[i], MAX_OPTION_NAME) == 0){
			if(sub_option->next != NULL) {
				if(indexInList(config, index) != 0) {
					config->amount += 1;
					current = config->amount-1;
					config->rowindex[current] = index;
				}
				else {
					current = config->amount-1;
				}
				config->sub_option_amount[current] += 1;
				sub = config->sub_option_amount[current]-1;
				strncpy(config->option_name[current][sub],
						sub_option->member, MAX_OPTION_NAME);
				strncpy(config->option_value[current][sub],
						sub_option->next->member, MAX_OPTION);

				goto option_good;
			}
			else {
				goto option_novalue;
			}
		}
	}
	goto option_notfound;

	option_good:
		free_substring(sub_option);
		return OPTION_SUCCESS;

	option_notfound:
		free_substring(sub_option);
		return OPTION_NOTFOUND;

	option_novalue:
		free_substring(sub_option);
		return OPTION_NOVALUE;
}

/**
 * @brief	Checks if the index of a row is within the set of configcontent
 *
 * The index are added in order to the structure, if the search index is
 * below any element, then it cannot be inside the structure.
 *
 * @param[in]	config	pointer to the configcontent structure
 * @param[in]	index	integer of the specific row index
 * @retval	0	index within the set
 * @retval -1	index not within the set
 */
int indexInList(struct configcontent *config, int index)
{
	for(int i = 0 ; i < config->amount ; i++) {
		if(index < config->rowindex[i]) {
			return -1;
		}
		if(config->rowindex[i] == index) {
			return 0;
		}
	}
	return -1;
}

/**
 * @brief	syncronize config values with the current time and flags
 *
 * create or add a delay from the delay flag to the config,
 * toggle the notify and cancel flags and set the interval value
 * check for expired exclusions and store the indeces of expired/changed rows
 * into the change structure.
 *
 * @param[in]	config	config struct pointer to the parsed config values
 * @param[in]	flag	flags struct pointer to the cli options
 * @param[in]	time	tm struct pointer to the current time & date
 * @param[out]	change	pointer to structure of recorded changes
 *
 * @retval	0	synchronize successfully WITHOUT changes
 * @retval	1	synchronize successfully WITH changes
 * @retval -1	synchronize failed
 */
int syncConfig(struct config *config, struct flags *flag,struct tm *time)
{
	DELAY_CHECK check = 0;
	int change = 0;
	check = checkDelay(flag->delay, &config->delay, time);

	switch(check) {
		case VALID:
			break;
		case VALID_PLUS_NEW:
			if(config->delay.tm_year+config->delay.tm_mon+config->delay.tm_mday == 0) {
				copyTm(&config->delay, time);
			}
			increaseTime(flag->delay, &config->delay);
			break;
		case INVALID_PLUS_NEW:
			copyTm(&config->delay, time);
			increaseTime(flag->delay, &config->delay);
			break;
		case INVALID:
			resetTm(&config->delay);
			break;
		case ERROR:
			return -1;
	};
	if(check != VALID && check != ERROR) {
		change = 1;
	}

	if(flag->cancel_on == 0 || flag->cancel_on == 1) {
		config->cancel = flag->cancel_on;
		change = 1;
	}
	if(flag->notify_on == 0 || flag->notify_on == 1) {
		config->notify = flag->notify_on;
		change = 1;
	}

	if(flag->cron_interval > 0) {
		config->interval = flag->cron_interval;
		change = 1;
	}

	if(checkExclusion(&config->excl, time) == 1) {
		change = 1;
	}

	return change;
}

/**
 * @brief	write the current state of the config structure to a file
 *
 * @param[in]	config	pointer to the config structure containing the parsed options
 * @param[in]	path	string containing the path to the file
 *
 * @retval	0	the content was successfully written to the file
 * @retval	-1	error encountered while writing to the file	
 */
int writeConfig(struct config* config, char* path)
{
	char tmp_name[PATH_MAX] = {"/tmp/.config_write"};
	char buffer[MAX_ROW] = {0};
	FILE* new_file = NULL;


	errno = 0;
	new_file = fopen(tmp_name, "w");
	if(!new_file) {
		return -1;
	}

	for(int i = 0 ; i < config->zone_amount ; i++) {
		snprintf(buffer, MAX_ROW, "Zone=%s;Start=%02d:%02d;End=%02d:%02d;Context=%s\n",
				config->zone_name[i], config->ztime[i].start_hour,
				config->ztime[i].start_minute, config->ztime[i].end_hour,
				config->ztime[i].end_minute, config->zone_context[i]);
		fprintf(new_file, "%s", buffer);
	}	
	for(int i = 0 ; i < config->excl.amount ; i++) {
		buildExclFormat(&config->excl.type[i], config->excl.type_name[i], buffer);
		fprintf(new_file, "%s", buffer);
	}
	if(config->delay.tm_year + config->delay.tm_mon + config->delay.tm_mday > 0) {
		buildDelayFormat(&config->delay, buffer);
		fprintf(new_file, "%s", buffer);
	}
	buildBoolFormat(config->cancel, "Cancel", buffer);
	fprintf(new_file, "%s", buffer);
	buildBoolFormat(config->notify, "Notify", buffer);
	fprintf(new_file, "%s", buffer);
	snprintf(buffer, MAX_ROW, "Interval=%dmin\n", config->interval);
	fprintf(new_file, "%s", buffer);
	fclose(new_file);
	remove(path);
	rename(tmp_name, path);
	return 0;
}

/**
 * @brief	check if any temporary exclusion has expired
 *
 * @param[in]	excl	pointer to the exclusion struct instance
 * @param[in]	time	the current time as tm struct instance
 * @param[out]	delete	array with amount of the rows to be deleted
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
