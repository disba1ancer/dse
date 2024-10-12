#ifndef RAWFILE_H
#define RAWFILE_H

#include "../status.h"
#include <dse/util/TagInvoke.h>
#include <dse/util/execution.h>
#include <coroutine>
#include <cstddef>

namespace dse::core {

enum class OpenMode : unsigned {
    Read = 0x1U,
    Write = 0x2U,
    Append = 0x4U,
    Clear = 0x8U, // clear file contents when opened
    Existing = 0x10U, // open only existing
};

enum class StPoint {
    Start,
    Current,
    End,
};

class API_DSE_CORE File;

}

namespace dse::core::raw_file_impl {

struct FileOpResult {
    std::size_t transferred;
    Status ecode;
};

using Callback = util::FunctionPtr<void(std::size_t, Status)>;

struct TagRead;
struct TagWrite;

template <typename op>
struct FileOpBuf;

template <>
struct FileOpBuf<TagRead> {
    using Type = void;
};

template <>
struct FileOpBuf<TagWrite> {
    using Type = const void;
};

template <typename TagOp>
using FileOpBufT = typename FileOpBuf<TagOp>::Type;

template <typename TagOp, typename R>
class FileOpstate;

template <typename R>
void dse_TagInvoke(util::TagT<util::Start>, FileOpstate<TagRead, R>& opstate);
template <typename R>
void dse_TagInvoke(util::TagT<util::Start>, FileOpstate<TagWrite, R>& opstate);

template <typename TagOp, typename R>
class FileOpstate {
    File *file;
    FileOpBufT<TagOp> *buf;
    std::size_t size;
    std::remove_cvref_t<R> recv;
    void callback(std::size_t size, Status error)
    {
        util::SetValue(std::move(recv), size, error);
    }
public:
    FileOpstate(
        File *file,
        FileOpBufT<TagOp> *buf,
        std::size_t size,
        std::remove_cvref_t<R> recv
        ) : file(file), buf(buf), size(size), recv(recv)
    {}
    friend void dse_TagInvoke<TagOp, R>(util::TagT<util::Start>, FileOpstate& opstate);
};

template <typename TagOp>
class file_awaiter;

template <typename TagOp>
class FileSender {
    File *file;
    FileOpBufT<TagOp> *buf;
    std::size_t size;
public:
    template <template<typename ...> typename T>
    using ValueTypes = T<std::size_t, Status>;
    using ErrorType = std::exception_ptr;
    static constexpr bool SendsDone = false;
    friend class file_awaiter<TagOp>;
    FileSender(File *file, FileOpBufT<TagOp>* buf, std::size_t size) :
        file(file), buf(buf), size(size)
    {}
    template <util::ReceiverOf<std::size_t, Status> R>
    friend auto dse_TagInvoke(util::TagT<util::Connect>, FileSender&& sndr, R&& recv)
        -> FileOpstate<TagOp, R>
    {
        return { sndr.file, sndr.buf, sndr.size, std::forward<R>(recv) };
    }
};

///////////////////////////////////////////////////////////////////

template <template <typename...> typename T, std::size_t r, typename... TT>
using remove_last_args
    = decltype([]<size_t... i>(std::index_sequence<i...>) -> T<std::tuple_element_t<i, std::tuple<TT...>>...> {
      }(std::make_index_sequence<sizeof...(TT) - r>{}));

template <typename T> struct return_traits;

template <> struct return_traits<Status> {
    static auto to_status(const Status &st) -> Status { return st; }
    static auto from_callback(const std::size_t &, const Status &st) -> Status
    {
        return st;
    }
};

template <> struct return_traits<FileOpResult> {
    static auto to_status(const FileOpResult &r) -> Status { return r.ecode; }
    static auto from_callback(const std::size_t &size, const Status &st)
        -> FileOpResult
    {
        return {size, st};
    }
};

template <auto op, auto check, typename... Args> struct file_sender3;

template <auto op, auto check, typename... Args> struct file_awaiter3 {
    using sender_t = file_sender3<op, check, Args...>;
    using result_t = std::invoke_result_t<decltype(op), Args..., Callback>;
    file_awaiter3(sender_t &sender) :
        sender(sender)
    {}
    bool await_ready()
    {
        if (!std::apply(check, sender.args)) {
            return false;
        }
        result = std::apply(
            [](Args &...args) {
                return std::invoke(op, std::forward<Args>(args)..., Callback{});
            },
            sender.args
        );
        return true;
    }
    bool await_suspend(std::coroutine_handle<> handle)
    {
        resumable = handle;
        result_t r = std::apply(
            [this](Args &&...args) {
                return std::invoke(
                    op, std::forward<Args>(args)...,
                    util::FunctionPtr{*this, util::fn_tag<&file_awaiter3::callback>}
                );
            },
            std::move(sender.args)
        );
        if (return_traits<result_t>::to_status(r)
            != status::Code::PendingOperation)
        {
            result = r;
            return false;
        }
        return true;
    }
    auto await_resume() -> result_t { return result; }

private:
    void callback(std::size_t size, Status st)
    {
        result = return_traits<result_t>::from_callback(size, st);
        resumable.resume();
    }

    sender_t &sender;
    std::coroutine_handle<> resumable;
    result_t result;
};

template <auto op, auto check, typename... Args> struct file_sender3 {
    file_sender3(Args &&...args) :
        args(args...)
    {}

    friend struct file_awaiter3<op, check, Args...>;

    friend auto operator co_await(file_sender3 &&sndr)
        -> file_awaiter3<op, check, Args...>
    {
        return {sndr};
    }

    friend auto operator co_await(file_sender3 &sndr)
        -> file_awaiter3<op, check, Args...>
    {
        return {sndr};
    }

private:
    std::tuple<Args...> args;
};

} // namespace dse::core::raw_file_impl

#endif // RAWFILE_H
