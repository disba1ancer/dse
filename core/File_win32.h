/*
 * IOTargetwin32.h
 *
 *  Created on: 11 апр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef FILE_WIN32_H_
#define FILE_WIN32_H_

#include <mutex>
#include <swal/handle.h>
#include <system_error>
#include <dse/core/File.h>
#include "IOContext_win32.h"

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

class File_win32 final : private OVERLAPPED {
public:
	using FilePos = File::FilePos;
	using FileOff = File::FileOff;

	File_win32();
	File_win32(IOContext& ctx, std::u8string_view filepath, OpenMode mode);
	~File_win32();
	File_win32(const File_win32 &other) = delete;
	File_win32(File_win32 &&other) = default;
	File_win32& operator=(const File_win32 &other) = delete;
	File_win32& operator=(File_win32 &&other) = default;
	bool IsValid() const;
	auto Read(std::byte buf[], std::size_t size) -> impl::FileOpResult;
	auto Write(const std::byte buf[], std::size_t size) -> impl::FileOpResult;
	FilePos Tell() const;
	auto Seek(FilePos pos) -> Status;
	auto Seek(FileOff offset, StPoint rel) -> Status;
	auto Resize() -> Status;
	bool IsEOF() const;
	auto ReadAsync(std::byte buf[], std::size_t size, const File::Callback& cb) -> Status;
	auto WriteAsync(const std::byte buf[], std::size_t size, const File::Callback& cb) -> Status;
	bool IsBusy();
	auto Cancel() -> Status;
    auto GetStatus() const -> Status;
	void Release();
private:
    static void Complete(OVERLAPPED* ovl, DWORD transfered, DWORD error);
	void Complete(DWORD transfered, DWORD error);
	void IncPtr(DWORD transfered);
	void IncRefs();
	bool SetLastError(std::system_error& err);

    std::shared_ptr<IOContext_impl> context;
	int lastError;
	swal::File handle;
//	swal::Event event {true, true};
	FilePos pos = 0; // be careful with positions and file sizes more than max value for FileOff (unreal large files)
	bool eof = false;
	File::Callback cb;
	int references = 1;
	mutable std::mutex dataMtx;
};

} // namespace dse::core

#endif /* FILE_WIN32_H_ */
