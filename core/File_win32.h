/*
 * IOTargetwin32.h
 *
 *  Created on: 11 апр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef FILE_WIN32_H_
#define FILE_WIN32_H_

#include <swal/handle.h>
#include <system_error>
#include <dse/core/File.h>

namespace dse::core {

class File_win32;

class File_win32 final : private OVERLAPPED {
public:
	using FilePos = File::FilePos;
	using FileOff = File::FileOff;
    using Callback = File::Callback;

    File_win32();
	File_win32(IOContext& ctx, std::u8string_view filepath, OpenMode mode);
	~File_win32();
	File_win32(const File_win32 &other) = delete;
	File_win32(File_win32 &&other) = default;
    auto operator=(const File_win32 &other) -> File_win32& = delete;
    auto operator=(File_win32 &&other) -> File_win32& = default;
    auto Read(void* buf, std::size_t size) -> raw_file_impl::FileOpResult;
    auto Write(const void* buf, std::size_t size) -> raw_file_impl::FileOpResult;
    auto Tell() const -> FilePos;
	auto Seek(FilePos pos) -> Status;
	auto Seek(FileOff offset, StPoint rel) -> Status;
    auto Resize() -> Status;
    auto ReadAsync(void* buf, std::size_t size, const Callback& cb) -> raw_file_impl::FileOpResult;
    auto WriteAsync(const void* buf, std::size_t size, const Callback& cb) -> raw_file_impl::FileOpResult;
	auto Cancel() -> Status;
    auto OpenStatus() const -> Status;
private:
    static void Complete(OVERLAPPED* ovl, DWORD transfered, DWORD error);
    static auto SysErrToStatus(std::system_error& err) -> Status;
	void Complete(DWORD transfered, DWORD error);
    void IncPtr(DWORD transfered);

    swal::File handle;
    IOContext_impl* context = nullptr;
    FilePos pos = 0; // be careful with positions and file sizes more than max value for FileOff (unreal large files)
    Callback cb = nullptr;
    Status openError = Make(status::Code::Unexpected);
    bool append:1 = false;
};

} // namespace dse::core

#endif /* FILE_WIN32_H_ */
