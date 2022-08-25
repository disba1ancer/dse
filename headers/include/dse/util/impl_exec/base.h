#ifndef UTIL_IMPL_EXEC_BASE_H
#define UTIL_IMPL_EXEC_BASE_H

#include <exception>
#include "../TagInvoke.h"

namespace dse::util {

inline constexpr struct SetDoneT {
    template <typename R>
    auto operator()(R&& r) const
    {
        return dse_TagInvoke(*this, std::forward<R>(r));
    }
} SetDone;

inline constexpr struct SetErrorT {
    template <typename R, typename E>
    auto operator()(R&& r, E&& e) const
    {
        return dse_TagInvoke(*this, std::forward<R>(r), std::forward<E>(e));
    }
} SetError;

inline constexpr struct SetValueT {
    template <typename R, typename ... Args>
    auto operator()(R&& r, Args&& ... args) const
    {
        return dse_TagInvoke(*this, std::forward<R>(r), std::forward<Args>(args)...);
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
        return dse_TagInvoke(*this, std::forward<S>(s), std::forward<R>(r));
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
    typename S::ErrorType;
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
    using ErrorType = typename S::ErrorType;
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
        return dse_TagInvoke(*this, op);
    }
} Start;

template <typename Op>
concept OperationState = requires (Op& op) {
    Start(op);
};

}

#endif // UTIL_IMPL_EXEC_BASE_H
