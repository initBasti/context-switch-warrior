#ifndef EXCLUDE_H
#define EXCLUDE_H

#ifndef CONFIG_H
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "types.h"
#include "helper.h"
#include "substring.h"

extern size_t strnlen(const char*, size_t);
#endif /* CONFIG_H */

PARSER_STATE parseExclusion(struct exclusion*, char*);
int parsePermanent(char*, struct exclusion*);
int parseTemporary(char*, struct exclusion*);
int parseTime(int*, int*, char*);
int parseDate(struct tm*, char*);
int parseWeekday(char*, int);
void initExclusionStruct(struct exclusion*);
void showExclusions(struct exclusion*);
int checkExclusion(struct exclusion*, struct tm*);
void resetExclusion(struct exclusion*, int index);
void buildExclFormat(struct format_type*, char*, char*);
void buildTempExclFormat(struct format_type*, char*);
void buildPermExclFormat(struct format_type*, char*);
int exclTokenLength(int*, char*, char*);
int dayToSet(int*, int, int);
void bubbleSort(int*, int);
#endif /* EXCLUDE_H */
