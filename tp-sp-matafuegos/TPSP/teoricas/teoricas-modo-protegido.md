
Primera parte: Definiendo la GDT

1.Explorando el manual Intel Volumen 3: System Programming. Sección 2.2 Modes of Operation. 
¿A qué nos referimos con modo real y con modo protegido en un procesador Intel? ¿Qué particularidades tiene cada modo?

```c
En un procesador intel cuando hablamos de **modo real** nos referimos al modo que proporciona el entorno de programación del procesador Intel 8086,es el que se utiliza al iniciar el sistema operativo para conservar la retrocompatibilidad.
Tiene las siguientes particularidades : utiliza un conjunto de instrucciones reducido (ISA 8086),direccionamiento a 1mb (utilizando 20bits),sin proteccion de acceso a memoria,sin niveles de privilegio

cuando nos referimos al modo protegido nos referimos al modo nativo de los procesadores actuales que tiene grandes ventajas como mayor flexibilidad,performance,y compatibilidad con el software existente
tiene las siguientes particularidades : utiliza el conjunto de instrucciones de 32 bits (IA-32),direccionamiento a 4gb (utilizando 32bits),proteccion de acceso a memoria,niveles de privilegio,posibilidad de utilizar paginacion,soporte para multitarea.
```

2.Comenten en su equipo, ¿Por qué debemos hacer el pasaje de modo real a modo protegido? 
¿No podríamos simplemente tener un sistema operativo en modo real? ¿Qué desventajas tendría?

```c
Debemos hacer el pasaje a modo protegido ya que con ello obtenemos muchas ventajas
la mas importante es la de proteccion de memoria ya que sin ella un programa podria modificar el codigo del S.O y con ello vulnerar la seguridad del usuario,ademas
podremos direccionar a mas memoria ,tener un conjunto de instrucciones mas amplio y tener mas herramientas para gestionar el acceso a memoria (paginacion y mas control en esquema de segmentacion)

Si se podria utilizar el modo real para el sistema operativo,pero tendria muchas desventajas entre ellas direccionamiento reducido,nulo control al acceso de memoria,set de instrucciones reducidos.
```

3.Busquen el manual volumen 3 de Intel en la sección 3.4.5 Segment Descriptors. ¿Qué es la GDT? ¿Cómo es el formato de un descriptor de segmento, bit a bit? Expliquen brevemente para qué sirven los campos Limit, Base, G, P, DPL, S. También pueden referirse a los slides de la clase teórica, aunque recomendamos que se acostumbren a consultar el manual.

```c
la **GDT** (Global Descriptor Table) es un arreglo en memoria de longitud variable que puede contener hasta 213 elementos,estos elementos son descriptores de segmentos
que tiene como proposito proporcionar al procesador caractericas de dicho segmento (tamaño,ubicacion,informacion de control y acceso).
el formato de un descriptor de segmento es el siguiente.

31         24 23  22 21  19                16 15  12 11       8 7            0 
______________________________________________________________________________   
|Base 31:24|G |D/B|L| AVL |SegmentLimit 19:16|P|DPL|S|  Type   |Base 23:16   |
¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯

31                                         16 15                             0
______________________________________________________________________________
|     BaseAddress 15:00                      |      SegmentLimit 15:00       |
¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯¯

Limit: Especifica el tamaño del segmento.
Base: Define la ubicación del byte 0 del segmento 
G:Determina la escala del campo de límite de segmento. Cuando el indicador de granularidad está desactivado, el límite de segmento se interpreta en unidades de bytes; cuando el indicador está activado, el límite de segmento se interpreta en unidades de 4 Kbytes.
P:Indica si el segmento está presente en la memoria (activado) o no está presente (desactivado)
DPL:Especifica el nivel de privilegio requerido para acceder al segmento. El nivel de privilegio puede oscilar entre 0 y 3, siendo 0
el nivel más privilegiado. El DPL se utiliza para controlar el acceso al segmento
S:Especifica si el descriptor de segmento es para un segmento del sistema (el indicador S está desactivado) o un segmento de código o datos
(el indicador S está activado)
```

