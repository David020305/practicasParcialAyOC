Primera parte: Preguntas teoricas

a) ¿Cuántos niveles de privilegio podemos definir en las estructuras de paginación?
```c
Existen dos niveles de privilegio: User y Supervisor
```

b) ¿Cómo se traduce una dirección lógica en una dirección física? ¿Cómo participan la dirección lógica, el registro de control CR3, el directorio y la tabla de páginas? Recomendación: describan el proceso en pseudocódigo

```c
---#Segmentacion
segment_selector = logical_address.segment
offset = logical_address.offset
segment descriptor = gdt[segment_selector]
linear_address = segment_descriptor.base + offset

---#Paginacion
page_directory = cr3.20_high_bits
page_directory_entry = page_directory + linear_address.dir
page_table = page_directory_entry
page_table_entry = page_table + linear_address.table
physical_adress = page_table_entry + linear_addres.offset
```

c)  ¿Cuál es el efecto de los siguientes atributos en las entradas de la
    tabla de página?

```c
  - D (Dirty): Indica que la página fue modificada (sucia). Inicia en 0 y se modifica al escribir en la página. En el swap, no copia al disco si "D" está en 0.
  - A (Accesed): Indica si se accedió a memoria controlada por esta PTE. Lo escribe el procesador al traducir
  - PCD : Page-Level Cache Disable. Establece que una página integre el tipo de memoria no cacheable.
  - PWT : Page-Level Write Through. Establece el modo de escritura que tendrá página en el Cache.
  - U/S : Indica el privilegio de la pagina 0 si es supervisor (Kernel),y 1 si es usuario.En general se utiliza 0 con DPL = 00 y 1 en el resto de valores de DPL,adeas el procesador utiliza este campo para autorizar el acceso al segmento
  - R/W : Establece si la página es Read Only (0) o si puede ser escrita (1).
  - P (Present): Indica si la página está en memoria, si se intenta acceder a una página que no está presente, salta un page fault.
```

d)  ¿Qué sucede si los atributos U/S y R/W del directorio y de la tabla de páginas difieren? ¿Cuáles terminan siendo los atributos de una página determinada en ese caso? Hint: buscar la tabla *Combined Page-Directory and Page-Table Protection* del manual 3 de Intel

```c
Si el nivel de privilegio del page directory y el page table difieren, la página terminará teniendo nivel Supervisor.
con respecto al Read/Write ocurre lo siguiente
En el caso de Supervisor:
Si ambos son read write entonces la pagina sera read write.
Si alguno no es read write entonces depende del bit CR0.WP,que en caso de ser 1 sera read only ,y caso contrario sera read write
En el caso de User:
solo será read write si en el page directory y en el page table son read write
```

e) Suponiendo que el código de la tarea ocupa dos páginas y utilizaremos una página para la pila de la tarea. ¿Cuántas páginas hace falta pedir a la unidad de manejo de memoria para el directorio, tablas de páginas y la memoria de una tarea?

```c
para la tarea en si mismo se utiliza 3 paginas (2 de codigo y 1 de datos (pila))
para el manejo de memoria (esquema de paginacion) utlizara al menos 2 paginas ,pero puede incrementarse dependiendo como esten distribuidas (aumentando 1 pagina por cada page table que se necesite)
el total seria como minimo 5 a 7 paginas
```

g) ¿Qué es el buffer auxiliar de traducción (translation lookaside buffer o TLB) y por qué es necesario purgarlo (tlbflush) al introducir modificaciones a nuestras estructuras de paginación (directorio, tabla de páginas)? ¿Qué atributos posee cada traducción en la TLB? Al desalojar una entrada determinada de la TLB ¿Se ve afectada la homóloga en la tabla original para algún caso?

```c
el buffer auxiliar de traduccion TLB es una memoria cache que tiene como proposito almacenar traducciones de direcciones virtuales a fisicas para evitar tener
que ir al esquema de paginacion (acceder al page directory,luego al page table y por ultimo al page frame. 
Es necesario purgarlo cuando se realizan modificaciones en el esquema de paginacion ya que podria quedar una traduccion de una seccion de memoria invalida/desactualizada.
Los atributos que posee una traduccion de la TLB son los bits de la direccion lineal que se utilizan para acceder al page directory y a la page table,y los bits de la direccion fisica contenidos en el descriptor de pla pagina direccionada,es decir,32 a 12.
ademas guarda bits de control referentes al Combined Page-Directory and Page-Table Protection.
Al desalojar una entrada de la TLB no hay que hacer ninguna modificacion sobre la homologa.
```

Segunda parte: Activando el mecanismo de paginación.

a) Escriban el código de las funciones mmu_next_free_kernel_page, mmu_next_free_user_page y de mmu_init_kernel_dir de mmu.c para completar la inicialización del directorio y tablas de páginas para el kernel.

Recuerden que las entradas del directorio y la tabla deben realizar un mapeo por identidad (las direcciones lineales son iguales a las direcciones físicas) para el rango reservado para el kernel, de 0x00000000 a 0x003FFFFF, como ilustra la figura [2]. Esta función debe inicializar también el directorio de páginas en la dirección 0x25000 y las tablas de páginas según muestra la figura [1] ¿Cuántas entradas del directorio de página hacen falta?

```c
el directorio de paginas tendra una unica entrada ya que cada page table puede direccionar 4mb ,y como lo definimos en memoria contigua con una page table alcanza
```

Tercera parte: Definiendo la MMU.

b) Completen el código de copy_page, ¿por qué es necesario mapear y desmapear las páginas de destino y fuente? ¿Qué función cumplen SRC_VIRT_PAGE y DST_VIRT_PAGE? ¿Por qué es necesario obtener el CR3 con rcr3()?

```c
Es necesario mapearlas porque ya estamos en un esquema de paginacion, y para poder acceder a las posiciones de memoria utilizamos direcciones virtuales. Las desmapeamos luego porque este mapeo fue solo para hacer el copy, y no queremos que nos quede una "referencia" a esa posicion de memoria el lugar donde tenemos el identity maping, porque se romperia el mismo.
SRC_VIRT_PAGE y DST_VIRT_PAGE son las direcciones dentro del identity mping donde se mapean las direcciones fisicas con las que trabajamos.
Es necesario obtener el CR3 ya que en el se encuentra el puntero al page directory que utilizamos para poder "acceder" la memoria.
```