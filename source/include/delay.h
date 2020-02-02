#ifndef DELAY_H
#define DELAY_H

#ifndef TYPES_H
#include "types.h"
#endif /* TYPES_H */

#ifndef CONFIG_H
#include <stdio.h>
#include <string.h>
#include "helper.h"
#endif /* CONFIG_H */

void buildDelayFormat(struct tm*, char*);
DELAY_CHECK checkDelay(int, struct tm*, struct tm*);

#endif /* DELAY_H */
