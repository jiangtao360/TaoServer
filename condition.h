#ifndef _CONDITION_H
#define _CONDITION_H

#include "mutex.h"

class Condition 
{
 public:
  	Condition(MutexLock &_mutex) : mutex(_mutex)
  	{
    	pthread_cond_init(&pcond, NULL);
  	}

  	~Condition()
  	{
    	pthread_cond_destroy(&pcond);
  	}

  	void Wait()
  	{
    	pthread_cond_wait(&pcond, mutex.GetPthreadMutex());
  	}

  	void Notify()
  	{
   	 	pthread_cond_signal(&pcond);
  	}

  	void NotifyAll()
  	{
    	pthread_cond_broadcast(&pcond);
  	}

 private:
  	MutexLock &mutex;
  	pthread_cond_t pcond;
};

#endif


