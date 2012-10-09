#include "thread.h"

Thread::Thread(thread_func_t _thread_func, void *_arg, const string &_name) : 
		thread_id(0), started(false), thread_func(_thread_func), 
		thread_arg(_arg), thread_name(_name)
{
	
}

Thread::~Thread()
{

}

void Thread::Start()
{
	if(started)
	{
		global_logger.WriteLog(Logger::ERROR, -1, 
			"Start failed, thread has started");
	}
	started = true;

	int ret = pthread_create(&thread_id, NULL, StartThread, this);
	if(ret != 0)
	{
		global_logger.WriteLog(Logger::ERROR, errno, 
			"Start thread failed");
	}
}

void Thread::Join()
{
	if(!started)
	{
		global_logger.WriteLog(Logger::ERROR, -1, 
			"Join failed, thread has stopped");
	}

	if(pthread_join(thread_id, NULL) != 0)
	{
		global_logger.WriteLog(Logger::ERROR, errno, "Join thread failed");
	}
}

void* Thread::StartThread(void * thread)
{
	Thread *pthis = static_cast<Thread*>(thread);

	pthis->RunInThread();
	return NULL;
}

void Thread::RunInThread()
{
	thread_func(thread_arg);
}

