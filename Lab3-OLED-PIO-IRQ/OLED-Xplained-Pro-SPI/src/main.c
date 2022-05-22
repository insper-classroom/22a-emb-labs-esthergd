#include <asf.h>
#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"


// LEDS

// LED 0
#define LED0_PIO           PIOC
#define LED0_PIO_ID        ID_PIOC
#define LED0_PIO_IDX       8
#define LED0_PIO_IDX_MASK  (1 << LED0_PIO_IDX)

// LED 1
#define LED1_PIO           PIOA
#define LED1_PIO_ID        ID_PIOA
#define LED1_PIO_IDX       0
#define LED1_PIO_IDX_MASK  (1 << LED1_PIO_IDX)

// LED 2
#define LED2_PIO           PIOC
#define LED2_PIO_ID        ID_PIOC
#define LED2_PIO_IDX       30
#define LED2_PIO_IDX_MASK  (1 << LED2_PIO_IDX)

// LED 3
#define LED3_PIO           PIOB
#define LED3_PIO_ID        ID_PIOB
#define LED3_PIO_IDX       2
#define LED3_PIO_IDX_MASK  (1 << LED3_PIO_IDX)

// BUTTONS

// BUTTON 0
#define BUT0_PIO      PIOA
#define BUT0_PIO_ID   ID_PIOA
#define BUT0_IDX      11
#define BUT0_IDX_MASK (1 << BUT0_IDX)

// BUTTON 1
#define BUT1_PIO      PIOD
#define BUT1_PIO_ID   ID_PIOD
#define BUT1_IDX      28
#define BUT1_IDX_MASK (1 << BUT1_IDX)

// BUTTON 2
#define BUT2_PIO      PIOC
#define BUT2_PIO_ID   ID_PIOC
#define BUT2_IDX      31
#define BUT2_IDX_MASK (1 << BUT2_IDX)

// BUTTON 3
#define BUT3_PIO      PIOA
#define BUT3_PIO_ID   ID_PIOA
#define BUT3_IDX      19
#define BUT3_IDX_MASK (1 << BUT3_IDX)

/* Prototype */
void io_init(void);
void pisca_led(Pio*, const uint32_t, int n, int t);

volatile char change_freq;
volatile char start_flag;
volatile char increase_flag;


void but_callback(void){
	if(pio_get(BUT1_PIO, PIO_INPUT, BUT1_IDX_MASK)){
		change_freq = 0;
	}
	else{
		change_freq = 1;
	}
}

void change_oled(int seg) {
	char seg_str[128];
	gfx_mono_draw_string("          ", 5, 5, &sysfont);
	sprintf(seg_str, "%d s", seg);
	gfx_mono_draw_string(seg_str, 5, 5, &sysfont);
}


int muda_freq(int seg){
	if(increase_flag){
		seg += 5;
		change_oled(seg);
		increase_flag = 0;
		return seg;
	}
	for(double i =0; i < 2000000; i++){
		if(!change_freq){
			seg += 100;
			change_oled(seg);
			return seg;
		}
	}
	change_freq = 0;
	seg -= 100;
	change_oled(seg);
	return seg;
}

void start_stop_callback(void) {
	start_flag = 1;
}

void increase_freq_callback(void) {
	increase_flag = 1;
}


void pisca_led(Pio *p_pio, const uint32_t mask, int n, int t){
	int contador = 0;
	gfx_mono_generic_draw_horizontal_line(90, 20, 30, GFX_PIXEL_SET);
	for (int i=0; i<n; i++) {
		if(start_flag) {
			pio_set(p_pio, mask);
			start_flag = 0;
			break;
		}
		gfx_mono_generic_draw_vertical_line(90 + contador, 10, 10, GFX_PIXEL_SET);
		pio_clear(p_pio, mask);
		delay_ms(t/2);
		pio_set(p_pio, mask);
		delay_ms(t/2);
		contador++;
	}

	gfx_mono_generic_draw_filled_rect(90, 10, 30, 11, GFX_PIXEL_CLR);
}

