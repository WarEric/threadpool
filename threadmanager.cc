#include "threadmanager.h"
using std::shared_ptr;

ThreadManager::ThreadManager(int max = 10)
{
	MAX_PTHREAD_NUM = max;
	idle_pthread_num = 0;
	run_pthread_num = 0;

	if(!pthread_mutex_init(&task_queue_mutex))
	{
		perror("task_queue_mutex init fail");
		throw "task_queue_mutex init fail";
	}

	if(!pthread_mutex_init(&thread_map_mutex))
	{
		perror("thread_map_mutex init fail");
		throw "thread_map_mutex init fail";
	}

	if(!pthread_mutex_init(&idle_pthread_num_mutex))
	{
		perror("idle_pthread_num_mutex init fail");
		throw "idle_pthread_num_mutex init fail";
	}

	if(!pthread_mutex_init(&run_pthread_num_mutex))
	{
		perror("run_pthread_num_mutex init fail");
		throw "run_pthread_num_mutex init fail";
	}

	if(!pthread_mutex_init(&MAX_PTHREAD_NUM_mutex))
	{
		perror("MAX_PTHREAD_NUM_mutex init fail");
		throw "MAX_PTHREAD_NUM_mutex init fail";
	}

	if(!pthread_mutex_init(&exit_mutex))
	{
		perror("exit_mutex init fail");
		throw "exit_mutex init fail";
	}

	if(!pthread_cond_init(&task_cond_t, NULL))
	{
		perror("task_cond_t init fail");
		throw "task_cond_t init fail";
	}
}

ThreadManager::~ThreadManager()
{
	pthread_mutex_destory(&task_queue_mutex);
	pthread_mutex_destory(&thead_map_mutex);
	pthread_mutex_destory(&idle_pthread_num_mutex);
	pthread_mutex_destory(&run_pthread_num_mutex);
	pthread_mutex_destory(&MAX_PTHREAD_NUM_mutex);
	pthread_mutex_destory(&exit_mutex);

	pthread_cond_destory(&task_cond_t);

	task_queue.clear();
	thread_map.clear();
}

void ThreadManager::submit(shared_ptr<Task> task)
{
	pthread_mutex_lock(&task_queue_mutex);
	task_queue.push(task);
	int task_size = task_queue.size();
	pthread_mutex_unlock(&task_queue_mutex);

	pthread_mutex_lock(&idle_pthread_num_mutex);
	int nthread = task_size - idle_pthread_num;
	pthread_mutex_unlock(&idle_pthread_num_mutex);

	//clear dead thread
	pthread_mutex_lock(&thread_map_mutex);
	for(auto iter = thread_map_mutex.begin(); iter != thread_map_mutex.end();)
	{
		if(iter->status == Thread::SLEEP)
			thread_map_mutex.erase(iter++);
		else
			iter++;
	}
	pthread_mutex_unlock(&thread_map_mutex);

	for(int i = 0; i < nthread; i++)
	{
		shared_ptr<Thread> ptr(new Thread(*this));
		pthread_mutex_lock(&thread_map_mutex);
		thread_map[ptr->get_pid_t()] = ptr;
		ptr->startup();
		pthread_mutex_unlock(&thread_map_mutex);
	}

	pthread_cond_signal(&task_cond_t);
}

void ThreadManager::inc_idle_pthread_num()
{
	pthread_mutex_lock(&idle_pthread_num_mutex);
	idle_pthread_num++;
	pthread_mutex_unlock(&idle_pthread_num_mutex);
}

void ThreadManager::dec_idle_pthread_num()
{
	pthread_mutex_lock(&idle_pthread_num_mutex);
	idle_pthread_num--;
	pthread_mutex_unlock(&idle_pthread_num_mutex);
}

int ThreadManager::get_idle_pthread_num()
{
	pthread_mutex_lock(&idle_pthread_num_mutex);
	int num = idle_pthread_num;
	pthread_mutex_unlock(&idle_pthread_num_mutex);
	return num;
}

void ThreadManager::inc_run_pthread_num()
{
	pthread_mutex_lock(&run_pthread_num);
	run_pthread_num++;
	pthread_mutex_unlock(&run_pthread_num);
}

void ThreadManager::dec_run_pthread_num()
{
	pthread_mutex_lock(&run_pthread_num);
	run_pthread_num--;
	pthread_mutex_unlock(&run_pthread_num);
}

int ThreadManager::get_run_pthread_num()
{
	pthread_mutex_lock(&run_pthread_num);
	int num = run_pthread_num;
	pthread_mutex_unlock(&run_pthread_num);

	return num;
}

int ThreadManager::get_max_pthread_num()
{
	int max;
	pthread_mutex_lock(&MAX_PTHREAD_NUM_mutex);
	max = MAX_PTHREAD_NUM;
	pthread_mutex_unlock(&MAX_PTHREAD_NUM_mutex);

	return max;
}

void ThreadManager::set_max_pthread_num(int max)
{
	pthread_mutex_lock(&MAX_PTHREAD_NUM_mutex);
	MAX_PTHREAD_NUM = max;
	pthread_mutex_unlock(&MAX_PTHREAD_NUM_mutex);
}

void ThreadManager::add_pthread(shared_ptr<Thread> ptr)
{
	pthread_mutex_lock(&thread_map_mutex);
	thread_map[ptr->get_pid_t()] = ptr;
	pthread_mutex_unlock(&thread_map_mutex);
}

void ThreadManager::remove_pthread(pthread_t id)
{
	pthread_mutex_lock(&thread_map);
	auto iter = thread_map.find(id);
	if(iter != thread_map.end())
		thread_map.erase(iter);
	pthread_mutex_unlock(&thread_map);
}

int ThreadManager::is_exit()
{
	pthread_mutex_lock(&exit_mutex);
	exit = 1;
	pthread_mutex_unlock(&exit_mutex);
}

/**
 * return the number of threads still alive
 */
int ThreadManager::idle_exit()
{
	pthread_mutex_lock(&exit_mutex);
	eixt = 1;
	pthread_mutex_unlock(&exit_mutex);

	pthread_cond_signal(&task_cond_t);

	//clear dead thread
	pthread_mutex_lock(&thread_map_mutex);
	for(auto iter = thread_map_mutex.begin(); iter != thread_map_mutex.end();)
	{
		if(iter->status == Thread::SLEEP)
			thread_map_mutex.erase(iter++);
		else
			iter++;
	}
	pthread_mutex_unlock(&thread_map_mutex);

	int alive = get_idle_pthread_num() + get_run_pthread_num();

	return alive;
}
