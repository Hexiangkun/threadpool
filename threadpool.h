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
	std::vector<std::thread> threads;	//�����̶߳���
	std::queue<std::function<void()>> tasks;	//�������
	std::mutex tasks_mtx;		//������
	std::condition_variable cv;	//��������
	bool stop;	//�̳߳ؿ���
};

template<typename T, typename Y>
auto add(T v1, Y v2) -> decltype(v1 + v2)	//����ֵ���ͺ���
{
	return v1 + v2;
}


//FΪ����ģ�壬ArgsΪ�ɱ����
template<class F, class... Args>
auto threadpool::add(F&& f, Args&&... args) ->std::future<typename std::result_of<F(Args...)>::type>
{
	using return_type = typename std::result_of<F(Args...)>::type;//��F(Args...)�ķ���ֵ��������Ϊreturn_type����
	auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
	std::future<return_type> res = task->get_future();
	{
		std::unique_lock<std::mutex> lock(tasks_mtx);
		if (stop) {
			throw std::runtime_error("Threadpool has already stopped!");
		}
		tasks.emplace([task]() {(*task)(); });
	}
	cv.notify_one();//���һ�����ڵȴ����ѵ��̵߳�����̬
	return res;
}
/*
	std::future���Ա���ĳһ�����Ľ����std::futureͨ�����̵߳Ľ���ŵ�һ��future�����У���һ���߳��п���wait��get��������Ľ��
	std::packaged_task�ṩ��ĳ�����ķ�װ��Ȼ��ͬ��/�첽���иú������������ͨ�����������ڲ���һ��future������
	std::make_shared���Է���һ��ָ�����͵�std::shared_ptr
*/

