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
    GPIO_Init(GPIOA, &GPIO_InitStructure); //��ʼ��PA9
   
    //USART1_RX	  PA.10
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//
    GPIO_Init(GPIOA, &GPIO_InitStructure);  //��ʼ��PA10

   //Usart1 NVIC ����

    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
   //USART ��ʼ������

	USART_InitStructure.USART_BaudRate = bound;//һ������Ϊ9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

    USART_Init(USART1, &USART_InitStructure); //��ʼ������
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//�����ж�
    USART_Cmd(USART1, ENABLE);                    //ʹ�ܴ��� 
}
#if EN_USART1_RX   //���ʹ���˽���
void USART1_IRQHandler(void) {
	u8 Res;
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
		Res =USART_ReceiveData(USART1);//(USART1->DR);
		if((USART_RX_STA&0x8000)==0) {
			if(USART_RX_STA&0x4000) {
				if(Res!=0x0a)USART_RX_STA=0;//���մ���,���¿�ʼ
				else USART_RX_STA|=0x8000;	//��������� 
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

