# Lab: shell

### Búsqueda en $PATH
__¿Cuales son las diferencias entre la _syscall_ `execve(2)` y la familia de _wrappers_ proporcionados por la biblioteca estándar de C (_libc_) `exec(3)`?__

La familia de _wrappers_ propocionados añaden nuevas funcionalidades a la _syscall_, Segun los caracteres que le siguen a `exec`
- `l`: Permite recibir una lista para sus arumentos, en lugar de un vector. Todos los elementos deberan ser punteros a cadenas terminantes en el caracter nulo. Por convencion, el primer argumento sera el nombre del binario.
- `v`: En contraste con el anterior, recibe los argumentos dentro de un vector (_array_).
- `p`: Busca los binarios en el _path_, si este no contiene el caracter `/`. Lo que permite ejecutar facilmente comandos sin tener que insertar la direccion completa
- `e`: Ejecuta el comand con el _environment_ especificado. En el resto de casos, lo hereda del proceso que lo llama.

__¿Puede la llamada a exec(3) fallar? ¿Como se comporta la implementacion de la _shell_ en ese caso?__

Las llamadas pueden fallar de la misma forma que falla la _syscall_ `execve(2)`. 
En la shell, si fallan, se imprime el nombre del comando que fallo, seguido de la razon por la cual fallo.

---

### Comandos built-in

__¿Entre `cd` y `pwd`, alguno de los dos se podría implementar sin necesidad de ser built-in? ¿Por qué? ¿Si la respuesta es sí, cuál es el motivo, entonces, de hacerlo como _built-in_? (para esta última pregunta pensar en los _built-in_ como `true` y `false`)__

Los _built-in_ son necesarios cuando se busca modificar el estado de la _shell_. Es por esta razon que `pwd` no necesita serlo, unicamente accede al directorio actual.
Por otro lado, `cd` debe modificar la _shell_ por lo que necesita estar programada dentro de la misma.

Sin embargo, al ser un comando tan usado, se puede optar por hacerlo un _built-in_. De esta forma, obtenemos la ventaja de velocidad que estos nos proveen.

---

### Variables de entorno adicionales

__¿Por qué es necesario hacerlo luego de la llamada a `fork(2)`?__

Si la incorporacion de variables de entorno se realiza antes de la llamada a `fork`, entonces estas variables de entorno estarian disponibles para la _shell_. Esta no es la intencion. Se debe realizar despues, para que estas variables unicamente esten disponibles para el nuevo proceso.

__En algunos de los _wrappers_ de la familia de funciones de `exec(3)` (las que finalizan con la letra e), se les puede pasar un tercer argumento (o una lista de argumentos dependiendo del caso), con nuevas variables de entorno para la ejecución de ese proceso. Supongamos, entonces, que en vez de utilizar `setenv(3)` por cada una de las variables, se guardan en un _array_ y se lo coloca en el tercer argumento de una de las funciones de `exec(3)`.__


__¿El comportamiento resultante es el mismo que en el primer caso? Explicar qué sucede y por qué.__

El comportamiento resultante no es el mismo. Las _wrappers_ mencionados no heredan el entorno del proceso que los llama, sino que toman el entorno indicado en sus argumentos. Por el otro lado, utilizar `setenv` permite mantener el entorno del padre, pudiendo incorporar nuevas variables.

__Describir brevemente (sin implementar) una posible implementación para que el comportamiento sea el mismo.__

Para que el comportamiento sea el mismo, es necesario obtener todas las variables de entorno del proceso. Para luego llamar a algunas de las funciones de `exec(3)`. De esta forma, como tercer argumento tendriamos un vector con no solo las variables nuevas a incorporar, sino con las ya existentes.

---

### Procesos en segundo plano

__Detallar cual es el mecanismo utilizado para implementar procesos en segundo plano.__

Si el comando ejecutado no es de tipo `BACK`, entonces se esperara a que el proceso hijo termine antes de seguir con la ejecucion. Si lo es, entonces no se esperara al proceso hijo y se continuara la ejecucion. Para que no queden procesos _huerfanos_, luego de leer cada linea, se ejecutara un `wait` _oportunistico_ para captar algun proceso hijo que haya finalizado su ejecucion.

