/*
 * ThreadPool.h
 *
 *  Created on: 22 дек. 2019 г.
 *      Author: disba1ancer
 */

#ifndef THREADPOOL_H_
#define THREADPOOL_H_

#include <vector>
#include <thread>
#include <deque>
#include <functional>
#include <mutex>
#include <condition_variable>

namespace dse {
namespace threadutils {

class ThreadPool {
	std::vector<std::thread> threads;
	std::deque<std::function<bool()>> tasks;
	std::mutex mtx;
	std::condition_variable cond_var;
	bool m_isRun = false;

	void poolEntry();
public:
	void startThreads(std::size_t num);
	void terminate();
	void join();
	void addTask(std::function<bool()>&& task);
	void clearTasks();
	bool isRun();
};

} /* namespace threadutils */
} /* namespace dse */

#endif /* THREADPOOL_H_ */
