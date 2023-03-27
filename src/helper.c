#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "helper.h"

char *log_strdup(char *str)
{
	char *str_dup = strdup(str);

    // strdup fails
	if (!str_dup)
		fprintf(stderr, "error: %s\n", strerror(errno));

	return str_dup;
}

int tokenize(char *str, char **argv, char *delimeter, int max)
{
	int argc = 0;

	for (argv[argc] = strtok(str, delimeter); 
            argv[argc] != NULL;
			argv[argc] = strtok(NULL, delimeter)) {

		if (++argc == max) {
			fprintf(stderr, "error: too many tokens\n");
			return -1;
		}
	}

	return argc;
}

static int is_empty(const char *str)
{
	while (*str != '\0') {
		if (!isspace(*str)) {
			return 0;
        }
        str++;
	}

	return 1;
}

/*
 * reference to
 * http://stackoverflow.com/questions/8705844/
 */
static char *single_del_strtok(char *str, char const *delimiter)
{
	static char *src = "";
	char *p = 0;
	char *ret = 0;

	if (str)
		src = str;

	if (!src)
		return NULL;

	p = strpbrk(src, delimiter);

	if (p) {
		*p  = 0;
		ret = src;
		src = ++p;
	} else if (*src) {
		ret = src;
		src = NULL;
	}

	return ret;
}

static int get_num_delimeter(const char *str, const char delimeter)
{
	int num_delim = 0;

	while (*str != '\0') {
		if (*str == delimeter) {
			num_delim++;
        }
        str++;
	}

	return num_delim;
}

static int get_num_tokens(char *str, const char *delimeter)
{
	int num = 0;
    int len = strlen(str);

	char str_dup[len + 1];
	char *arg;

	strncpy(str_dup, str, len);
	str_dup[strlen(str)] = '\0';

	arg = single_del_strtok(str_dup, delimeter);

	while (arg) {
		if (!is_empty(arg)) {
			num++;
        }
		arg = single_del_strtok(NULL, delimeter);
	}

	return num;
}

int check_pipeline(char *input)
{
	int num_of_tokens = get_num_tokens(input, PIPE_CALL_DEL);
	int num_of_delimeters = get_num_delimeter(input, *PIPE_CALL_DEL);

    // if no cmd is provided or no data exists between tokens
	if (num_of_tokens < 1 || num_of_tokens != num_of_delimeters + 1) {
		fprintf(stderr, "error: invalid pipeline\n");
		return 0;
	}

	return 1;
}