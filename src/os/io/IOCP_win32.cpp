/*
 * IOCP_win32.cpp
 *
 *  Created on: 28 апр. 2020 г.
 *      Author: disba1ancer
 */

#include "IOCP_win32.h"
#include "threadutils/unlock_guard.h"

namespace dse {
namespace os {
namespace io {

IOCP_win32::IOCP_win32() : iocp(), iocpThread(&IOCP_win32::iocpThrFunc, this)
{}

IOCP_win32::~IOCP_win32() {
	iocp.PostQueuedCompletionStatus(0, reinterpret_cast<ULONG_PTR>(INVALID_HANDLE_VALUE), nullptr);
	iocpThread.join();
}

void IOCP_win32::iocpThrFunc() noexcept {
	while (true) {
		File_win32* target = nullptr;
		auto result = iocp.GetQueuedCompletionStatus(INFINITE);
		auto handle = reinterpret_cast<HANDLE>(result.key);
		if (handle == INVALID_HANDLE_VALUE || !result.ovl) {
			break;
		} else {
			std::lock_guard lock(hndlsMtx);
			auto it = preventedCallbacks.find(handle);
			if (it == preventedCallbacks.end()) {
				threadutils::unlock_guard lock(hndlsMtx);
				target = reinterpret_cast<ExtendedOverlapped*>(result.ovl)->target;
				target->complete(result.bytesTransfered, result.error);
			} else {
				preventedCallbacks.erase(it);
			}
		}
	}
}

IOCP_win32 IOCP_win32::instance;

void IOCP_win32::attach(File_win32* target) {
	//CreateIoCompletionPort(target->handle, iocp, reinterpret_cast<ULONG_PTR>(HANDLE(target->handle)), 0);
	iocp.AssocFile(target->handle, reinterpret_cast<ULONG_PTR>(HANDLE(target->handle)));
}

void IOCP_win32::preventCallback(File_win32 *target) {
	std::lock_guard lock(hndlsMtx);
	preventedCallbacks.insert(target->handle);
}

} /* namespace io */
} /* namespace os */
} /* namespace dse */
