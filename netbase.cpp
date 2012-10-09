#include "netbase.h"

Netbase::Netbase()
{

}

Netbase::~Netbase()
{

}

void Netbase::Initialize()
{
	char buffer[256];

	if(get_profile_string("netbase", "port", NULL, buffer, 
		256, GNSERVER_CONFIG_PATH) <= 0)
	{
		global_logger.WriteLog(Logger::ERROR, -1, 
			"Get port failed when initia Netbase");
	}
	port_num = atoi(buffer);

	if(get_profile_string("netbase", "max_listen_num", NULL, buffer, 
		256, GNSERVER_CONFIG_PATH) <= 0)
	{
		global_logger.WriteLog(Logger::ERROR, -1, 
			"Get max_listen_num failed when initia Netbase");
	}
	max_listen_num = atoi(buffer);

	if(get_profile_string("netbase", "server_ip", NULL, buffer,
		256, GNSERVER_CONFIG_PATH) <= 0)
	{
		global_logger.WriteLog(Logger::ERROR, -1, 
			"Get server_ip failed when initia Netbase");
	}
	strcpy(server_ip, buffer);
}

void Netbase::Startup()
{
	CreateSocket();
	
	SetNonBlocking(socket_fd);
	
	SetReuseAddr(socket_fd);
	
	Bind();
	
	Listen();
}

void Netbase::CreateSocket()
{
	socket_fd= socket(AF_INET, SOCK_STREAM, 0);
	if(socket_fd== -1)
	{
		global_logger.WriteLog(Logger::ERROR, errno, "Create listen socket failed");	
	}
}

void Netbase::Connect()
{
	bzero(&socket_address, sizeof(socket_address));
	socket_address.sin_family = AF_INET;
	socket_address.sin_addr.s_addr = inet_addr(server_ip);
	socket_address.sin_port = htons(port_num);

	if(connect(socket_fd, (struct sockaddr*)&socket_address, sizeof(socket_address)) == -1)
	{
		global_logger.WriteLog(Logger::ERROR, errno, "Connect server failed");
	}
}

void Netbase::Bind()
{
	bzero(&socket_address, sizeof(socket_address));
	socket_address.sin_family = AF_INET;
	socket_address.sin_addr.s_addr = htonl(INADDR_ANY);
	socket_address.sin_port = htons(port_num);

	if(bind(socket_fd, (struct sockaddr*)&socket_address, 
		    sizeof(struct sockaddr)) == -1)
	{
		global_logger.WriteLog(Logger::ERROR, errno, "Blind address failed");
	}
}

void Netbase::Listen()
{
	if(listen(socket_fd, max_listen_num) == -1)
	{
		global_logger.WriteLog(Logger::ERROR, errno, "Listen failed");
	}
}

int Netbase::Accept()
{
	socklen_t client_len = sizeof(socket_address);

	int client_socket = accept(socket_fd, (struct sockaddr*)&socket_address, 
							&client_len);

	if(client_socket == -1)
	{
		global_logger.WriteLog(Logger::INFO, errno, "%s",
			"Accept socket failed");
		return -1;
	}
	else
	{
		global_logger.WriteLog(Logger::DEBUG, -1,
			"Accept ip address:%s, port number:%d, client socket:%d",
			inet_ntoa(socket_address.sin_addr), ntohs(socket_address.sin_port),
			client_socket);
		return client_socket;
	}
}

void Netbase::CloseSocket()
{
	if(close(socket_fd) == -1)
	{
		global_logger.WriteLog(Logger::ERROR, errno, "Close listen socket failed");
	}
}

int Netbase::Send(int _socket_fd, const char *buf, int len)
{
	int ret;

	if((ret = send(_socket_fd, buf, len, 0)) == -1)
	{
		if(errno == EAGAIN || errno == EWOULDBLOCK)
		{
			return -2;
		}
		else
		{
			global_logger.WriteLog(Logger::INFO, errno, "Send failed");
			return -1;
		}
	}
	else
		return ret;
}

int Netbase::Recv(int _socket_fd, char *buf, int len)
{
	int ret;

	if((ret = recv(_socket_fd, buf, len, 0)) == -1)
	{
		if(errno == EAGAIN)
		{
			return -2;
		}
		else
		{
			global_logger.WriteLog(Logger::INFO, errno, "Recv failed");
			return -1;
		}
	}
	else
		return ret;
}

bool Netbase::SetNonBlocking(int _socket_fd)
{
	int flags = fcntl(_socket_fd, F_GETFL);

	if(flags == -1)
	{
		global_logger.WriteLog(Logger::WARNING, errno, "Set non-blocking failed");
		return false;
	}

	if(fcntl(_socket_fd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		global_logger.WriteLog(Logger::WARNING, errno, "Set non-blocking failed");
		return false;
	}

	return true;
}

void Netbase::SetReuseAddr(int _socket_fd)
{
	int opt = SO_REUSEADDR;
	if(setsockopt(_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		global_logger.WriteLog(Logger::ERROR, errno, "Set reuse address failed");
	}
}

void Netbase::SetRecvBuf(int _socket_fd, int size)
{
	if(setsockopt(_socket_fd, SOL_SOCKET, SO_RCVBUF, 
		(const char*)&size, sizeof(size)) == -1)
	{
		global_logger.WriteLog(Logger::ERROR, errno, "Set receive buffer failed");
	}
}

void Netbase::SetSendBuf(int _socket_fd, int size)
{
	if(setsockopt(_socket_fd, SOL_SOCKET, SO_SNDBUF, 
		(const char*)&size, sizeof(size)) == -1)
	{
		global_logger.WriteLog(Logger::ERROR, errno, "Set send buffer failed");

	}
}

void Netbase::SetKeepAlive(int _socket_fd, int keepalive_time, 
						   int keepalive_intvl, int keepalive_probes)
{
	int keepalive = 1;

	if(setsockopt(_socket_fd, SOL_SOCKET, SO_KEEPALIVE, (void*)&keepalive, sizeof(keepalive)) == -1)
		global_logger.WriteLog(Logger::ERROR, errno, "Set keep alive failed");

	if(setsockopt(_socket_fd, SOL_TCP, TCP_KEEPIDLE, (void*)&keepalive_time, sizeof(keepalive_time)) == -1)
		global_logger.WriteLog(Logger::ERROR, errno, "Set keep alive failed");

	if(setsockopt(_socket_fd, SOL_TCP, TCP_KEEPINTVL, (void*)&keepalive_intvl, sizeof(keepalive_intvl)) == -1)
		global_logger.WriteLog(Logger::ERROR, errno, "Set keep alive failed");

	if(setsockopt(_socket_fd, SOL_TCP, TCP_KEEPCNT, (void*)&keepalive_probes, sizeof(keepalive_probes)) == -1)
		global_logger.WriteLog(Logger::ERROR, errno, "Set keep alive failed");
}


