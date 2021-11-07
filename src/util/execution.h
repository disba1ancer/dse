#ifndef EXECUTION_H
#define EXECUTION_H

#include <exception>
#include <mutex>
#include <condition_variable>
#include "TagInvoke.h"

namespace dse::util {

inline constexpr struct SetDoneT {
    template <typename R>
    auto operator()(R&& r) const
    {
        return (dse_TagInvoke)(*this, std::forward<R>(r));
    }
} SetDone;

inline constexpr struct SetErrorT {
    template <typename R, typename E>
    auto operator()(R&& r, E&& e) const
    {
        return (dse_TagInvoke)(*this, std::forward<R>(r), std::forward<E>(e));
    }
} SetError;

inline constexpr struct SetValueT {
    template <typename R, typename ... Args>
    auto operator()(R&& r, Args&& ... args) const
    {
        return (dse_TagInvoke)(*this, std::forward<R>(r), std::forward<Args>(args)...);
    }
} SetValue;

namespace impl {

struct dummy_receiver {
    friend void dse_TagInvoke(TagT<SetDone>, dummy_receiver&&)
    {}
    template <typename E>
    friend void dse_TagInvoke(TagT<SetError>, dummy_receiver&&, E&&)
    {}
    template <typename ... Args>
    friend void dse_TagInvoke(TagT<SetValue>, dummy_receiver&&, Args&& ...)
    {}
};

}

template <typename R, typename E = std::exception_ptr>
concept Receiver = requires (std::remove_cvref_t<R>&& r, E&& e) {
    SetDone(std::move(r));
    SetError(std::move(r), std::forward<E>(e));
};

template <typename R, typename ... Args>
concept ReceiverOf =
    Receiver<R> &&
    requires (R&& r, Args&& ... args) {
        SetValue(std::move(r), std::forward<Args>(args)...);
    };

inline constexpr struct ConnectT {
    template <typename S, Receiver R>
    auto operator()(S&& s, R&& r) const
    {
        return (dse_TagInvoke)(*this, std::forward<S>(s), std::forward<R>(r));
    }
} Connect;

//template <typename S>
//concept Sender = requires (S&& s, impl::dummy_receiver&& r) {
//    Connect(std::forward<S>(s), std::move(r));
//};

//template <typename S, typename R>
//concept SenderTo = requires (S&& s, R&& r) {
//    Connect(std::forward<S>(s), std::forward<R>(r));
//};

struct SenderBase {};

namespace impl {

template <typename ... Tps>
struct TypeTupleHelper {};

template <typename S>
concept HasSenderTypes = requires {
    typename S::template ValueTypes<TypeTupleHelper>;
    typename S::template ErrorTypes<TypeTupleHelper>;
    { S::SendsDone } -> std::convertible_to<bool>;
};

template <typename S>
struct SenderTraitsBase {
    using NotASenderTypename = void;
};

template <HasSenderTypes S>
struct SenderTraitsBase<S> {
    template <template<typename...> typename Tup>
    using ValueTypes = typename S::template ValueTypes<Tup>;
    template <template<typename...> typename Var>
    using ErrorTypes = typename S::template ErrorTypes<Var>;
    static constexpr bool SendsDone = S::SendsDone;
};

template <typename S>
requires (
    !HasSenderTypes<S> &&
    std::derived_from<S, SenderBase>
)
struct SenderTraitsBase<S> {};

}

template <typename S>
struct SenderTraits : impl::SenderTraitsBase<S> {};

template <typename S>
concept Sender = !requires {
    typename SenderTraits<std::remove_cvref_t<S>>::NotASenderTypename;
};

template <typename S, typename R>
concept SenderTo =
    Sender<S> &&
    Receiver<R> &&
    requires (S&& s, R&& r) {
        util::Connect(std::forward<S>(s), std::forward<R>(r));
    };

inline constexpr struct StartT {
    template <typename Op>
    auto operator()(Op& op) const
    {
        return (dse_TagInvoke)(*this, op);
    }
} Start;

template <typename Op>
concept OperationState = requires (Op& op) {
    Start(op);
};

namespace impl {

template <typename S, typename F>
class then_sender;

template <typename F, typename R>
class then_sndr_recv {
    std::remove_cvref_t<F> func;
    std::remove_cvref_t<R> downrecv;
    template <typename ... Args>
    void set_value_internal(Args&& ... args)
    requires (std::same_as<std::invoke_result_t<F, Args&&...>, void>)
    {
        func(std::forward<Args>(args)...);
        SetValue(std::move(downrecv));
    }
    template <typename ... Args>
    void set_value_internal(Args&& ... args)
    requires (!std::same_as<std::invoke_result_t<F, Args&&...>, void>)
    {
        SetValue(std::move(downrecv), func(std::forward<Args>(args)...));
    }
public:
    then_sndr_recv(F&& func, R&& recv) : func(std::forward<F>(func)), downrecv(std::forward<R>(recv))
    {}
    template <typename ... Args>
    friend void dse_TagInvoke(TagT<SetValue>, then_sndr_recv&& op, Args&& ... args)
    {
        try {
            op.set_value_internal(std::forward<Args>(args)...);
        } catch (...) {
            SetError(std::move(op.downrecv), std::current_exception());
        }
    }
    friend void dse_TagInvoke(TagT<SetDone>, then_sndr_recv&& op)
    {
        SetDone(std::move(op.downrecv));
    }
    friend void dse_TagInvoke(TagT<SetError>, then_sndr_recv&& op, std::exception_ptr e)
    {
        SetError(std::move(op.downrecv), e);
    }
};

template <typename S, typename F, typename R>
class then_sndr_opstate {
    std::invoke_result_t<
        decltype(Connect),
        S&&,
        then_sndr_recv<F, R>&&
    > upopstate;
public:
    then_sndr_opstate(S&& sender, F&& func, R&& receiver) :
        upopstate(Connect(std::forward<S>(sender), then_sndr_recv<F, R>(std::forward<F>(func), std::forward<R>(receiver))))
    {}
    friend void dse_TagInvoke(TagT<Start>, then_sndr_opstate& op)
    {
        Start(op.upopstate);
    }
};

template <typename S, typename F>
class then_sender : public SenderBase {
    std::remove_cvref_t<S> upsender;
    std::remove_cvref_t<F> func;
public:
    then_sender(S&& sender, F&& func) :
        upsender(std::forward<S>(sender)),
        func(std::forward<F>(func))
    {}
    template <typename R>
    friend auto dse_TagInvoke(TagT<Connect>, then_sender&& sndr, R&& receiver)
    {
        return then_sndr_opstate<S, F, R&&>{ std::move(sndr.upsender), std::move(sndr.func), std::forward<R>(receiver) };
    }
};

}

struct ThenT;

inline constexpr struct ThenT {
    template <Sender S, typename F>
    auto operator()(S&& sndr, F&& func) const -> impl::then_sender<S, F>
    {
        return (dse_TagInvoke)(*this, std::forward<S>(sndr), std::forward<F>(func));
    }
} Then;

template <typename S, typename F>
auto dse_TagInvoke(TagT<Then>, S&& sndr, F&& func) -> impl::then_sender<S, F>
{
    return { std::forward<S>(sndr), std::forward<F>(func) };
}

namespace impl {

struct wait_store {
    std::mutex mtx;
    std::condition_variable cv;
    bool notified;
    std::exception_ptr eptr;
};

struct wait_recv {
    wait_store& store;
    wait_recv(wait_store& store) : store(store)
    {}
    auto lock_and_signal() -> std::unique_lock<std::mutex>
    {
        std::unique_lock lck(store.mtx);
        store.notified = true;
        store.cv.notify_one();
        return lck;
    }
    friend void dse_TagInvoke(const SetValueT&, wait_recv&& recv)
    {
        auto lck = recv.lock_and_signal();
    }
    friend void dse_TagInvoke(const SetDoneT&, wait_recv&& recv)
    {
        auto lck = recv.lock_and_signal();
    }
    friend void dse_TagInvoke(const SetErrorT&, wait_recv&& recv, std::exception_ptr eptr)
    {
        auto lck = recv.lock_and_signal();
        recv.store.eptr = eptr;
    };
};

}

template <Sender S>
void Wait(S&& sender) {
    impl::wait_store store;
    std::unique_lock lck(store.mtx);
    OperationState auto s = Connect(std::forward<S>(sender), impl::wait_recv(store));
    Start(s);
    store.cv.wait(lck, [&store]{ return store.notified; });
    if (store.eptr) {
        std::rethrow_exception(store.eptr);
    }
}

}

#endif // EXECUTION_H
