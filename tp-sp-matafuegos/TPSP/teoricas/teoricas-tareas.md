Primera parte: Inicialización de tareas

1. Si queremos definir un sistema que utilice sólo dos tareas, ¿Qué nuevas estructuras, cantidad de nuevas entradas en las estructuras ya definidas, y registros tenemos que configurar?¿Qué formato tienen? ¿Dónde se encuentran almacenadas?

```c
Nuevas Estructuras                                          |        TSS            |TSS Descriptor |Descriptor de Puerta de Tarea  |Task Register      |   Flag NT
Cantidad de nuevas entradas en las estructuras ya definidas |        -              |       2       |       -                       |   -               |   -
Donde se encuentran almacenadas                             |Task Control Block(TCB)|   GDT o LDT   |GDT,LDT o  IDT                 |Registro Especifico|   EFLAGS(Bit 14)
                                                            |Accesible Por Kernel   |
Qué formato tienen                                          |Img Teorica30          |Img Teorica32  |Img Teorica34                  |Img Teorica33      |   -
Registros que tenemos que configurar                        |       -               |       -       |       -                       |         Si        |   Si
```

2. ¿A qué llamamos cambio de contexto? ¿Cuándo se produce? ¿Qué efecto tiene sobre los registros del procesador? Expliquen en sus palabras que almacena el registro TR y cómo obtiene la información necesaria para ejecutar una tarea después de un cambio de contexto.

```c
¿A qué llamamos cambio de contexto? Restaurar el contexto (estado de registros) de un programa para reanudar su ejecucion,(guardando el contexto de ejecucion de la tarea actual) 
¿Cuándo se produce? Al ejecutarse alguna de estas intrucciones ,call,jmp,iret,call implicito del procesador al handler de una interrupcion o excepcion manejado por una tarea
¿Qué efecto tiene sobre los registros del procesador? Modifica los registros de proposito general,tr,cr3,registro de segmento,cr0
Expliquen en sus palabras que almacena el registro TR y cómo obtiene la información necesaria para ejecutar una tarea después de un cambio de contexto.
en el registro TR se encuentra el indice que se utiliza dentro de la GDT para encontrar el TSS Descriptor
con el TSS Descriptor obtengo el limite y la base de donde se encuentra la TSS en el cual esta el contexto de ejecucion de dicha tarea antes de ser switcheada a otra.
con la TSS ya encontrada el procesador se encarga de cargar el contexto y seguir la ejecucion.
```

3.Al momento de realizar un cambio de contexto el procesador va almacenar el estado actual de acuerdo al selector indicado en el registro **TR** y ha de restaurar aquel almacenado en la TSS cuyo selector se asigna en el *jmp* far. ¿Qué consideraciones deberíamos tener para poder realizar el primer cambio de contexto? ¿Y cuáles cuando no tenemos tareas que ejecutar o se encuentran todas suspendidas?

```c
las consideracion a tener en cuenta en el primer cambio de contexto es :
1 - inicializar 2 Descriptores de TSS uno para la tarea actual (la inicial) y otro para la tarea iddle en la GDT.
2 - cargar el selector de tss en el task register para guardar el contexto de la tarea inicial.

la consideracion que tenemos que tener cuando no tenemos mas tareas que ejecutar es haber inicializado la tarea idle para poder hacer el task switch cuando no tenga otras tareas que ejecutar
```

4. ¿Qué hace el scheduler de un Sistema Operativo? ¿A qué nos referimos con que usa una política?

```c
El scheduler es el encargado de tener el algoritmo que indica cual es la proxima tarea a ejecutar. Usa una politica porque debe poder decidir de alguna forma que tarea es la siguiente a ejecutar por el procesador.
```

5. En un sistema de una única CPU, ¿cómo se hace para que los programas parezcan ejecutarse en simultáneo?

```c
Para que parezca que los programas se ejecutan en simultaneo, el scheduler va indicando el cambio de tareas muy rapidamente
```


9. Utilizando **info tss**, verifiquen el valor del **TR**. También, verifiquen los valores de los registros **CR3** con **creg** y de los registros de segmento **CS,** **DS**, **SS** con ***sreg***. ¿Por qué hace falta tener definida la pila de nivel 0 en la tss?

```c
Hace falta tener definida la pila de nivel 0 en la tss porque cuando salta una interrupcion (por ejemplo del scheduler) tenes que pasar a ejecutar codigo de nivel de kernel, pero en el contexto de la tarea que se esta ejecutando en el momento.
```
Segunda parte: Poniendo todo en marcha

11. Estando definidas **sched_task_offset** y **sched_task_selector**:
```
  sched_task_offset: dd 0xFFFFFFFF
  sched_task_selector: dw 0xFFFF
```

Y siendo la siguiente una implementación de una interrupción del reloj:

```
global _isr32
  
_isr32:
  pushad
  call pic_finish1
  
  call sched_next_task
  
  str cx
  cmp ax, cx
  je .fin
  
  mov word [sched_task_selector], ax
  jmp far [sched_task_offset]
  
  .fin:
  popad
  iret
```

a)  Expliquen con sus palabras que se estaría ejecutando en cada tic del reloj línea por línea

