//class template ThreadPool implementation file

template<typename T>
ThreadPool<T>::ThreadPool(const string &_name) : pool_name(_name), threads(NULL), 
							threads_count(0), max_threads_count(0), started(false),
							task_mutex(), task_cond(task_mutex), 
							result_mutex(), result_cond(result_mutex)
{

}

template<typename T>
ThreadPool<T>::~ThreadPool()
{
	if(threads != NULL)
	{
		for(int i=0; i<threads_count; i++)
		{
			delete threads[i];
		}

		delete [] threads;
	}
}

template<typename T>
void ThreadPool<T>::Initialize()
{
	char buffer[256];
	
	if(get_profile_string("thread_pool", "max_threads_count", NULL, buffer, 
		256, GNSERVER_CONFIG_PATH) <= 0)
	{
		global_logger.WriteLog(Logger::ERROR, -1, 
			"Get max_threads_count failed when initia thread pool");
	}
	max_threads_count = atoi(buffer);
}

template<typename T>
void ThreadPool<T>::StartThreads(int _threads_count)
{
	if(started)
	{
		global_logger.WriteLog(Logger::ERROR, -1, 
		"Start failed, thread pool have started");
	}
	started = true;

	threads_count = _threads_count < max_threads_count ? _threads_count : max_threads_count;

	try
	{
		threads = new Thread*[threads_count];

		for(int i=1; i<=threads_count; i++)
		{
		
			char id[8];
			sprintf(id, "%d", i);
			
			threads[i] = new Thread(WorkThreadProc, this, pool_name+id);
			threads[i]->Start();
		}
	}
	catch(const std::bad_alloc &ex)
	{
		global_logger.WriteLog(Logger::ERROR, -1, 
			"Allocate thread pool space failed");
	}
}

template<typename T>
void ThreadPool<T>::StopThreads()
{
	if(!started)
	{
		global_logger.WriteLog(Logger::ERROR, -1, 
			"Stop failed, thread poll has stopped");
	}
	started = false;

	result_cond.NotifyAll();

	for(int i=0; i<threads_count; i++)
	{
		threads[i]->Join();
	}

	return 0;
}


template<typename T>
void* ThreadPool<T>::WorkThreadProc(void *arg)
{
	ThreadPool *pthis = static_cast<ThreadPool*> (arg);

	pthis->RunInThread();

	return NULL;
}

template<typename T>
void ThreadPool<T>::RunInThread()
{
	while(started)
	{
		T task;
		TakeTask(task);
		
		worker.ProcessTask(task);
		
		PostResult(task);
	}
}

template<typename T>
void ThreadPool<T>::RunTask(T &_task)
{
	try
	{
		MutexLockGuard lock(task_mutex);
	
		task_queue.push_back(_task);
		task_cond.Notify();
	}
	catch(const std::bad_alloc &ex)
	{
		global_logger.WriteLog(Logger::ERROR, -1, 
			"Push back task_queue failed");
	}
}

template<typename T>
void ThreadPool<T>::TakeTask(T &_task)
{
	MutexLockGuard lock(task_mutex);

	while(task_queue.empty() && started)
	{
		task_cond.Wait();
	}

	if(!task_queue.empty())
	{
		_task = task_queue.front();
		task_queue.pop_front();
	}
}

template<typename T>
void ThreadPool<T>::PostResult(T &task)
{
	try 
	{
		MutexLockGuard lock(result_mutex);

		result_queue.push_back(task);
		result_cond.Notify();
	}
	catch(const std::bad_alloc &ex)
	{
		global_logger.WriteLog(Logger::ERROR, -1, 
			"Push back result_queue failed");
	}
}

template<typename T>
void ThreadPool<T>::TakeResult(T &_task)
{
	MutexLockGuard lock(result_mutex);

	while(result_queue.empty() && started)
	{
		result_cond.Wait();
	}

	if(!result_queue.empty())
	{
		_task = result_queue.front();
		result_queue.pop_front();
	}
}


