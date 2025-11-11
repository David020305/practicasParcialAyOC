/* ** por compatibilidad se omiten tildes **
================================================================================
 TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================

  Definicion de funciones del manejador de memoria
*/

#include "mmu.h"
#include "i386.h"

#include "kassert.h"

static pd_entry_t* kpd = (pd_entry_t*)KERNEL_PAGE_DIR;
static pt_entry_t* kpt = (pt_entry_t*)KERNEL_PAGE_TABLE_0;

static const uint32_t identity_mapping_end = 0x003FFFFF;
static const uint32_t user_memory_pool_end = 0x02FFFFFF;

static paddr_t next_free_kernel_page = 0x100000;
static paddr_t next_free_user_page = 0x400000;

/**
 * kmemset asigna el valor c a un rango de memoria interpretado
 * como un rango de bytes de largo n que comienza en s
 * @param s es el puntero al comienzo del rango de memoria
 * @param c es el valor a asignar en cada byte de s[0..n-1]
 * @param n es el tamaño en bytes a asignar
 * @return devuelve el puntero al rango modificado (alias de s)
*/
static inline void* kmemset(void* s, int c, size_t n) {
  uint8_t* dst = (uint8_t*)s;
  for (size_t i = 0; i < n; i++) {
    dst[i] = c;
  }
  return dst;
}

/**
 * zero_page limpia el contenido de una página que comienza en addr
 * @param addr es la dirección del comienzo de la página a limpiar
*/
static inline void zero_page(paddr_t addr) {
  kmemset((void*)addr, 0x00, PAGE_SIZE);
}


void mmu_init(void) {}


/**
 * mmu_next_free_kernel_page devuelve la dirección física de la próxima página de kernel disponible. 
 * Las páginas se obtienen en forma incremental, siendo la primera: next_free_kernel_page
 * @return devuelve la dirección de memoria de comienzo de la próxima página libre de kernel
 */
paddr_t mmu_next_free_kernel_page(void) {
  paddr_t page = next_free_kernel_page;
  next_free_kernel_page = next_free_kernel_page + PAGE_SIZE;
  return page;
}

/**
 * mmu_next_free_user_page devuelve la dirección de la próxima página de usuarix disponible
 * @return devuelve la dirección de memoria de comienzo de la próxima página libre de usuarix
 */
paddr_t mmu_next_free_user_page(void) {
  paddr_t page = next_free_user_page;
  next_free_user_page = next_free_user_page + PAGE_SIZE; // Pato antes en vez de PAGE_SIZE estaba 0xFFF
  return page;
}

/**
 * mmu_init_kernel_dir inicializa las estructuras de paginación vinculadas al kernel y
 * realiza el identity mapping
 * @return devuelve la dirección de memoria de la página donde se encuentra el directorio
 * de páginas usado por el kernel
 */
paddr_t mmu_init_kernel_dir(void) {
  // Page directory 
  zero_page((paddr_t)kpd); // Patozo
  kpd->attrs = MMU_P | MMU_W ;
  kpd->pt = (uint32_t) kpt >> 12; //Pato 
  // Page Table 
  // Primero hago el identity mapping
  uint32_t cr3 = (uint32_t) kpd; // calculo cr3 a partir del KERNEL_PAGE_DIR y poniendo los atributos en 0
  for (size_t virtual_address = 0; virtual_address < identity_mapping_end;virtual_address += PAGE_SIZE) {
    mmu_map_page(cr3, virtual_address, virtual_address, MMU_P | MMU_W);
  }
  return (paddr_t) cr3;
}

/**
 * mmu_map_page agrega las entradas necesarias a las estructuras de paginación de modo de que
 * la dirección virtual virt se traduzca en la dirección física phy con los atributos definidos en attrs
 * @param cr3 el contenido que se ha de cargar en un registro CR3 al realizar la traducción
 * @param virt la dirección virtual que se ha de traducir en phy
 * @param phy la dirección física que debe ser accedida (dirección de destino)
 * @param attrs los atributos a asignar en la entrada de la tabla de páginas
 */
void mmu_map_page(uint32_t cr3, vaddr_t virt, paddr_t phy, uint32_t attrs) {

  pd_entry_t* page_directory = (pd_entry_t*) CR3_TO_PAGE_DIR(cr3);
  uint32_t directory_entry_index = VIRT_PAGE_DIR(virt);
  uint32_t table_entry_index = VIRT_PAGE_TABLE(virt);
  if ((page_directory[directory_entry_index].attrs & MMU_P) == 0){  // no esta definido  la traduccion del bloque de 4mb
    paddr_t nueva_page_table; 
    if ((page_directory[directory_entry_index].attrs & MMU_U) == 0){
      nueva_page_table = mmu_next_free_kernel_page();
    }
    else {
      nueva_page_table = mmu_next_free_user_page();
    }
    zero_page(nueva_page_table);
    page_directory[directory_entry_index].pt = nueva_page_table >> 12;
    page_directory[directory_entry_index].attrs = MMU_P | MMU_W |MMU_U;
  }

  pt_entry_t* page_table = (pt_entry_t*) MMU_ENTRY_PADDR(page_directory[directory_entry_index].pt);
  //pt_entry_t page_table_entry = page_table[table_entry_index];
  page_table[table_entry_index].page = phy >> 12;
  page_table[table_entry_index].attrs = attrs;
  tlbflush();
}

/**
 * mmu_unmap_page elimina la entrada vinculada a la dirección virt en la tabla de páginas correspondiente
 * @param virt la dirección virtual que se ha de desvincular
 * @return la dirección física de la página desvinculada
 */
