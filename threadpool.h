#ifndef THREAD_POOL_H_H
#define THREAD_POOL_H_H
#include <map>
#include <queue>
#include <pthread.h>
#include <queue>
#include "task.h"

class ThreadPool{
	public:
		ThreadPool(int max = 10);
		~ThreadPool();

		static ThreadPool* get_instance(int num);
		void submit(Task *task);

	private:
		static void* thread_func(void *arg);
		static void  run_task(ThreadPool *pool, Task *task);
		static void  thread_exit(ThreadPool* pool);

		void	inc_idle_pthread_num();
		void	dec_idle_pthread_num();
		int	get_idle_pthread_num();
		
		void	inc_run_pthread_num();
		void	dec_run_pthread_num();
		int	get_run_pthread_num();

		int	get_max_pthread_num();
		void	set_max_pthread_num(int max);

		int	is_exit();
		int	idle_exit();

		int	pool_exit();

		std::queue<Task*> task_queue;

		int idle_pthread_num;
		int run_pthread_num;
		int MAX_PTHREAD_NUM;
		int exit;//0 no, 1 yes

		pthread_mutex_t task_queue_mutex;
		pthread_mutex_t idle_pthread_num_mutex;
		pthread_mutex_t run_pthread_num_mutex;
		pthread_mutex_t MAX_PTHREAD_NUM_mutex;
		pthread_mutex_t exit_mutex;

		pthread_cond_t task_cond_t;
};
#endif
