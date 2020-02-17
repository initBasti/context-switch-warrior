#ifndef SUBSTRING_H
#define SUBSTRING_H
#include "types.h"
#ifdef HELPER_H
#include "helper.h"
#endif

struct substr *allocateSubstring();
void freeSubstring(struct substr*);
int getSubstring(char*, struct substr*, int*, char);
int addSubstring(struct substr*, char*, char);
int equalListElements(struct substr*);
#endif
