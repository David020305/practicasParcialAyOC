Primera parte: Definiendo la IDT

1.a)Observen que la macro IDT_ENTRY0 corresponde a cada entrada de la IDT de nivel 0 ¿A qué se refiere cada campo? ¿Qué valores toma el campo offset?

```c
#define IDT_ENTRY0(numero)                                                     
  idt[numero] = (idt_entry_t) {\                     
    .offset_31_16 = HIGH_16_BITS(&_isr##numero),    // indica la direccion en memoria en donde se encuentra la rutina de interrupcion\ 
    .offset_15_0 = LOW_16_BITS(&_isr##numero),      // indica la direccion en memoria en donde se encuentra la rutina de interrupcion\                 
    .segsel = GDT_CODE_0_SEL,                       // selector de segmento la gdt\                       
    .type = INTERRUPT_GATE_TYPE,                    // tipo de descriptor de segmento\ 
    .dpl = 0x00,                                    // nivel de privilegio necesario para ingresar al segmento de la interrupcion\
    .present = 0x01                                 // bit que indica si el segmento esta presente\
  }
```

1.b)Completar los campos de Selector de Segmento (segsel) y los atributos (attr) de manera que al usarse la macro defina una Interrupt Gate de nivel 0. Para el Selector de Segmento, recuerden que la rutina de atención de interrupción es un código que corre en el nivel del kernel. ¿Cuál sería un selector de segmento apropiado acorde a los índices definidos en la GDT[segsel]? ¿Y el valor de los atributos si usamos Gate Size de 32 bits?

```c
el selector apropiado es de codigo ya que tiene que ejecutar la rutina de atencion a la interrupcion. Y es de nivel 0 ya que las interrupciones no quisieramos que se puedan ejecutar desde nivel usuario porque esto podria traer problemas de seguridad.

Si usamos gate type de 32 bits, el INTERRUPT_GATE_TYPE hay que setearlo en 0b1110. Este esta formado por varios bits entre los cuales se encuentra el de la flag "D" (bit 11 que indica gate size). El resto de los attr no cambian nada si usamos 32 o 16.
```

1.c)De manera similar, completar la macro IDT_ENTRY3 para que defina interrupciones que puedan ser disparadas por código no privilegiado (nivel 3).

Completar la función idt_init() con las entradas correspondientes a las interrupciones de reloj y teclado ¿Qué macro utilizarían?

```c
Utilizamos las macros que definimos IDT_ENTRY0 ,IDT_ENTRY3 ya que solo utilizaremos dos niveles de privilegio uno para usuario y otro para kernel
```

Segunda parte: Rutinas de Atención de Interrupción

3.c)Las rutinas de atención de interrupción son definidas en el archivo isr.asm. Cada una está definida usando la etiqueta _isr## donde ## es el número de la interrupción. Busquen en el archivo la rutina de atención de interrupción del reloj.

Completar la rutina asociada al reloj, para que por cada interrupción llame a la función next_clock. La misma se encarga de mostrar, cada vez que se llame, la animación de un cursor rotando en la esquina inferior derecha de la pantalla. La función next_clock está definida en isr.asm.

¿Qué oficiaría de prólogo y epílogo de estas rutinas? ¿Qué marca el iret y por qué no usamos ret?

```c
el prologo es el pushad (encargado de guardar todos los registros en la pila para luego restaurarlos) y el epilogo es el popad (encargado de restaurar los registros).
utilizamos iret dentro de las rutinas de interrupcion ya que en dichas rutinas se encuentra en la pila mas datos (EFLAGS ,CS y en algunos casos el codigo de error, aunque este ultimo es responsabilidad del programador sacarlo de la pila),el iret se encarga de "popear" estos datos para poder cargar el EIP que sera la direccion de retorno.
```