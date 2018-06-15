#ifndef __LIST_H
#define __LIST_H
#include "sys.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

struct Transh {
	char start[2];			//开始##
	char number[4];			//编号
	char channel[2];		//信道
	char status[1];			//状态
	char total[3];			//垃圾桶容量
	char remain[3];			//剩余容量
	char stop[1];				//结束#
	struct Transh *next;	//链表指向下一地址
};

struct Transh *Creat(u8 *rec_buff);
u8 Isempty(struct Transh *head);
int Getlen(struct Transh *head);
struct Transh *Insert(struct Transh *head, u8 *rec_buff);
struct Transh *Delte(struct Transh *head, int pos);
#endif

