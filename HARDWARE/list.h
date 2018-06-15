#ifndef __LIST_H
#define __LIST_H
#include "sys.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

struct Transh {
	char start[2];			//��ʼ##
	char number[4];			//���
	char channel[2];		//�ŵ�
	char status[1];			//״̬
	char total[3];			//����Ͱ����
	char remain[3];			//ʣ������
	char stop[1];				//����#
	struct Transh *next;	//����ָ����һ��ַ
};

struct Transh *Creat(u8 *rec_buff);
u8 Isempty(struct Transh *head);
int Getlen(struct Transh *head);
struct Transh *Insert(struct Transh *head, u8 *rec_buff);
struct Transh *Delte(struct Transh *head, int pos);
#endif

