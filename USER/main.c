#include "stdio.h"
#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"

int main(void)
 {
	delay_init();		
	NVIC_Configuration();	// set the interrupt priority group
	LED_Init();		  		
	
	uart_init(9600);
	usart1_send("***---System init success---***\r\n");

	while(1) {
		LED0 = 1;
		LED1 = 0;
		delay_ms(1000);
		LED0 = 0;
		LED1 = 1;
		delay_ms(1000);			
	}	//while(1);
}		//main();


