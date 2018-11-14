/*
 Author: Marcello Maugeri
 Date: 06/11/2018

 Other codes: "https://github.com/Maray97/"
*/
#include "stm32_unict_lib.h"
#include<stdio.h>
#define MAX 50

void getString(char* string){
	int i=0;
	__io_putchar('>');
	__io_putchar(32);
	while(1){
		if(kbhit()){
			char c = readchar();
			if(c==13){ //Enter
				__io_putchar('\n');
				DISPLAY_puts(0,"    ");
				return;
			} else if(c==8){ //Backspace
				if(i!=0){
					__io_putchar(8);
					__io_putchar(32);
					__io_putchar(8);
					string[i-1]='\0';
					i--;
				}
			} else if(i!=MAX-1) { //Check full buffer
					__io_putchar(c);
					string[i]=c;
					string[i+1]='\0';
					i++;
			}
		}
		(i-4)<0 ? DISPLAY_putc(0,' ') : DISPLAY_putc(0,string[i-4]);
		(i-3)<0 ? DISPLAY_putc(0,' ') : DISPLAY_putc(1,string[i-3]);
		(i-2)<0 ? DISPLAY_putc(0,' ') : DISPLAY_putc(2,string[i-2]);
		(i-1)<0 ? DISPLAY_putc(0,' ') : DISPLAY_putc(3,string[i-1]);
	}
}

int main(){
	CONSOLE_init();
	DISPLAY_init();
	char string[MAX];
	RCC->AHB1ENR |= (1 << 1);
	GPIO_config_alternate(GPIOB, 0, 2);
	TIM_init(TIM3);
	TIM_config_timebase(TIM3, 60000, 858);
	TIM_config_PWM(TIM3, 3); // TIMER3, CH3
	TIM3->CCR3 = 0;
	TIM_set(TIM3, 0);
	TIM_on(TIM3);

	for (;;) {
		getString(string);
		if(string[0] > 48 && string[0] < 58 && string[1]=='\0'){
				if(string[0]-48 >0 && string[0]-48 <10){
					TIM3->CCR3 = (string[0]-48)*100;
					TIM_set(TIM3, 0);
					TIM_on(TIM3);
				}
		}
		printf("%s\n",string);
	}
}
