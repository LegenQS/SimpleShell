#ifndef _UTILS_H_
#define _UTILS_H_

#define PIPE_CALL_DEL "|"

/*
 * Copy a string and logs if strdup fails.
 * return: copied string pointer
 */
char *log_strdup(char *str);

/*
 * Tokenizes the string by delimeter. Stores pointers to each token
 * in array. Returns the number of tokens.
 * return: int, number of tokens tokenized
 */
int tokenize(char *str, char **argv, char *delims, int max_tokens);

/*
 * Checks to see if given command is a valid pipeline by comparing
 * number of processes against number of pipes.
 * return: 
 */
int check_pipeline(char *input);

#endif
