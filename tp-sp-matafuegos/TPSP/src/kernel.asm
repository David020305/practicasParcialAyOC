; ** por compatibilidad se omiten tildes **
; ==============================================================================
; TALLER System Programming - Arquitectura y Organizacion de Computadoras - FCEN
; ==============================================================================

%include "print.mac" 
global start
; COMPLETAR - Agreguen declaraciones extern según vayan necesitando
extern tasks_screen_draw
extern KERNEL_STACK
extern GDT_DESC 
extern IDT_DESC
extern screen_draw_layout
extern A20_enable
extern pic_enable
extern pic_reset
extern copy_page
extern mmu_init_kernel_dir
extern mmu_init_task_dir
extern idt_init
extern sched_init
extern tasks_init
extern tss_init
; COMPLETAR - Definan correctamente estas constantes cuando las necesiten
%define CS_RING_0_SEL 0b0000000000001000    ;codigo de segmento de nivel 0 de la gdt
%define DS_RING_0_SEL 0b0000000000011000    ;datos de segmento de nivel 0 de la gdt 
%define KERNEL_STACK 0x25000

%define  GDT_IDX_TASK_INITIAL_SEL    (11<<3)

%define GDT_IDX_TASK_IDLE_SEL (12<<3)
%define DIVISOR 0b10001110 ;18206 * 2, reducir a la mitad los ticks 
BITS 16
;; Saltear seccion de datos
jmp start

;;
;; Seccion de datos.
;; -------------------------------------------------------------------------- ;;
start_rm_msg db     'Iniciando kernel en Modo Real'
start_rm_len equ    $ - start_rm_msg

start_pm_msg db     'Iniciando kernel en Modo Protegido'
start_pm_len equ    $ - start_pm_msg

start_clear_msg db     '                          '
start_clear_len equ    $ - start_clear_msg

;;
;; Seccion de código.
;; -------------------------------------------------------------------------- ;;0x000a

;; Punto de entrada del kernel.
BITS 16
start:
    ; ==============================
    ; ||  Salto a modo protegido  ||
    ; ==============================

    ; COMPLETAR - Deshabilitar interrupciones (Parte 1: Pasaje a modo protegido)
    cli 
    
    ; Cambiar modo de video a 80 X 50
    mov ax, 0003h
    int 10h ; set mode 03h
    xor bx, bx
    mov ax, 1112h
    int 10h ; load 8x8 font

    ; COMPLETAR - Imprimir mensaje de bienvenida - MODO REAL (Parte 1: Pasaje a modo protegido)
    ; (revisar las funciones definidas en print.mac y los mensajes se encuentran en la
    ; sección de datos)

    
    
    print_text_rm start_rm_msg, start_rm_len, 0x4, 0x000a, 0x000a

    ; COMPLETAR - Habilitar A20 (Parte 1: Pasaje a modo protegido)
    ; (revisar las funciones definidas en a20.asm)
    call A20_enable

    ; COMPLETAR - los defines para la GDT en defines.h y las entradas de la GDT en gdt.c
    ; COMPLETAR - Cargar la GDT (Parte 1: Pasaje a modo protegido)
    
    lgdt [GDT_DESC] 
    
    ;---------------------------------------------------------------------------------------------------------------------------------------
    ; COMPLETAR - Setear el bit PE del registro CR0 (Parte 1: Pasaje a modo protegido) punto 14
    mov eax, cr0
    or  eax, 1  ; pongo en 1 PE
    mov cr0, eax
    
    ; COMPLETAR - Saltar a modo protegido (far jump) (Parte 1: Pasaje a modo protegido)
    ; (recuerden que un far jmp se especifica como jmp CS_selector:address)
    ; Pueden usar la constante CS_RING_0_SEL definida en este archivo
    jmp dword CS_RING_0_SEL:modo_protegido ; paso un el primer selector de segmento de la GDT y salto a modo protegido 

