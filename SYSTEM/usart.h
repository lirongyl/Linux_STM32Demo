#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 
#include "delay.h"
#include "string.h"
#include "stdlib.h"
#define USART_REC_LEN  			200  	
#define EN_USART1_RX 			1		
	  	
extern u8  USART_RX_BUF[USART_REC_LEN]; //
extern u16 USART_RX_STA;         		//

void uart_init(u32 bound);
void usart1_send_char(u8 temp);
void usart1_send_buff(char *buff, u16 len);
void usart1_send(char *buff);
void message_rec(void);
#endif


