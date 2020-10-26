# Sistemas Operativos
## Trabajo Práctico Nro. 2
#### Construcción del Núcleo de un Sistema Operativo y estructuras de administración de recursos
## Autores

* Arca, Gonzalo - Legajo: 60303
* Parma, Manuel - Legajo: 60425
* Rodriguez, Manuel Joaquin - Legajo: 60258

### Dependencias del proyecto

* GNU Binutils (probado en version 2.25)
* NASM (probado en version 2.11.05)
* QEMU (probado en versiones 4.2.1 y 3.1.0)
* GCC (probado en version 4.9.2)
* Make (probado en version 4.0)

### Como compilar

1- Situarse dentro de la carpeta del proyecto y ejecute los siguientes comandos
NOTA: si utiliza Docker, siga el instructivo incluido para poder acceder a la carpeta del proyecto desde Docker

```
cd RowDaBoat-x64barebones-d4e1c147f975
cd Toolchain
make all
cd ..
```

Para compilar el proyecto utilizando el Memory Manager de Kernighan&Ritchie, ejecutar desde la carpeta RowDaBoat:

```
make all
```

Para compilarlo utilizando el Memory Manager basado en Buddy system, ejecutar desde la carpeta RowDaBoat:

```
make buddy
```

### Como ejecutar el proyecto

Ejecute el siguiente comando

```
./run.sh
```

#### Nota: Para ejecutar los tests desde la shell, o por cualquier otra cosa que quiera saberse sobre los comandos disponibles, ejecute 'help' desde ésta para obtener la información correspondiente.
-----------------------

## Lista de funcionalidades implementadas en el proyecto

A continuación se mencionan las funciones y llamadas al sistema que se implementaron en el proyecto, junto con una descripción de sus comportamientos. El resto de las syscalls (tanto las implementadas en este proyecto como las de Arquitecturas) se encuentra en la sección de **Lista de syscalls**


### Alocación de memoria dinámica


<table>
  <tr>
   <td>Función
   </td>
   <td>Descripción
   </td>
  </tr>
  <tr>
   <td><code>void *<strong>malloc</strong>(size_t size)</code>
   </td>
   <td>Syscall que alloca memoria suficiente para almacenar el size indicado por parámetro, en caso de no haber memoria suficiente disponible retorna NULL
   </td>
  </tr>
  <tr>
   <td><code>void <strong>free</strong>(void *ptr)</code>
   </td>
   <td>Syscall que libera la memoria alocada que arranca en la dirección indicada por parámetro. Si tal memoria nunca fue alocada anteriormente o no existe, se obtendrá undefined behaviour
   </td>
  </tr>
  <tr>
   <td><code>typedef <strong>struct </strong>{</code>
<code>	size_t totalMem;</code>
<p>
<code>	size_t occMem;</code>
<p>
<code>	size_t freeMem;</code>
<p>
<code>} <strong>MemStatus</strong></code>
   </td>
   <td>Estructura utilizada para imprimir el estado de la memoria disponible para alocar. 
<p>
<code>totalMem</code> se refiere al tamaño del heap entero
<p>
<code>occMem</code> se refiere a la cantidad de memoria del heap ocupada
<p>
<code>freeMem</code> se refiere a la cantidad de memoria del heap libre
   </td>
  </tr>
  <tr>
   <td><code>void <strong>getMemStatus</strong>(MemStatus *stat)</code>
   </td>
   <td>Syscall que escribe en la estructura indicada los valores que permiten saber el estado de la memoria disponible para alocar en dicho instante
   </td>
  </tr>
</table>

------

### Creación y administración de procesos


<table>
  <tr>
   <td>Función
   </td>
   <td>Descripción
   </td>
  </tr>
  <tr>
   <td><code>unsigned int <strong>startProcessFg</strong>(int (*mainptr)(int, char const **), int argc, char const *argv[])</code>
   </td>
   <td>Syscall para correr un proceso en <em>foreground </em>(bloquea al proceso en <em>foreground </em>actual).
