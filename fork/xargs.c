#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#ifndef NARGS
#define NARGS 4
#endif

#define HIJO 0

char *siguiente_linea();

char *
siguiente_linea()
{
	size_t longitud = 0;
	char *linea = NULL;
	int lectura = getline(&linea, &longitud, stdin);
	if (lectura == -1) {
		free(linea);
		return NULL;
	}
	if (linea[lectura - 1] == '\n') {
		linea[lectura - 1] = '\0';
	}

	return linea;
}


int
main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("Argumentos Invalidos\n");
		exit(-1);
	}

	char *comando = argv[1];

	char *linea;
	while ((linea = siguiente_linea())) {
		char *args[NARGS + 2] = { comando, linea };
		size_t tope_args = 2;

		while (tope_args < NARGS + 1 && (linea = siguiente_linea())) {
			args[tope_args] = linea;
			tope_args++;
		}
		args[tope_args] = NULL;

		int retorno_fork = fork();
		if (retorno_fork == HIJO) {
			execvp(comando, args);
		}

		int i = 1;
		while (args[i] != NULL) {
			free(args[i]);
			i++;
		}

		wait(NULL);
	}

	return 0;
}
