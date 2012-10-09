#ifndef _STRUCT_H
#define _STRUCT_H

#include "header.h"

//const int CONNECTIONS_LIMIT = 1024;
const int SEND_WAIT_TIME = 10000;

struct Packet
{
	int packet_len;
	char packet_data[];
};
const int PRE_RECV_LEN = sizeof(int);
const int PACK_HEAD_LEN = sizeof(Packet);
const int MAX_PACK_LEN = 4096;
const int MAX_PACK_DATA_LEN = MAX_PACK_LEN-PACK_HEAD_LEN;

struct TaskInfo
{	
	TaskInfo() : task_id(-1), ptask_info(NULL) {}
	TaskInfo(int _task_id, char *_ptask_info) : 
		task_id(_task_id), ptask_info(_ptask_info){}
	
	int task_id;
	char *ptask_info;
};

struct ClientContext
{
	int client_socket;
	char *buffer;
	int last_recv_byte;
};



#endif