<p>
Devuelve el <em>PID</em> del proceso nuevo
   </td>
  </tr>
  <tr>
   <td><code>unsigned int <strong>startProcessBg</strong>(int (*mainptr)(int, char const **), int argc, char const *argv[])</code>
   </td>
   <td>Syscall que permite iniciar un proceso en <em>background </em>que corre la función <code>mainptr </code>con <code>argc</code> argumentos que recibe en <code>argv</code>.
<p>
Devuelve el <em>PID </em>del proceso nuevo
   </td>
  </tr>
  <tr>
   <td><code>void <strong>exit</strong>()</code>
   </td>
   <td>Syscall que finaliza la ejecución de un proceso, marcando su estado como "<code>KILLED</code>", desalocando sus recursos, y finalmente llamando a que se ejecute el siguiente proceso en la cola de listos.
   </td>
  </tr>
  <tr>
   <td><code>unsigned int <strong>getpid</strong>()</code>
   </td>
   <td>Syscall que retorna el <em>PID </em>del proceso que la llama
   </td>
  </tr>
  <tr>
   <td><code>void <strong>listProcess</strong>()</code>
   </td>
   <td>Syscall que imprime por salida estándar los procesos corriendo en el momento
   </td>
  </tr>
  <tr>
   <td><code>int <strong>kill</strong>(unsigned int pid, char state)</code>
   </td>
   <td>Syscall para cambiar el estado de un proceso según su <em>PID</em>. Si no llega a encontrar el <em>PID</em> indicado, o si <code>state</code> no representa un valor válido, retorna -1. Cuando su ejecución es exitosa, retorna 0.
<p>
<code>state</code> puede tomar el valor de <code>BLOCKED</code> para bloquear a un proceso, <code>ACTIVE</code> para desbloquearlo, y <code>KILLED</code> para matarlo. 
   </td>
  </tr>
  <tr>
   <td><code>void <strong>runNext</strong>()</code>
   </td>
   <td>Syscall para que el proceso corriendo en el momento renuncie al CPU y se corra el siguiente proceso
   </td>
  </tr>
  <tr>
   <td><code>int <strong>nice</strong>(unsigned int pid, unsigned int priority)</code>
   </td>
   <td>Syscall para cambiar la prioridad de un proceso según su <em>PID</em>, retorna -1 en caso de error.
<p>
<code>priority</code> puede tomar valores desde 0 (mayor prioridad), hasta <code>MAX_QUANTUM-1</code> (menor prioridad). A medida que la prioridad crece, crece la cantidad de quantums de tiempo por los que corre el proceso.
   </td>
  </tr>
</table>

------

### Canales de comunicación


<table>
  <tr>
   <td>Función
   </td>
   <td>Descripción
   </td>
  </tr>
  <tr>
   <td><code>int <strong>createChannel</strong>()</code>
   </td>
   <td>Syscall para crear un canal de comunicación para señales de <code>sleep </code>y <code>wakeup</code>. Devuelve el<em> ID</em> del canal, y en caso de error devuelve -1.
   </td>
  </tr>
  <tr>
   <td><code>int <strong>deleteChannel</strong>(unsigned int id)</code>
   </td>
   <td>Syscall destruye un canal de comunicación para señales de <code>sleep </code>y <code>wakeup </code>dado su <em>ID. </em>Si el <em>ID </em>no se corresponde con ningún canal existente, devuelve -1. Sino devuelve 0.
   </td>
  </tr>
  <tr>
   <td><code>int <strong>sleep</strong>(unsigned int id)</code>
   </td>
   <td>Syscall que manda a dormir al proceso actual en el caso que corresponda. Si ya habían señales pendientes, retorna 1. En caso contrario, devuelve 0. En caso de error, devuelve -1.
   </td>
  </tr>
  <tr>
   <td><code>int <strong>wakeup</strong>(unsigned int id)</code>
   </td>
   <td>Syscall que despierta a los procesos esperando en el canal pasado como argumento por su <em>ID</em>. Si no había nadie durmiendo, incrementa el contador de señales. En este caso, retorna 1. Si había alguien durmiendo, lo despierta y retorna 0. En caso de error retorna -1. Recibe el <em>ID </em>del canal correspondiente
   </td>
  </tr>
  <tr>
   <td><code>void <strong>printChannelPIDs</strong>(unsigned int channelId)</code>
   </td>
   <td>Syscall que imprime los <em>PID</em> de los procesos bloqueados por el canal indicado
   </td>
  </tr>
