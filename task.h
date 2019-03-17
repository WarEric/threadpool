#ifndef TASK_H_H
#define TASK_H_H
#include <pthread.h>

class Task{
	public:
		Task();
		~Task();

		virtual void run();
	private:
		int 		done;//0 false, 1 true
		pthread_mutex	done_mutex;
};
#endif
