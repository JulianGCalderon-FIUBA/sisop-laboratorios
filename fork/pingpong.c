#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

#define HIJO 0

#define LECTURA 0
#define ESCRITURA 1

#define ERROR -1

int
main(void)
{
	int pfd1[2];
	int pfd2[2];

	if (pipe(pfd1) == ERROR) {
		return ERROR;
	}
	if (pipe(pfd2) == ERROR) {
		return ERROR;
	}


	printf("Hola, soy PID %i:\n", getpid());
	printf("  - primer pipe me devuelve: [%i, %i]\n", pfd1[0], pfd1[1]);
	printf("  - segundo pipe me devuelve: [%i, %i]\n\n", pfd2[0], pfd2[1]);

	pid_t i = fork();

	if (i == HIJO) {
		close(pfd1[ESCRITURA]);
		close(pfd2[LECTURA]);

		pid_t pid = getpid();
		pid_t ppid = getppid();

		int lectura;
		if (read(pfd1[LECTURA], &lectura, sizeof(int)) == ERROR) {
			return ERROR;
		}

		printf("Donde fork me devuelve %i:\n", i);
		printf("  - getpid me devuelve: %i\n", pid);
		printf("  - getppid me devuelve: %i\n", ppid);

		printf("  - recibo valor %i via fd=%i\n", lectura, pfd1[LECTURA]);

		if (write(pfd2[ESCRITURA], &lectura, sizeof(int)) == ERROR) {
			return ERROR;
		}

		printf("  - reenvio valor en fd=%i\n\n", pfd2[ESCRITURA]);
	} else {
		close(pfd1[LECTURA]);
		close(pfd2[ESCRITURA]);

		pid_t pid = getpid();
		pid_t ppid = getppid();

		srandom(3);
		int rnd = random();

		printf("Donde fork me devuelve %i:\n", i);
		printf("  - getpid me devuelve: %i\n", pid);
		printf("  - getppid me devuelve: %i\n", ppid);

		printf("  - random me devuelve: %i\n", rnd);
		if (write(pfd1[ESCRITURA], &rnd, sizeof(int)) == ERROR) {
			return ERROR;
		}

		printf("  - envio valor %i a traves de fd=%i\n\n",
		       rnd,
		       pfd1[ESCRITURA]);

		int lectura;
		if (read(pfd2[LECTURA], &lectura, sizeof(int)) == ERROR) {
			return ERROR;
		}

		printf("Hola, de nuevo PID %i:\n", pid);
		printf("  - recibi valor %i via fd=%i\n", lectura, pfd2[LECTURA]);

		wait(NULL);
	}

	return 0;
}
