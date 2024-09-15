#ifndef CACHEDFILE_H
#define CACHEDFILE_H

#include "dse/core/File.h"
#include <coroutine>

namespace dse::core::cached_file_impl {

using raw_file_impl::FileOpResult;
using Callback = File::Callback;

template <template <typename...> typename T, std::size_t r, typename... TT>
using remove_last_args
    = decltype([]<size_t... i>(std::index_sequence<i...>)
               -> T<std::tuple_element_t<i, std::tuple<TT...>>...> {
      }(std::make_index_sequence<sizeof...(TT) - r>{}));

template <typename T>
struct return_traits;

template <>
struct return_traits<Status> {
    static auto to_status(const Status& st) -> Status { return st; }
    static auto from_callback(const std::size_t&, const Status& st) -> Status
    {
        return st;
    }
};

template <>
struct return_traits<FileOpResult> {
    static auto to_status(const FileOpResult& r) -> Status { return r.ecode; }
    static auto from_callback(const std::size_t& size, const Status& st)
        -> FileOpResult
    {
        return {size, st};
    }
};

template <auto op, auto check, typename... Args>
struct file_sender3;

template <auto op, auto check, typename... Args>
struct file_awaiter3 {
    using sender_t = file_sender3<op, check, Args...>;
    using result_t = std::invoke_result_t<decltype(op), Args..., Callback>;
    file_awaiter3(sender_t& sender) :
        sender(sender)
    {}
    bool await_ready()
    {
        if (!std::apply(check, sender.args)) {
            return false;
        }
        result = std::apply(
            [](Args&... args) {
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
            [this](Args&&... args) {
                return std::invoke(
                    op, std::forward<Args>(args)...,
                    util::FunctionPtr{*this, util::fnTag<&file_awaiter3::callback>}
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

    sender_t& sender;
    std::coroutine_handle<> resumable;
    result_t result;
};

template <auto op, auto check, typename... Args>
struct file_sender3 {
    file_sender3(Args&&... args) :
        args(args...)
    {}

    friend class file_awaiter3<op, check, Args...>;

    friend auto operator co_await(file_sender3&& sndr
    ) -> file_awaiter3<op, check, Args...>
    {
        return {sndr};
    }

    friend auto operator co_await(file_sender3& sndr
    ) -> file_awaiter3<op, check, Args...>
    {
        return {sndr};
    }

private:
    std::tuple<Args...> args;
};

template <auto op, auto check, typename... Args>
auto make_sender(Args&&... args) -> file_sender3<op, check, Args...>;

template <auto op, typename... Args>
auto make_sender(Args&&... args)
    -> file_sender3<op, [](Args...) -> bool { return false; }, Args...>;

} // namespace dse::core::cached_file_impl

#endif // CACHEDFILE_H
