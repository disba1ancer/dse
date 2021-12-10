#ifndef COROUTINE_H
#define COROUTINE_H

// Hack for Qt Creator from https://bugreports.qt.io/browse/QTCREATORBUG-24634?focusedCommentId=565543&page=com.atlassian.jira.plugin.system.issuetabpanels:comment-tabpanel#comment-565543
// Bypass GCC / MSVC coroutine guards when using clang code model
#if defined(__GNUC__) && defined(__clang__) && !defined(__cpp_impl_coroutine)
#define __cpp_impl_coroutine true
#elif defined(_MSC_VER) && defined(__clang__) && !defined(__cpp_lib_coroutine)
#define __cpp_lib_coroutine true
#endif
// Clang requires coroutine types in std::experimental

#include <coroutine>

#if defined(__clang__)
namespace std::experimental {
using std::coroutine_traits;
using std::coroutine_handle;
using std::suspend_always;
using std::suspend_never;
}
#endif

#include <tuple>
#include <variant>
#include "execution.h"

namespace dse::util {

namespace impl {

template <typename Ret>
class SenderAwaiterRecv;

template <typename ... Tps>
class SenderAwaiterRecv<std::tuple<Tps...>> {
	std::coroutine_handle<> handle;
	std::tuple<Tps...>& value;
	std::exception_ptr& eptr;
public:
	SenderAwaiterRecv(std::tuple<Tps...>& value, std::exception_ptr& eptr, std::coroutine_handle<> handle) : handle(handle), value(value), eptr(eptr)
	{}
    template <typename ... Tps2>
    requires (sizeof... (Tps) == sizeof... (Tps2))
	friend void dse_TagInvoke(
		TagT<SetValue>,
		SenderAwaiterRecv<std::tuple<Tps...>>&& recv,
		Tps2&& ... args
    ) {
        recv.value = std::make_tuple(std::forward<Tps2>(args)...);
        recv.handle.resume();
    }
	friend void dse_TagInvoke(TagT<SetDone>, SenderAwaiterRecv&& recv)
	{
		recv.handle.resume();
	}
	friend void dse_TagInvoke(TagT<SetError>, SenderAwaiterRecv&& recv, std::exception_ptr&& eptr)
	{
		recv.eptr = eptr;
		recv.handle.resume();
	}
};

//template <typename ... Tps>
//void dse_TagInvoke(
//	TagT<SetValue>,
//	SenderAwaiterRecv<std::tuple<std::remove_reference_t<Tps>...>>&& recv,
//	Tps&& ... args
//) {
//	recv.value = std::make_tuple(std::forward<Tps>(args)...);
//	recv.handle.resume();
//}

template <typename T>
struct ReturnFromTuple {
	using Type = T;
};

template <template<typename ...> typename T>
struct ReturnFromTuple<T<>> {
	using Type = void;
};

template <template<typename ...> typename T, typename Single>
struct ReturnFromTuple<T<Single>> {
	using Type = Single;
};

template <typename T>
using ReturnFromTupleT = typename ReturnFromTuple<T>::Type;

template <typename S>
requires (
	std::same_as<typename SenderTraits<S>::template ErrorTypes<TypeTupleHelper>, TypeTupleHelper<std::exception_ptr>>
)
class SenderAwaiter {
	using ReturnedValue = typename SenderTraits<S>::template ValueTypes<std::tuple>;
	using OpState = std::invoke_result_t<decltype(Connect), std::remove_cvref_t<S>&&, SenderAwaiterRecv<ReturnedValue>&&>;
	std::variant<std::remove_cvref_t<S>, OpState> sndropstate;
	ReturnedValue value;
	std::exception_ptr eptr;
public:
	SenderAwaiter(S&& sndr) : sndropstate(std::forward<S>(sndr))
	{}
	bool await_ready()
	{ return false; }
	void await_suspend(std::coroutine_handle<> handle)
	{
		auto sndr = std::get<std::remove_cvref_t<S>>(sndropstate);
		sndropstate.template emplace<OpState>(Connect(std::move(sndr), SenderAwaiterRecv<ReturnedValue>(value, eptr, handle)));
		Start(std::get<OpState>(sndropstate));
	}
	ReturnFromTupleT<ReturnedValue> await_resume()
	{
		if (eptr) {
			std::rethrow_exception(eptr);
		}
		if constexpr (std::tuple_size_v<ReturnedValue> > 1) {
			return std::move(value);
		} else if constexpr (std::tuple_size_v<ReturnedValue> == 1) {
			return std::move(std::get<0>(value));
		} else {
			return;
		}
	}
};

}

template <typename T>
struct Task;

namespace impl {

template <typename T>
class TaskAwaiter;

template <typename Ret, typename Recv>
class TaskOpstate;

template <typename T>
struct TaskBase;

template <typename T>
struct FinalAwaiter;

template <typename T>
struct CoroReceiver {
	virtual void SetValue(T&&) = 0;
	virtual void SetError(std::exception_ptr&& e) = 0;
	virtual void SetDone() = 0;
};

template <>
struct CoroReceiver<void> {
	virtual void SetValue() = 0;
	virtual void SetError(std::exception_ptr&& e) = 0;
	virtual void SetDone() = 0;
};

template <typename T>
struct PromiseTypeBase {
    friend Task<T>;
    friend TaskBase<T>;
    friend FinalAwaiter<T>;
private:
    std::variant<std::monostate, T, std::exception_ptr> value;
public:
    void return_value(const T& val)
    {
        value.template emplace<1>(val);
    }
    void SetValueForRecv(CoroReceiver<T>* recv)
    {
        recv->SetValue(std::move(std::get<1>(value)));
    }
};

template <>
struct PromiseTypeBase<void> {
    friend Task<void>;
    friend TaskBase<void>;
    friend FinalAwaiter<void>;
private:
    std::variant<std::monostate, std::monostate, std::exception_ptr> value;
public:
    void return_void()
    {
        value.template emplace<1>();
    }
    void SetValueForRecv(CoroReceiver<void>* recv)
    {
        recv->SetValue();
    }
};

template <typename T>
struct PromiseTypeBase<T&> {
    friend Task<T&>;
    friend TaskBase<T&>;
    friend FinalAwaiter<T&>;
private:
    std::variant<std::monostate, std::reference_wrapper<T>, std::exception_ptr> value;
public:
    void return_value(T& val)
    {
        value.template emplace<1>(std::ref(val));
    }
    void SetValueForRecv(CoroReceiver<T&>* recv)
    {
        recv->SetValue(static_cast<T&>(std::get<1>(value).get()));
    }
};

template <typename T>
struct PromiseTypeBase<T&&> {
    friend Task<T&&>;
    friend TaskBase<T&&>;
    friend FinalAwaiter<T&&>;
private:
    std::variant<std::monostate, std::reference_wrapper<T>, std::exception_ptr> value;
public:
    void return_value(T&& val)
    {
        value.template emplace<1>(std::ref(val));
    }
    void SetValueForRecv(CoroReceiver<T&&>* recv)
    {
        recv->SetValue(static_cast<T&&>(std::get<1>(value).get()));
    }
};

template <typename T>
struct TaskBase {
    struct PromiseType;
    using Handle = std::coroutine_handle<PromiseType>;
    friend class TaskAwaiter<T>;
    template <typename Ret, typename Recv>
    friend class TaskOpstate;
protected:
    Handle handle;
    TaskBase(std::coroutine_handle<PromiseType> handle) : handle(handle)
    {}
    void ThrowIfExcept()
    {
        auto& promise = handle.promise();
        if (promise.value.index() == 2) {
            std::rethrow_exception(std::get<2>(promise.value));
        }
    }
public:
    TaskBase(const TaskBase&) = delete;
    TaskBase(TaskBase&& task) noexcept :
        handle(task.handle)
    {
        task.handle = nullptr;
    }
    TaskBase& operator =(const TaskBase&) = delete;
    TaskBase& operator =(TaskBase&& task) noexcept
    {
        std::swap(handle, task.handle);
    }
    ~TaskBase()
    {
        if (handle) handle.destroy();
    }
    void Resume() {
        handle.resume();
    }
    void operator()() {
        handle();
    }
    bool Done() {
        return handle.done();
    }
};

template <typename T>
struct TaskBase<T>::PromiseType : PromiseTypeBase<T> {
    friend class TaskAwaiter<T>;
    friend class FinalAwaiter<T>;
    template <typename Ret, typename Recv>
    friend class TaskOpstate;
    auto get_return_object() -> Task<T>
    {
        return { Task<T>::Handle::from_promise(*this) };
    }
    auto initial_suspend() -> std::suspend_always
    {
        return {};
    }
    void unhandled_exception()
    {
        this->value.template emplace<2>(std::current_exception());
    }
    auto final_suspend() noexcept -> FinalAwaiter<T>
    {
        return {};
    }
private:
    std::variant<std::monostate, std::coroutine_handle<>, CoroReceiver<T>*> rsm;
};

template <typename T>
struct FinalAwaiter {
    bool await_ready() noexcept
    {
        return false;
    }
    auto await_suspend(std::coroutine_handle<typename TaskBase<T>::PromiseType> handle) noexcept
    -> std::coroutine_handle<>
    {
        auto& promise = handle.promise();
        switch (promise.rsm.index()) {
            case 1:
                return std::get<1>(promise.rsm);
            case 2: {
                auto recv = std::get<2>(promise.rsm);
                switch (promise.value.index()) {
                    case 2:
                        recv->SetError(std::move(std::get<2>(promise.value)));
                        break;
                    case 1:
                        try {
                            promise.SetValueForRecv(recv);
                        } catch (...) {
                            recv->SetError(std::move(std::current_exception()));
                        }
                        [[fallthrough]];
                    default:
                        break;
                }
            } [[fallthrough]];
            default:
                break;
        }
        return std::noop_coroutine();
    }
    void await_resume() noexcept
    {}
};

}

template <typename T>
struct Task : impl::TaskBase<T> {
    using TaskBase = impl::TaskBase<T>;
    using promise_type = typename TaskBase::PromiseType;
    friend promise_type;
    Task(std::coroutine_handle<promise_type> handle) :
        TaskBase(handle)
    {}
    T Result()
    {
        this->ThrowIfExcept();
        return std::move(std::get<1>(this->handle.promise().value));
    }
};

template <>
struct Task<void> : impl::TaskBase<void> {
    using TaskBase = impl::TaskBase<void>;
    using promise_type = typename TaskBase::PromiseType;
    friend promise_type;
    Task(std::coroutine_handle<promise_type> handle) :
        TaskBase(handle)
    {}
    void Result()
    {
        this->ThrowIfExcept();
    }
};

template <typename T>
struct Task<T&> : impl::TaskBase<T&> {
    using TaskBase = impl::TaskBase<T&>;
    using promise_type = typename TaskBase::PromiseType;
    friend promise_type;
    Task(std::coroutine_handle<promise_type> handle) :
        TaskBase(handle)
    {}
    T& Result()
    {
        this->ThrowIfExcept();
        return std::get<1>(this->handle.promise().value);
    }
};

template <typename T>
struct Task<T&&> : impl::TaskBase<T&&> {
    using TaskBase = impl::TaskBase<T&&>;
    using promise_type = typename TaskBase::PromiseType;
    friend promise_type;
    Task(std::coroutine_handle<promise_type> handle) :
        TaskBase(handle)
    {}
    T&& Result()
    {
        this->ThrowIfExcept();
        return static_cast<T&&>(std::get<1>(this->handle.promise().value).get());
    }
};

namespace impl {

template <typename T>
class TaskAwaiter {
    Task<T>& task;
public:
    TaskAwaiter(Task<T>& task) :
        task(task)
    {}
    auto await_ready() -> bool {
        return false;
    }
    auto await_suspend(std::coroutine_handle<> handle) -> std::coroutine_handle<> {
        auto& promise = task.handle.promise();
        promise.rsm = handle;
        return task.handle;
    }
    T await_resume()
    {
        return task.Result();
    }
};

}

template <typename T>
auto operator co_await(Task<T>&& task) -> impl::TaskAwaiter<T>
{
	return impl::TaskAwaiter(task);
}

template <typename T>
auto operator co_await(Task<T>& task) -> impl::TaskAwaiter<T>
{
	return impl::TaskAwaiter(task);
}

template <typename T>
struct SenderTraits<Task<T>> {
	template <template<typename ...> typename TT>
	using ValueTypes = TT<T>;
	template <template<typename ...> typename TT>
	using ErrorTypes = TT<std::exception_ptr>;
	static constexpr bool SendsDone = false;
};

namespace impl {

template <typename Ret, typename Recv>
class TaskOpstate : CoroReceiver<Ret> {
	Task<Ret> task;
	Recv receiver;
	void SetValue(Ret&& val) override
	{
		util::SetValue(std::move(receiver), val);
	}
	void SetError(std::exception_ptr&& e) override
	{
		util::SetError(std::move(receiver), std::move(e));
	}
	void SetDone() override
	{}
public:
	TaskOpstate(Task<Ret>&& task, Recv&& receiver) :
		task(std::move(task)),
		receiver(std::move(receiver))
	{}
	void dse_TagInvoke(util::TagT<util::Start>, TaskOpstate& sndr)
	{
		auto handle = task.handle;
		auto& promise = handle.promise();
		promise.rsm = *this;
		handle.resume();
	}
};

}

template <typename Ret, typename Recv>
auto dse_TagInvoke(util::TagT<util::Connect>, Task<Ret>&& sndr, Recv&& recv)
-> impl::TaskOpstate<Ret, Recv>
{
	return { std::move(sndr), std::move(recv) };
}

}

template <dse::util::Sender S>
auto operator co_await(S&& sndr) -> dse::util::impl::SenderAwaiter<S>
{
	return { std::forward<S>(sndr) };
}

#endif // COROUTINE_H