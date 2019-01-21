/*
 Author: Marcello Maugeri
 Date: 21/01/2018
 Text: "http://www.dmi.unict.it/~santoro/teaching/lsm/slides/Raccolta-Compiti.pdf"
 Other codes: "https://github.com/Maray97/"
*/
#include "stm32_unict_lib.h"
#include<stdio.h>

enum{
	xti,
	xSt,
	xEt,
	zti,
	zSt,
	zEt,
	tti,
	tSt,
	tEt,
	idle,
	running
};

//Current time
int minutes=0, seconds=0;
//Start time
int startMinutes=60, startSeconds=60;
//End time
int endMinutes=60, endSeconds=60;
//Variables to set time
int newMinutes=0, newSeconds=0;
int state=idle;
char s[5];

int main(){
	//Display initialization
	DISPLAY_init();
	//UART initialization
	CONSOLE_init();
	//GPIOB initialization
	GPIO_init(GPIOB);
	//GPIOC initialization
	GPIO_init(GPIOC);
	//Red led config output -PB0
	GPIO_config_output(GPIOB, 0);
	//Yellow led config output -PC2
	GPIO_config_output(GPIOC, 2);
	//X key config input and enable interrupt -PB10
	GPIO_config_input(GPIOB,10);
	GPIO_config_EXTI(GPIOB,EXTI10);
	EXTI_enable(EXTI10, FALLING_EDGE);
	//Y key config input and enable interrupt -PB4
	GPIO_config_input(GPIOB,4);
	GPIO_config_EXTI(GPIOB, EXTI4);
	EXTI_enable(EXTI4, FALLING_EDGE);
	//Z key config input and enable interrupt -PB5
	GPIO_config_input(GPIOB,5);
	GPIO_config_EXTI(GPIOB, EXTI5);
	EXTI_enable(EXTI5, FALLING_EDGE);
	//Timer initialization
	TIM_init(TIM2);
	TIM_config_timebase(TIM2, 8400, 10000); // PSC 8400 ARR 20 -> 84.000.000/8.400=10.000/10.000=1 Hz->1 s
	TIM_enable_irq(TIM2, IRQ_UPDATE);
	TIM_set(TIM2,0);
	//Timer start
	TIM_on(TIM2);
	//ADC initialization - PC0 down - PC1 up
	ADC_init(ADC1, ADC_RES_8, ADC_ALIGN_RIGHT);
	ADC_channel_config(ADC1, GPIOC, 0, 10);
	ADC_channel_config(ADC1, GPIOC, 1, 11);
	ADC_on(ADC1);
	//Infinite loop
	while(1){
		switch(state){
				//I primi tre case servono a mostrare la configurazione attuale prima di usare gli ADC
				case tti:
					sprintf(s, "%2d%2d", minutes, seconds);
					DISPLAY_puts(0,s);
					delay_ms(2000);
					state=zti;
				break;
				case tSt:
					sprintf(s, "%2d%2d", startMinutes, startSeconds);
					DISPLAY_puts(0,s);
					delay_ms(2000);
					state=zSt;
				break;
				case tEt:
					sprintf(s, "%2d%2d", endMinutes, endSeconds);
					DISPLAY_puts(0,s);
					delay_ms(2000);
					state=zEt;
				break;
				//Stato base, visualizzazione
				case idle:
					sprintf(s, "%2d%2d", minutes, seconds);
					DISPLAY_puts(0,s);
				break;
				//Stato per l'irrigazione
				case running:
					sprintf(s, "%2d%2d", minutes, seconds);
					//Irrigation
					DISPLAY_puts(0,s);
				break;
				//Set menu
				case xti:
					sprintf(s, "ti  ");
					DISPLAY_puts(0,s);
				break;
				case xSt:
					sprintf(s, "St  ");
					DISPLAY_puts(0,s);
				break;
				case xEt:
					sprintf(s, "Et  ");
					DISPLAY_puts(0,s);
				break;
				//Config mode
				case zti:
				case zSt:
				case zEt:
					ADC_sample_channel(ADC1, 10);
					ADC_start(ADC1);
					while (!ADC_completed(ADC1)) {}
					newMinutes = ADC_read(ADC1);
					newMinutes=newMinutes*(59)/255;
					ADC_sample_channel(ADC1, 11);
					ADC_start(ADC1);
					while (!ADC_completed(ADC1)) {}
					newSeconds = ADC_read(ADC1);
					newSeconds=newSeconds*(59)/255;
					sprintf(s, "%2d%2d", newMinutes, newSeconds);
					DISPLAY_puts(0,s);
				break;
		};
	}
}

