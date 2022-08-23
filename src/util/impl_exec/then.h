#ifndef UTIL_IMPL_EXEC_THEN_H
#define UTIL_IMPL_EXEC_THEN_H

#include "base.h"

namespace dse::util {

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
    TagInvokeResultT<
        TagT<Connect>,
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
        return dse_TagInvoke(*this, std::forward<S>(sndr), std::forward<F>(func));
    }

    template <typename S, typename F>
    friend auto dse_TagInvoke(ThenT, S&& sndr, F&& func) -> impl::then_sender<S, F>
    {
        return { std::forward<S>(sndr), std::forward<F>(func) };
    }
} Then;

} // namespace dse::util

#endif // UTIL_IMPL_EXEC_THEN_H
