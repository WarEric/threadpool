#include <cstdio>
#include "task.h"

Task::Task()
{
	if(pthread_mutex_init(&done_mutex, nullptr) != 0)
	{
		perror("init done_mutex fail");
		throw "init done_mutex fail";
	}
	
	if(pthread_mutex_lock(&done_mutex))
	{
		perror("lock done_mutex fail");
		throw "lock done_mutex fail";
	}
}

Task::~Task()
{
	pthread_mutex_destroy(&done_mutex);
}

void Task::wait()
{
	pthread_mutex_lock(&done_mutex);//wait util task done
	pthread_mutex_unlock(&done_mutex);
}
