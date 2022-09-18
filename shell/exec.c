#include "exec.h"

void execvp_or_exit(char **args);
void pipe_or_exit(int *fds);
int fork_or_exit(void);

void
execvp_or_exit(char **args)
{
	if (execvp(args[0], args) != 0) {
		perror(args[0]);
		exit(EXIT_FAILURE);
	};
}

void
pipe_or_exit(int *fds)
{
	if (pipe(fds) == -1) {
		perror("error creating pipe");
		exit(EXIT_FAILURE);
	}
}

int
fork_or_exit()
{
	int i = fork();
	if (i == -1) {
		exit(EXIT_FAILURE);
	}
	return i;
}

// sets "key" with the key part of "arg"
// and null-terminates it
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  key = "KEY"
//
static void
get_environ_key(char *arg, char *key)
{
	int i;
	for (i = 0; arg[i] != '='; i++)
		key[i] = arg[i];

	key[i] = END_STRING;
}

// sets "value" with the value part of "arg"
// and null-terminates it
// "idx" should be the index in "arg" where "=" char
// resides
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  value = "value"
//
static void
get_environ_value(char *arg, char *value, int idx)
{
	size_t i, j;
	for (i = (idx + 1), j = 0; i < strlen(arg); i++, j++)
		value[j] = arg[i];

	value[j] = END_STRING;
}

// sets the environment variables received
// in the command line
//
// Hints:
// - use 'block_contains()' to
// 	get the index where the '=' is
// - 'get_environ_*()' can be useful here
static void
set_environ_vars(int eargc, char **eargv)
{
	for (int i = 0; i < eargc; i++) {
		char key[ARGSIZE];
		char value[ARGSIZE];
		get_environ_key(eargv[i], key);
		get_environ_value(eargv[i], value, block_contains(eargv[i], '='));
		setenv(key, value, 1);
	}
}

// opens the file in which the stdin/stdout/stderr
// flow will be redirected, and returns
// the file descriptor
//
// Find out what permissions it needs.
// Does it have to be closed after the execve(2) call?
//
// Hints:
// - if O_CREAT is used, add S_IWUSR and S_IRUSR
// 	to make it a readable normal file
static int
open_redir_fd(char *file, int flags)
{
	int modes = 0;
	flags = flags | O_CLOEXEC;

	if (flags & O_WRONLY) {
		flags = flags | O_CREAT | O_TRUNC;
		modes = S_IRUSR | S_IWUSR;
	}

	int fd = open(file, flags, modes);
	if (fd < 0) {
		exit(-1);
	}

	return fd;
}

// executes a command - does not return
//
// Hint:
// - check how the 'cmd' structs are defined
// 	in types.h
// - casting could be a good option
void
exec_cmd(struct cmd *cmd)
{
	// To be used in the different cases
	struct execcmd *e = (struct execcmd *) cmd;
	struct backcmd *b = (struct backcmd *) cmd;
	struct execcmd *r = (struct execcmd *) cmd;
	struct pipecmd *p = (struct pipecmd *) cmd;

	switch (cmd->type) {
	case EXEC:
		set_environ_vars(e->eargc, e->eargv);
		execvp_or_exit(e->argv);
		break;

	case BACK: {
		exec_cmd(b->c);
		break;
	}

	case REDIR: {
		if (strlen(r->in_file) > 0) {
			int fd = open_redir_fd(r->in_file, O_RDONLY);
			dup2(fd, STDIN_FILENO);
		}

		if (strlen(r->out_file) > 0) {
			int fd = open_redir_fd(r->out_file, O_WRONLY);
			dup2(fd, STDOUT_FILENO);
		}

		if (strlen(r->err_file) > 0) {
			int fd;
			if (r->err_file[0] == '&' && r->err_file[1] == '1') {
				fd = 1;
			} else {
				fd = open_redir_fd(r->err_file, O_WRONLY);
			}
			dup2(fd, STDERR_FILENO);
		}

		r->type = EXEC;
		exec_cmd((struct cmd *) r);

		break;
	}

	case PIPE: {
		int fds[2];
		pipe_or_exit(fds);

		int p1 = fork_or_exit();
		if (p1 == 0) {
			dup2(fds[WRITE], STDOUT_FILENO);
			close(fds[READ]);
			close(fds[WRITE]);

			exec_cmd(p->leftcmd);
		}

		int p2 = fork_or_exit();
		if (p2 == 0) {
			dup2(fds[READ], STDIN_FILENO);
			close(fds[READ]);
			close(fds[WRITE]);

			exec_cmd(p->rightcmd);
		}

		close(fds[READ]);
		close(fds[WRITE]);

		// free the memory allocated
		// for the pipe tree structure
		free_command(parsed_pipe);

		waitpid(p1, NULL, 0);
		int status;
		waitpid(p2, &status, 0);

		exit(status);

		break;
	}
	}
}
