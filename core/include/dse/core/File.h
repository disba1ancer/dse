/*
 * IOTarget.h
 *
 *  Created on: 11 апр. 2020 г.
 *      Author: disba1ancer
 */

#ifndef DSE_CORE_FILE_H_
#define DSE_CORE_FILE_H_

#include "detail/RawFile.h"
#include "IOContext.h"
// #include "detail/impexp.h"
// #include "status.h"
// #include <coroutine>
#include <dse/util/enum_bitwise.h>
// #include <dse/util/execution.h>
// #include <dse/util/functional.h>
// #include <string_view>

namespace dse::core {

#ifdef _WIN32

class File_win32;

typedef File_win32 File_impl;

#endif

class API_DSE_CORE File {
public:
	using FilePos = std::uint_least64_t;
	using FileOff = std::int_least64_t;
	using Callback = raw_file_impl::Callback;
public:
	File();
	File(IOContext& ctx, std::u8string_view filepath, OpenMode mode);
    File(File&& oth);
    File& operator=(File&& oth);
	~File(); // may cause undefined behavior if called while async operation
    auto Read(void* buf, std::size_t size) -> raw_file_impl::FileOpResult;
    auto Write(const void* buf, std::size_t size) -> raw_file_impl::FileOpResult;
	auto Resize() -> Status;
    auto ReadAsync(void* buf, std::size_t size, const Callback& cb) -> raw_file_impl::FileOpResult;
    auto WriteAsync(const void* buf, std::size_t size, const Callback& cb) -> raw_file_impl::FileOpResult;
    auto ReadAsync(void* buf, std::size_t size)
        -> raw_file_impl::file_sender3<
            &File::ReadAsync,
            [](decltype(this) self, void* buf, std::size_t size) {
                return false;
            },
            decltype(this), decltype(buf), decltype(size)>
    {
        return {
            std::forward<decltype(this)>(this),
            std::forward<decltype(buf)>(buf), std::forward<decltype(size)>(size)
        };
    }
    auto WriteAsync(const void* buf, std::size_t size)
        -> raw_file_impl::file_sender3<
            &File::WriteAsync,
            [](decltype(this) self, const void* buf, std::size_t size) {
                return false;
            },
            decltype(this), decltype(buf), decltype(size)>
    {
        return {
            std::forward<decltype(this)>(this),
            std::forward<decltype(buf)>(buf),
            std::forward<decltype(size)>(size)
        };
    }
    auto Cancel() -> Status;
	auto Seek(FilePos pos) -> Status;
	auto Seek(FileOff offset, StPoint rel) -> Status;
    auto OpenStatus() const -> Status;
    auto Tell() const -> FilePos;
private:
    util::impl_ptr<File_impl> impl;
};

namespace raw_file_impl {

template <typename R>
void dse_TagInvoke(util::TagT<util::Start>, FileOpstate<TagRead, R>& opstate)
{
    opstate.file->ReadAsync(
        opstate.buf, opstate.size,
        {opstate, util::fnTag<&FileOpstate<TagRead, R>::callback>}
    );
}

template <typename R>
void dse_TagInvoke(util::TagT<util::Start>, FileOpstate<TagWrite, R>& opstate)
{
    opstate.file->WriteAsync(
        opstate.buf, opstate.size,
        {opstate, util::fnTag<&FileOpstate<TagWrite, R>::callback>}
    );
}

template <typename TagOp>
auto do_op(
    File* file, FileOpBufT<TagOp>* buf, std::size_t size, File::Callback cb
) -> FileOpResult;

template <>
inline auto do_op<TagRead>(
    File* file, void* buf, std::size_t size, File::Callback cb
) -> FileOpResult
{
    return file->ReadAsync(buf, size, cb);
}

template <>
inline auto do_op<TagWrite>(
    File* file, const void* buf, std::size_t size, File::Callback cb
) -> FileOpResult
{
    return file->WriteAsync(buf, size, cb);
}

template <typename TagOp>
struct file_awaiter {
    file_awaiter(FileSender<TagOp>&& sender) :
        sender(std::move(sender))
    {}
    bool await_ready() { return false; }
    void callback(std::size_t size, Status st)
    {
        result.transferred = size;
        result.ecode = st;
        resumable.resume();
    }
    bool await_suspend(std::coroutine_handle<> handle)
    {
        FileOpResult r = do_op<TagOp>(
            sender.file, sender.buf, sender.size,
            {*this, util::fnTag<&file_awaiter::callback>}
        );
        if (r.ecode != status::Code::PendingOperation) {
            result = r;
            return false;
        }
        resumable = handle;
        return true;
    }
    auto await_resume() -> FileOpResult { return result; }

private:
    FileSender<TagOp> sender;
    FileOpResult result;
    std::coroutine_handle<> resumable;
};

template <typename TagOp>
auto operator co_await(FileSender<TagOp>&& sndr) -> file_awaiter<TagOp>
{
    return {std::move(sndr)};
}

template <typename TagOp>
auto operator co_await(FileSender<TagOp>& sndr) -> file_awaiter<TagOp>
{
    return {std::move(sndr)};
}

} // namespace

} // namespace dse::core

namespace dse::util {

template <>
struct enable_enum_bitwise<enum core::OpenMode> : public std::true_type {};

} // namespace dse::util

#endif /* DSE_CORE_FILE_H_ */
