#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <string.h>

#ifndef NARGS
#define NARGS 4
#endif

#define HIJO 0
#define ERROR -1

char *siguiente_linea(void);

void liberar_vector(char *vec[]);

void ejecutar_comando(char *comando, char *argumentos[]);

char *
siguiente_linea()
{
	size_t longitud = 0;
	char *linea = NULL;
	int lectura = getline(&linea, &longitud, stdin);
	if (lectura == -1) {
		free(linea);
		return (char *) NULL;
	}
	if (linea[lectura - 1] == '\n') {
		linea[lectura - 1] = '\0';
	}

	return linea;
}


int
main(int argc, char *argv[])
{
	int maximos_hijos = 1;
	char *comando;

	if (argc == 3) {
		if (strcmp(argv[1], "-P") == 0) {
			maximos_hijos = 4;
			comando = argv[2];
		} else {
			perror("Argumentos invalidos");
			exit(ERROR);
		}
	} else if (argc == 2) {
		comando = argv[1];
	} else {
		perror("Argumentos invalidos");
		exit(ERROR);
	}

	int cantidad_hijos = 0;
	char *linea;
	while ((linea = siguiente_linea())) {
		char *args[NARGS + 2] = { comando, linea };

		size_t tope_args = 2;
		while (tope_args < NARGS + 1 && (linea = siguiente_linea())) {
			args[tope_args] = linea;
			tope_args++;
		}
		args[tope_args] = NULL;

		if (cantidad_hijos >= maximos_hijos) {
			wait(NULL);
			cantidad_hijos--;
		}
		cantidad_hijos++;
		ejecutar_comando(comando, args);

		liberar_vector(args + 1);
	}

	for (int i = 0; i < cantidad_hijos; i++) {
		wait(NULL);
	}

	return 0;
}

void
ejecutar_comando(char *comando, char *argumentos[])
{
	int retorno_fork = fork();
	if (retorno_fork == HIJO) {
		execvp(comando, argumentos);
	} else if (retorno_fork == ERROR) {
		exit(ERROR);
	}
}

void
liberar_vector(char *vec[])
{
	int i = 0;
	while (vec[i] != NULL) {
		free(vec[i]);
		i++;
	}
}
