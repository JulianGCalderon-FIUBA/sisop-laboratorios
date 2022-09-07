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

int leer_linea_sin_nueva_linea(char **linea, size_t *longitud);

int
main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("Argumentos Invalidos\n");
		exit(-1);
	}
	char *comando = argv[1];

	char *linea = NULL;
	size_t longitud = 0;
	int lectura;
	while ((lectura = leer_linea_sin_nueva_linea(&linea, &longitud)) > 0) {
		char *args[NARGS + 2] = { comando, linea };
		int tope_args = 2;
		for (; tope_args < NARGS + 1; tope_args++) {
			char *linea_ = NULL;
			size_t longitud_ = 0;
			int lectura_;
			if ((lectura_ = leer_linea_sin_nueva_linea(
			             &linea_, &longitud_)) > 0) {
				args[tope_args] = linea_;
			} else {
				break;
			}
		}
		args[tope_args] = NULL;

		int retorno_fork = fork();
		if (retorno_fork == HIJO) {
			execvp(comando, args);
		}
		wait(NULL);
	}

	free(linea);
	return 0;
}

int
leer_linea_sin_nueva_linea(char **linea, size_t *longitud)
{
	int lectura = getline(linea, longitud, stdin);
	if (lectura >= 0) {
		if ((*linea)[lectura - 1] == '\n') {
			(*linea)[lectura - 1] = '\0';
		}
	}

	return lectura;
}
