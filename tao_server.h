#ifndef _GN_SERVER_H
#define _GN_SERVER_H

#include "epoll.h"
#include "mutex.h"
#include "thread_pool.h"
#include "memory_pool.h"

class TaoServer : public Epoll
{
public:
	TaoServer();
	~TaoServer();

	void Initialize();
	void Startup();

private:
	void StartRecvThread();
	static void* RecvThreadProc(void *arg);
	void ProcessRecv();

	void StartSendThread();
	static void* SendThreadProc(void *arg);
	void ProcessSend();

	void InitClientContext(int client_socket);
	void DisconnectClient(int sock_fd, bool is_graceful = true);

	int RecvPacket(int sock_fd);
	int SendPacket(int sock_fd, char *buffer);

private:
	int active_connections_num;
	int max_connections_num;
	MutexLock connections_mutex;
	ClientContext *context_list;

	pthread_t recv_thread_id;
	pthread_t send_thread_id;
	
	ThreadPool<TaskInfo> thread_pool;
	MemoryPool memory_pool;
};

#endif

