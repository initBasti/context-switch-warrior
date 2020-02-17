#ifndef ARGS_H
#define ARGS_H

#include <unistd.h>
#include <sys/types.h>
#include <limits.h>
#include <getopt.h>

#ifndef CONFIG_H
#include "types.h"
#include "helper.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#endif

/* argument parser */
int getArgs(struct flags*, int, char**, char*);
void showHelp();
str_to_int_err strToInt(int*, char*);
#endif /* ARGS_H */
