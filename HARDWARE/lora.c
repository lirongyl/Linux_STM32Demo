#include "sys.h"
#include "lora.h"	  
////////////////////////////////////////////////////////////////////////////////// 	 
//////////////////////////////////////////////////////////////////////////////////	 
//V1.3修改说明 
//支持适应不同频率下的串口波特率设置.
//增加了串口接收命令功能.
//1,修改串口初始化IO的bug
//2,修改了USART_RX_STA,使得串口最大接收字节数为2的14次方
//3,增加了USART_REC_LEN,用于定义串口最大允许接收的字节数(不大于2的14次方)
//4,修改了EN_USART1_RX的使能方式
////////////////////////////////////////////////////////////////////////////////// 	  
 

//////////////////////////////////////////////////////////////////
//串口3中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误   	
u8 USART3_RX_BUF[USART3_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART3_RX_STA=0;       //接收状态标记	  
u8 rev_flag = 0;
//初始化IO 串口3 
//bound:波特率
void lora_init(u32 bound){
    //GPIO端口设置
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);	//使能USART3
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	//使能GPIOB时钟
 	USART_DeInit(USART3);  //复位串口3
	//USART3_TX   PB10
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; //PB10
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOB, &GPIO_InitStructure); //初始化PB10
   
  //USART3_RX	  PB11
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
  GPIO_Init(GPIOB, &GPIO_InitStructure);  //初始化PB11
	
	//配置LoRa模块的M0和M1
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;	//PB0和PB1;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOB, &GPIO_InitStructure);  //初始化PB0,1
	/* Clears the GPIOB port pin 0 and pin 1 */
	GPIO_ResetBits(GPIOB, GPIO_Pin_0 | GPIO_Pin_1);
  //Usart3 NVIC 配置

  NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x01 ;//抢占优先级3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
  
  //USART3 初始化设置

	USART_InitStructure.USART_BaudRate = bound;//一般设置为9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
	USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
	USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式

  USART_Init(USART3, &USART_InitStructure); //初始化串口
  USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//开启中断
  USART_Cmd(USART3, ENABLE);                    //使能串口 

}
#if EN_USART3_RX   //如果使能了接收
void USART3_IRQHandler(void)                	//串口3中断服务程序
{
	u8 Res;
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		Res =USART_ReceiveData(USART3);//(USART3->DR);	//读取接收到的数据
		
		if((USART3_RX_STA&0x8000)==0)//接收未完成
		{
			if(USART3_RX_STA&0x4000)//接收到了0x0d
			{
				if(Res!=0x0a)USART3_RX_STA=0;//接收错误,重新开始
				else USART3_RX_STA|=0x8000;	//接收完成了 
			}
			else //还没收到0X0D
			{	
				if(Res==0x0d)USART3_RX_STA|=0x4000;
				else
				{
					USART3_RX_BUF[USART3_RX_STA&0X3FFF]=Res ;
					USART3_RX_STA++;
					if(USART3_RX_STA>(USART3_REC_LEN-1))USART3_RX_STA=0;//接收数据错误,重新开始接收	  
				}		 
			}
		}   		 
	} 
} 
#endif	

void usart3_send_char(u8 temp)
{
	USART_SendData(USART3, (u8)temp);
	while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
}

void usart3_send_buff(char *buff, u8 len)
{
	u8 i;
	for(i = 0; i < len; i++) {
		usart3_send_char(buff[i]);
	}
}

void lora_send(u8 *buff, u8 len)
{
	u8 i;
	u8 data[3] = {0x00, 0x03, 0x03};
	usart3_send_char(data[0]);
	usart3_send_char(data[1]);
	usart3_send_char(data[2]);
	for(i = 0; i < len; i++) {
		usart3_send_char(buff[i]);
	}
	usart3_send_buff("\r\n", 2);
}
void lora_rec(void)
{
	u8 len, t;
	if(USART3_RX_STA&0x8000)
	{					   
		len=USART3_RX_STA&0x3fff;
		for(t=0;t<len;t++)
		{
			USART3->DR=USART3_RX_BUF[t];
			USART1->DR=USART3_RX_BUF[t];
			while((USART3->SR&0X40)==0);
		}
		usart1_send_buff("\r\n", 2);
		USART3_RX_STA=0;
		rev_flag = 0x01;	//准备数据处理
	}
}
