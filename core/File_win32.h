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

namespace dse::core {

class File_win32;

// struct ExtendedOverlapped {
// 	OVERLAPPED ovl;
// 	File_win32* target;
// };

class File_win32 final : private OVERLAPPED {
public:
	using FilePos = File::FilePos;
	using FileOff = File::FileOff;

	File_win32();
	File_win32(IOContext& ctx, std::u8string_view filepath, OpenMode mode);
	~File_win32();
	File_win32(const File_win32 &other) = delete;
	File_win32(File_win32 &&other) = default;
    auto operator=(const File_win32 &other) -> File_win32& = delete;
    auto operator=(File_win32 &&other) -> File_win32& = default;
	auto Read(std::byte buf[], std::size_t size) -> impl::FileOpResult;
	auto Write(const std::byte buf[], std::size_t size) -> impl::FileOpResult;
    auto Tell() const -> FilePos;
	auto Seek(FilePos pos) -> Status;
	auto Seek(FileOff offset, StPoint rel) -> Status;
    auto Resize() -> Status;
    auto ReadAsync(std::byte buf[], std::size_t size, const File::Callback& cb) -> impl::FileOpResult;
    auto WriteAsync(const std::byte buf[], std::size_t size, const File::Callback& cb) -> impl::FileOpResult;
	auto Cancel() -> Status;
    auto OpenStatus() const -> Status;
private:
    static void Complete(OVERLAPPED* ovl, DWORD transfered, DWORD error);
    static auto SysErrToStatus(std::system_error& err) -> Status;
	void Complete(DWORD transfered, DWORD error);
    void IncPtr(DWORD transfered);

    union {
        IOContext_impl* context;
        int openError;
    };
	swal::File handle;
    FilePos pos = 0; // be careful with positions and file sizes more than max value for FileOff (unreal large files)
    bool append;
    File::Callback cb;
};

} // namespace dse::core

#endif /* FILE_WIN32_H_ */
