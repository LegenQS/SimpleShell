#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "helper.h"

#define DELIMETER " "
#define EXECV_ERROR 977

struct command {
	const char *cmd_name;
	int (*cmd_handler) (int argc, char **argv);
};

static void pprint_error();

char *input;

static void release(void)
{
	free(input);
}

static int cd_cmd_handler(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "error: only one argument is expected\n");
		return -1;
	}

	if (chdir(argv[1]) < 0) {
		pprint_error();
		return -1;
	}

	return 0;
}

static int exit_cmd_handler(int argc, char **argv)
{
	release();
	exit(0);

	return -1; 
}

static const struct command builtin_cmd[] = {
	{ "cd", cd_cmd_handler },
	{ "exit", exit_cmd_handler },
	{ NULL, NULL }
};

const struct command *check_builtin_cmd(char *input)
{
	const struct command *cmd = NULL;

	for (cmd = builtin_cmd; cmd->cmd_name != NULL; cmd++) {
		if (!strcmp(input, cmd->cmd_name))
			goto out;
	}

out:
	return cmd;
}

static int check_syntax(char *line, const struct command **cmd_)
{
	char *line_dup, *line_dup_, *token;
	const struct command *cmd;

	// duplicate the string in case modified
	line_dup = log_strdup(line);

	line_dup_ = line_dup; 
	if (!line_dup_)
		return -1;

	token = strtok(line_dup, DELIMETER);
	if (!token)
		goto out;

	cmd = check_builtin_cmd(token);
	if (cmd)
		*cmd_ = cmd;

out:
	free(line_dup_);

	return 0;
}

int execute_handler(char *line)
{
	char *argv[_POSIX_ARG_MAX];
	pid_t pid;

	// invalid command
	if (tokenize(line, argv, DELIMETER, _POSIX_ARG_MAX) == -1)
		return -1;

	pid = fork();
	if (pid == -1)
		goto error;

	if (!pid) {
		execv(argv[0], argv);

		// below should not be executed (error instead)
		fprintf(stderr, "error: %s.\n", strerror(errno));
		release();
		exit(EXECV_ERROR);
	}

	return 0;

error:
	pprint_error();
	return -1;
}

static void process_line(char *line)
{
	int exit_status = 0, len = strlen(line);
	char line_dup[len + 1];
	char *argv[_POSIX_ARG_MAX];
	const struct command *cmd = NULL;
	char *cur_cmd;

	strncpy(line_dup, line, len);
	line_dup[len] = '\0';

	if (check_syntax(line_dup, &cmd) == -1)
		goto clean;

	cur_cmd = line_dup;

	if (cmd != NULL) {
		int argc = tokenize(cur_cmd, argv, DELIMETER,
				_POSIX_ARG_MAX);

		if (cmd->cmd_handler(argc, argv))
			goto clean;

		cmd = NULL;
	} else if (execute_handler(cur_cmd) < 0) {
		goto clean;
	}

clean:
	while (1) {
		if (wait(&exit_status) == -1) {
			/*
			 * ECHILD indicates that there are no children left to
			 * wait for.
			 */
			if (errno == ECHILD)
				break;
			pprint_error();
		}
	}
}

static inline void print_prompt(void)
{
	fprintf(stderr, "$ ");
	fflush(stderr);
}

static void control_c_handler(int unused)
{
	release();
	exit(0);
}

int main(int argc, char **argv)
{
	ssize_t n = 0;
	size_t len = 0;

	print_prompt();

	signal(SIGINT, control_c_handler);

	while ((n = getline(&input, &len, stdin)) > 0) {
		if (n > 1) {
			input[n - 1] = '\0';
			process_line(input);
		}
		print_prompt();
	}

	// check if end-of-file indicator has been set to stdin
	if (n < 0 && !feof(stdin))
		pprint_error();

	release();

	return 0;
}

static void pprint_error() {
	fprintf(stderr, "error: %s\n", strerror(errno));
}
