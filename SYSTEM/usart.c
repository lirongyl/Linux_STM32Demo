#include "sys.h"
#include "usart.h"	  
#include "string.h"

u8 USART_RX_BUF[USART_REC_LEN];     //
u16 USART_RX_STA=0;       //

void uart_init(u32 bound){
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);	//
 	USART_DeInit(USART1);  //
	 //USART1_TX   PA.9
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; //PA.9
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//
    GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化PA9
   
    //USART1_RX	  PA.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//
    GPIO_Init(GPIOA, &GPIO_InitStructure);  //初始化PA10

   //Usart1 NVIC 配置

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
   //USART 初始化设置

	USART_InitStructure.USART_BaudRate = bound;//一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

    USART_Init(USART1, &USART_InitStructure); //初始化串口
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启中断
    USART_Cmd(USART1, ENABLE);                    //使能串口 
}
#if EN_USART1_RX   //如果使能了接收
void USART1_IRQHandler(void) {
	u8 Res;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
		Res =USART_ReceiveData(USART1);//(USART1->DR);
		if((USART_RX_STA&0x8000)==0) {
			if(USART_RX_STA&0x4000) {
				if(Res!=0x0a)USART_RX_STA=0;//接收错误,重新开始
				else USART_RX_STA|=0x8000;	//接收完成了 
				}
			else {	
				if(Res==0x0d)USART_RX_STA|=0x4000;
				else {
					USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;
				}		 
			}
		}   		 
     }  
} 
#endif	

void usart1_send_char(u8 temp)
{
	USART_SendData(USART1, (u8)temp);
	while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}

void usart1_send_buff(char *buff, u16 len)
{
	u8 i;
	for(i = 0; i < len; i++) {
		usart1_send_char(buff[i]);
	}
}

void usart1_send(char *buff)
{
	u8 i;
	u8 len = 0;

	len = strlen(buff);
	for(i = 0; i < len; i++) {
		usart1_send_char(buff[i]);
	}
}


void message_rec(void)
{
	u8 len, t;
	if(USART_RX_STA&0x8000) {					   
		len = USART_RX_STA&0x3fff;
		for(t = 0; t < len; t++)
		{
			USART1->DR=USART_RX_BUF[t];
			while((USART1->SR&0X40)==0);
		}
		usart1_send("\r\n");
		USART_RX_STA=0;
	}
}

