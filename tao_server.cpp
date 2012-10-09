#include "tao_server.h"

TaoServer::TaoServer() : active_connections_num(0), max_connections_num(0), 
	context_list(NULL)
{
	
}

TaoServer::~TaoServer()
{
	if(context_list != NULL)
		delete [] context_list;
}

void TaoServer::Initialize() 
{
	Epoll::Initialize();
	thread_pool.Initialize();
	memory_pool.Initialize();

	char buffer[256];

	if(get_profile_string("tao_server", "max_connections_num", NULL, buffer, 
		256, GNSERVER_CONFIG_PATH) <= 0)
	{
		global_logger.WriteLog(Logger::ERROR, -1, 
			"Get max_connections_num failed when initia GnServer");
	}
	max_connections_num = atoi(buffer);
}


void TaoServer::Startup()
{
	Epoll::Startup();

	try
	{
		context_list = new ClientContext[max_connections_num+16];
	}
	catch(const std::bad_alloc &ex)
	{
		global_logger.WriteLog(Logger::ERROR, -1, 
			"Allocate client context space failed");
	}

	StartRecvThread();

	StartSendThread();

	int processors_num = get_processors_num();
	int threads_num = (processors_num-2) < 2 ? 2 : (processors_num-2);
	thread_pool.StartThreads(threads_num);
}

void TaoServer::StartRecvThread()
{
	int ret = pthread_create(&recv_thread_id, NULL, RecvThreadProc, (void*)this);

	if(ret != 0)
	{
		global_logger.WriteLog(Logger::ERROR, -1, "Start recv thread failed");
	}
}

void* TaoServer::RecvThreadProc(void *arg)
{
	TaoServer *pThis = (TaoServer*)arg;
	pThis->ProcessRecv();
	return 0;
}

void TaoServer::StartSendThread()
{
	int ret = pthread_create(&send_thread_id, NULL, SendThreadProc, (void*)this);

	if(ret != 0)
	{
		global_logger.WriteLog(Logger::ERROR, -1, "Start send thread failed");
	}
}

void* TaoServer::SendThreadProc(void *arg)
{
	TaoServer *pThis = (TaoServer*)arg;
	pThis->ProcessSend();
	return 0;
}

void TaoServer::ProcessRecv()
{
	while(true)
	{
		int nready = 0;
		int client_socket = -1;

		nready = EpollWait();

		for(int i=0; i<nready; i++)
		{
			if(epoll_events[i].events & EPOLLIN)
			{
				if(epoll_events[i].data.fd == socket_fd)
				{
					while(true)
					{
						if( (client_socket = Accept()) == -1)
							break;

						if(!SetNonBlocking(client_socket))
						{
							close(client_socket);
							break;
						}
						if(!EpollAdd(client_socket))
						{
							close(client_socket);
							break;
						}
						
						InitClientContext(client_socket);
		
						if(active_connections_num > max_connections_num)
						{
							DisconnectClient(client_socket);
							break;
						}
					}
				}
				else
				{
					int sock_fd = epoll_events[i].data.fd; 
					
					while(true)
					{
						if(context_list[sock_fd].buffer == NULL)
						{
							context_list[sock_fd].buffer = static_cast<char*> 
								(memory_pool.Malloc());
						}
						int ret = RecvPacket(sock_fd);

						if(ret == -1 || ret == 0)
						{
							DisconnectClient(sock_fd);
							break;
						}
						else if(ret == -2)
						{
							break;
						}
						else
						{
							TaskInfo task(sock_fd, context_list[sock_fd].buffer);
							thread_pool.RunTask(task);
							context_list[sock_fd].buffer = NULL;
						}
					}
				}
			}
			else
			{
				
			}
		}
	}
}


void TaoServer::InitClientContext(int client_socket)
{
	MutexLockGuard lock(connections_mutex);
	active_connections_num++;
	
	context_list[client_socket].client_socket = client_socket;
	context_list[client_socket].buffer = NULL;
	context_list[client_socket].last_recv_byte = 0;
}

