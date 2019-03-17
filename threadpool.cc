#include <unistd.h>
#include "threadpool.h"

ThreadPool::ThreadPool(int max)
{
	MAX_PTHREAD_NUM = max;
	idle_pthread_num = 0;
	run_pthread_num = 0;

	if(pthread_mutex_init(&task_queue_mutex, nullptr))
	{
		perror("task_queue_mutex init fail");
		throw "task_queue_mutex init fail";
	}

	if(pthread_mutex_init(&idle_pthread_num_mutex, nullptr))
	{
		perror("idle_pthread_num_mutex init fail");
		throw "idle_pthread_num_mutex init fail";
	}

	if(pthread_mutex_init(&run_pthread_num_mutex, nullptr))
	{
		perror("run_pthread_num_mutex init fail");
		throw "run_pthread_num_mutex init fail";
	}

	if(pthread_mutex_init(&MAX_PTHREAD_NUM_mutex, nullptr))
	{
		perror("MAX_PTHREAD_NUM_mutex init fail");
		throw "MAX_PTHREAD_NUM_mutex init fail";
	}

	if(pthread_mutex_init(&exit_mutex, nullptr))
	{
		perror("exit_mutex init fail");
		throw "exit_mutex init fail";
	}

	if(pthread_cond_init(&task_cond_t, nullptr))
	{
		perror("task_cond_t init fail");
		throw "task_cond_t init fail";
	}
}

ThreadPool::~ThreadPool()
{
	pthread_mutex_destroy(&task_queue_mutex);
	pthread_mutex_destroy(&idle_pthread_num_mutex);
	pthread_mutex_destroy(&run_pthread_num_mutex);
	pthread_mutex_destroy(&MAX_PTHREAD_NUM_mutex);
	pthread_mutex_destroy(&exit_mutex);

	pthread_cond_destroy(&task_cond_t);
}

ThreadPool* ThreadPool::get_instance(int num)
{
	static ThreadPool* instance = new ThreadPool(num);
	return instance;
}

void ThreadPool::submit(Task *task)
{
	pthread_mutex_lock(&task_queue_mutex);
	task_queue.push(task);
	int task_size = task_queue.size();
	pthread_mutex_unlock(&task_queue_mutex);

	pthread_mutex_lock(&idle_pthread_num_mutex);
	int nthread = task_size - idle_pthread_num;
	pthread_mutex_unlock(&idle_pthread_num_mutex);

	for(int i = 0; i < nthread; i++)
	{
		pthread_t tid;
		pthread_create(&tid, nullptr, ThreadPool::thread_func, this);
	}

	pthread_cond_signal(&task_cond_t);
}

void* ThreadPool::thread_func(void *arg)
{
	ThreadPool* pool = (ThreadPool*)arg;
	pool->inc_idle_pthread_num();

	pthread_mutex_lock(&pool->task_queue_mutex);
	while(1)
	{
		if((pool->task_queue).size() <= 0){ //no task, sleep again
			//too many idle thread
			if(pool->get_idle_pthread_num() > pool->get_max_pthread_num())
				thread_exit(pool);

			if(pthread_cond_wait(&pool->task_cond_t, &pool->task_queue_mutex) != 0)
			{
				perror("pthread cond wait for task_queue_mutex error");
				throw "pthread cond wait for task_queue_mutex error";
			}

			if(pool->is_exit())
				thread_exit(pool);
			else
				continue;
		}

		Task *task = (pool->task_queue).front();
		(pool->task_queue).pop();

		pthread_mutex_unlock(&pool->task_queue_mutex);
		run_task(pool, task);
		pthread_mutex_lock(&pool->task_queue_mutex);
	}
}

void ThreadPool::run_task(ThreadPool *pool, Task *task)
{
	pool->dec_idle_pthread_num();
	pool->inc_run_pthread_num();

	task->run();
	task->done = 1;
	pthread_mutex_unlock(&task->done_mutex);

	pool->dec_run_pthread_num();
	pool->inc_idle_pthread_num();
}

void ThreadPool::thread_exit(ThreadPool *pool)
{
	pthread_mutex_unlock(&pool->task_queue_mutex);
	pool->dec_idle_pthread_num();
	pthread_exit(nullptr);
}

void ThreadPool::inc_idle_pthread_num()
{
	pthread_mutex_lock(&idle_pthread_num_mutex);
	idle_pthread_num++;
	pthread_mutex_unlock(&idle_pthread_num_mutex);
}

void ThreadPool::dec_idle_pthread_num()
{
	pthread_mutex_lock(&idle_pthread_num_mutex);
	idle_pthread_num--;
	pthread_mutex_unlock(&idle_pthread_num_mutex);
}

int ThreadPool::get_idle_pthread_num()
{
	pthread_mutex_lock(&idle_pthread_num_mutex);
	int num = idle_pthread_num;
	pthread_mutex_unlock(&idle_pthread_num_mutex);
	return num;
}

void ThreadPool::inc_run_pthread_num()
{
	pthread_mutex_lock(&run_pthread_num_mutex);
	run_pthread_num++;
	pthread_mutex_unlock(&run_pthread_num_mutex);
}

void ThreadPool::dec_run_pthread_num()
{
	pthread_mutex_lock(&run_pthread_num_mutex);
	run_pthread_num--;
	pthread_mutex_unlock(&run_pthread_num_mutex);
}

int ThreadPool::get_run_pthread_num()
{
	pthread_mutex_lock(&run_pthread_num_mutex);
	int num = run_pthread_num;
	pthread_mutex_unlock(&run_pthread_num_mutex);

	return num;
}

int ThreadPool::get_max_pthread_num()
{
	pthread_mutex_lock(&MAX_PTHREAD_NUM_mutex);
	int max = MAX_PTHREAD_NUM;
	pthread_mutex_unlock(&MAX_PTHREAD_NUM_mutex);

	return max;
}

void ThreadPool::set_max_pthread_num(int max)
{
	pthread_mutex_lock(&MAX_PTHREAD_NUM_mutex);
	MAX_PTHREAD_NUM = max;
	pthread_mutex_unlock(&MAX_PTHREAD_NUM_mutex);
}

int ThreadPool::is_exit()
{
	pthread_mutex_lock(&exit_mutex);
	int res = exit;
	pthread_mutex_unlock(&exit_mutex);
	return res;
}

/**
 * return the number of threads still alive
 */
int ThreadPool::pool_exit()
{
	pthread_mutex_lock(&exit_mutex);
	exit = 1;
	pthread_mutex_unlock(&exit_mutex);

	pthread_cond_signal(&task_cond_t);
	usleep(500000);

	int alive = get_idle_pthread_num() + get_run_pthread_num();
	return alive;
}
