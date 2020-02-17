/**
 * @file substring.c
 * @author	Sebastian Fricke
 * @date	2019-11-21
 * @license	GNU Public License
 *
 * @brief	Linked list implementation for splitting strings into sub-parts
 */

#include "include/substring.h"
#include "include/helper.h"
#ifndef CONFIG_H
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#endif

#ifdef DOXYGEN_SHOULD_SKIP_THIS
extern size_t strnlen (__const char *__string, size_t __maxlen);
#endif /*DOXYGEN_SHOULD_SKIP_THIS*/

/**
 * @brief	create and heap allocate a instance of the substr structure
 *
 * @retval	new		success return the instance pointer
 * @retval	NULL	failure
 */
struct substr *allocateSubstring()
{
	struct substr *new = calloc(1, sizeof(struct substr));
	if(!new) {
		return NULL;
	}
	memset(new->member, 0, MAX_FIELD);
	new->next = NULL;
	return new;
}

/**
 * @brief	free the linked list, move through the chain elements
 *
 * @param[in]	head	pointer to the substr struct allocated in allocateSubstring
 */
void freeSubstring(struct substr *head)
{
	while(head) {
		struct substr *tmp = head;
		head = head->next;
		free(tmp);
	}
}

/**
 * @brief	add a new element to the substr structure
 *
 * @param[in]	head	pointer to the substr structure instance
 * @param[in]	input	content for the member of field of the instance
 * @param[in]	sep		separator that was used in getSubstring, remove if any remain
 *
 * @retval	0	SUCCESS
 * @retval	-1	FAILURE, invalid input or ENOMEM
 */
int addSubstring(struct substr *head, char *input, char sep)
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
			ptr->next = allocateSubstring();
			if(!ptr->next) {
				return -1;
			}
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
 * @brief	split input into parts separated by sep and save into a substr linked list
 *
 * @param[in]	input	string for separation
 * @param[in]	sep		character used as separator
 * @param[out]	amount	length of the linked list
 * @param[out]	head	linked list
 *
 * @retval	0	SUCCESS, input separated and linked list created
 * @retval	-1	FAILURE, incorrect input or ENOMEM
 */
int getSubstring(char* input, struct substr* head, int *amount, char sep)
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
	if(addSubstring(head, options[0], sep) != 0) {
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
		if(addSubstring(head, options[i], sep) == 0) {
			*amount = *amount + 1;
		}
		sub_length = copy_size = 0;
	}

	return 0;

	error_return:
		*amount = 0;
		return -1;
}

/**
 * @brief	check if every member of a linked list is of equal size
 *
 * @param[in]	head	linked list
 *
 * @retval	0 on SUCCESS
 * 			!0 on FAILURE
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
