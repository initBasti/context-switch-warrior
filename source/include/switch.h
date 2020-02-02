#ifndef SWITCH_H
#define SWITCH_H

#include "types.h"

EXCLUSION_STATE switchExclusion(struct exclusion*, struct tm*);
SWITCH_STATE switchContext(struct config*, int, char*, char*);
int dateMatch(struct tm*, int, int, int);
int rangeMatch(struct format_type*, int, int, int);
int sendCommand(char *);
int activeTask();
int stopTask(); 
#endif
