#include "stm32_unict_lib.h"
#include<stdio.h>

enum{
	IDLE,
	OPENING,
	WAITING,
	CLOSING
};

int state, closeT=20, waitT=8, gate=20, timerWait;

int main(){
	GPIO_init(GPIOB);
	GPIO_init(GPIOC);
	DISPLAY_init();
	GPIO_config_input(GPIOB, 4);
	GPIO_config_input(GPIOB, 5);
	GPIO_config_input(GPIOB, 10);
	GPIO_config_output(GPIOB, 0);
	GPIO_config_output(GPIOC, 2);
	GPIO_config_output(GPIOC, 3);
	GPIO_config_EXTI(GPIOB, EXTI4);
	GPIO_config_EXTI(GPIOB, EXTI5);
	GPIO_config_EXTI(GPIOB, EXTI10);
	EXTI_enable(EXTI4, FALLING_EDGE);
	EXTI_enable(EXTI5, FALLING_EDGE);
	EXTI_enable(EXTI10, FALLING_EDGE);
	TIM_init(TIM2);
	TIM_config_timebase(TIM2, 8400, 5000);
	TIM_enable_irq(TIM2, IRQ_UPDATE);
	for(;;){
		if(gate>19) DISPLAY_puts(0,"----");
		else if(gate>13) DISPLAY_puts(0,"--- ");
		else if(gate>7) DISPLAY_puts(0,"--  ");
		else if(gate>0) DISPLAY_puts(0,"-   ");
		else DISPLAY_puts(0,"    ");
	}
};

void TIM2_IRQHandler(void){
	if(TIM_update_check(TIM2)){
		switch(state){
		case OPENING:
				GPIO_toggle(GPIOB, 0);
				gate--;
				if(gate==0){
					GPIO_write(GPIOB, 0, 0);
					timerWait=waitT;
					state=WAITING;
				}
			break;
		case WAITING:
				timerWait--;
				GPIO_toggle(GPIOC, 2);
				if(timerWait==0){
					GPIO_write(GPIOC, 2, 0);
					state=CLOSING;
				}
			break;
		case CLOSING:
				gate++;
				GPIO_toggle(GPIOC, 3);
				if(gate==20){
					GPIO_write(GPIOC, 3, 0);
					state=IDLE;
				}
			break;
		}
		TIM_update_clear(TIM2);
	}
}

// Tasto X
void EXTI15_10_IRQHandler(void){
	if(EXTI_isset(EXTI10)){
		if(state==IDLE || state==CLOSING){
			TIM_set(TIM2,0);
			TIM_on(TIM2);
			state=OPENING;
		}
		EXTI_clear(EXTI10);
	}
}

// Tasto Y
void EXTI4_IRQHandler(void){
	if(EXTI_isset(EXTI4)){
		if(state==WAITING){
			GPIO_write(GPIOC, 2, 0);
			state=CLOSING;
		}
		EXTI_clear(EXTI4);
	}
}

// Tasto Z
void EXTI9_5_IRQHandler(void){
	if(EXTI_isset(EXTI5)){
		if(state==WAITING)	timerWait=waitT;
		if(state==CLOSING){
			GPIO_write(GPIOC, 3, 0);
			state=OPENING;
		}
		EXTI_clear(EXTI5);
	}
}
