/*
 * IOTargetwin32.h
 *
 *  Created on: 11 апр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef OS_IO_FILE_WIN32_H_
#define OS_IO_FILE_WIN32_H_

#include "os/win32.h"
#include <mutex>
#include <swal/handle.h>
#include "File.h"
#include "threadutils/ThreadPool_win32.h"

namespace dse {
namespace os {
namespace io {

class IOCP_win32;

/*class HandleWrapper {
	HANDLE handle;
public:
	HandleWrapper(HANDLE handle = NULL);
	HandleWrapper(const HandleWrapper&) = delete;
	HandleWrapper(HandleWrapper&& orig);
	HandleWrapper& operator =(const HandleWrapper&) = delete;
	HandleWrapper& operator =(HandleWrapper&& orig);
	~HandleWrapper();
	operator HANDLE() const;
};*/

class File_win32;

struct ExtendedOverlapped {
	OVERLAPPED ovl;
	File_win32* target;
};

class File_win32 final : private threadutils::IAsyncIO {
//	ExtendedOverlapped ovl = {};
	DWORD lastError = ERROR_SUCCESS;
	swal::File handle;
	swal::Event event {true, true};
	FilePos pos = 0; // be careful with positions and file sizes more than max value for FileOff (unreal large files)
	bool eof = false;
	bool error = false;
	DWORD lastTransfered = 0;
	std::function<void()> callback;
	int references = 1;
	mutable std::mutex dataMtx;
public:
	File_win32();
	File_win32(threadutils::ThreadPool& pool, std::u8string_view filepath, OpenMode mode);
	~File_win32();
	File_win32(const File_win32 &other) = delete;
	File_win32(File_win32 &&other) = default;
	File_win32& operator=(const File_win32 &other) = delete;
	File_win32& operator=(File_win32 &&other) = default;
	bool isValid() const;
	void read(std::byte buf[], std::size_t size);
	void write(std::byte buf[], std::size_t size);
	FilePos tell() const;
	void seek(FilePos pos);
	void seek(FileOff offset, StPoint rel);
	void resize();
	bool isEOF() const;
	bool isError() const;
	Result getLastResult() const;
	std::size_t getTransfered() const;
	std::u8string getResultString() const;
	void read_async(std::byte buf[], std::size_t size, std::function<void()>&& onFinish);
	void write_async(std::byte buf[], std::size_t size, std::function<void()>&& onFinish);
	bool isBusy();
	void cancel();
	void release();
	friend class IOCP_win32;
private:
	virtual void complete(DWORD transfered, DWORD error) override;
	void incPtr(DWORD transfered);
	void incRefs();
};

} /* namespace io */
} /* namespace os */
} /* namespace dse */

#endif /* OS_IO_FILE_WIN32_H_ */
