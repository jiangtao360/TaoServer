#ifndef _MUTEX_H
#define _MUTEX_H

#include "header.h"

class MutexLock
{
 public:
 	MutexLock()
  	{
    	pthread_mutex_init(&mutex, NULL);
  	}

  	~MutexLock()
  	{
    	pthread_mutex_destroy(&mutex);
  	}

  	void Lock()
  	{
    	pthread_mutex_lock(&mutex);
  	}

  	void Unlock()
  	{
    	pthread_mutex_unlock(&mutex);
  	}

  	pthread_mutex_t* GetPthreadMutex() 
  	{
    	return &mutex;
  	}

 private:
  	pthread_mutex_t mutex;
};


class MutexLockGuard 
{
 public:
  	explicit MutexLockGuard(MutexLock &_mutex) : mutex(_mutex)
  	{
   		mutex.Lock();
  	}

  	~MutexLockGuard()
  	{
    	mutex.Unlock();
  	}

 private:
  	MutexLock &mutex;
};

#define MutexLockGuard(x) error "Missing guard object name"

#endif


