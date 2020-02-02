#include "include/substring.h"
#include "include/helper.h"
#ifndef CONFIG_H
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#endif

extern size_t strnlen (__const char *__string, size_t __maxlen);

struct substr *allocate_substring()
{
	struct substr *new = calloc(1, sizeof(struct substr));
	if(!new) {
		return NULL;
	}
	memset(new->member, 0, MAX_FIELD);
	new->next = NULL;
	return new;
}

void free_substring(struct substr *head)
{
	while(head) {
		struct substr *tmp = head;
		head = head->next;
		free(tmp);
	}
}

int add_substring(struct substr *head, char *input, char sep)
{
	int in_len = strnlen(input, MAX_EXCL_LINE);
	int mem_len = 0;

	if(in_len<=1 || in_len>MAX_FIELD) {
		return -1;
	}
	if(strnlen(head->member, MAX_EXCL_LINE)<=1) {
		stripChar(input, sep);
		strncpy(head->member, input, in_len+1);
		head->next = NULL;
		return 0;
	}
	for(struct substr *ptr = head ; ptr != NULL ; ptr=ptr->next){
		if(ptr->next == NULL) {
			ptr->next = allocate_substring();
			if(!ptr->next) {
				return -1;
			}
			/* delete any remaining separator in the string */
			stripChar(input, sep);
			mem_len = strnlen(input, MAX_ROW);
			strncpy(ptr->next->member, input, mem_len);
			ptr->next->next = NULL;
			return 0;
		}
	}
	return -1;
}

/*
 * get_substring:
 * split the input line into smaller pieces between the given separator
 * fill a linked list and return it together with it's length
 * @parameter(in): input line, separator char
 * @parameter(out): the head to the linked list, the pointer to the length var
 * return:			0 on SUCCESS
 * 				   -1 on FAILURE
 */
int get_substring(char* input, struct substr* head, int *amount, char sep)
{
	char subsets[MAX_EXCL][MAX_EXCL_LINE] = {"0"};
	char options[MAX_EXCL][MAX_FIELD] = {"0"};
	char *ptr = NULL;
	int length = strnlen(input, MAX_ROW);
	int sub_length = 0;
	int copy_size = 0;
	int index = 0;

	if(length <= 1) {
		goto error_return;
	}

	strncpy(subsets[index], input, length);

	while((ptr=strchr(input, sep)) != NULL) {
		index++;
		strncpy(subsets[index], ptr, strlen(ptr));
		input = subsets[index]+1;
	}

	copy_size = strnlen(subsets[0], MAX_EXCL_LINE) -
				strnlen(subsets[1], MAX_EXCL_LINE);

	strncpy(options[0],subsets[0],copy_size);
	/* possibly wasted lines of code */
	if(!options[0]) {
		goto error_return;
	}
	if(add_substring(head, options[0], sep) != 0) {
		goto error_return;
	}
	*amount = *amount + 1;

	for(int i = 1 ; i <= index ; i++) {
		length = strlen(subsets[i]+1);
		if(i<index) {
			sub_length = strlen(subsets[i+1]+1);
		}
		copy_size = length - sub_length;
		strncpy(options[i], subsets[i]+1, copy_size);
		if(add_substring(head, options[i], sep) == 0) {
			*amount = *amount + 1;
		}
		sub_length = copy_size = 0;
	}

	return 0;

	error_return:
		*amount = 0;
		return -1;
}

/*
 * equalListSize:
 * checks the different elements of a linked list for their length
 * as soon as one of the elements deviates or is greater than the Maximum
 * length of a Date the function returns a non-zero integer.
 * This function is used specifically for the parser.c
 * @parameter(in): the head of the linked list
 * return:		0 on SUCCESS
 * 			not 0 on FAILURE
 */
int equalListElements(struct substr* head)
{
	int size = 0;
	for(struct substr *ptr = head ; ptr != NULL ; ptr = ptr->next) {
		if(size == 0) {
			size = strnlen(ptr->member, MAX_FIELD);
		}
		else {
			if((int)strnlen(ptr->member, MAX_FIELD) != size) {
				return size - strnlen(ptr->member, MAX_FIELD);
			}
		}
		if(size > DATE) {
			return (DATE) - size;
		}
	}
	return 0;
}
