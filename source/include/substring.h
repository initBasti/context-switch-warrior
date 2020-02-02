#ifndef SUBSTRING_H
#define SUBSTRING_H
#include "types.h"
#ifdef HELPER_H
#include "helper.h"
#endif

struct substr *allocate_substring();
void free_substring(struct substr*);
int get_substring(char*, struct substr*, int*, char);
int add_substring(struct substr*, char*, char);
int equalListElements(struct substr*);
#endif
