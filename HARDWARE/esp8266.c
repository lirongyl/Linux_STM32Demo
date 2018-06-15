#include "esp8266.h"
#include "stdarg.h"
//#include "hmi.h"
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//ATK-ESP8266 WIFIģ�� ������������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2014/4/3
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
u8  USART2_TX_BUF[USART2_MAX_SEND_LEN]; 			//���ͻ���,���USART3_MAX_SEND_LEN�ֽ�
u8 	USART2_RX_BUF[USART2_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART2_RX_STA=0;       //����״̬���	  

void TIM6_Int_Init(u16 arr,u16 psc)
{	
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);//TIM7ʱ��ʹ��    
	
	//��ʱ��TIM6��ʼ��
	TIM_TimeBaseStructure.TIM_Period = arr; //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure); //����ָ���Ĳ�����ʼ��TIMx��ʱ�������λ
 
	TIM_ITConfig(TIM6,TIM_IT_Update,ENABLE ); //ʹ��ָ����TIM6�ж�,��������ж�
	
	TIM_Cmd(TIM6,ENABLE);//������ʱ��6
	
	NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0 ;//��ռ���ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;		//�����ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
}

void uart2_init(u32 bound){
  GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	//RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2|RCC_APB2Periph_GPIOA, ENABLE);	//ʹ��USART2
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	// GPIOBʱ��
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE); //����2ʱ��ʹ��
 	USART_DeInit(USART2);  //��λ����2
	//USART2_TX   PA2
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA2
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
	GPIO_Init(GPIOA, &GPIO_InitStructure); //��ʼ��PA2
   
  //USART2_RX	  PA3
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
	//GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;//��������
  GPIO_Init(GPIOA, &GPIO_InitStructure);  //��ʼ��PA3

  //Usart2 NVIC ����

  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
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

  USART_Init(USART2, &USART_InitStructure); //��ʼ������
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//�����ж�
  USART_Cmd(USART2, ENABLE);                    //ʹ�ܴ��� 
	TIM6_Int_Init(1000-1,7200-1);		//100ms�ж�
	USART2_RX_STA=0;		//����
	TIM_Cmd(TIM6,DISABLE);			//�رն�ʱ��7
}
#if EN_USART2_RX   //���ʹ���˽���
void USART2_IRQHandler(void)                	//����2�жϷ������
{
	u8 res;	      
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)//���յ�����
	{	 
		res =USART_ReceiveData(USART2);		 
		if((USART2_RX_STA&(1<<15))==0)//�������һ������,��û�б�����,���ٽ�����������
		{ 
			if(USART2_RX_STA<USART2_REC_LEN)	//�����Խ�������
			{
				TIM_SetCounter(TIM6,0);//���������          				//���������
				if(USART2_RX_STA==0) 				//ʹ�ܶ�ʱ��7���ж� 
				{
					TIM_Cmd(TIM6,ENABLE);//ʹ�ܶ�ʱ��6
				}
				USART2_RX_BUF[USART2_RX_STA++]=res;	//��¼���յ���ֵ	 
			}else 
			{
				USART2_RX_STA|=1<<15;				//ǿ�Ʊ�ǽ������
			} 
		}
	} 
//	u8 Res;
//	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
//	{
//		Res =USART_ReceiveData(USART2);//(USART2->DR);	//��ȡ���յ�������
//		//printf("%c", Res);//usart1_send_char(Res);
//		if((USART2_RX_STA&0x8000)==0)//����δ���
//		{
//			if(USART2_RX_STA&0x4000)//���յ���0x0d
//			{
//				if(Res!=0x0a)USART2_RX_STA=0;//���մ���,���¿�ʼ
//				else {
//					USART2_RX_STA|=0x8000;	//���������
//					printf("%s\r\n", USART2_RX_BUF);
//				} 
//			}
//			else //��û�յ�0X0D
//			{	
//				if(Res==0x0d)USART2_RX_STA|=0x4000;
//				else
//				{
//					USART2_RX_BUF[USART2_RX_STA&0X3FFF]=Res;
//					USART2_RX_STA++;
//					if(USART2_RX_STA>(USART2_REC_LEN-1))USART2_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
//				}		 
//			}
//		}   
//	}
} 
#endif	

