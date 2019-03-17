#ifndef THEAD_MANAGER_H_H
#define THEAD_MANAGER_H_H
#include <memory>
#include <pthread.h>
#include <queue>
#include "thread.h"
class Thread;

class ThreadManager{
	friend class Thread;
	public:
		ThreadManager(int max = 10);
		~ThreadManager();
		void submit(std::shared_ptr<Task> task);

	private:
		void	inc_idle_pthread_num();
		void	dec_idle_pthread_num();
		int	get_idle_pthread_num();
		
		void	inc_run_pthread_num();
		void	dec_run_pthread_num();
		int	get_run_pthread_num();

		int	get_max_pthread_num();
		void	set_max_pthread_num(int max);

		void	add_pthread(shared_ptr<Thread> ptr);
		void	remove_pthread(pthread_t id);

		int	is_exit();
		int	idle_exit();

		std::queue<std::shared_ptr<Task>> task_queue;
		std::map<pthread_t, std::shared_ptr<Thread>> thread_map;

		int idle_pthread_num;
		int run_pthread_num;
		int MAX_PTHREAD_NUM;
		int exit;//0 no, 1 yes

		pthread_mutex_t task_queue_mutex;
		pthread_mutex_t thread_map_mutex;
		pthread_mutex_t idle_pthread_num_mutex;
		pthread_mutex_t run_pthread_num_mutex;
		pthread_mutex_t MAX_PTHREAD_NUM_mutex;
		pthread_mutex_t exit_mutex;

		pthread_cond_t task_cond_t;
};
#endif
