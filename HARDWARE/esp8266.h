#ifndef __ESP8266_H__
#define __ESP8266_H__
#include "sys.h"
#include "usart.h"		
#include "delay.h"	
#include "led.h"   	 
#include "string.h" 		 

#define USART2_REC_LEN  		200  	//定义最大接收字节数 200
#define USART2_MAX_SEND_LEN		200
#define EN_USART2_RX 			1		//使能（1）/禁止（0）串口1接收
	  	
extern u8  USART2_RX_BUF[USART2_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u8  USART2_TX_BUF[USART2_MAX_SEND_LEN]; 			//发送缓冲,最大USART3_MAX_SEND_LEN字节
extern u16 USART2_RX_STA;         		//接收状态标记	
//如果想串口中断接收，请不要注释以下宏定义
void uart2_init(u32 bound);
void usart2_send_char(u8 temp);
void usart2_send_buff(char *buff, u8 len);
void esp8266_rec(void);
void u2_printf(char* fmt,...);

/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
#define AT_MODE_CONFIG		0				//0,采用ES/RST硬件控制进入AT模式;
											//1,采用串口特殊序列(+++1B1B1B)进入AT模式
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
void atk_8266_init(void);

u8 atk_8266_mode_cofig(u8 netpro);
void atk_8266_at_response(u8 mode);
u8* atk_8266_check_cmd(u8 *str);
u8 atk_8266_apsta_check(void);
u8 atk_8266_send_data(u8 *data,u8 *ack,u16 waittime);
u8 atk_8266_send_cmd(u8 *cmd,u8 *ack,u16 waittime);
u8 atk_8266_quit_trans(void);
u8 atk_8266_consta_check(void);
void atk_8266_load_keyboard(u16 x,u16 y);
void atk_8266_key_staset(u16 x,u16 y,u8 keyx,u8 sta);
u8 atk_8266_get_keynum(u16 x,u16 y);
void atk_8266_get_wanip(u8* ipbuf);
void atk_8266_get_ip(u8 x,u8 y);
void atk_8266_msg_show(u16 x,u16 y,u8 wanip);
void atk_8266_wificonf_show(u16 x,u16 y,u8* rmd,u8* ssid,u8* encryption,u8* password);
u8 atk_8266_netpro_sel(u16 x,u16 y,u8* name);
void atk_8266_mtest_ui(u16 x,u16 y);

u8 atk_8266_ip_set(u8* title,u8* mode,u8* port,u8* ip);
void atk_8266_test(void);



u8 atk_8266_apsta_test(void);	//WIFI AP+STA模式测试
u8 atk_8266_wifista_test(void);	//WIFI STA测试
u8 atk_8266_wifiap_test(void);	//WIFI AP测试

//用户配置参数
extern const u8* portnum;			//连接端口
 
extern const u8* wifista_ssid;		//WIFI STA SSID
extern const u8* wifista_encryption;//WIFI STA 加密方式
extern const u8* wifista_password; 	//WIFI STA 密码

extern const u8* wifiap_ssid;		//WIFI AP SSID
extern const u8* wifiap_encryption;	//WIFI AP 加密方式
extern const u8* wifiap_password; 	//WIFI AP 密码

extern u8* kbd_fn_tbl[2];
extern const u8* ATK_ESP8266_CWMODE_TBL[3];
extern const u8* ATK_ESP8266_WORKMODE_TBL[3];
extern const u8* ATK_ESP8266_ECN_TBL[5];
#endif





