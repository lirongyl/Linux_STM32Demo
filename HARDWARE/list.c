#include "list.h"
#include "usart.h"
//extern u8  USART2_RX_BUF[USART2_REC_LEN]; 
//data:# # 0 0 0 3 0 3 L 5  0  0  1  0  0 #
//num: 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15

//struct Transh {
//	char start[2];	
//	char number[4];		
//	char channel[2];	
//	char status[1];		
//	char total[3];		
//	char remain[3];			
//	char stop[1];		
//	struct Transh *next;	
//};

struct Transh *Creat(u8 *rec_buff)
{
	struct Transh *head, *p1;
	head = NULL;
	p1 = (struct Transh*)malloc(sizeof(struct Transh));
	if(p1 == NULL) {	//
		//usart1_send_buff("malloc failed!\r\n", 16);
		free(p1);
		return head;
	} else {
		//usart1_send_buff("malloc success!\r\n", 17);
		if(rec_buff[0] == '#' && rec_buff[1] == '#' && rec_buff[15] == '#'){
			strncpy(p1->start, (char *)rec_buff, 2);
			strncpy(p1->number, (char *)rec_buff+2, 4);
			strncpy(p1->channel, (char *)rec_buff+6, 2);
			strncpy(p1->status, (char *)rec_buff+8, 1);
			strncpy(p1->total, (char *)rec_buff+9, 3);
			strncpy(p1->remain, (char *)rec_buff+12, 3);
			strncpy(p1->stop, (char *)rec_buff+15, 1);
		}
		head = p1;
		p1->next = NULL;
	}
	return head;
}

//
u8 Isempty(struct Transh *head)
{
	if(head == NULL) {
		return 1;
	} else {
		return 0;
	}
}

//
int Getlen(struct Transh *head)
{
	int len = 0;
	struct Transh *current;
	current = head;
	if(current == NULL) {
		return len;
	}
	while(current != NULL) {
		len++;
		current = current->next;
	};
	return len;
}

//
struct Transh *Insert(struct Transh *head, u8 *rec_buff)
{
	struct Transh *current, *p1, *temp;
	p1 = (struct Transh*)malloc(sizeof(struct Transh));
	if(p1 == NULL) {	//
		//usart1_send_buff("add malloc failed!\r\n", 20);
		free(p1);
	} 
	
	if(p1 != NULL) {	//
		//usart1_send_buff("add malloc success!\r\n", 21);
		if(rec_buff[0] == '#' && rec_buff[1] == '#' && rec_buff[15] == '#'){
			//
			strncpy(p1->start, (char *)rec_buff, 2);
			strncpy(p1->number, (char *)rec_buff+2, 4);
			strncpy(p1->channel, (char *)rec_buff+6, 2);
			strncpy(p1->status, (char *)rec_buff+8, 1);
			strncpy(p1->total, (char *)rec_buff+9, 3);
			strncpy(p1->remain, (char *)rec_buff+12, 3);
			strncpy(p1->stop, (char *)rec_buff+15, 1);
			current = head;
			while(current != NULL) {
				temp = current;
				current = current->next;
			}
			temp->next = p1;
			p1->next = NULL;
		}
	}
	return head;
}

//
struct Transh *Delte(struct Transh *head, int pos)
{
	u8 i = 0;
	struct Transh *current, *temp;
	current = head;

	if( (pos > 1) && (pos <= Getlen(head)) ) {
		for(i = 1; i < pos; i++) {
			temp = current;
			current = current->next;
		}
		temp->next = current->next;
		free(current);
	}
	if(pos == 1) {
		head = current->next;
		free(current);
	}
	return head;
}