BITS 32
modo_protegido:
    ; COMPLETAR (Parte 1: Pasaje a modo protegido) - A partir de aca, todo el codigo se va a ejectutar en modo protegido
    ; Establecer selectores de segmentos DS, ES, GS, FS y SS en el segmento de datos de nivel 0
    ; Pueden usar la constante DS_RING_0_SEL definida en este archivo
    mov ax, DS_RING_0_SEL      
    mov ds, ax
    mov es, ax
    mov gs, ax
    mov fs, ax
    mov ss, ax

    ; COMPLETAR - Establecer el tope y la base de la pila (Parte 1: Pasaje a modo protegido)
    
    mov esp, KERNEL_STACK       ; ESP apunta al tope \\Pato segun la imagen de paginacion deber arrancar en 0x24 PATO
    mov ebp, esp              ; base inicial de frames

    ; COMPLETAR - Imprimir mensaje de bienvenida - MODO PROTEGIDO (Parte 1: Pasaje a modo protegido)
    print_text_pm start_pm_msg, start_pm_len, 0xF, 0x000a, 0x000a

    ; COMPLETAR - Inicializar pantalla (Parte 1: Pasaje a modo protegido)
    call screen_draw_layout
    
    ; ===================================
    ; ||     (Parte 3: Paginación)     ||
    ; ===================================

    ; COMPLETAR - los defines para la MMU en defines.h -- Listo
    ; COMPLETAR - las funciones en mmu.c -- Faltan algunas
    ; COMPLETAR - reemplazar la implementacion de la interrupcion 88 (ver comentarios en isr.asm) -- 
    ; COMPLETAR - La rutina de atención del page fault en isr.asm

    ; COMPLETAR - Inicializar el directorio de paginas
    cli
    
    call mmu_init_kernel_dir ; Inicializo las estructuras de paginacion 
    

    ; COMPLETAR - Cargar directorio de paginas 
    mov cr3, eax ; Inicializo el registro cr3

    ; COMPLETAR - Habilitar paginacion 
    ; Hablitamos el bit 31 de CR0 para activas paginacion
    mov eax, cr0
    or  eax, 0x80000000         
    mov cr0, eax

    
    
    ;--------------------
    ;test para copy_page|
    ;mov eax, 0x403000  |
    ;push eax           |
    ;mov eax, 0x404000  |
    ;push eax           |
    ;call copy_page     |
    ;pop eax            |
    ;pop eax            |
    ;--------------------    
    




    ; ========================
    ; ||  (Parte 4: Tareas) ||
    ; ========================

    ; COMPLETAR - reemplazar la implementacion de la interrupcion 88 (ver comentarios en isr.asm)
    ; COMPLETAR - las funciones en tss.c
    ; COMPLETAR - Inicializar tss
    call tss_init
    ; COMPLETAR - Inicializar el scheduler
    call sched_init
    ; COMPLETAR - Inicializar las tareas
    call tasks_init

    ; ===================================
    ; ||   (Parte 2: Interrupciones)   ||
    ; ===================================

    ; COMPLETAR - las funciones en idt.c
    
    ; COMPLETAR - Inicializar y cargar la IDT
    call idt_init
    lidt [IDT_DESC]
    
    ; COMPLETAR - Reiniciar y habilitar el controlador de interrupciones (ver pic.c)
    call pic_reset
    call pic_enable
    


    ; COMPLETAR - Rutinas de atención de reloj, teclado, e interrupciones 88 y 89 (en isr.asm)
    ;int 32
    ;mov al ,0b0110100
    ;out 0x43, al       ; 0x43 es el puerto de control del timer,y el numero es la instruccion de control que le avisa que va a modificar
                              ; el ratio de conteo y lo hara en dos escritura (alta y baja)        
    ;mov al ,0b10001110
    ;out 0x40 ,al        ;18206 *2
  
    ;mov al ,0b00111100
    ;out 0x41 ,al

     ; El PIT (Programmable Interrupt Timer) corre a 1193182Hz.

    ; Cada iteracion del clock decrementa un contador interno, cuando éste llega

    ; a cero se emite la interrupción. El valor inicial es 0x0 que indica 65536,

    ; es decir 18.206 Hz

    mov ax, DIVISOR
    out 0x40, al
    rol ax, 8
    out 0x40, al
    
    sti ;habilitar interrupciones
    ;-----------------------------------------------------------------------------------------------
    ;prueba del page fault. punto f de paginacion
    ;mov eax ,cr3
    ;push eax    ; guardo el cr3 para restaurar 
    
    ;mov eax, 0x18000
    ;push eax
    ;call mmu_init_task_dir
    ;mov cr3, eax ;cambio de contexto, empiezo a leer la memoria como si estoy en la tarea
    
    
    ; primera escritura en on-demand
    ;mov dword [0x07000000], 0x200000
    ;print_text_pm start_clear_msg, start_clear_len, 0xF, 0x000, 0x000 ;borra de de pantalla el page fault
    ; segunda escritura en on-demand
    ;mov dword [0x07000000 + 4], 0x300000

    ;pop eax    
    ;pop eax     ; pop solo para poder sacar despues el cr3 del kernel
    ;mov cr3, eax ;volvemos al contexto del kernel el cr3 
    ;--------------------------------------------------------------------------------------------------------

    
    ;Divide la pantalla por tareas (4)
    call tasks_screen_draw
    ; COMPLETAR (Parte 4: Tareas)- Cargar tarea inicial
    mov ax ,GDT_IDX_TASK_INITIAL_SEL ; SELECTOR DE TAREA INITIAL  --> GDT_IDX_TASK_INITIAL<<3
    ltr ax    
    ; COMPLETAR - Habilitar interrupciones (!! en etapas posteriores, evaluar si se debe comentar este código !!)
    
    ; NOTA: Pueden chequear que las interrupciones funcionen forzando a que se
    ;       dispare alguna excepción (lo más sencillo es usar la instrucción
    ;       `int3`)
    ;int3

    ; COMPLETAR - Probar Sys_call (para etapas posteriores, comentar este código)

    ; COMPLETAR - Probar generar una excepción (para etapas posteriores, comentar este código)
    
    ; ========================
    ; ||  (Parte 4: Tareas)  ||
    ; ========================
    
    ; COMPLETAR - Inicializar el directorio de paginas de la tarea de prueba

    ; COMPLETAR - Cargar directorio de paginas de la tarea

    ; COMPLETAR - Restaurar directorio de paginas del kernel

    ; COMPLETAR - Saltar a la primera tarea: Idle
    jmp GDT_IDX_TASK_IDLE_SEL:0 ; SELECTOR DE TAREA IDLE     --> GDT_IDX_TASK_IDLE<<3
    ;jmp far [TASK_IDLE_OFFSET]
    ; Ciclar infinitamente 
    mov eax, 0xFFFF
    mov ebx, 0xFFFF
    mov ecx, 0xFFFF
    mov edx, 0xFFFF
    jmp $

;; -------------------------------------------------------------------------- ;;

%include "a20.asm"
