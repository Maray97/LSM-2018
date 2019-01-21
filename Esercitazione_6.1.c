/*
 Author: Marcello Maugeri
 Date: 21/01/2018

 Other codes: "https://github.com/Maray97/"
*/
#include "stm32_unict_lib.h"
#include<stdio.h>

int numeroServito=0;
int numeroRilascio=0;
int tempoMedio=0;
int timerCount=0;
char s[5];
int rilascio=0;
int secondsCount=0;

int main(){
	//Display initialization
	DISPLAY_init();
	//UART initialization
	CONSOLE_init();
	//GPIOB initialization
	GPIO_init(GPIOB);
	//Red led config output -PB0
	GPIO_config_output(GPIOB, 0);
	//X key config input and enable interrupt -PB10
	GPIO_config_input(GPIOB,10);
	GPIO_config_EXTI(GPIOB,EXTI10);
	EXTI_enable(EXTI10, FALLING_EDGE);
	//Y key config input and enable interrupt -PB4
	GPIO_config_input(GPIOB,4);
	GPIO_config_EXTI(GPIOB, EXTI4);
	EXTI_enable(EXTI4, FALLING_EDGE);
	//Timer initialization
	TIM_init(TIM2);
	TIM_config_timebase(TIM2, 8400, 5000); // PSC 8400 ARR 20 -> 84.000.000/8.400=10.000/5000=2 Hz=500 ms
	TIM_enable_irq(TIM2, IRQ_UPDATE);
	TIM_set(TIM2,0);
	//Timer start
	TIM_on(TIM2);
	//Infinite loop
	printf("Execution start\n");
	while(1){
	}
}

//X (PB10) key interrupt handler
void EXTI15_10_IRQHandler(void){
	if(EXTI_isset(EXTI10)){
		numeroRilascio++;
		printf("Rilasciato il numero %d\n",numeroRilascio);
		timerCount=0;
		rilascio=1;
		EXTI_clear(EXTI10);
	}
}

//Y (PB4) key interrupt handler
void EXTI4_IRQHandler(void){
	if(EXTI_isset(EXTI4)) {
		if(numeroServito<numeroRilascio){
			numeroServito++;
			printf("Serviamo il numero %d\n",numeroServito);
			tempoMedio = (tempoMedio + (secondsCount/2)) / 2;
			secondsCount=0;
		}
		EXTI_clear(EXTI4);
	}
}

//Timer 2 interrupt handler
void TIM2_IRQHandler(void){
	if (TIM_update_check(TIM2)) {
		if(numeroServito<numeroRilascio)
			secondsCount++;
		if(!rilascio){
			sprintf(s, "%2d%2d", (numeroServito%100), tempoMedio);
			DISPLAY_puts(0,s);
		} else {
			GPIO_toggle(GPIOB, 0);
			sprintf(s, "%4d", numeroRilascio);
			DISPLAY_puts(0,s);
			timerCount++;
			if(timerCount>4)
				rilascio=0;
		}
		TIM_update_clear(TIM2);
	}
}
