#include "threadpool.h"
#include <iostream>

/*
template <class Predicate>
void wait (unique_lock<mutex>& lck, Predicate pred);
如果第二个参数不满足，那么wait将解锁互斥量并堵塞到本行，
直到某一个线程调用notify_one或notify_all为止，
被唤醒后，wait重新尝试获取互斥量，如果得不到，线程会卡在这里，直到获取到互斥量，然后继续判断第二个参数，
如果表达式为false，wait对互斥量解锁，然后休眠，如果为true，则进行后面的操作。
*/
/*
emplace_back在实现时，直接在容器尾部创建这个元素，省去了拷贝和移动元素的过程
push_back首先会创建这个元素，然后将这个元素拷贝或者移动到容器中去
*/

/*
lambda表达式本身就是一个匿名函数，
[捕获列表](参数列表)->returntype {函数体}
*/
threadpool::threadpool(int size):stop(false)
{
	for (int i = 0; i < size; i++) {
		threads.emplace_back(std::thread([this]() {
			while (true) {
				std::function<void()> task;
				{
					std::unique_lock<std::mutex> lock(tasks_mtx);
					cv.wait(lock, [this]() {//使当前线程进入阻塞状态
						return stop || !tasks.empty();	//线程池停止或者任务队列不为空，返回true
						});
					if (stop && tasks.empty()) {
						return;
					}
					task = tasks.front();//取任务
					tasks.pop();		//从任务队列移除
				}
				task();				//执行任务
			}
		}));
	}
}

threadpool::~threadpool()
{
	{
		std::unique_lock<std::mutex> lock(tasks_mtx);
		stop = true;
	}
	cv.notify_all();//唤醒所有线程
	for (std::thread& th : threads) {
		if (th.joinable()) {//当前线程是一个可执行线程
			th.join();//使主线程在此阻塞，等待子线程运行结束并回收其资源，再往下运行。
		}
	}
}

//void threadpool::add(std::function<void()> func)
//{
//	std::cout << "void threadpool::add(std::function<void()> func)" << std::endl;
//	{
//		std::unique_lock<std::mutex> lock(tasks_mtx);
//		if (stop) {
//			throw std::runtime_error("ThreadPool already stop!");
//		}
//		tasks.emplace(func);
//	}
//	cv.notify_one();
//}
