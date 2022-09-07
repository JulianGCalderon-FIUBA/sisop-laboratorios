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
#define OK 0

void imprimir_primos(int max);

void enviar_secuencia_inicial(int max, int pipe_derecho[2]);

void siguiente_primo(int pipe_izquierdo[2]);

void transmitir_secuencia(int primo, int pipe_izquierdo[2], int pipe_derecho[2]);


int
main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("Debe ingresar un numero maximo para el algoritmo\n");
		return ERROR;
	}

	int limite = atoi(argv[1]);
	imprimir_primos(limite);

	return OK;
}

void
imprimir_primos(int max)
{
	int pipe_derecho[2];
	if (pipe(pipe_derecho) == ERROR) {
		exit(ERROR);
	}

	int retorno_fork = fork();
	if (retorno_fork == HIJO) {
		close(pipe_derecho[ESCRITURA]);
		siguiente_primo(pipe_derecho);
	} else if (retorno_fork == ERROR) {
		exit(ERROR);
	} else {
		close(pipe_derecho[LECTURA]);
		enviar_secuencia_inicial(max, pipe_derecho);
		close(pipe_derecho[ESCRITURA]);
		wait(NULL);
	}
}

void
enviar_secuencia_inicial(int max, int pipe_derecho[2])
{
	int i = 2;
	while (i <= max) {
		if (write(pipe_derecho[ESCRITURA], &i, sizeof(int)) == ERROR) {
			exit(ERROR);
		}
		i++;
	}
}

void
siguiente_primo(int pipe_izquierdo[2])
{
	int pipe_derecho[2];
	if (pipe(pipe_derecho) == ERROR) {
		exit(ERROR);
	}

	int primo;
	int lectura = read(pipe_izquierdo[LECTURA], &primo, sizeof(int));
	if (lectura == 0) {
		close(pipe_izquierdo[LECTURA]);
		close(pipe_derecho[LECTURA]);
		close(pipe_derecho[ESCRITURA]);
		return;
	} else if (lectura == ERROR) {
		exit(ERROR);
	}

	printf("primo %i\n", primo);

	int retorno_fork = fork();
	if (retorno_fork == HIJO) {
		close(pipe_izquierdo[LECTURA]);
		close(pipe_derecho[ESCRITURA]);
		siguiente_primo(pipe_derecho);
	} else if (retorno_fork == ERROR) {
		exit(ERROR);
	} else {
		close(pipe_derecho[LECTURA]);
		transmitir_secuencia(primo, pipe_izquierdo, pipe_derecho);
		close(pipe_derecho[ESCRITURA]);
		close(pipe_izquierdo[LECTURA]);

		wait(NULL);
		exit(OK);
	}
}

void
transmitir_secuencia(int primo, int pipe_izquierdo[2], int pipe_derecho[2])
{
	int i;
	while (read(pipe_izquierdo[LECTURA], &i, sizeof(int)) == sizeof(int)) {
		if (i % primo != 0) {
			if (write(pipe_derecho[ESCRITURA], &i, sizeof(int)) ==
			    ERROR) {
				exit(ERROR);
			}
		}
	}
}
