#ifndef _NETBASE_H
#define _NETBASE_H

#include "header.h"
#include "utility.h"

class Netbase
{
public:
	Netbase();
	virtual ~Netbase();

	virtual void Initialize();
	virtual void Startup();

	void CreateSocket();
	void Connect();
	void Bind();
	void Listen();
	int Accept();
	void CloseSocket();
	int Send(int _socket_fd, const char *buf, int len);
	int Recv(int _socket_fd, char *buf, int len);


	bool SetNonBlocking(int _socket_fd);
	void SetReuseAddr(int _socket_fd);
	void SetRecvBuf(int _socket_fd, int size);
	void SetSendBuf(int _socket_fd, int size);
	void SetKeepAlive(int _socket_fd, int keepalive_time, 
					  int keepalive_intvl, int keepalive_probes);

protected:
	int socket_fd;
	int port_num;
	char server_ip[16];
	int max_listen_num;
	struct sockaddr_in socket_address;
};

#endif