4.La tabla de la sección 3.4.5.1 Code- and Data-Segment Descriptor Types del volumen 3 del manual del Intel nos permite completar el Type, los bits 11, 10, 9, 8. ¿Qué combinación de bits tendríamos que usar si queremos especificar un segmento para ejecución y lectura de código?

```c
Segmento para ejecucion y lectura de codigo tiene el Type: 0b1010 (10 en decimal)
```

6.En el archivo gdt.h observen las estructuras: struct gdt_descriptor_t y el struct gdt_entry_t. ¿Qué creen que contiene la variable extern gdt_entry_t gdt; y extern gdt_descriptor_t GDT_DESC;?

```c
extern gdt_entry_t gdt : es el arreglo en memoria de descriptores de segmentos es decir la gdt.
extern gdt_descriptor_t GDT_DES :es el descriptor de la gdt el cual contiene el tamaño de la gdt y su direccion en memoria.
```

10.Busquen qué hace la instrucción LGDT en el Volumen 2 del manual de Intel. Expliquen con sus palabras para qué sirve esta instrucción. En el código, ¿qué estructura indica donde está almacenada la dirección desde la cual se carga la GDT y su tamaño? ¿dónde se inicializa en el código?

```c
la instruccion LGDT carga los valores del operando de origen en el registro de la tabla descriptora global (GDTR),se ejecuta en modo real para para permitir la inicializacion del procesador antes de cambiar a modo protegido
la estructura que indica la direccion y tamaño de la gdt es el gdt_descriptor_t , la estructura esta definida en gdt.h y la inicializacion de la variable en gdt.c
```

Segunda parte: Pasaje a modo protegido

13.Investiguen en el manual de Intel sección 2.5 Control Registers, el registro CR0. ¿Deberíamos modificarlo para pasar a modo protegido? Si queremos modificar CR0, no podemos hacerlo directamente. Sólo mediante un MOV desde/hacia los registros de control (pueden leerlo en el manual en la sección citada).

```c
Si deberiamos modificar el cr0 ya que en dicho registro de control el bit 0 es el que indica si esta activado el modo protegido o no.
*CR0.PE (bit 0 de CR0): habilita el modo protegido cuando está activado; habilita el modo de dirección real cuando
está desactivado. Esta bandera no habilita la paginación directamente. Solo habilita la protección a nivel de segmento.*
```

15. Notemos que a continuación debe hacerse un jump far para posicionarse en el código de modo protegido. Miren el volumen 2 de Intel para ver los distintos tipos de JMPs disponibles y piensen cuál sería el formato adecuado. ¿Qué usarían como selector de segmento?

```c
utilizamos el segmento de codigo de nivel 0 ya que cuando pasamos a modo protegido todavia estamos configurando el sistema operativo y necesitamos tener nivel de privilegio maximo y seguir ejecutando codigo.
```

Tercera parte: Configurando la pantalla

22.Observen el método screen_draw_box en screen.c y la estructura ca en screen.h . ¿Qué creen que hace el método screen_draw_box? ¿Cómo hace para acceder a la pantalla? ¿Qué estructura usa para representar cada carácter de la pantalla y cuanto ocupa en memoria?

```c
el metodo screen_draw_box lo que hace es dibujar un rectangulo en el cual cada celda tendra el caracter que se le paso por argumento.
accede al buffer de memoria que se encuentra en la direccion VIDEO (0xB8000).
la estructura que usa para representar cada caracter en la pantalla es el struct ca que ocupa 2 bytes
```
```c
24.nos parecieron interesente dos cosas en particular: 
 - una que el bootloader cargue en una direccion particular el kernel y luego el kernel mismo se pasa a otro bloque en memoria.
 - lo otro que nos llamo la atencion es que se tenga que inicializar el SO en modo real unicamente por retrocompatibilidad.
```