---

### Flujo estándar

__Investigar el significado de `2>&1`, explicar como funciona su forma _general_ y mostrar que sucede con la salida de `cat out.txt` en el ejemplo. Luego repetirlo invirtiendo el orden de las redirecciones. ¿Cambio algo?__

La secuencia `2>&1` significa: Reemplazar el _file descriptor_ 2 (`stderr`)  por un duplicado del _file descriptor_ 1 (`stdout`). En otras palabras, el `stderr` se redirige hacia `stdout`.

Primero, vemos que el comando trata de acceder a dos diretorios. El primer directorio `/home` existe, por lo que se imprimen sus contenidos correctamente. Al tratar de acceder a un directorio inexsitente, se imprime un error (a traves de `stdout`)


```bash
$ ls -C /home /noexiste
ls: cannot access '/noexiste': No such file or directory
/home:
julian
```

En el ejemplo mostrado, se redirige `stdoud` hacia un archivo, y luego se redirige `stderr` hacia `stdout`. Esto significa que ambos estaran apuntando al mismo archivo: `out.txt`. Por lo que el comando no producira ningun resultado visible en la terminal. Si leemos el archivo, deberiamos encontrar el resultado del comando.

```bash
$ ls -C /home /noexiste >out.txt 2>&1
$ cat out.txt
ls: cannot access '/noexiste': No such file or directory
/home:
julian
```

Por ultimo, se pide intercambiar el orden de las redirecciones. Se ejecuta el mismo comando, redirigiendo `stderr` hacia el `stdout`. Luego se redirige `stdout` hacia un archivo. Sin embargo, `stderr` no se modifico, por lo que seguira apuntando hacia `stdout`.

```bash
$ ls -C /home /noexiste 2>&1 >out.txt
ls: cannot access '/noexiste': No such file or directory
$ cat out.txt
/home:
julian
```
De esta forma, `stdout` apuntara hacia un archivo mientras que `stderr` se mostrara en la terminal.

---

### Tuberías simples (pipes)

__Investigar que ocurre con el _exit code_ rportado por la _shell_ si se ejecuta un pipe. ¿Cambia en algo? ¿Que ocurre si, en un pipe, alguno de los dos comandos falla? Mostrar evidencia (e.g. saldas de terminal) de este comportamiento usando _bash_. Comparar con la implementaión de este lab.__

Investigando la terminal, vemos que la shell guarda el exit code del ultimo comando del pipe
```bash
$ echo ok | ls noexiste
ls: cannot access 'noexiste': No such file or directory
$ echo $?
2 #CODIGO DE ERROR DE LS
```

Podemos observar que si el comando que falla es el primero, igualmente obtenemos el codigo de error del ultimo comando.
```bash
$ ls noexiste | echo ok
ok
ls: cannot access 'noexiste': No such file or directory
$ echo $?
0 #CODIGO DE EXITO DE ECHO
```

Incluso si el primer comando termina despues, el comportamiento es el mismo.
```bash
$ sleep 1 | ls noexiste
ls: cannot access 'noexiste': No such file or directory
$ echo $?
2 #CODIGO DE ERROR DE LS
```
En nuestra shell, el codigo de salida del comando ejecutado (en el caso de los pipes) no se guarda.

---

__Investigar al menos otras tres variables mágicas estándar, y describir su propósito. Incluir un ejemplo de su uso en bash (u otra terminal similar).__

- `$?`: Devuelve el codigo de salida del ultimo proceso en ejecutarse.
    ```bash
    $ ls noexiste
    ls: cannot access 'noexiste': No such file or directory.
    $ echo $?
    2
    
    ```

- `$!`: Devuelve el PID del ultimo proceso en correr en segundo plano.
    ```bash
    $ sleep 5 &
    [1] 46280
    $ echo $!
    46280

    ```

- `$_`: Devuelve el ultimo argumento del comando anterior.
    ```bash
    $ echo arg1 arg2
    arg1 arg2
    $ echo $_
    arg2

    ```

- `$$`: Devuelve el PID del script (en este caso la shell)
    ```bash
    $ ps | grep bash | tr -s
    46396 pts/4    00:00:00 bash
    $ echo $$
    46396

    ```

---