void TaoServer::DisconnectClient(int sock_fd, bool is_graceful)
{
	if(!is_graceful)
	{
		struct linger liner_struct;
		liner_struct.l_onoff = 1;
		liner_struct.l_linger = 0;
		setsockopt(sock_fd, SOL_SOCKET, SO_LINGER, 
			      (void*)&liner_struct, sizeof(liner_struct));
	}

	EpollDel(sock_fd);
	close(sock_fd);

	MutexLockGuard lock(connections_mutex);
	active_connections_num--;

	context_list[sock_fd].client_socket= -1;
	context_list[sock_fd].buffer= NULL;
	context_list[sock_fd].last_recv_byte = 0;
	
	global_logger.WriteLog(Logger::DEBUG, -1, 
		"-----------Disconnect client: %d----------", sock_fd);
}

int TaoServer::RecvPacket(int sock_fd)
{
	int n = 0;
	int ret = 0;
	char *buffer = context_list[sock_fd].buffer;
	Packet *pack = reinterpret_cast<Packet*> (buffer);

	if(context_list[sock_fd].last_recv_byte == 0)
	{
		while(true)
		{
			if((ret = Recv(sock_fd, buffer+n, 
				PRE_RECV_LEN-n)) > 0)
			{
				n += ret;
				if(n == PRE_RECV_LEN)
					break;
			}
			else
			{
				context_list[sock_fd].last_recv_byte = n;

				return ret;
			}
		}

		while(true)
		{
			if((ret = Recv(sock_fd, buffer+n, pack->packet_len-n)) > 0)
			{
				n += ret;
				if(n == pack->packet_len)
					break;
			}
			else
			{
				context_list[sock_fd].last_recv_byte = n;

				return ret;
			}
		}
	}
	else
	{
		n = context_list[sock_fd].last_recv_byte;

		if(n < PRE_RECV_LEN)
		{
			while(true)
			{
				if((ret = Recv(sock_fd, buffer+n, 
					PRE_RECV_LEN-n)) > 0)
				{
					n += ret;
					if(n == PRE_RECV_LEN)
						break;
				}
				else
				{
					context_list[sock_fd].last_recv_byte = n;

					return ret;
				}
			}

			while(true)
			{
				if((ret = Recv(sock_fd, buffer+n, pack->packet_len-n)) > 0)
				{
					n += ret;
					if(n == pack->packet_len)
						break;
				}
				else
				{
					context_list[sock_fd].last_recv_byte = n;

					return ret;
				}
			}
		}
		else
		{
			while(true)
			{
				if((ret = Recv(sock_fd, buffer+n, pack->packet_len-n)) > 0)
				{
					n += ret;
					if(n == pack->packet_len)
						break;
				}
				else
				{
					context_list[sock_fd].last_recv_byte = n;

					return ret;
				}
			}
		}
	}
	
	context_list[sock_fd].last_recv_byte = 0;
	return n;	
}


void TaoServer::ProcessSend()
{
	TaskInfo result_info;

	while(true)
	{
		thread_pool.TakeResult(result_info);

		int ret = SendPacket(result_info.task_id, result_info.ptask_info);

		if(ret == -1)
		{
			DisconnectClient(result_info.task_id);
		}

		memory_pool.Free(result_info.ptask_info);
	}
}

int TaoServer::SendPacket(int sock_fd, char * buffer)
{
	int ret = 0;
	int n = 0;
	Packet *pack = reinterpret_cast<Packet*> (buffer);
	
	while(true)
	{
		if((ret = Send(sock_fd, buffer+n, pack->packet_len-n)) > 0)
		{
			n += ret;
			if(n == pack->packet_len)
				break;
		}
		else if(ret == -2)
		{
			//usleep(SEND_WAIT_TIME);
		}
		else
		{
			return ret;
		}
	}
	
	return n;
}