```c
pushad                                  ,es el prologo, que pushea todos los registros y demas a la pila.
call pic_finish1                        ,le avisa al pic quue ya se atendio la interrupcion.
call sched_next_task                    ,llama al scheduler para obtener el segmento de tss de la proxima tarea
str cx                                  ,almacena en cx el segmento de tss de la tarea actual
cmp ax,cx                               ,compara si el segmento de tss de la tarea actual es igual al de la proxima tarea
je .fin                                 ,si son iguales va a fin
mov word [sched_task_selector], ax      ;si son distintos ent nces guarda en la direccion de memoria sched_task_selector el selector  
jmp far [sched_task_offset]             ,realiza el cambio de contexto
.fin:
popad                                   ,epilogo que restaura el contexto de ejecucion anterior a ejecutar la rutina de interrupcion
iret                                    ,retorno especial de una interrupcion
```

b)  En la línea que dice ***jmp far \[sched_task_offset\]*** ¿De que tamaño es el dato que estaría leyendo desde la memoria? ¿Qué indica cada uno de estos valores? ¿Tiene algún efecto el offset elegido?

```c
el dato es de 6 bytes (?).
los 2 bytes indican un selector de la gdt (?) y los 4 para offset.
el offset no afecta en nada ya que al realizar un cambio de contexto se carga el eip del nuevo contexto.
```

c)  ¿A dónde regresa la ejecución (***eip***) de una tarea cuando vuelve a ser puesta en ejecución?

```c
regresa a donde estaba anteriormente antes de que haga un switch, que es en el epilogo de la rutina de atencion a la interrupcion del switch. Esto sabe donde estaba porque lo guarda en la tss de la tarea.
```

12. Para este Taller la cátedra ha creado un scheduler que devuelve la próxima tarea a ejecutar.

a)  En los archivos **sched.c** y **sched.h** se encuentran definidos los métodos necesarios para el Scheduler. Expliquen cómo funciona el mismo, es decir, cómo decide cuál es la próxima tarea a ejecutar. Pueden encontrarlo en la función ***sched_next_task***.

```c
Se fija en el orden en que fueron cargadas al array sched_tasks. verifica el atributo state del struct sched_entry_t si es TASK_RUNNABLE entonces retorna el selector de tss de dicha tarea,caso contrario sigue buscando en el resto,si no hay ninguna con el atributo TASK_RUNNABLE entonces retorna el selector de tss de la tarea idle
```

Tercera parte: Tareas? Qué es eso?

14. Como parte de la inicialización del kernel, en kernel.asm se pide agregar una llamada a la función **tasks\_init** de **task.c** que a su vez llama a **create_task**. Observe las siguientes líneas:
```C
int8_t task_id = sched_add_task(gdt_id << 3);

tss_tasks[task_id] = tss_create_user_task(task_code_start[tipo]);

gdt[gdt_id] = tss_gdt_entry_for_task(&tss_tasks[task_id]);
```
a)  ¿Qué está haciendo la función ***tss_gdt_entry_for_task***?

```c
inicializa un descriptor de tss para la tss pasada por parametro.
```

b)  ¿Por qué motivo se realiza el desplazamiento a izquierda de **gdt_id** al pasarlo como parámetro de ***sched_add_task***?

```c
porque necesita armar una sched_entry para agregarla al array de tareas, y para eso necesitamos pasarle como argumento un selector,al desplazar 3 bits transformamos un indice de la gdt en un selector.
```

15. Ejecuten las tareas en *qemu* y observen el código de estas superficialmente.

a) ¿Qué mecanismos usan para comunicarse con el kernel?
```c
para comunicarse con el kernel utiliza la syscall (syscall_draw) que ejecuta la interrupcion de software 88.
```

b) ¿Por qué creen que no hay uso de variables globales? ¿Qué pasaría si una tarea intentase escribir en su `.data` con nuestro sistema?
no utilizamos variable globales ya que en el caso de querer compartir datos entre tareas podriamos hacerlo a travez del bloque de memoria on demand.
como las tareas no tiene mapeado una pagina para el .data esto genera un page fault.

16. Observen **tareas/task_prelude.asm**. El código de este archivo se ubica al principio de las tareas.

a. ¿Por qué la tarea termina en un loop infinito?
```c
la tarea termina en un loop ya que hay algunas tareas como (GameOfLife) que queremos que se ejecuten siempre,por mas que termine de ejecutarse una partida.
```
Análisis:

18. Analicen el *Makefile* provisto. ¿Por qué se definen 2 "tipos" de tareas? ¿Como harían para ejecutar una tarea distinta? Cambien la tarea *Snake* por una tarea *PongScoreboard*.
```c
se definen dos tipos de tareas para poder clonarlas,en el caso inicial 3 de tipo "Pong" y una de tipo "Snake"
para ejecutar una tarea distinta tenemos que cambiar las variables TASKA y TASKB que se encuentran en el makefile.
```

19. Mirando la tarea *Pong*, ¿En que posición de memoria escribe esta tarea el puntaje que queremos imprimir? ¿Cómo funciona el mecanismo propuesto para compartir datos entre tareas?

```c
las tareas guardan los puntajes a partir de la direccion SHARED_SCORE_BASE_VADDR.
el mecanismo para compartir datos entre tareas en el bloque de memoria on demand.
```