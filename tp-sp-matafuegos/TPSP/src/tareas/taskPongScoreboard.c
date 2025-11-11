#include "task_lib.h"

#define WIDTH TASK_VIEWPORT_WIDTH
#define HEIGHT TASK_VIEWPORT_HEIGHT

#define SHARED_SCORE_BASE_VADDR (PAGE_ON_DEMAND_BASE_VADDR + 0xF00)
#define CANT_PONGS 3


void task(void) {
	screen pantalla;
	// ¿Una tarea debe terminar en nuestro sistema? ->No
	// Pintamos todo de negro
	task_draw_box(pantalla, 0, 0, WIDTH, HEIGHT, ' ', C_BG_BLACK);
	uint8_t task_id = ENVIRONMENT->task_id;
	task_print(pantalla, "PongScoreboard",WIDTH / 2 -5,0, C_FG_LIGHT_GREY);
	uint32_t* ptrPuntaje = (uint32_t*) SHARED_SCORE_BASE_VADDR;
	while (true) {
		for (int i = 0;i < 4 ; i++){	
			if(i != task_id){
				task_print_dec(pantalla, ptrPuntaje[2*i], 2, WIDTH / 2 - 3, i +2, C_FG_CYAN);		// Pintamos el puntaje de un jugador
				task_print_dec(pantalla, ptrPuntaje[2*i + 1], 2, WIDTH / 2 + 3, i +2, C_FG_MAGENTA);		// Pintamos el puntaje de un jugador
				task_print_dec(pantalla,i,2,WIDTH/2,i+2,C_FG_LIGHT_GREY);
				syscall_draw(pantalla);
			}
		}
	}
}
 