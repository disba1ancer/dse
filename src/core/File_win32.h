/*
 * IOTargetwin32.h
 *
 *  Created on: 11 апр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef OS_IO_FILE_WIN32_H_
#define OS_IO_FILE_WIN32_H_

#include <mutex>
#include <swal/handle.h>
#include <system_error>
#include "File.h"
#include "ThreadPool_win32.h"

namespace dse::core {

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

class File_win32 final : private IAsyncIO {
public:
	using Result = File::Result;
	using FilePos = File::FilePos;
	using FileOff = File::FileOff;
private:
	std::error_code lastError;
	swal::File handle;
	swal::Event event {true, true};
	FilePos pos = 0; // be careful with positions and file sizes more than max value for FileOff (unreal large files)
	bool eof = false;
	util::promise<Result> pr;
	int references = 1;
	mutable std::mutex dataMtx;
public:
	File_win32();
	File_win32(ThreadPool& pool, std::u8string_view filepath, OpenMode mode);
	~File_win32();
	File_win32(const File_win32 &other) = delete;
	File_win32(File_win32 &&other) = default;
	File_win32& operator=(const File_win32 &other) = delete;
	File_win32& operator=(File_win32 &&other) = default;
	bool isValid() const;
	auto read(std::byte buf[], std::size_t size) -> Result;
	auto write(std::byte buf[], std::size_t size) -> Result;
	FilePos tell() const;
	auto seek(FilePos pos) -> std::error_code;
	auto seek(FileOff offset, StPoint rel) -> std::error_code;
	auto resize() -> std::error_code;
	bool isEOF() const;
	auto readAsync(std::byte buf[], std::size_t size) -> util::future<Result>;
	auto writeAsync(std::byte buf[], std::size_t size) -> util::future<Result>;
	bool isBusy();
	auto cancel() -> std::error_code;
	auto status() const -> std::error_code;
	void release();
private:
	virtual void complete(DWORD transfered, DWORD error) override;
	void incPtr(DWORD transfered);
	void incRefs();
	void setLastError(std::system_error& err);
};

} // namespace dse::core

#endif /* OS_IO_FILE_WIN32_H_ */
