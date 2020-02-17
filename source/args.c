/**
 * @file args.c
 * @author Sebastian Fricke
 * @date	2020-02-13
 *
 * @brief	read different argument from the CLI and parse correctly
 */

#include "include/args.h"

#ifndef DOXYGEN_SHOULD_SKIP_THIS
extern size_t strnlen(const char *s, size_t maxlen);
extern int opterr, optind, optopt;
extern char *optarg;
#endif

/**
 * @brief	use the gnu function getopt to read arguments into the flag struct
 *
 * @param[in]	argc	number of arguments
 * @param[in]	argv	2D array of argument strings
 * @param[in]	opt_string	arg parse options
 * @param[out]	flag	pointer to the flag structure containg the parsed options
 *
 * @retval	0	SUCCESS
 * @retval	-1	FAILURE
 */
int getArgs(struct flags *flag, int argc, char **argv, char *opt_string)
{
	int arg_int = 0;
	int delay = 0;
	int c = 0;
	opterr = 0;
	optind = 0;
	optopt = 0;
	optarg = NULL;

	if(argv == NULL || opt_string == NULL) {
		return -1;
	}

	while((c=getopt (argc, argv, opt_string)) != -1) {
		switch(c) {
			case 'd':
				if(optarg ==  NULL) {
					fprintf(stderr, "Option -d: requires an argument\n");
					return -1;
				}
				if((delay = parseTimeSpan(optarg)) != -1) {
					flag->delay = delay;
				}
				else {
					fprintf(stderr, "Valid for -d: int/float + min/h/d\n");
					return -1;
				}
				break;
			case 'h':
				flag->help = 1;
				break;
			case 's':
				flag->show = 1;
				break;
			case 'v':
				if(optarg == NULL) {
					if(flag->verbose != NULL) {
						*flag->verbose = 1;
					}
				}
				else {
					if(strToInt(&arg_int, optarg) == STR_TO_INT_SUCCESS) {
						if( arg_int == 1 || arg_int == 2 ) {
							*flag->verbose = arg_int;
						}
						else {
							fprintf(stderr, "Valid for -v: 1 | 2\n");
							return -1;
						}
					}
				}
				break;
			case 'n':
				if(optarg ==  NULL) {
					fprintf(stderr, "Option -n: requires an argument\n");
					return -1;
				}
				if(strToInt(&arg_int, optarg) == STR_TO_INT_SUCCESS) {
					if( arg_int == 0 || arg_int == 1 ) {
						flag->notify_on = arg_int;
					}
					else {
						fprintf(stderr, "Valid for -n: 0 | 1\n");
						return -1;
					}
				}
				break;
			case 'c':
				if(optarg ==  NULL) {
					fprintf(stderr, "Option -c: requires an argument\n");
					return -1;
				}
				if(strToInt(&arg_int, optarg) == STR_TO_INT_SUCCESS) {
					if( arg_int == 0 || arg_int == 1 ) {
						flag->cancel_on = arg_int;
					}
					else {
						fprintf(stderr, "Valid for -c: 0 | 1\n");
						return -1;
					}
				}
				break;
			case 'i':
				if(optarg == NULL) {
					fprintf(stderr, "Option -i: requires an argument\n");
					return -1;
				}
				if(strToInt(&arg_int, optarg) == STR_TO_INT_SUCCESS) {
					if(arg_int >= 0) {
						flag->cron_interval = arg_int;
					}
					else {
						fprintf(stderr, "Valid for -i: integer > 0\n");
						return -1;
					}
				}
				else {
					return -1;
				}
				break;
			case '?':
				fprintf(stderr, "unknown option %c\n", optopt);
				return -1;
				break;
			default:
				printf("DEBUG hit return -1; @ default \n");
				return -1;
		}
	}
	for (int index = optind; index < argc; index++) {
		printf ("Non-option argument %s\n", argv[index]);
	}
	return 0;
}
#ifndef DOXYGEN_SHOULD_SKIP_THIS
void showHelp()
{
	printf("[C]ontext [S]witch [W]arrior\n");
	printf("Usage information:\n");
	printf("Command line options:\n");
	printf("-h - help (show usage information)\n");
	printf("-v - verbose (show messages about the details of a run)\n");
	printf("-s - show (show the options from the config)\n");
	printf("-d - delay (add a delay to the switch of a context)\n");
	printf("     requires an argument, valid values:\n");
	printf("     integer/float number & m|min|minute or h|hour or d|day)\n");
	printf("-c - cancel (Set if csw should cancel active tasks)\n");
	printf("     requires an argument, valid values:\n");
	printf("     1(yes) or 0(no) (change the behavior permanently)\n");
	printf("-n - notify (Set if csw should send notifications)\n");
	printf("     requires an argument, valid values:\n");
	printf("     1(yes) or 0(no) (change the behavior permanently)\n");
}
#endif /* DOXYGEN_SHOULD_SKIP_THIS */

/*
 * @brief	wrapper around strtol, with robust error handling
 *
 * @param[in] input		input string 
 * @param[out] output	integer of the parsed number 
 *
 * @retval	STR_TO_INT_INCONVERTIBLE	string empty	
 * @retval	STR_TO_INT_OVERFLOW			exceed integer data type boundary
 * @retval	STR_TO_INT_UNDERFLOW		exceed integer data type negative boundary
 * @retval	STR_TO_INT_SUCCESS			parse SUCCESSful
 */
str_to_int_err strToInt(int *output, char* input)
{
	char *end = NULL;
	long number = 0;
	if(input[0] == '\0') {
		return STR_TO_INT_INCONVERTIBLE;
	}

	number = strtol(input, &end, 10);
	errno = 0;

	if(number > INT_MAX || (errno == ERANGE && number == LONG_MAX)) {
		return STR_TO_INT_OVERFLOW;
	}

	if(number < INT_MIN || (errno == ERANGE && number == LONG_MIN)) {
		return STR_TO_INT_UNDERFLOW;
	}

	if(*end != '\0') {
		return STR_TO_INT_INCONVERTIBLE;
	}

	*output = number;

	return STR_TO_INT_SUCCESS;
}
