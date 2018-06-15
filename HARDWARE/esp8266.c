#include "esp8266.h"
#include "stdarg.h"
//#include "hmi.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK STM32开发板
//ATK-ESP8266 WIFI模块 公用驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2014/4/3
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
u8  USART2_TX_BUF[USART2_MAX_SEND_LEN]; 			//发送缓冲,最大USART3_MAX_SEND_LEN字节
u8 	USART2_RX_BUF[USART2_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART2_RX_STA=0;       //接收状态标记	  

void TIM6_Int_Init(u16 arr,u16 psc)
{	
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);//TIM7时钟使能    
	
	//定时器TIM6初始化
	TIM_TimeBaseStructure.TIM_Period = arr; //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure); //根据指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(TIM6,TIM_IT_Update,ENABLE ); //使能指定的TIM6中断,允许更新中断
	
	TIM_Cmd(TIM6,ENABLE);//开启定时器6
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;//抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//子优先级1
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
}

void uart2_init(u32 bound){
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	//RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2|RCC_APB2Periph_GPIOA, ENABLE);	//使能USART2
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	// GPIOB时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE); //串口2时钟使能
 	USART_DeInit(USART2);  //复位串口2
	//USART2_TX   PA2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA2
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
	GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化PA2
   
  //USART2_RX	  PA3
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//浮空输入
  GPIO_Init(GPIOA, &GPIO_InitStructure);  //初始化PA3

  //Usart2 NVIC 配置

  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
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

  USART_Init(USART2, &USART_InitStructure); //初始化串口
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启中断
  USART_Cmd(USART2, ENABLE);                    //使能串口 
	TIM6_Int_Init(1000-1,7200-1);		//100ms中断
	USART2_RX_STA=0;		//清零
	TIM_Cmd(TIM6,DISABLE);			//关闭定时器7
}
#if EN_USART2_RX   //如果使能了接收
void USART2_IRQHandler(void)                	//串口2中断服务程序
{
	u8 res;	      
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)//接收到数据
	{	 
		res =USART_ReceiveData(USART2);		 
		if((USART2_RX_STA&(1<<15))==0)//接收完的一批数据,还没有被处理,则不再接收其他数据
		{ 
			if(USART2_RX_STA<USART2_REC_LEN)	//还可以接收数据
			{
				TIM_SetCounter(TIM6,0);//计数器清空          				//计数器清空
				if(USART2_RX_STA==0) 				//使能定时器7的中断 
				{
					TIM_Cmd(TIM6,ENABLE);//使能定时器6
				}
				USART2_RX_BUF[USART2_RX_STA++]=res;	//记录接收到的值	 
			}else 
			{
				USART2_RX_STA|=1<<15;				//强制标记接收完成
			} 
		}
	} 
//	u8 Res;
//	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
//	{
//		Res =USART_ReceiveData(USART2);//(USART2->DR);	//读取接收到的数据
//		//printf("%c", Res);//usart1_send_char(Res);
//		if((USART2_RX_STA&0x8000)==0)//接收未完成
//		{
//			if(USART2_RX_STA&0x4000)//接收到了0x0d
//			{
//				if(Res!=0x0a)USART2_RX_STA=0;//接收错误,重新开始
//				else {
//					USART2_RX_STA|=0x8000;	//接收完成了
//					printf("%s\r\n", USART2_RX_BUF);
//				} 
//			}
//			else //还没收到0X0D
//			{	
//				if(Res==0x0d)USART2_RX_STA|=0x4000;
//				else
//				{
//					USART2_RX_BUF[USART2_RX_STA&0X3FFF]=Res;
//					USART2_RX_STA++;
//					if(USART2_RX_STA>(USART2_REC_LEN-1))USART2_RX_STA=0;//接收数据错误,重新开始接收	  
//				}		 
//			}
//		}   
//	}
} 
#endif	

void TIM6_IRQHandler(void)
{ 	
	if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)//是更新中断
	{	 			   
		USART2_RX_STA|=1<<15;	//标记接收完成
		TIM_ClearITPendingBit(TIM6, TIM_IT_Update  );  //清除TIM6更新中断标志    
		TIM_Cmd(TIM6, DISABLE);  //关闭TIM6 
	}	    
}

void usart2_send_char(u8 temp)
{
	USART_SendData(USART2, (u8)temp);
	while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
}

