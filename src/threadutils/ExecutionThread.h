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
#include "TaskState.h"
#include <list>
#include <optional>

namespace dse {
namespace threadutils {

class ExecutionThread {
public:
	typedef std::function<TaskState()> Task;
	class TaskID {
		friend class ExecutionThread;
		std::list<Task>::iterator taskid;
		TaskID(std::list<Task>::iterator taskid);
	public:
		TaskID() = default;
	};
private:
	std::mutex mtx;
	std::list<Task> tasks;
	std::deque<TaskID> queuedTasks;
	std::thread thread;
	std::condition_variable cond_var;
	bool m_isRun = false;
	bool isExit = false;
	int result = 0;
	TaskID currentTask;
	static thread_local ExecutionThread* currentThread;

	void threadEntry();
public:
	void run();
	int runOnCurent();
	void join();
	void terminate();
	void exit(int r);
	std::optional<TaskID> addTask(Task&& task);
	bool isRun();
	bool getResult();
	static ExecutionThread& getCurrentThread();
	void yieldTasks();
	bool resumeTask(const TaskID&);
	TaskID getCurrentTask();
};

} /* namespace threadutils */
} /* namespace dse */

#endif /* EXECUTIONTHREAD_H_ */
