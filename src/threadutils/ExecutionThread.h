/*
 * ExecutionThread.h
 *
 *  Created on: 22 дек. 2019 г.
 *      Author: disba1ancer
 */

#ifndef EXECUTIONTHREAD_H_
#define EXECUTIONTHREAD_H_

#include <thread>
#include <deque>
#include <functional>
#include <mutex>
#include <condition_variable>

namespace dse {
namespace threadutils {

class ExecutionThread {
	std::mutex mtx;
	std::deque<std::function<bool()>> tasks;
	std::thread thread;
	std::condition_variable cond_var;
	bool m_isRun = false;
	bool isExit = false;
	int result = 0;
	static thread_local ExecutionThread* currentThread;

	void threadEntry();
public:
	void run();
	int runOnCurent();
	void join();
	void terminate();
	void exit(int r);
	bool addTask(std::function<bool()>&& task);
	bool isRun();
	bool getResult();
	static ExecutionThread& getCurrentThread();
};

} /* namespace threadutils */
} /* namespace dse */

#endif /* EXECUTIONTHREAD_H_ */