</table>

-------

### Semáforos/locks


<table>
  <tr>
   <td>Función
   </td>
   <td>Descripción
   </td>
  </tr>
  <tr>
   <td><code>Semaphore *<strong>semOpen</strong>(unsigned int id, unsigned int init)</code>
   </td>
   <td>Función para abrir un Name Semaphore según su <code>id</code>. En <code>init</code> se indica su valor inicial, el cuál es ignorado si el semáforo ya existe en el sistema. El puntero al semáforo es devuelto en la función.
   </td>
  </tr>
  <tr>
   <td><code>void <strong>semWait</strong>(Semaphore *sem)</code>
   </td>
   <td>Se hace una verificación del valor del semáforo: si es mayor que 0, se decrementa y regresa la función; si en cambio es 0, el proceso actual se bloquea hasta que se le notifique que el valor del semáforo cambie. Este bloqueo se realiza a través del sistema de canales de comunicación, en un canal específico para el semáforo. Al despertar también se realiza un decremento del valor del semáforo.
   </td>
  </tr>
  <tr>
   <td><code>void <strong>semPost</strong>(Semaphore *sem)</code>
   </td>
   <td>Incremente el valor del semáforo en 1 y despierta a los procesos que se encuentren durmiendo en el canal del semáforo.
   </td>
  </tr>
  <tr>
   <td><code>int <strong>semClose</strong>(Semaphore *sem)</code>
   </td>
   <td>Función para indicar al sistema que el proceso ya no utilizará más este semáforo. Si no queda ningún proceso utilizándolo, se liberan sus recursos del sistema.
   </td>
  </tr>
  <tr>
   <td><code>void <strong>listSems</strong>()</code>
   </td>
   <td>Imprime en pantalla información sobre los semáforos abiertos actualmente.
   </td>
  </tr>
</table>

-----

### Pipes


<table>
  <tr>
   <td>Función
   </td>
   <td>Descripción
   </td>
  </tr>
  <tr>
   <td><code>int <strong>pipeOpen</strong>(unsigned int pipeId, int pipefd[2])</code>
   </td>
   <td>Syscall para abrir un Named Pipe en base a su <code>pipeID</code>, asignando a los menores file descriptors libres del proceso este pipe, los cuales se devuelven en el array <code>pipefd</code>.
   </td>
  </tr>
  <tr>
   <td><code>int <strong>pipeClose</strong>(int fd)</code>
   </td>
   <td>Syscall para cerrar la referencia del pipe que se encuentra en el file descriptor <code>fd</code> (ya no se podrá acceder al pipe por ese descriptor). En caso de que el pipe representado en el file descriptor ya no tenga procesos abiertos, el pipe se quita del sistema.
   </td>
  </tr>
  <tr>
   <td><code>int <strong>dup2</strong>(int oldfd, int newfd)</code>
   </td>
   <td>Copia la referencia del pipeEnd en <code>oldfd</code> a <code>newfd</code>, por lo que ahora ambos descriptores son equivalentes a <code>oldfd</code>. Tener en cuenta que esta syscall realiza <strong><code>pipeClose(newfd)</code></strong> antes de la clonación.
   </td>
  </tr>
  <tr>
   <td><code>void <strong>listPipes</strong>()</code>
   </td>
   <td>Imprime en pantalla la lista de los pipes activos actualmente en el sistema.
   </td>
  </tr>
</table>

----------
## Lista de syscalls

