#include <iostream>
#include <string>
#include <vector>
#include <future>
#include "threadpool.h"


void print(int a, double b, const char* c, std::string d) {
    std::cout << a << b << c << d << std::endl;
}

int thread_three(int a, int b) {
    int res = a + b;
    return res;
}

void test() {
    std::cout << "hellp" << std::endl;
}

void test_packaged_task()
{
    std::packaged_task<int(int, int)> task([](int a, int b) { return a + b;  });
    std::future<int> result = task.get_future();
    task(1, 2);
    std::cout << result.get() << '\n';
}


int main(int argc, char const* argv[])
{
    ThreadPool* poll = new ThreadPool();

    auto res_three = poll->add(thread_three, 2, 3);
    // 等待计算结果
    auto three = res_three.get();
    std::cout << "Three: " << three << std::endl;

    delete poll;
    //test_packaged_task();
    return 0;
}
