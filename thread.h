#ifndef THREAD_H_H
#define THREAD_H_H
#include <memory>
#include <pthread.h>
#include "threadmanager.h"

class ThreadManager;

class Thread{
	public:
		Thread(ThreadManager& manger);
		virtual ~Thread();

		int		startup();
		void		cancel();
		pthread_t	get_pid_t();

		enum STATUS{READY, RUN, SLEEP, DEAD};
	private:
		void*	run(void *arg);
		void	run_task(std::shared_ptr<Task> ptr);
		void 	thread_exit();

		ThreadManageru& _manager;
		int		status;
		pthread_t	tid;
};
#endif