//X (PB10) key interrupt handler
void EXTI15_10_IRQHandler(void){
	if(EXTI_isset(EXTI10)){
		switch(state){
		case idle:
			state=xti;
			GPIO_write(GPIOB, 0, 1);
		break;
		case xti:
			state=xSt;
		break;
		case xSt:
			state=xEt;
		break;
		case xEt:
			state=xti;
		break;
		case zti:
			//Imposto l'orario attuale
			seconds=newSeconds;
			minutes=newMinutes;
			//Spengo il led rosso
			GPIO_write(GPIOB, 0, 0);
			state=idle;
		break;
		case zSt:
			//Controllo se il tempo di inizio è stato impostato dopo di quello di fine
			if((newMinutes*60+newSeconds)<(endMinutes*60+endSeconds) || (startSeconds*60+startSeconds)==60*60+60){
				startSeconds=newSeconds;
				startMinutes=newMinutes;
			}
			//Spengo il led rosso
			GPIO_write(GPIOB, 0, 0);
			state=idle;
		break;
		case zEt:
			//Controllo se il tempo di fine è stato impostato prima di quello di inizio
			if((newMinutes*60+newSeconds)>(startMinutes*60+startSeconds) || (endMinutes*60+endSeconds)==60*60+60){
				endSeconds=newSeconds;
				endMinutes=newMinutes;
			}
			//Spengo il led rosso
			GPIO_write(GPIOB, 0, 0);
			state=idle;
		break;
		};
		EXTI_clear(EXTI10);
	}
}

//Y (PB4) key interrupt handler
void EXTI4_IRQHandler(void){
	if(EXTI_isset(EXTI4)) {
		switch(state){
		//Il tasto Y ha la funzione di annullare tutto nei seguenti casi
		case xti:
		case xSt:
		case xEt:
			state=idle;
			GPIO_write(GPIOB, 0, 0);
		break;
		//Il tasto Y ritorna al menù nei seguenti casi
		case zti:
			state=xti;
		break;
		case zSt:
			state=xSt;
		break;
		case zEt:
			state=xEt;
		break;
		};
		EXTI_clear(EXTI4);
	}
}

//Z (PB5) and T (PB6) keys interrupt handler
void EXTI9_5_IRQHandler(void){
	if(EXTI_isset(EXTI5)) {
		//Cambi di stato dovuti alla pressione di Z
		switch(state){
		case xti:
			state=tti;
			break;
		case xSt:
			state=tSt;
		break;
		case xEt:
			state=tEt;
			break;
		};
		EXTI_clear(EXTI5);
	}
}

//Timer 2 interrupt handler
void TIM2_IRQHandler(void){
	if (TIM_update_check(TIM2)) {
		//Incremento dei secondi
		seconds++;
		//Incremento dei minuti
		if(seconds==60){
			seconds=0;
			minutes++;
		}
		//Reset dell'ora
		if(minutes==60){
			seconds=0;
			minutes=0;
		}
		//Controllo se l'irrigazione dev'essere accesa
		if((minutes*60+seconds)>=(startMinutes*60+startSeconds) && (minutes*60+seconds)<=(endMinutes*60+endSeconds)){
			printf("Irrigazione attiva\n");
			state=running;
			GPIO_write(GPIOC, 2, 1);
		} else if(state==running){
			//Spengo l'irrigazione
			state=idle;
			printf("Spegnimento\n");
			GPIO_write(GPIOC, 2, 0);
		}

		TIM_update_clear(TIM2);
	}
}
