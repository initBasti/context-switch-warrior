#include <stdlib.h>
#include "include/config.h"
#include "include/switch.h"
#include "include/cronjob.h"

int verbose = 0;

int main(int argc, char **argv) {
	CRON_STATE cronjob_state = 0;
	CONFIG_STATE config_state = 0;
	FILE_STATE file_state = 0;
	SWITCH_STATE switch_state = 0;
	struct tm datetime = {0};
	char config_path[PATH_MAX] = {0};
	time_t rawtime;
	struct config config = {
		.zone_name={{0}}, .ztime={{0}}, .zone_context={{0}}, .zone_amount=0,
		.excl={
			.type={{.weekdays={0}, .single_days={{0}}, .holiday_start={0},
					.holiday_end={0}, .list_len=0, .sub_type={0}}},
			.type_name={{0}}, .amount=0,
		},
		.delay={0}, .cancel=0, .notify=0, .interval=0
	}; 
	struct error error = {
		.amount = 0, .rowindex = {0}, .error_code = {0}, .error_msg = {{0}},
	};
	struct configcontent content = {
		.amount = 0, .rowindex = {0}, .option_name = {{{0}}}, .option_value = {{{0}}},
		.sub_option_amount = {0}
	};
	struct flags flag = {
		.verbose = &verbose,.cancel_on=-1,
		.notify_on=-1,.cron_interval=-1
	};
	char current_context[MAX_CONTEXT] = {0};
	char command[MAX_COMMAND] = {0};

	if(getArgs(&flag, argc, argv, "hd:si:c:n:v::") == -1) {
		return 1;
	}
	if(flag.notify_on == 1) {
		if(checkNotificationSetup() != 0) {
			fprintf(stderr, "notify-send not found, documentation for instruction\n");
			return EXIT_FAILURE;
		}
	}
	if(flag.help == 1) {
		showHelp();
		return EXIT_SUCCESS;
	}
	cronjob_state = handleCrontab("csw", flag.cron_interval);
	switch(cronjob_state) {
		case CRON_ACTIVE:
			if(verbose) {
				printf("Active cronjob\n");
			}
			break;
		case CRON_CHANGE:
			if(verbose) {
				printf("Active cronjob, modification successful\n");
			}
			break;
		case CRON_INACTIVE:
			if(verbose) {
				printf("Inactive cronjob\n");
			}
			break;
		case CRON_DELETE:
			if(verbose) {
				printf("Deleted cronjob\n");
			}
			break;
		default:
			fprintf(stderr, "ERROR, handleCrontab failed\n");
	};

	time(&rawtime);
	if(getDate(&datetime, rawtime) == -1) {
		return EXIT_FAILURE;
	}

	file_state = findConfig("config", &config_path[0]);
	switch(file_state) {
		case FILE_GOOD:
			if(verbose) {
				printf("File found and in good state at: %s\n", config_path);
			}
			break;
		case FILE_NOTFOUND:
			if(verbose) {
				printf("File was not found in .task/csw/\n");
			}
			return EXIT_FAILURE;
		case FILE_ERROR:
			fprintf(stderr,"ERROR: config file finder caused an error\n");
			return EXIT_FAILURE;
	}

	config_state=readConfig(&content, &error, config_path);
	switch(config_state) {
		case CONFIG_SUCCESS:
			if(verbose)
				printf("config read success\n");
			break;
		case CONFIG_BAD:
			fprintf(stderr, "config has a bad format, reading failed!\n");
			return EXIT_FAILURE;
		case CONFIG_NOTFOUND:
			fprintf(stderr, "Config file was not found!\n");
			return EXIT_FAILURE;
	}

	if(currentContext(current_context) != 0) {
		fprintf(stderr, "ERROR: Couldn't aquire the active context\n");
		return EXIT_FAILURE;
	}
	if(parseConfig(&content, &error, &config) != 0) {
		return EXIT_FAILURE;
	}
	if(flag.show == 1) {
		showZones(&config);
	}

	if(syncConfig(&config, &flag, &datetime) == 1) {
		if(verbose) {
			printf("write changes to the config file\n");
		}
		if(writeConfig(&config, config_path) == -1) {
			return EXIT_FAILURE;
		}		
	}

	if(config.notify == 1 && error.amount > 0) {
		if(notifyError(&error) == -1 && verbose) {
			fprintf(stderr, "WARNING: sending notification to notify daemon failed\n");
		}
	}

	if(verbose) {
		printf("current date & time : %d%s day of the week\t%4d-%02d-%02dT%02d:%02dZ\n",
				datetime.tm_wday, datetime.tm_wday==1?
				"st":datetime.tm_wday==2?"nd":
				datetime.tm_wday==3?"rd":"th",
				datetime.tm_year+1900, datetime.tm_mon+1,
				datetime.tm_mday, datetime.tm_hour,
				datetime.tm_min);
	}

	if(flag.show == 1 && config.excl.amount > 0) {
		showExclusions(&config.excl);
	}
	if(switchExclusion(&config.excl, &datetime) == 0) {
		if(verbose) {
			printf("found a exclusion that matches\n");
		}
		return EXIT_SUCCESS;
	}

	switch_state = switchContext(&config, (datetime.tm_hour*60+datetime.tm_min),
								&command[0], current_context);
	switch(switch_state) {
		case SWITCH_SUCCESS:
			if(sendCommand(command) != 0) {
				fprintf(stderr, "Sending the command failed.\n");
				return EXIT_FAILURE;
			}
			if(config.cancel && activeTask()) {
				if(stopTask() != 0) {
					fprintf(stderr, "Task stop failed!\n");
				}
			}
			if(verbose)
				printf("Switch succesful!\n");
			break;
		case SWITCH_NOTNEEDED:
			if(verbose)
				printf("Switch not needed!\n");
			return EXIT_SUCCESS;
		case SWITCH_FAILURE:
			if(verbose)
				printf("Switch failed!\n");
			return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
