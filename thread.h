#ifndef _THREAD_H
#define _THREAD_H

#include "header.h"
#include "log.h"

class Thread
{
public:
	typedef void* (*thread_func_t)(void*);

	Thread(thread_func_t _thread_func,
			 void *_arg, const string &_name = string());
	~Thread();

	void Start();
	void Join();

	const string& GetName() const
	{
		return thread_name;
	}
	pthread_t GetThreadId() const
	{
		return thread_id;
	}
	bool Started () const

	{
		return started;
	}

private:
	static void* StartThread(void *thread);
	void RunInThread();

	pthread_t thread_id;
	bool started;
	thread_func_t thread_func;
	void *thread_arg;
	string thread_name;
};

#endif
