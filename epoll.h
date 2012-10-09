#ifndef _EPOLL_H
#define _EPOLL_H

#include "netbase.h"

class Epoll : public Netbase
{
public:
	Epoll();
	~Epoll();

	void Initialize();
	void Startup();

	void EpollCreate();
	bool EpollAdd(int _socket_fd);
	bool EpollDel(int _socket_fd);
	bool EpollModIn(int _socket_fd);
	bool EpollModOut(int _socket_fd);
	int EpollWait();
	void EpollClose();

protected:
	int epoll_fd;
	struct epoll_event *epoll_events;
	int nevents;
	int max_nevents;
	int epoll_timeout;
};

#endif


