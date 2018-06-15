#include "sys.h"
#include "hmi.h"	  
#include "usart.h"
#include "string.h"
u8 USART4_RX_BUF[USART4_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART4_RX_STA=0;       //����״̬���	  

//��ʼ��IO ����2 
//bound:������
void HMI_init(u32 bound){
    //GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);	//ʹ��USART4
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);    
 	USART_DeInit(UART4);  //��λ����4
	//USART4_TX   PC10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //�����������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
  //UART4_RX	  PC11
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//��������
	GPIO_Init(GPIOC, &GPIO_InitStructure);  //��ʼ��PA11

  //Usart2 NVIC ����

	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//��ռ���ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//�����ȼ�2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
  
  //USART3 ��ʼ������

	USART_InitStructure.USART_BaudRate = bound;//һ������Ϊ9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ

  USART_Init(UART4, &USART_InitStructure); //��ʼ������
  USART_ITConfig(UART4, USART_IT_RXNE, DISABLE);//�����ж�
  USART_Cmd(UART4, ENABLE);                    //ʹ�ܴ��� 

}
#if EN_USART4_RX   //���ʹ���˽���
void UART4_IRQHandler(void)                	//����2�жϷ������
{
	u8 Res;
	if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
	{
		Res =USART_ReceiveData(UART4);//(USART2->DR);	//��ȡ���յ�������
		if((USART4_RX_STA&0x8000)==0)//����δ���
		{
			if(USART4_RX_STA&0x4000)//���յ���0x0d
			{
				if(Res!=0x0a)USART4_RX_STA=0;//���մ���,���¿�ʼ
				else USART4_RX_STA|=0x8000;	//��������� 
			}
			else //��û�յ�0X0D
			{	
				if(Res==0x0d)USART4_RX_STA|=0x4000;
				else
				{
					USART4_RX_BUF[USART4_RX_STA&0X3FFF]=Res ;
					USART4_RX_STA++;
					if(USART4_RX_STA>(USART4_REC_LEN-1))USART4_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
				}		 
			}
		}   		 
	} 
} 
#endif	

void usart4_send_char(u8 temp)
{
	USART_SendData(UART4, (u8)temp);
	while(USART_GetFlagStatus(UART4, USART_FLAG_TXE) == RESET);
}

void hmi_send(char *buff, u16 len)
{
	u8 i;
	for(i = 0; i < len; i++) {
		usart4_send_char(buff[i]);
	}
}

void hmi_send_end(void)
{
	usart4_send_char(0xFF);
	usart4_send_char(0xFF);
	usart4_send_char(0xFF);
}