void usart2_send_buff(char *buff, u8 len)
{
	u8 i;
	for(i = 0; i < len; i++) {
		usart2_send_char(buff[i]);
	}
}

void u2_printf(char* fmt,...)  
{  
	u16 i,j; 
	va_list ap; 
	va_start(ap,fmt);
	vsprintf((char*)USART2_TX_BUF,fmt,ap);
	va_end(ap);
	i=strlen((const char*)USART2_TX_BUF);		//此次发送数据的长度
	for(j=0;j<i;j++)							//循环发送数据
	{
	  while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET); //循环发送,直到发送完毕   
		USART_SendData(USART2,USART2_TX_BUF[j]); 
	} 
}

void esp8266_rec(void)
{
	u8 len, t;
	if(USART2_RX_STA&0x8000)
	{					   
		len=USART2_RX_STA&0x3fff;
		for(t=0;t<len;t++)
		{
			USART2->DR=USART2_RX_BUF[t];
			//USART1->DR=USART3_RX_BUF[t];
			while((USART2->SR&0X20)==0);
		}
		//usart1_send_buff("\r\n", 2);
		USART2_RX_STA=0;
		//rev_flag++;	//准备数据处理
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//用户配置区

//连接端口号:8086,可自行修改为其他端口.
//const u8* portnum="8086";		 

////WIFI STA模式,设置要去连接的路由器无线参数,请根据你自己的路由器设置,自行修改.
//const u8* wifista_ssid="ALIENTEK";			//路由器SSID号
//const u8* wifista_encryption="wpawpa2_aes";	//wpa/wpa2 aes加密方式
//const u8* wifista_password="15902020353"; 	//连接密码

////WIFI AP模式,模块对外的无线参数,可自行修改.
//const u8* wifiap_ssid="ATK-ESP8266";			//对外SSID号
//const u8* wifiap_encryption="wpawpa2_aes";	//wpa/wpa2 aes加密方式
//const u8* wifiap_password="12345678"; 		//连接密码 

///////////////////////////////////////////////////////////////////////////////////////////////////////////// 
////4个网络模式
//const u8 *ATK_ESP8266_CWMODE_TBL[3]={"STA模式 ","AP模式 ","AP&STA模式 "};	//ATK-ESP8266,3种网络模式,默认为路由器(ROUTER)模式 
////4种工作模式
//const u8 *ATK_ESP8266_WORKMODE_TBL[3]={"TCP服务器","TCP客户端"," UDP 模式"};	//ATK-ESP8266,4种工作模式
////5种加密方式
//const u8 *ATK_ESP8266_ECN_TBL[5]={"OPEN","WEP","WPA_PSK","WPA2_PSK","WPA_WAP2_PSK"};
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 

//usmart支持部分
//将收到的AT指令应答数据返回给电脑串口
//mode:0,不清零USART3_RX_STA;
//     1,清零USART3_RX_STA;
//void atk_8266_at_response(u8 mode)
//{
//	if(USART4_RX_STA&0X8000)		//接收到一次数据了
//	{ 
//		USART4_RX_BUF[USART4_RX_STA&0X7FFF]=0;//添加结束符
//		usart4_send_buff(USART4_RX_BUF, strlen(USART4_RX_BUF));
//		if(mode)USART4_RX_STA=0;
//	} 
//}
//ATK-ESP8266发送命令后,检测接收到的应答
//str:期待的应答结果
//返回值:0,没有得到期待的应答结果
//    其他,期待应答结果的位置(str的位置)
u8* atk_8266_check_cmd(u8 *str)
{
	
	char *strx=0;
	if(USART2_RX_STA&0X8000)		//接收到一次数据了
	{ 
		USART2_RX_BUF[USART2_RX_STA&0X7FFF]=0;//添加结束符
		//usart1_send_buff(USART2_RX_BUF,strlen(USART2_RX_BUF));
		strx=strstr((const char*)USART2_RX_BUF,(const char*)str);
	} 
	return (u8*)strx;
	//return (u8 *)1;
}
//向ATK-ESP8266发送命令
//cmd:发送的命令字符串
//ack:期待的应答结果,如果为空,则表示不需要等待应答
//waittime:等待时间(单位:10ms)
//返回值:0,发送成功(得到了期待的应答结果)
//       1,发送失败
u8 atk_8266_send_cmd(u8 *cmd,u8 *ack,u16 waittime)
{
	u8 res=0; 
	USART2_RX_STA=0;
	u2_printf("%s\r\n", cmd);
	if(ack&&waittime)		//需要等待应答
	{
		while(--waittime)	//等待倒计时
		{
			delay_ms(10);
			if(USART2_RX_STA&0X8000)//接收到期待的应答结果
			{
				//usart1_send_buff("rec\r\n", 5);
				if(atk_8266_check_cmd(ack))
				{
//					usart1_send_buff("ack:",4);
//					usart1_send_buff((char *)ack, strlen((char *)ack));
//					usart1_send_buff("\r\n", 2);
					printf("ack:%s\r\n",(u8*)ack);
					break;//得到有效数据 
				}
					USART2_RX_STA=0;
			} 
		}
		if(waittime==0)res=1; 
	}
	return res;
} 
//向ATK-ESP8266发送指定数据
//data:发送的数据(不需要添加回车了)
//ack:期待的应答结果,如果为空,则表示不需要等待应答
//waittime:等待时间(单位:10ms)
//返回值:0,发送成功(得到了期待的应答结果)luojian
//u8 atk_8266_send_data(u8 *data,u8 *ack,u16 waittime)
//{
//	u8 res=0; 
//	USART4_RX_STA=0;
//	usart4_send_buff(data,strlen(data));
//	usart4_send_buff("\r\n",2);
//	//u3_printf("%s",data);	//发送命令
//	if(ack&&waittime)		//需要等待应答
//	{
//		while(--waittime)	//等待倒计时
//		{
//			delay_ms(10);
//			if(USART4_RX_STA&0X8000)//接收到期待的应答结果
//			{
//				if(atk_8266_check_cmd(ack))break;//得到有效数据 
//				USART4_RX_STA=0;
//			} 
//		}
//		if(waittime==0)res=1; 
//	}
//	return res;
//}
//ATK-ESP8266退出透传模式
//返回值:0,退出成功;
//       1,退出失败
//u8 atk_8266_quit_trans(void)
//{
//	while((USART3->SR&0X40)==0);	//等待发送空
//	USART3->DR='+';      
//	delay_ms(15);					//大于串口组帧时间(10ms)
//	while((USART3->SR&0X40)==0);	//等待发送空
//	USART3->DR='+';      
//	delay_ms(15);					//大于串口组帧时间(10ms)
//	while((USART3->SR&0X40)==0);	//等待发送空
//	USART3->DR='+';      
//	delay_ms(500);					//等待500ms
//	return atk_8266_send_cmd("AT","OK",20);//退出透传判断.
//}
//获取ATK-ESP8266模块的AP+STA连接状态
//返回值:0，未连接;1,连接成功
//u8 atk_8266_apsta_check(void)
//{
//	if(atk_8266_quit_trans())return 0;			//退出透传 
//	atk_8266_send_cmd("AT+CIPSTATUS",":",50);	//发送AT+CIPSTATUS指令,查询连接状态
//	if(atk_8266_check_cmd("+CIPSTATUS:0")&&
//		 atk_8266_check_cmd("+CIPSTATUS:1")&&
//		 atk_8266_check_cmd("+CIPSTATUS:2")&&
//		 atk_8266_check_cmd("+CIPSTATUS:4"))
//		return 0;
//	else return 1;
//}
//获取ATK-ESP8266模块的连接状态
//返回值:0,未连接;1,连接成功.
//u8 atk_8266_consta_check(void)
//{
//	u8 *p;
//	u8 res;
//	if(atk_8266_quit_trans())return 0;			//退出透传 
//	atk_8266_send_cmd("AT+CIPSTATUS",":",50);	//发送AT+CIPSTATUS指令,查询连接状态
//	p=atk_8266_check_cmd("+CIPSTATUS:"); 
//	res=*p;									//得到连接状态	
//	return res;
//}
//键盘码表
//const u8* kbd_tbl[13]={"1","2","3","4","5","6","7","8","9",".","0","#","DEL"}; 


//获取ip地址
//ipbuf:ip地址输出缓存区
//void atk_8266_get_wanip(u8* ipbuf)
//{
//	u8 *p,*p1;
//		if(atk_8266_send_cmd("AT+CIFSR","OK",50))//获取WAN IP地址失败
//		{
//			ipbuf[0]=0;
//			return;
//		}		
//		p=atk_8266_check_cmd("\"");
//		p1=(u8*)strstr((const char*)(p+1),"\"");
//		*p1=0;
//		sprintf((char*)ipbuf,"%s",p+1);	
//}





















































