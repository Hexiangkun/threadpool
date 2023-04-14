#include "threadpool.h"
#include <iostream>

/*
template <class Predicate>
void wait (unique_lock<mutex>& lck, Predicate pred);
����ڶ������������㣬��ôwait�����������������������У�
ֱ��ĳһ���̵߳���notify_one��notify_allΪֹ��
�����Ѻ�wait���³��Ի�ȡ������������ò������̻߳Ῠ�����ֱ����ȡ����������Ȼ������жϵڶ���������
������ʽΪfalse��wait�Ի�����������Ȼ�����ߣ����Ϊtrue������к���Ĳ�����
*/
/*
emplace_back��ʵ��ʱ��ֱ��������β���������Ԫ�أ�ʡȥ�˿������ƶ�Ԫ�صĹ���
push_back���Ȼᴴ�����Ԫ�أ�Ȼ�����Ԫ�ؿ��������ƶ���������ȥ
*/

/*
lambda���ʽ�������һ������������
[�����б�](�����б�)->returntype {������}
*/
threadpool::threadpool(int size):stop(false)
{
	for (int i = 0; i < size; i++) {
		threads.emplace_back(std::thread([this]() {
			while (true) {
				std::function<void()> task;
				{
					std::unique_lock<std::mutex> lock(tasks_mtx);
					cv.wait(lock, [this]() {//ʹ��ǰ�߳̽�������״̬
						return stop || !tasks.empty();	//�̳߳�ֹͣ����������в�Ϊ�գ�����true
						});
					if (stop && tasks.empty()) {
						return;
					}
					task = tasks.front();//ȡ����
					tasks.pop();		//����������Ƴ�
				}
				task();				//ִ������
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
	cv.notify_all();//���������߳�
	for (std::thread& th : threads) {
		if (th.joinable()) {//��ǰ�߳���һ����ִ���߳�
			th.join();//ʹ���߳��ڴ��������ȴ����߳����н�������������Դ�����������С�
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
