#ifndef TASK_H_H
#define TASK_H_H
#include <pthread.h>

class ThreadPool;

class Task{
	friend class ThreadPool;
	public:
		Task();
		~Task();

		void wait();
		virtual void run() = 0;
	private:
		int 		done;//0 false, 1 true
		pthread_mutex_t	done_mutex;
};
#endif
