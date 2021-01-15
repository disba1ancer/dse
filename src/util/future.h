#ifndef FUTURE_H
#define FUTURE_H

#include <memory>
#include <atomic>
#include <variant>
#include <queue>
#include <type_traits>
#include <future>
#include <mutex>
#include <condition_variable>
#include <coroutine>
#include <deque>
#include "functional.h"

namespace dse::util {

namespace impl {

template <typename T>
class refcounter {
	std::atomic<int> refs = 1;
public:
	refcounter() = default;
	refcounter(const refcounter&) = delete;
	refcounter(refcounter&&) = delete;
	refcounter& operator=(const refcounter&) = delete;
	refcounter& operator=(refcounter&&) = delete;
	~refcounter() = default;
protected:
	virtual void destroy() noexcept = 0; /*{
		delete static_cast<T*>(this);
	}*/
public:
	void release() noexcept {
		if (refs.fetch_sub(1, std::memory_order_release) == 1) {
			this->destroy();
		}
	}
	struct deleter {
		void operator()(T* t) const noexcept {
			t->refcounter<T>::release();
		}
	};
	using pointer_type = std::unique_ptr<T, deleter>;
	[[nodiscard]]
	pointer_type share() {
		refs.fetch_add(1, std::memory_order_acquire);
		return pointer_type{static_cast<T*>(this)};
	}
};

}

template <typename T>
class shared_state_base;

template <typename T>
class shared_state_type;

template <typename T>
class shared_state;

template <typename T>
class future;

template <typename T>
class promise_base;

template <typename T>
class promise_typed;

template <typename T>
class promise;

template <typename T>
class shared_state_base : public impl::refcounter<shared_state<T>> {
private:
	using continuation_type = function_view<void(future<T>)>;
	std::mutex mtx;
	std::condition_variable cvar;
	continuation_type continuations;
public:
	friend class shared_state_type<T>;
	friend class shared_state<T>;
	enum types {
		Undefined,
		Value,
		Exception
	};
private:
	void notify_and_continue();
};

template <typename T>
class shared_state_type : public shared_state_base<T> {
public:
	using value_type = T;
	using store_type = T;
	using shared_state_base<T>::mtx;
	using shared_state_base<T>::cvar;
	using shared_state_base<T>::Undefined;
	using shared_state_base<T>::Value;
	friend class shared_state<T>;
private:
	std::variant<std::monostate, store_type, std::exception_ptr> value;
	//void notify_and_continue();
public:
	void set_value(const value_type& val) {
		{
			std::scoped_lock lck(mtx);
			if (value.index() != Undefined) {
				throw std::future_error(std::future_errc::promise_already_satisfied);
			}
			value.template emplace<Value>(val);
		}
		this->notify_and_continue();
	}
	void set_value(value_type&& val) {
		{
			std::scoped_lock lck(mtx);
			if (value.index() != Undefined) {
				throw std::future_error(std::future_errc::promise_already_satisfied);
			}
			value.template emplace<Value>(std::move(val));
		}
		this->notify_and_continue();
	}
	void return_value(const value_type& val) {
		set_value(val);
	}
	void return_value(value_type&& val) {
		set_value(std::move(val));
	}
};

template <>
class shared_state_type<void> : public shared_state_base<void> {
public:
	using value_type = void;
	using store_type = std::monostate;
	using shared_state_base<value_type>::mtx;
	using shared_state_base<value_type>::cvar;
	using shared_state_base<value_type>::Undefined;
	friend class shared_state<value_type>;
private:
	std::variant<std::monostate, store_type, std::exception_ptr> value;
public:
	void set_value() {
		{
			std::scoped_lock lck(mtx);
			if (value.index() != Undefined) {
				throw std::future_error(std::future_errc::promise_already_satisfied);
			}
			value.emplace<Value>(std::monostate());
		}
		this->notify_and_continue();
	}
	void return_void() {
		set_value();
	}
};

template <typename T>
class shared_state_type<T&> : public shared_state_base<T&> {
public:
	using value_type = T&;
	using store_type = std::reference_wrapper<T>;
	using shared_state_base<value_type>::mtx;
	using shared_state_base<value_type>::cvar;
	using shared_state_base<value_type>::Value;
	using shared_state_base<value_type>::Undefined;
	friend class shared_state<value_type>;
private:
	std::variant<std::monostate, store_type, std::exception_ptr> value;
public:
	void set_value(value_type& val) {
		{
			std::scoped_lock lck(mtx);
			if (value.index() != Undefined) {
				throw std::future_error(std::future_errc::promise_already_satisfied);
			}
			value.template emplace<Value>(std::ref(val));
		}
		this->notify_and_continue();
	}
	void return_value(value_type& val) {
		set_value(val);
	}
};

template <typename T>
class shared_state_type<T&&>;

template <typename T>
class shared_state final :
	public shared_state_type<T>
{
public:
	using typename impl::refcounter<shared_state<T>>::deleter;
	using typename shared_state_type<T>::value_type;
	using typename shared_state_base<T>::continuation_type;
private:
	bool isCoroPromise = true;
	shared_state(bool coro) : isCoroPromise(coro) {}
public:
	shared_state() noexcept = default;
	static std::unique_ptr<shared_state<T>, deleter> constuct() {
		return { new shared_state(false), deleter() };
	}
protected:
	virtual void destroy() noexcept override {
		if (isCoroPromise) {
			std::coroutine_handle<shared_state<T>>::from_promise(*this).destroy();
		} else {
			delete this;
		}
	}
public:
	void set_exception(std::exception_ptr eptr) {
		this->value.template emplace<this->Exception>(eptr);
		this->notify_and_continue();
	}
	void wait() {
		std::unique_lock lck(this->mtx);
		if (this->value.index() == this->Undefined) {
			this->cvar.wait(lck);
		}
	}
	template <typename Rep, typename Period>
	void wait_for(const std::chrono::duration<Rep, Period>& timeout) {
		std::unique_lock lck(this->mtx);
		if (this->value.index == this->Undefined) {
			this->cvar.wait_for(lck, timeout);
		}
	}
	template <typename Clock, typename Duration>
	void wait_until(const std::chrono::time_point<Clock, Duration>& timeout) {
		std::unique_lock lck(this->mtx);
		if (this->value.index == this->Undefined) {
			this->cvar.wait_until(lck, timeout);
		}
	}
	T get() {
		wait();
		if (this->value.index() == this->Exception) {
			std::rethrow_exception(std::get<this->Exception>(this->value));
		}
		return static_cast<value_type>(std::get<this->Value>(this->value));
	}
	void then(continuation_type continuation) {
		{
			std::scoped_lock lck(this->mtx);
			if (this->value.index() == this->Undefined) {
				this->continuations = continuation;
				return;
			}
		}
		continuation(this->share());
	}
	bool is_ready() const {
		return this->value.index() != this->Undefined;
	}
	future<value_type> get_return_object();
	std::suspend_never initial_suspend() {
		return {};
	}
	struct final_suspend_t {
		bool await_ready() { return false; }
		void await_suspend(std::coroutine_handle<shared_state> handle) {
			shared_state& state = handle.promise();
			state.release();
		}
		void await_resume() {};
	};

	final_suspend_t final_suspend() {
		return {};
	}
	void unhandled_exception() {
		set_exception(std::current_exception());
	}
};

template <typename T>
class future {
public:
	using value_type = T;
	friend class promise_base<value_type>;
	friend class shared_state_base<value_type>;
	friend class shared_state<value_type>;
	using promise_type = shared_state<value_type>;
private:
	using sh_state = shared_state<value_type>;
	using pointer_type = typename sh_state::pointer_type;
	using continuation_type = typename sh_state::continuation_type;
	pointer_type state;
	future(pointer_type&& state) : state(std::move(state)) {}
public:
	future() noexcept = default;
	future(const future&) = delete;
	future(future&&) noexcept = default;
	future& operator=(const future&) = delete;
	future& operator=(future&&) noexcept = default;
	~future() = default;
	bool valid() const noexcept {
		return state;
	}
	void wait() const {
		state->wait();
	}
	template <typename Rep, typename Period>
	void wait_for(const std::chrono::duration<Rep, Period>& timeout) const {
		state->wait_for(timeout);
	}
	template <typename Clock, typename Duration>
	void wait_until(const std::chrono::time_point<Clock, Duration>& timeout) const {
		state->wait_until(timeout);
	}
	T get() {
		auto state = std::move(this->state);
		return state->get();
	}
	bool is_ready() const {
		return state->is_ready();
	}
	void then(continuation_type continuation) {
		auto state = std::move(this->state);
		state->then(continuation);
	}
};

template <typename T>
void shared_state_base<T>::notify_and_continue() {
	cvar.notify_all();
	if (continuations) continuations(this->share());
}

template <typename T>
auto shared_state<T>::get_return_object() -> future<value_type> {
	return this->share();
}

template <typename T>
class promise_base {
public:
	using value_type = T;
	friend class promise_typed<T>;
	friend class promise<T>;
private:
	using shared_state = shared_state<value_type>;
	using pointer_type = typename shared_state::pointer_type;
	pointer_type state;
	bool future_retrived = false;
public:
	promise_base() :
		state(shared_state::constuct())
	{}
	future<value_type> get_future() {
		future_retrived = true;
		return state->share();
	}
	void set_exception(std::exception_ptr eptr) {
		state->set_exception(eptr);
	}
};

template <typename T>
class promise_typed : public promise_base<T> {
public:
	using typename promise_base<T>::value_type;
	using typename promise_base<T>::shared_state;
	using promise_base<T>::state;
	void set_value(const value_type& val) {
		if (!state) throw std::future_error(std::future_errc::no_state);
		state->set_value(val);
	}
	void set_value(value_type&& val) {
		if (!state) throw std::future_error(std::future_errc::no_state);
		state->set_value(val);
	}
};

template <typename T>
class promise_typed<T&> : public promise_base<T&> {
public:
	using typename promise_base<T&>::value_type;
	using typename promise_base<T&>::shared_state;
	using promise_base<T&>::state;
	void set_value(value_type val) {
		if (!state) throw std::future_error(std::future_errc::no_state);
		state->set_value(val);
	}
};

template <>
class promise_typed<void> : public promise_base<void> {
public:
	using typename promise_base<void>::shared_state;
	using promise_base<void>::state;
	void set_value() {
		if (!state) throw std::future_error(std::future_errc::no_state);
		state->set_value();
	}
};

template <typename T>
class promise_typed<T&&>;

template <typename T>
class promise : public promise_typed<T> {
public:
	void swap(promise& other) {
		auto t = std::move(*this);
		*this = std::move(other);
		other = std::move(t);
	}
};

template <typename T>
class future_awaiter {
	future<T> ft;
	std::coroutine_handle<> handle;
public:
	future_awaiter(future<T>&& ft) : ft(std::move(ft)) {}
	bool await_ready() {
		return ft.is_ready();
	}
	void operator()(future<T> ft) {
		this->ft = std::move(ft);
		handle.resume();
	}
	void await_suspend(std::coroutine_handle<> handle) {
		this->handle = handle;
		ft.then(*this);
	}
	T await_resume() {
		return ft.get();
	}
};

template <typename T>
auto operator co_await(future<T>& ft) -> future_awaiter<T> {
	return { std::move(ft) };
}

template <typename T>
auto operator co_await(future<T>&& ft) -> future_awaiter<T> {
	return { std::move(ft) };
}

}

#endif // FUTURE_H
