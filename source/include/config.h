#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#include "types.h"
#include "parser.h"
#include "substring.h"
#include "helper.h"
#include "delay.h"

extern FILE *popen( const char *command, const char *modes);
extern int pclose(FILE *stream);
extern int ftruncate(int, off_t);
extern size_t strnlen(const char*, size_t);


FILE_STATE findConfig(char *, char*);
CONFIG_STATE readConfig(struct configcontent*, struct error*, char*);
OPTION_STATE getOption(struct configcontent*, char*, int);
int indexInList(struct configcontent*, int);
int syncConfig(struct config*,struct flags*,struct tm*);
int writeConfig(struct config*, char*);
int checkExclusion(struct exclusion*, struct tm*);
#endif