A continuación se muestran las llamadas al sistema disponibles, indicando los parametros que reciben, y su respectivo número para su invocacion desde la interrupcion 80h

| Syscall                                                                                       | RAX | Descripción                                                                                                                                                                                                                                                                                                                                           |
| --------------------------------------------------------------------------------------------- | --- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| int read(int fd, char *buffer, unsigned long count)                                           | 3   | Syscall que lee de el file descriptor indicado, escribiendolo en buffer hasta que se llegue a "count" caracteres o se llegue al caracter '\n' o a EOF. Retorna la cantidad de caracteres leidos. Recibe en RBX el file descriptor RCX el puntero al buffer y en RDX el valor de count                                                                 |
| int write(unsigned int fd, char *buffer, unsigned long count)                                 | 4   | Syscall que imprime en el file descriptor indicado los primeros count caracteres de buffer. Por defecto, fd = 1 es STDOUT y fd = 2 es STDERR. Retorna la cantidad de caracteres escritos. Recibe en RBX el fd, en RCX el puntero al buffer y en RDX el valor de count                                                                                 |
| void clrScreen()                                                                              | 7   | Syscall que limpia la pantalla actual, no retorna ni recibe parametros                                                                                                                                                                                                                                                                                |
| int changeWindowColor(int rgb)                                                                | 9   | Syscall que cambia el color de los caracteres de la ventana actual. Retorna -1 en error y 0 caso contrario. Recibe en RBX el int que indica el nuevo color(RGB)                                                                                                                                                                                       |
| void getTime(TimeFormat *time)                                                                | 12  | Syscall que rellena una estructura formada por 3 unsigned ints de 32 bits, secs (segundos), mins (minutos), hours (horas) para rellenarla con informacion del tiempo actual. No retorna nada y recibe en RBX el puntero a la estructura                                                                                                               |
| int getCPUTemp()                                                                              | 13  | Syscall que retorna la temperatura del cpu, no recibe parametros                                                                                                                                                                                                                                                                                      |
| RegistersType* getRegisters()                                                                 | 14  | Syscall que retorna un puntero a una estructura con 17 unsigned longs de 64 bits (llamados rax, rbx, rcx, rbp, rdi, rsi, r8, r9, r10, r11, r12, r13, r14, r15, rsp, rip) con los valores de los registros de la ultima vez que se presiono la tecla especial F1, no recibe parametros                                                                 |
| void getMemory(memType* answer, char * address)                                               | 25  | Syscall que escribe en la estructura indicada los 32 bytes de informacion de la memoria a partir de address. No retorna nada y recibe en RBX el puntero a la estructura memType y en RCX el char* que indica la address                                                                                                                               |
| void *malloc(size_t size)                                                                     | 26  | Syscall que aloca memoria suficiente para almacenar el size indicado por parametro, en caso de no haber memoria suficiente disponible retorna NULL caso contrario retorna el puntero al espacio alocado. Recibe en RBX el size_t                                                                                                                      |
| void free(void *ptr)                                                                          | 27  | Syscall que libera la memoria alocada que arranca en la direccion indicada por parametro, si no existe realiza undefined behavior. No retorna nada y recibe en RBX el void* que indica la direccion                                                                                                                                                   |
| void getMemStatus(MemStatus *stat)                                                            | 28  | Syscall que escribe en la estructura indicada (la cual con tiene 3 campos de tipo size_t, totalMem (memoria total), occMem (memoria ocupada), freeMem (memoria libre)) los valores que permiten saber el estado de la memoria disponible para alocar en dicho instante en bytes. No retorna nada y recibe en RBX el puntero a la estructura MemStatus |
| unsigned int startProcessFg(int (*mainptr)(int, char const **), int argc, char const *argv[]) | 29  | Syscall que inicia un proceso en foreground que corre la funcion mainptr con los argumentos indicados. Retorna el PID y recibe en RAX la funcion, en RBX la cantidad de argumentos y en RCX los argumentos                                                                                                                                            |
| unsigned int startProcessBg(int (*mainptr)(int, char const **), int argc, char const *argv[]) | 30  | Syscall analoga a la 29 pero corre el proceso en background                                                                                                                                                                                                                                                                                           |
| void exit()                                                                                   | 31  | Syscall que se encarga de setear el estado como KILLED del proceso que la llama para que luego sea eliminado, no retorna ni recibe parametros                                                                                                                                                                                                         |
| unsigned int getpid()                                                                         | 32  | Syscall que retorna el pid del proceso actual, no recibe parametros                                                                                                                                                                                                                                                                                   |
| void listProcess()                                                                            | 33  | Syscall que imprime en salida estandar el listado de los procesos actuales, no recibe parametros                                                                                                                                                                                                                                                      |
| int kill(unsigned int pid, char state)                                                        | 34  | Syscall para cambiar el estado de un proceso al indicado en RCX segun un pid dado en RBX                                                                                                                                                                                                                                                              |
| void runNext()                                                                                | 35  | Syscall para que el proceso corriendo en el momento renuncie al CPU y se corra el siguiente proceso, no retorna ni recibe parametros                                                                                                                                                                                                                  |
| int nice(unsigned int pid, unsigned int priority)                                             | 36  | Syscall para cambiar la prioridad de un proceso segun un pid dado, retorna -1 en caso de error y 1 en caso exitoso. Recibe en RBX el pid y en RCX la prioridad                                                                                                                                                                                        |
| int createChannel()                                                                           | 37  | Syscall que crea un canal de comunicacion para señales de sleep y wakeup, devuelve el ID del canal, y en caso de error devuelve -1. No recibe parametros                                                                                                                                                                                              |
| int deleteChannel(unsigned int id)                                                            | 38  | Syscall que destruye un canal de comunicacion para señales de sleep y wakeup dado su ID. Si el ID no se corresponde con ningun canal existente, devuelve -1. Sino devuelve 0. Recibe el ID del canal por RBX                                                                                                                                          |
| int sleep(unsigned int id)                                                                    | 39  | Syscall que manda a dormir al proceso actual en el caso que corresponda. Si ya habian señales pendientes, retorna 1. En caso contrario, devuelve 0. En caso de error, devuelve -1. Recibe el id del canal correspondiente por RBX                                                                                                                     |
| int wakeup(unsigned int id)                                                                   | 40  | Syscall que despierta a los procesos esperando en el canal pasado como argumento por su ID. Si no habia nadie durmiendo, incrementa el contador de señales. En este caso, retorna 1. Si habia alguien durmiendo, lo despierta y retorna 0. En caso de error retorna -1. Recibe el id del canal correspondiente por RBX                                |
| void printChannelPIDs(unsigned int channelId)                                                 | 41  | Syscall que imprime los pids bloqueados por el canal indicado en RBX                                                                                                                                                                                                                                                                                  |
| int pipeOpen(unsigned int pipeId, int pipefd[2])                                              | 42  | Syscall que abre un pipe de comunicacion, devuelve un puntero a un vector que en la primer posicion contiene el indice del pipe de lectura y en el segundo el de escritura, y en caso de error devuelve -1. Recibe en RBX el pipeId y en RCX el vector de indices                                                                                     |
| int pipeClose(unsigned int index)                                                             | 43  | Syscall 43 cierra para el proceso actual el acceso al pipe que se encuentra en el indice indicado por paramtro dentro de su vector de pipes. En caso de error, devuelve -1. Caso contrario, 0.                                                                                                                                                        |
| int sys_dup2(int oldfd, int newfd)                                                            | 44  | Syscall que copia oldfd en newfd y devuelve newfd en caso exitoso, sino -1. Recibe en RBX oldfd y en RCX newfd                                                                                                                                                                                                                                        |
| void listPipes()                                                                              | 45  | Syscall que imprime los pipes. No recibe parametros                                                                                                                                                                                                                                                                                                   |