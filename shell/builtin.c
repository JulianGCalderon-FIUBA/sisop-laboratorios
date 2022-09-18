#include "builtin.h"

void update_prompt(void);

// returns true if the 'exit' call
// should be performed
//
// (It must not be called from here)
int
exit_shell(char *cmd)
{
	return strcmp(cmd, "exit") == 0;
}


void
update_prompt()
{
	char cwd[PRMTLEN - 2];
	if (getcwd(cwd, sizeof cwd) == NULL) {
		perror("pwd: error reading directory");
	};
	snprintf(prompt, sizeof prompt, "(%s)", cwd);
}

// returns true if "chdir" was performed
//  this means that if 'cmd' contains:
// 	1. $ cd directory (change to 'directory')
// 	2. $ cd (change to $HOME)
//  it has to be executed and then return true
//
//  Remember to update the 'prompt' with the
//  	new directory.
//
// Examples:
//  1. cmd = ['c','d', ' ', '/', 'b', 'i', 'n', '\0']
//  2. cmd = ['c','d', '\0']
int
cd(char *cmd)
{
	if (strcmp(cmd, "cd") == 0) {
		if (chdir(getenv("HOME")) != 0) {
			perror("cd: error changing directory\n");
		}
		update_prompt();
		return 1;
	} else if (strncmp(cmd, "cd ", 3) == 0) {
		if (chdir(cmd + 3) != 0) {
			perror("cd: error changing directory\n");
		}
		update_prompt();
		return 1;
	}

	return 0;
}

// returns true if 'pwd' was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
pwd(char *cmd)
{
	if (strcmp(cmd, "pwd") != 0) {
		return 0;
	}

	char pwd[PRMTLEN - 2];
	if (getcwd(pwd, sizeof pwd) == NULL) {
		perror("pwd: error reading directory");
	};
	printf("%s\n", pwd);
	return 1;
}
