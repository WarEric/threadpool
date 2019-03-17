#include <iostream>
#include <memory>
#include "threadmanager.h"
#include "task.h"
using namespace std;

class MyTask : public{
	public:
		MyTask(string str):name(str){}

		run(){
			for(int i = 0; i < 10; i++)
				cout << name << " = " << i << endl;
		}

	private:
		string name;
}

int main()
{
	ThreadManager threadmanager(10);
	shared_ptr<Task> ptr(new MyTask("Eric"));
	threadmanager.submit(shared_ptr);
	shared_ptr->wait();

	return 0;
}