void io_init(void)
{
	// ativa clock
	sysclk_init();
	
	//desativa clock
	board_init();
	
	// Configura led
	pmc_enable_periph_clk(LED0_PIO_ID);
	pmc_enable_periph_clk(LED1_PIO_ID);
	pmc_enable_periph_clk(LED2_PIO_ID);
	pmc_enable_periph_clk(LED3_PIO_ID);
	
	
	pio_configure(LED0_PIO, PIO_OUTPUT_0, LED0_PIO_IDX_MASK, PIO_DEFAULT);
	pio_configure(LED1_PIO, PIO_OUTPUT_0, LED1_PIO_IDX_MASK, PIO_DEFAULT);
	pio_configure(LED2_PIO, PIO_OUTPUT_0, LED2_PIO_IDX_MASK, PIO_DEFAULT);
	pio_configure(LED3_PIO, PIO_OUTPUT_0, LED3_PIO_IDX_MASK, PIO_DEFAULT);
	
	
	// inicializa o Botao
	pmc_enable_periph_clk(BUT0_PIO_ID);
	pmc_enable_periph_clk(BUT1_PIO_ID);
	pmc_enable_periph_clk(BUT2_PIO_ID);
	pmc_enable_periph_clk(BUT3_PIO_ID);

	// Configura PIO para lidar com o pino do botão como entrada com pull-up
	pio_configure(BUT0_PIO, PIO_INPUT, BUT0_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT0_PIO, BUT0_IDX_MASK, 60);
	pio_configure(BUT1_PIO, PIO_INPUT, BUT1_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT1_PIO, BUT1_IDX_MASK, 60);
	pio_configure(BUT2_PIO, PIO_INPUT, BUT2_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT2_PIO, BUT2_IDX_MASK, 60);
	pio_configure(BUT3_PIO, PIO_INPUT, BUT3_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT3_PIO, BUT3_IDX_MASK, 60);

	// Configura interrupção no pino referente ao botao e associa
	// função de callback caso uma interrupção for gerada
	// a função de callback é a: but_callback()
	pio_handler_set(BUT0_PIO, BUT0_PIO_ID, BUT0_IDX_MASK, PIO_IT_EDGE, but_callback);
	pio_handler_set(BUT1_PIO, BUT1_PIO_ID, BUT1_IDX_MASK, PIO_IT_EDGE, but_callback);
	pio_handler_set(BUT2_PIO, BUT2_PIO_ID, BUT2_IDX_MASK, PIO_IT_FALL_EDGE, start_stop_callback);
	pio_handler_set(BUT3_PIO, BUT3_PIO_ID, BUT3_IDX_MASK, PIO_IT_FALL_EDGE, increase_freq_callback);

	// Ativa interrupção e limpa primeira IRQ gerada na ativacao
	pio_enable_interrupt(BUT0_PIO, BUT0_IDX_MASK);
	pio_get_interrupt_status(BUT0_PIO);
	pio_enable_interrupt(BUT1_PIO, BUT1_IDX_MASK);
	pio_get_interrupt_status(BUT1_PIO);
	pio_enable_interrupt(BUT2_PIO, BUT2_IDX_MASK);
	pio_get_interrupt_status(BUT2_PIO);
	pio_enable_interrupt(BUT3_PIO, BUT3_IDX_MASK);
	pio_get_interrupt_status(BUT3_PIO);
	
	// Configura NVIC para receber interrupcoes do PIO do botao
	// com prioridade 4 (quanto mais próximo de 0 maior)
	NVIC_EnableIRQ(BUT0_PIO_ID);
	NVIC_SetPriority(BUT0_PIO_ID, 4);
	NVIC_EnableIRQ(BUT1_PIO_ID);
	NVIC_SetPriority(BUT1_PIO_ID, 4);
	NVIC_EnableIRQ(BUT2_PIO_ID);
	NVIC_SetPriority(BUT2_PIO_ID, 5);
	NVIC_EnableIRQ(BUT3_PIO_ID);
	NVIC_SetPriority(BUT3_PIO_ID, 4);
}

int main (void)
{
	io_init();
	
	int seg = 0;

	// Init OLED
	gfx_mono_ssd1306_init();
	
	pio_set(LED2_PIO, LED2_PIO_IDX_MASK);
	
	char str_seg[128];
	sprintf(str_seg, seg);
	gfx_mono_draw_string("            ", 5, 5, &sysfont);
	gfx_mono_draw_string(str_seg, 5, 5, &sysfont);

	/* Insert application code here, after the board has been initialized. */
	while(1) {
		if(change_freq || start_flag || increase_flag) {
			if(change_freq) {
				seg = muda_freq(seg);
			}
			else if(increase_flag) {
				seg = muda_freq(seg);
			}
			else if(start_flag) {
				start_flag = 0;
				pisca_led(LED2_PIO, LED2_PIO_IDX_MASK, 30, seg);
			}

			change_freq = 0;
			start_flag = 0;
			increase_flag = 0;
		}
		pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
	}
}