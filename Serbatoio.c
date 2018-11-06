#include "stm32_unict_lib.h"
#include<stdio.h>

enum{
	V2_ON,
	V2_OFF,
	Y
};

int V2_MAX = 430, V2_MIN = 150, tank=0, V1=0, V3=0, state=V2_ON;

int main(){
	//Display
	char screen[5];
	DISPLAY_init();

	//ADC
	ADC_init(ADC1, ADC_RES_8, ADC_ALIGN_RIGHT);
	ADC_channel_config(ADC1, GPIOC, 0, 10);
	ADC_channel_config(ADC1, GPIOC, 1, 11);
	ADC_on(ADC1);

	//Timer
	TIM_init(TIM2);
	TIM_config_timebase(TIM2, 8400, 5000);
	TIM_set(TIM2,0);
	TIM_on(TIM2);

	//Buttons
	GPIO_init(GPIOB);
	GPIO_config_input(GPIOB,10);
	GPIO_config_input(GPIOB,4);
	GPIO_config_EXTI(GPIOB,EXTI10);
	GPIO_config_EXTI(GPIOB,EXTI4);
	EXTI_enable(EXTI10,FALLING_EDGE);
	EXTI_enable(EXTI4,FALLING_EDGE);

	//Led
	GPIO_config_output(GPIOB,0);

	for(;;){
		ADC_sample_channel(ADC1, 10);
		ADC_start(ADC1);
		while (!ADC_completed(ADC1)) {}
		int V1 = ADC_read(ADC1);
		V1=V1*(50.0)/255;

		ADC_sample_channel(ADC1, 11);
		ADC_start(ADC1);
		while (!ADC_completed(ADC1)) {}
		int V3 = ADC_read(ADC1);
		V3=V3*(10.0/255);

		if (TIM_update_check(TIM2)) {
			switch(state){
						case V2_ON:
							GPIO_write(GPIOB,0,1);
							tank+=V1;
							tank-=V3;
							if(tank<0)
								tank=0;
							if(tank>V2_MAX){
								state=V2_OFF;
							}
						break;
						case Y:
						case V2_OFF:
							GPIO_write(GPIOB,0,0);
							tank-=V3;
							if(tank<V2_MIN){
								if(state!=Y)
									state=V2_ON;
								if(tank<0)
									tank=0;
							}
						break;
					}
			TIM_update_clear(TIM2);
		}
		sprintf(screen, "%4d", tank);
		DISPLAY_puts(0,screen);
	}
	return 0;
}

void EXTI4_IRQHandler(void){
	if(EXTI_isset(EXTI4)) {
		state=Y;
		EXTI_clear(EXTI4);
	}
}

void EXTI15_10_IRQHandler(void){
	if(EXTI_isset(EXTI10)){
		if(tank<V2_MAX)
			state=V2_ON;
		EXTI_clear(EXTI10);
	}
}
