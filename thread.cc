#include "thread.h"

Thread::Thread(ThreadManager& manager):_manager(manager)
{
	status = READY;
}

Thread::~Thread()
{

}

int Thread::startup()
{
	int err = pthread_create(&tid, NULL, run, NULL);

	if(err != 0)
		perror("can't create thread");

	return err;
}

void* Thread::run(void *arg)
{
	status = SLEEP;
	inc_idle_pthread_num();

	pthread_mutex_lock(&_manager.task_queue_mutex);
	while(1)
	{
		if(queue.size() <= 0){ //no task, sleep again
			if(get_idle_pthread_num() > get_max_pthread_num())
				thread_exit();

			if(pthread_cond_wait(&_manager.task_cond_t, &_manager.task_queue_mutex) != 0)
			{
				perror("pthread cond wait for task_queue_mutex error");
				throw "pthread cond wait for task_queue_mutex error";
			}

			if(is_exit())
				thread_exit();
			else
				continue;
		}

		shared_ptr<Task> task_ptr = queue.front();
		queue.pop();
		pthread_mutex_unlock(&_manager.task_queue_mutex);

		run_task(task_ptr);

		pthread_mutex_lock(&_manager.task_queue_mutex);
	}
}

void Thread::run_task(shared_ptr<Task> ptr)
{
	_manager.dec_idle_pthread_num();
	_manager.inc_run_pthread_num();
	status = RUN;

	task_ptr->run();
	task_ptr->done = 1;
	pthread_mutex_unlock(&task_ptr->done_mutex);

	_manager.inc_idle_pthread_num();
	_manager.dec_run_pthread_num();
	status = SLEEP;
}

void ThreadManager::thread_exit()
{
	pthread_mutex_unlock(&_manager.task_queue_mutex);

	_manager.dec_idle_pthread_num();
	status = DEAD;
	pthread_exit(nullptr);
}
