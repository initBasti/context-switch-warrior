#ifndef DELAY_H
#define DELAY_H

#ifndef TYPES_H
#include "types.h"
#endif /* TYPES_H */

#ifndef CONFIG_H
#include <stdio.h>
#include <string.h>
#include "helper.h"

extern size_t strnlen(const char*, size_t);
#endif /* CONFIG_H */

void buildDelayFormat(struct tm*, char*);
DELAY_CHECK checkDelay(int, struct tm*, struct tm*);
int parseDelay(struct tm*,char*);
int parseTimeSpan(char*);
int multiplierForType(char*);
int onlyDigits(char*, size_t);

#endif /* DELAY_H */
