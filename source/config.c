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

	/* the default location is .task in the user's home */
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
		option[i] = allocateSubstring(option[i]);
		if((substr_state=getSubstring(row[i], option[i],
										&amount, ';')) != 0) {
			fprintf(stderr,"ERROR: reading substrings failed %d!\n",
					substr_state);
			goto read_failed;
		}

		for(struct substr *ptr = option[i] ; ptr != NULL ; ptr=ptr->next) {
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
			freeSubstring(option[i]);
		}
		fclose(config_file);
		return CONFIG_SUCCESS;
	read_failed:
		fclose(config_file);
		return CONFIG_BAD;
}

/**
 * @brief	compares option title with the valid titles, save in configcontent
 *
 * Valid Titles are:
 * @li	zone, start, end, context
 * @li	exclude
 * @li	delay, cancel, notify
 *
 * @param[in]	option	the string to parse
 * @param[in]	row_index	the current line in the config
 * @param[out]	config	the pointer to the config struct
 *
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

	sub_option = allocateSubstring(sub_option);
	if(!sub_option || !option || strnlen(option,MAX_ROW) < 2) {
		free(sub_option);
		return OPTION_ERROR;
	}

	if(getSubstring(option, sub_option, &amount, '=') != 0) {
		free(sub_option);
		return OPTION_ERROR;
	}

	lowerCase(sub_option->member, strnlen(sub_option->member, MAX_ROW));

	if(amount <= 1) {
		goto option_novalue;
	}
	if(amount > 2) {
		freeSubstring(sub_option);
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
		freeSubstring(sub_option);
		return OPTION_SUCCESS;

	option_notfound:
		freeSubstring(sub_option);
		return OPTION_NOTFOUND;

	option_novalue:
		freeSubstring(sub_option);
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
