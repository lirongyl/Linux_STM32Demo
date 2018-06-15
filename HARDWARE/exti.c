#include "exti.h"
#include "led.h"
#include "key.h"
#include "delay.h"
#include "usart.h"

void EXTIX_Init(void)
{ 
	EXTI_InitTypeDef EXTI_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;

  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);//外部中断，需要使能AFIO时钟

	KEY_Init();//初始化按键对应io模式

    //GPIOC.1 中断线以及中断初始化配置
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource1);

  	EXTI_InitStructure.EXTI_Line=EXTI_Line1;
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//下降沿触发
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);	 	//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器

    //GPIOC.13	  中断线以及中断初始化配置
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource13);

  	EXTI_InitStructure.EXTI_Line=EXTI_Line13;
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);	  	//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器

  	NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQn;			//使能按键所在的外部中断通道
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//抢占优先级2 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;					//子优先级0
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
  	NVIC_Init(&NVIC_InitStructure);  	  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器
 
   	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;			//使能按键所在的外部中断通道
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//抢占优先级2， 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;					//子优先级1
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
  	NVIC_Init(&NVIC_InitStructure); 
 
}

 void EXTI1_IRQHandler(void)
{			
	delay_ms(10);   //消抖			 
	if(KEY0==0)	{
		LED0=!LED0;
		key0_num++;
	}
 	 EXTI_ClearITPendingBit(EXTI_Line1);    //清除LINE1上的中断标志位  
}


void EXTI15_10_IRQHandler(void)
{
  delay_ms(10);    //消抖			 
  if(KEY1==0)	{
		LED1=!LED1;
		key1_num++;
	}
	 EXTI_ClearITPendingBit(EXTI_Line13);  //清除LINE15线路挂起位
}
