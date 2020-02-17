#ifndef HELPER_H
#define HELPER_H

#include "types.h"
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>

#ifndef CONFIG_H
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include "delay.h"
#endif

/* small functions used in multiple modules */
void drawLine(int);
void lowerCase(char*, int);
void stripChar(char*, char);

/* context related functions */
struct context* initContext(struct context*);
void getContext(struct context*);
int contextValidation(struct context*, char*);
int currentContext(char*);

/* zone related functions */
int zoneValidation(char*, struct zonetime*, char*);
void showZones(struct config*);

/* parse of time input */
int multiplierForType(char*);
int onlyDigits(char*, size_t);
int parseTimeSpan(char*);

/* struct tm modification functions*/
void increaseTime(int, struct tm*);
TIME_CMP compareTime(struct tm*, struct tm*);
void resetTm(struct tm*);
void copyTm(struct tm*, struct tm*);
int getDate(struct tm *, time_t);

/* notification handling functions */
int notifyError(struct error*);
int sendNotification(char *);
int checkNotificationSetup();
#endif
