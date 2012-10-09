#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H

#include "thread.h"
#include "worker.h"
#include "mutex.h"
#include "condition.h"

template<typename T>
class ThreadPool
{
public:
	ThreadPool(const string &_name = string());
	~ThreadPool();

	void Initialize();
	void StartThreads(int _threads_count);
	void StopThreads();
	
	void RunTask(T &_task);
	void TakeTask(T &_task);
	void PostResult(T &_task);
	void TakeResult(T &_task);
	
	const string& GetName() const { return pool_name; }
	bool Started() const { return started; }

private:
	static void* WorkThreadProc(void *_arg);
	void RunInThread();



private:
	string pool_name;
	Thread **threads;
	int threads_count;
	int max_threads_count;
	bool started; 
	
	deque<T> task_queue;
	MutexLock task_mutex;
	Condition task_cond;
	
	deque<T> result_queue;
	MutexLock result_mutex;
	Condition result_cond;

	Worker worker;
};

#include "thread_pool.cpp"

#endif
