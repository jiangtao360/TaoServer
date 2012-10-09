#include "epoll.h"

Epoll::Epoll() : epoll_events(NULL)
{

}

Epoll::~Epoll()
{
	if(epoll_events != NULL)
		delete [] epoll_events;
}

void Epoll::Initialize()
{
	 Netbase::Initialize();

	char buffer[256];

	if(get_profile_string("epoll", "init_nevents", NULL, buffer, 
		256, GNSERVER_CONFIG_PATH) <= 0)
	{
		global_logger.WriteLog(Logger::ERROR, -1, 
			"Get init_nevents failed when initia Epoll");
	}
	nevents= atoi(buffer);

	if(get_profile_string("epoll", "max_events", NULL, buffer, 
		256, GNSERVER_CONFIG_PATH) <= 0)
	{
		global_logger.WriteLog(Logger::ERROR, -1, 
			"Get max_events failed when initia Epoll");
	}
	max_nevents = atoi(buffer);

	if(get_profile_string("epoll", "epoll_timeout", NULL, buffer, 
		256, GNSERVER_CONFIG_PATH) <= 0)
	{
		global_logger.WriteLog(Logger::ERROR, -1, 
			"Get epoll_timeout failed when initia Epoll");
	}
	epoll_timeout= atoi(buffer); 
}

void Epoll::Startup()
{
	Netbase::Startup();

	try
	{
		epoll_events = new struct epoll_event[nevents];
	}
	catch(const std::bad_alloc &ex)
	{
		global_logger.WriteLog(Logger::ERROR, -1, 
			"Epoll allocate space failed");
	}

	EpollCreate();

	if(!EpollAdd(socket_fd))
	{
		global_logger.WriteLog(Logger::ERROR, -1, 
			"Epoll add event of listen socket failed");
	}
}

void Epoll::EpollCreate()
{
	epoll_fd = epoll_create(32000);

	if(epoll_fd == -1)
	{
		perror("Epoll create failed");
		global_logger.WriteLog(Logger::ERROR, errno, 
			"Create epoll file descripter failed");
	}
}

bool Epoll::EpollAdd(int _socket_fd)
{
	epoll_event ev;

	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = _socket_fd;

	if(epoll_ctl(epoll_fd, EPOLL_CTL_ADD, _socket_fd, &ev) == -1)
	{
		global_logger.WriteLog(Logger::WARNING, errno, 
			"Add epoll event failed at socket %d", _socket_fd);
		return false;
	}
	else
		return true;
}

bool Epoll::EpollDel(int _socket_fd)
{
	epoll_event ev;

	if(epoll_ctl(epoll_fd, EPOLL_CTL_DEL, _socket_fd, &ev) == -1)
	{
		global_logger.WriteLog(Logger::WARNING, errno, 
			"Delete epoll event failed at socket %d", _socket_fd);
		return false;
	}
	else
		return true;
}

bool Epoll::EpollModIn(int _socket_fd)
{
	epoll_event ev;
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = _socket_fd;

	if(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, _socket_fd, &ev) < 0)
	{
		global_logger.WriteLog(Logger::WARNING, errno, 
			"Modify epoll event to epoll in failed at socket %d", _socket_fd);
		return false;
	}
	else
		return true;
}

bool Epoll::EpollModOut(int _socket_fd)
{
	epoll_event ev;
	ev.events = EPOLLOUT | EPOLLET;
	ev.data.fd = _socket_fd;

	if(epoll_ctl(epoll_fd, EPOLL_CTL_MOD, _socket_fd, &ev) < 0)
	{
		global_logger.WriteLog(Logger::WARNING, errno, 
			"Modify epoll event to epoll out failed at socket %d", _socket_fd);
		return false;
	}
	else
		return true;
}

int Epoll::EpollWait()
{
	int ready = epoll_wait(epoll_fd, epoll_events, nevents, epoll_timeout);

	if(ready == -1)
	{
		global_logger.WriteLog(Logger::WARNING, errno,
			"Epoll wait failed");
		return -1;
	}
	else
	{
		if(ready == nevents && nevents < max_nevents)
		{
			int new_nevents = nevents << 1;
			struct epoll_event *new_epoll_events = NULL;

			try
			{
				new_epoll_events = new struct epoll_event[new_nevents];
			}
			catch(const std::bad_alloc &ex)
			{
				global_logger.WriteLog(Logger::WARNING, -1, 
					"Reallocate epoll event space failed");
				return ready;
			}
			memcpy(new_epoll_events, epoll_events, nevents*sizeof(struct epoll_event));
			delete [] epoll_events;
			epoll_events = new_epoll_events;
		}
		
		return ready;
	}
}

void Epoll::EpollClose()
{
	if(close(epoll_fd) == -1)
	{
		perror("Epoll close");
		global_logger.WriteLog(Logger::ERROR, errno, 
			"Close epoll file descripter failed");
	}
}


