/*
 * IOCP_win32.h
 *
 *  Created on: 28 апр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef OS_IO_IOCP_WIN32_H_
#define OS_IO_IOCP_WIN32_H_

#include "../win32.h"
#include <thread>
#include "File_win32.h"
#include <set>
#include <mutex>

namespace dse {
namespace os {
namespace io {

class IOCP_win32 {
	HandleWrapper iocp;
	std::thread iocpThread;
	std::set<HANDLE> preventedCallbacks;
	std::mutex hndlsMtx;
	void iocpThrFunc() noexcept;
	IOCP_win32();
	~IOCP_win32();
public:
	static IOCP_win32 instance;
	void attach(File_win32* target);
	void preventCallback(File_win32* target);
};

} /* namespace io */
} /* namespace os */
} /* namespace dse */

#endif /* OS_IO_IOCP_WIN32_H_ */
