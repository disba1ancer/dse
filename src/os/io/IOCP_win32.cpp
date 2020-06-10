/*
 * IOCP_win32.cpp
 *
 *  Created on: 28 апр. 2020 г.
 *      Author: disba1ancer
 */

#include "IOCP_win32.h"
#include "../../threadutils/unlock_guard.h"

namespace dse {
namespace os {
namespace io {

IOCP_win32::IOCP_win32() : iocp(CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0)), iocpThread(&iocpThrFunc, this)
{}

IOCP_win32::~IOCP_win32() {
	PostQueuedCompletionStatus(iocp, 0, reinterpret_cast<ULONG_PTR>(INVALID_HANDLE_VALUE), nullptr);
	iocpThread.join();
}

void IOCP_win32::iocpThrFunc() noexcept {
	while (true) {
		DWORD transfered;
		ULONG_PTR key;
		OVERLAPPED* ovl;
		File_win32* target = nullptr;
		auto result = GetQueuedCompletionStatus(iocp, &transfered, &key, &ovl, INFINITE);
		auto error = (result ? ERROR_SUCCESS : GetLastError());
		if (result || ovl) {
			auto handle = reinterpret_cast<HANDLE>(key);
			if (handle == INVALID_HANDLE_VALUE || !ovl) {
				break;
			} else {
				std::lock_guard lock(hndlsMtx);
				auto it = preventedCallbacks.find(handle);
				if (it == preventedCallbacks.end()) {
					threadutils::unlock_guard lock(hndlsMtx);
					target = reinterpret_cast<ExtendedOverlapped*>(ovl)->target;
					target->complete(transfered, error);
				} else {
					preventedCallbacks.erase(it);
				}
			}
		} else if (error == ERROR_ABANDONED_WAIT_0) {
			break;
		}
	}
}

IOCP_win32 IOCP_win32::instance;

void IOCP_win32::attach(File_win32* target) {
	CreateIoCompletionPort(target->handle, iocp, reinterpret_cast<ULONG_PTR>(HANDLE(target->handle)), 0);
}

void IOCP_win32::preventCallback(File_win32 *target) {
	std::lock_guard lock(hndlsMtx);
	preventedCallbacks.insert(target->handle);
}

} /* namespace io */
} /* namespace os */
} /* namespace dse */