void TIM6_IRQHandler(void)
{ 	
	if (TIM_GetITStatus(TIM6, TIM_IT_Update) != RESET)//�Ǹ����ж�
	{	 			   
		USART2_RX_STA|=1<<15;	//��ǽ������
		TIM_ClearITPendingBit(TIM6, TIM_IT_Update  );  //���TIM6�����жϱ�־    
		TIM_Cmd(TIM6, DISABLE);  //�ر�TIM6 
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
	i=strlen((const char*)USART2_TX_BUF);		//�˴η������ݵĳ���
	for(j=0;j<i;j++)							//ѭ����������
	{
	  while(USART_GetFlagStatus(USART2,USART_FLAG_TC)==RESET); //ѭ������,ֱ���������   
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
		//rev_flag++;	//׼�����ݴ���
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
//�û�������

//���Ӷ˿ں�:8086,�������޸�Ϊ�����˿�.
//const u8* portnum="8086";		 

////WIFI STAģʽ,����Ҫȥ���ӵ�·�������߲���,��������Լ���·��������,�����޸�.
//const u8* wifista_ssid="ALIENTEK";			//·����SSID��
//const u8* wifista_encryption="wpawpa2_aes";	//wpa/wpa2 aes���ܷ�ʽ
//const u8* wifista_password="15902020353"; 	//��������

////WIFI APģʽ,ģ���������߲���,�������޸�.
//const u8* wifiap_ssid="ATK-ESP8266";			//����SSID��
//const u8* wifiap_encryption="wpawpa2_aes";	//wpa/wpa2 aes���ܷ�ʽ
//const u8* wifiap_password="12345678"; 		//�������� 

///////////////////////////////////////////////////////////////////////////////////////////////////////////// 
////4������ģʽ
//const u8 *ATK_ESP8266_CWMODE_TBL[3]={"STAģʽ ","APģʽ ","AP&STAģʽ "};	//ATK-ESP8266,3������ģʽ,Ĭ��Ϊ·����(ROUTER)ģʽ 
////4�ֹ���ģʽ
//const u8 *ATK_ESP8266_WORKMODE_TBL[3]={"TCP������","TCP�ͻ���"," UDP ģʽ"};	//ATK-ESP8266,4�ֹ���ģʽ
////5�ּ��ܷ�ʽ
//const u8 *ATK_ESP8266_ECN_TBL[5]={"OPEN","WEP","WPA_PSK","WPA2_PSK","WPA_WAP2_PSK"};
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 

//usmart֧�ֲ���
//���յ���ATָ��Ӧ�����ݷ��ظ����Դ���
//mode:0,������USART3_RX_STA;
//     1,����USART3_RX_STA;
//void atk_8266_at_response(u8 mode)
//{
//	if(USART4_RX_STA&0X8000)		//���յ�һ��������
//	{ 
//		USART4_RX_BUF[USART4_RX_STA&0X7FFF]=0;//��ӽ�����
//		usart4_send_buff(USART4_RX_BUF, strlen(USART4_RX_BUF));
//		if(mode)USART4_RX_STA=0;
//	} 
//}
//ATK-ESP8266���������,�����յ���Ӧ��
//str:�ڴ���Ӧ����
//����ֵ:0,û�еõ��ڴ���Ӧ����
//    ����,�ڴ�Ӧ������λ��(str��λ��)
u8* atk_8266_check_cmd(u8 *str)
{
	
	char *strx=0;
	if(USART2_RX_STA&0X8000)		//���յ�һ��������
	{ 
		USART2_RX_BUF[USART2_RX_STA&0X7FFF]=0;//��ӽ�����
		//usart1_send_buff(USART2_RX_BUF,strlen(USART2_RX_BUF));
		strx=strstr((const char*)USART2_RX_BUF,(const char*)str);
	} 
	return (u8*)strx;
	//return (u8 *)1;
}
//��ATK-ESP8266��������
//cmd:���͵������ַ���
//ack:�ڴ���Ӧ����,���Ϊ��,���ʾ����Ҫ�ȴ�Ӧ��
//waittime:�ȴ�ʱ��(��λ:10ms)
//����ֵ:0,���ͳɹ�(�õ����ڴ���Ӧ����)
//       1,����ʧ��
u8 atk_8266_send_cmd(u8 *cmd,u8 *ack,u16 waittime)
{
	u8 res=0; 
	USART2_RX_STA=0;
	u2_printf("%s\r\n", cmd);
	if(ack&&waittime)		//��Ҫ�ȴ�Ӧ��
	{
		while(--waittime)	//�ȴ�����ʱ
		{
			delay_ms(10);
			if(USART2_RX_STA&0X8000)//���յ��ڴ���Ӧ����
			{
				//usart1_send_buff("rec\r\n", 5);
				if(atk_8266_check_cmd(ack))
				{
//					usart1_send_buff("ack:",4);
//					usart1_send_buff((char *)ack, strlen((char *)ack));
//					usart1_send_buff("\r\n", 2);
					printf("ack:%s\r\n",(u8*)ack);
					break;//�õ���Ч���� 
				}
					USART2_RX_STA=0;
			} 
		}
		if(waittime==0)res=1; 
	}
	return res;
} 
//��ATK-ESP8266����ָ������
//data:���͵�����(����Ҫ��ӻس���)
//ack:�ڴ���Ӧ����,���Ϊ��,���ʾ����Ҫ�ȴ�Ӧ��
//waittime:�ȴ�ʱ��(��λ:10ms)
//����ֵ:0,���ͳɹ�(�õ����ڴ���Ӧ����)luojian
//u8 atk_8266_send_data(u8 *data,u8 *ack,u16 waittime)
//{
//	u8 res=0; 
//	USART4_RX_STA=0;
//	usart4_send_buff(data,strlen(data));
//	usart4_send_buff("\r\n",2);
//	//u3_printf("%s",data);	//��������
//	if(ack&&waittime)		//��Ҫ�ȴ�Ӧ��
//	{
//		while(--waittime)	//�ȴ�����ʱ
//		{
//			delay_ms(10);
//			if(USART4_RX_STA&0X8000)//���յ��ڴ���Ӧ����
//			{
//				if(atk_8266_check_cmd(ack))break;//�õ���Ч���� 
//				USART4_RX_STA=0;
//			} 
//		}
//		if(waittime==0)res=1; 
//	}
//	return res;
//}
//ATK-ESP8266�˳�͸��ģʽ
//����ֵ:0,�˳��ɹ�;
//       1,�˳�ʧ��
//u8 atk_8266_quit_trans(void)
//{
//	while((USART3->SR&0X40)==0);	//�ȴ����Ϳ�
//	USART3->DR='+';      
//	delay_ms(15);					//���ڴ�����֡ʱ��(10ms)
//	while((USART3->SR&0X40)==0);	//�ȴ����Ϳ�
//	USART3->DR='+';      
//	delay_ms(15);					//���ڴ�����֡ʱ��(10ms)
//	while((USART3->SR&0X40)==0);	//�ȴ����Ϳ�
//	USART3->DR='+';      
//	delay_ms(500);					//�ȴ�500ms
//	return atk_8266_send_cmd("AT","OK",20);//�˳�͸���ж�.
//}
//��ȡATK-ESP8266ģ���AP+STA����״̬
//����ֵ:0��δ����;1,���ӳɹ�
//u8 atk_8266_apsta_check(void)
//{
//	if(atk_8266_quit_trans())return 0;			//�˳�͸�� 
//	atk_8266_send_cmd("AT+CIPSTATUS",":",50);	//����AT+CIPSTATUSָ��,��ѯ����״̬
//	if(atk_8266_check_cmd("+CIPSTATUS:0")&&
//		 atk_8266_check_cmd("+CIPSTATUS:1")&&
//		 atk_8266_check_cmd("+CIPSTATUS:2")&&
//		 atk_8266_check_cmd("+CIPSTATUS:4"))
//		return 0;
//	else return 1;
//}
//��ȡATK-ESP8266ģ�������״̬
//����ֵ:0,δ����;1,���ӳɹ�.
//u8 atk_8266_consta_check(void)
//{
//	u8 *p;
//	u8 res;
//	if(atk_8266_quit_trans())return 0;			//�˳�͸�� 
//	atk_8266_send_cmd("AT+CIPSTATUS",":",50);	//����AT+CIPSTATUSָ��,��ѯ����״̬
//	p=atk_8266_check_cmd("+CIPSTATUS:"); 
//	res=*p;									//�õ�����״̬	
//	return res;
//}
//�������
//const u8* kbd_tbl[13]={"1","2","3","4","5","6","7","8","9",".","0","#","DEL"}; 


//��ȡip��ַ
//ipbuf:ip��ַ���������
//void atk_8266_get_wanip(u8* ipbuf)
//{
//	u8 *p,*p1;
//		if(atk_8266_send_cmd("AT+CIFSR","OK",50))//��ȡWAN IP��ַʧ��
//		{
//			ipbuf[0]=0;
//			return;
//		}		
//		p=atk_8266_check_cmd("\"");
//		p1=(u8*)strstr((const char*)(p+1),"\"");
//		*p1=0;
//		sprintf((char*)ipbuf,"%s",p+1);	
//}





















































