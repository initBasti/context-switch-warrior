#ifndef SWITCH_H
#define SWITCH_H

#include "types.h"
#include "helper.h"

EXCLUSION_STATE switchExclusion(struct exclusion*, struct tm*);
SWITCH_STATE switchContext(struct config*, int, char*, char*);
int rangeMatch(struct format_type*, struct tm*);
int sendCommand(char *);
int activeTask();
int stopTask(); 
#endif
