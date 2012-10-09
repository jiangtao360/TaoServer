#ifndef _WORKER_H
#define _WORKER_H

#include "struct.h"

class Worker
{
public:
	Worker();
	~Worker();
	
	void ProcessTask(TaskInfo &task);
	
private:

};

#endif
