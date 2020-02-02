#ifndef HELPER_H
#define HELPER_H
#include "types.h"
#include <unistd.h>
#include <sys/types.h>
#include <limits.h>

#ifndef CONFIG_H
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#endif

#ifndef PARSER_H
#include "parser.h"
#endif

void lowerCase(char*, int);
int dirExist(char*);
void stripChar(char*, char);
int dayToSet(int*, int, int);
void bubbleSort(int*, int);
void initExclusionStruct(struct exclusion*);
struct context* initContext(struct context*);
void getContext(struct context*);
int contextValidation(struct context*, char*);
int zoneValidation(char*, struct zonetime*, char*);
void showExclusions(struct exclusion*);
void buildExclFormat(struct format_type*, char*, char*);
void buildBoolFormat(int, char*, char*);
void showZones(struct config*);
void debugShowContent(struct configcontent*);
void debugShowError(struct error*);
int currentContext(char*);
int getArgs(struct flags*, int, char**, char*);
void showHelp();
int onlyDigits(char*, size_t);
str_to_int_err strToInt(int*, char*);
int valueForKey(struct keyvalue*, char*);
void addError(struct error*, int, char*, int);
void increaseTime(int, struct tm*);
TIME_CMP compareTime(struct tm*, struct tm*);
void resetExclusion(struct exclusion*, int index);
void resetTm(struct tm*);
void copyTm(struct tm*, struct tm*);
int getDate(struct tm *, time_t);
void buildTempExclFormat(struct format_type*, char*);
void buildPermExclFormat(struct format_type*, char*);
int notifyError(struct error*);
int sendNotification(char *);
int checkNotificationSetup();
#endif
