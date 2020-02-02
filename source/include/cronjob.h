#ifndef CRONJOB_H
#define CRONJOB_H

#ifndef CONFIG_H
#include <stdio.h>
#include <stdlib.h>
#include "helper.h"
#include "types.h"
extern FILE *popen( const char *command, const char *modes);
extern int pclose(FILE *stream);
extern size_t strnlen(const char*, size_t);
#endif /* IF CONFIG_H */

#include <regex.h>

CRON_STATE handleCrontab(char*, int);
int writeCrontab(char*, int, struct loc_env*);
int readCrontab(FILE*, int size, char[][size]);
int deleteCrontab(char*, struct loc_env*);
int parseCrontab(char[], int*);
int buildCronCommand(char*, int, struct loc_env*, char*);
void buildCronInterval(int, char*);
int checkTerm(char*);
int checkCrontab(char *, int size, char[][size], int*);
int checkCronEnv(int size, char[][size]);
#endif /* CRONJOB_H */
