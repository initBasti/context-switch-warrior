#ifndef PARSER_H
#define PARSER_H

#include <ctype.h>

#ifndef CONFIG_H
#include "substring.h"
#endif

#ifndef TYPES_H
#include "types.h"
#endif

#ifndef HELPER_H
#include "helper.h"
#endif

PARSER_STATE parseExclusion(struct exclusion*, char*);
int parsePermanent(char*, struct exclusion*);
int parseTemporary(char*, struct exclusion*);
int parseTime(int*, int*, char*);
int parseDate(struct tm*, char*);
int exclTokenLength(int*, char*, char*);
int parseWeekday(char*, int);
int parseConfig(struct configcontent*, struct error*, struct config*);
int parseTimeSpan(char*);
int parseDelay(struct tm*,char*);
int multiplierForType(char*);
#endif /* PARSER_H */