paddr_t mmu_unmap_page(uint32_t cr3, vaddr_t virt) {    // suponemos que esta traducido el bloque de 4mb correspondiente
  pd_entry_t* page_directory = (pd_entry_t*) CR3_TO_PAGE_DIR(cr3);
  uint32_t directory_entry_index = VIRT_PAGE_DIR(virt);
  uint32_t table_entry_index = VIRT_PAGE_TABLE(virt); 
  pt_entry_t* page_table = (pt_entry_t*) MMU_ENTRY_PADDR(page_directory[directory_entry_index].pt);
  page_table[table_entry_index].attrs = 0;
  tlbflush();
  return page_table[table_entry_index].page;
}

#define DST_VIRT_PAGE 0xA00000
#define SRC_VIRT_PAGE 0xB00000

/**
 * copy_page copia el contenido de la página física localizada en la dirección src_addr a la página física ubicada en dst_addr
 * @param dst_addr la dirección a cuya página queremos copiar el contenido
 * @param src_addr la dirección de la página cuyo contenido queremos copiar
 *
 * Esta función mapea ambas páginas a las direcciones SRC_VIRT_PAGE y DST_VIRT_PAGE, respectivamente, realiza
 * la copia y luego desmapea las páginas. Usar la función rcr3 definida en i386.h para obtener el cr3 actual
 */
 
void copy_page(paddr_t dst_addr, paddr_t src_addr) {
  uint32_t cr3 = rcr3();
  pd_entry_t* page_directory = (pd_entry_t*) cr3;
  mmu_map_page(cr3,SRC_VIRT_PAGE,src_addr,MMU_P);
  mmu_map_page(cr3,DST_VIRT_PAGE,dst_addr,MMU_P | MMU_W);

  uint8_t* posicionActual = (uint8_t*) SRC_VIRT_PAGE;
  uint8_t* destinoActual = (uint8_t*) DST_VIRT_PAGE;
  
  for(int i = 0;i< PAGE_SIZE;i++){
    destinoActual[i] = posicionActual[i];
  }  
  for(int i = 0;i< PAGE_SIZE;i++){
    if(destinoActual[i] != posicionActual[i])     // """"""test""""
      return;
  }
  tlbflush();
  mmu_unmap_page(cr3,SRC_VIRT_PAGE);
  mmu_unmap_page(cr3,DST_VIRT_PAGE);
}

 /**
 * mmu_init_task_dir inicializa las estructuras de paginación vinculadas a una tarea cuyo código se encuentra en la dirección phy_start
 * @pararm phy_start es la dirección donde comienzan las dos páginas de código de la tarea asociada a esta llamada
 * @return el contenido que se ha de cargar en un registro CR3 para la tarea asociada a esta llamada
 */
paddr_t mmu_init_task_dir(paddr_t phy_start) {
  //inicializar page directory
  paddr_t direccion_fisica_del_page_directory = mmu_next_free_kernel_page(); 
  zero_page(direccion_fisica_del_page_directory);
  // es necesario hacer el struct de page directory table, //copiamos la mmu del kernel, que seria la primera pos del page directory del kernel 
  pd_entry_t* page_directory = (pd_entry_t*) direccion_fisica_del_page_directory;  
  page_directory[0] = kpd[0]; 
  
  
  //mapeamos las dos paginas de codigo
  mmu_map_page(direccion_fisica_del_page_directory, TASK_CODE_VIRTUAL, phy_start, MMU_P|MMU_U); //Pato no sabemos las flags
  mmu_map_page(direccion_fisica_del_page_directory, TASK_CODE_VIRTUAL+PAGE_SIZE, phy_start+PAGE_SIZE, MMU_P|MMU_U);

  // mapeo del stack
  paddr_t nueva_pagina_para_stack = mmu_next_free_user_page(); // Pato esta bien esto la pila crece a numeros mas chicos  
  //zero_page(nueva_pagina_para_stack); Pato porque rompe esto
  mmu_map_page(direccion_fisica_del_page_directory, TASK_STACK_BASE - PAGE_SIZE, nueva_pagina_para_stack, MMU_P|MMU_U|MMU_W); // hago el mapeo restando 4096(1page) a la direccion del tope de la pila porque la pila crece a numero mas chicos

  //mapeo de la pagina de memoria compartida
  mmu_map_page(direccion_fisica_del_page_directory, TASK_SHARED_PAGE, SHARED, MMU_P|MMU_U); // pato sobre los flags y sobre SHARED
  
  return direccion_fisica_del_page_directory;
}

// COMPLETAR: devuelve true si se atendió el page fault y puede continuar la ejecución 
// y false si no se pudo atender
bool page_fault_handler(vaddr_t virt) {  
  print("Atendiendo page fault...", 0, 0, C_FG_WHITE | C_BG_BLACK);
  // Chequeemos si el acceso fue dentro del area on-demand
  if (ON_DEMAND_MEM_START_VIRTUAL <= virt && virt <= ON_DEMAND_MEM_END_VIRTUAL){
    // En caso de que si, mapear la pagina
   mmu_map_page(rcr3(), virt, ON_DEMAND_MEM_START_PHYSICAL, MMU_P|MMU_U|MMU_W );
   return true;
  }
  print("No se atendio el page fault", 0, 0, C_FG_WHITE | C_BG_BLACK);//nuestro print para ver si se atendio el page 
  return false;
}
  
