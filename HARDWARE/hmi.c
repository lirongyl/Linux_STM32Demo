#include "sys.h"
#include "hmi.h"	  
#include "usart.h"
#include "string.h"
u8 USART4_RX_BUF[USART4_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART4_RX_STA=0;       //接收状态标记	  

//初始化IO 串口2 
//bound:波特率
void HMI_init(u32 bound){
    //GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);	//使能USART4
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC,ENABLE);    
 	USART_DeInit(UART4);  //复位串口4
	//USART4_TX   PC10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;  
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
  //UART4_RX	  PC11
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//上拉输入
	GPIO_Init(GPIOC, &GPIO_InitStructure);  //初始化PA11

  //Usart2 NVIC 配置

	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		//子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
  //USART3 初始化设置

	USART_InitStructure.USART_BaudRate = bound;//一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

  USART_Init(UART4, &USART_InitStructure); //初始化串口
  USART_ITConfig(UART4, USART_IT_RXNE, DISABLE);//开启中断
  USART_Cmd(UART4, ENABLE);                    //使能串口 

}
#if EN_USART4_RX   //如果使能了接收
void UART4_IRQHandler(void)                	//串口2中断服务程序
{
	u8 Res;
	if(USART_GetITStatus(UART4, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		Res =USART_ReceiveData(UART4);//(USART2->DR);	//读取接收到的数据
		if((USART4_RX_STA&0x8000)==0)//接收未完成
		{
			if(USART4_RX_STA&0x4000)//接收到了0x0d
			{
				if(Res!=0x0a)USART4_RX_STA=0;//接收错误,重新开始
				else USART4_RX_STA|=0x8000;	//接收完成了 
			}
			else //还没收到0X0D
			{	
				if(Res==0x0d)USART4_RX_STA|=0x4000;
				else
				{
					USART4_RX_BUF[USART4_RX_STA&0X3FFF]=Res ;
					USART4_RX_STA++;
					if(USART4_RX_STA>(USART4_REC_LEN-1))USART4_RX_STA=0;//接收数据错误,重新开始接收	  
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
