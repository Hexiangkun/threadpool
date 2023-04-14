#pragma once
#include<vector>
#include<thread>
#include<queue>
#include<functional>
#include<mutex>
#include<condition_variable>
#include<future>

class ThreadPool
{
private:
	std::vector<std::thread> task_threads;
	std::queue<std::function<void()>> tasks_queue;
	std::mutex task_mtx;
	std::condition_variable cv;
	bool stop;
public:
	ThreadPool(int size=std::thread::hardware_concurrency()):stop(false)
	{
		for (int i = 0; i < size; i++) {
			task_threads.emplace_back(std::thread([this]() {
				while (true)
				{
					std::function<void()> task;
					{
						std::unique_lock<std::mutex> lock(task_mtx);
						cv.wait(lock, [this]() {
							return stop || !tasks_queue.empty();
							});
						if (stop && tasks_queue.empty()) {
							return;
						}
						task = std::move(tasks_queue.front());
						tasks_queue.pop();
					}
					task();
				}
			}));
		}
	}
	~ThreadPool()
	{
		{
			std::unique_lock<std::mutex> lock(task_mtx);
			stop = true;
		}
		cv.notify_all();
		for (std::thread &th: task_threads) {
			if (th.joinable()) {
				th.join();
			}
		}
	}

	template<class F,class... Args>
	auto add(F&& f, Args&&... args)->std::future<typename std::result_of<F(Args...)>::type>;
};
  

template<class F, class... Args>
auto ThreadPool::add(F&& f, Args&&...args) -> std::future<typename std::result_of<F(Args...)>::type>
{
	using return_type = typename std::result_of<F(Args...)>::type;           
	                                 
	auto task = std::make_shared<std::packaged_task<return_type()>> (std::bind(std::forward<F>(f), std::forward<Args>(args)...));

	std::future<return_type> res = task->get_future();
	{
		std::unique_lock<std::mutex> lock(task_mtx);
		if (stop) {
			throw std::runtime_error("ThreadPool has already stop!");
		}
		tasks_queue.emplace([task]() {(*task)(); });
	}
	cv.notify_one();
	return res;
}







#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>
#include <iostream>

#include "util.h"

class threadpool
{
public:
	explicit threadpool(int size= std::thread::hardware_concurrency());
	DISALLOW_COPY_AND_MOVE(threadpool);
	~threadpool();
	//void add(std::function<void()>);

	template<class F,class... Args>
	auto add(F&& f, Args&&... args)->std::future<typename std::result_of<F(Args...)>::type>;

private:
	std::vector<std::thread> threads;	//工作线程队列
	std::queue<std::function<void()>> tasks;	//任务队列
	std::mutex tasks_mtx;		//互斥锁
	std::condition_variable cv;	//条件变量
	bool stop;	//线程池开关
};

template<typename T, typename Y>
auto add(T v1, Y v2) -> decltype(v1 + v2)	//返回值类型后置
{
	return v1 + v2;
}


//F为函数模板，Args为可变参数
template<class F, class... Args>
auto threadpool::add(F&& f, Args&&... args) ->std::future<typename std::result_of<F(Args...)>::type>
{
	using return_type = typename std::result_of<F(Args...)>::type;//将F(Args...)的返回值类型声明为return_type类型
	auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
	std::future<return_type> res = task->get_future();
	{
		std::unique_lock<std::mutex> lock(tasks_mtx);
		if (stop) {
			throw std::runtime_error("Threadpool has already stopped!");
		}
		tasks.emplace([task]() {(*task)(); });
	}
	cv.notify_one();//解除一个正在等待唤醒的线程的阻塞态
	return res;
}
/*
	std::future可以保存某一函数的结果，std::future通常把线程的结果放到一个future对象中，另一个线程中可以wait或get这个将来的结果
	std::packaged_task提供对某函数的封装，然后同步/异步运行该函数，函数结果通常保存在其内部的一个future对象中
	std::make_shared可以返回一个指定类型的std::shared_ptr
*/

