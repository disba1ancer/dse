#ifndef UTIL_IMPL_EXEC_START_DETACHED_H
#define UTIL_IMPL_EXEC_START_DETACHED_H

#include "base.h"

namespace dse::util {

namespace impl {

template <Sender S>
class DetachedOpstate;

template <Sender S>
class DetachedReceiver {
    template <typename ... Args>
    friend void dse_TagInvoke(TagT<SetValue>, DetachedReceiver&& recv, Args&& ...)
    {
        delete static_cast<DetachedOpstate<S>*>(&recv);
    }

    template <typename E>
    friend void dse_TagInvoke(TagT<SetError>, DetachedReceiver&& recv, E&&)
    {
        std::terminate();
        delete static_cast<DetachedOpstate<S>*>(&recv);
    }

    friend void dse_TagInvoke(TagT<SetDone>, DetachedReceiver&& recv)
    {
        delete static_cast<DetachedOpstate<S>*>(&recv);
    }
};

template <Sender S>
class DetachedOpstate : DetachedReceiver<S> {
    using Opstate = TagInvokeResultT<TagT<Connect>, S&&, DetachedReceiver<S>&>;
    Opstate opstate;
public:
    DetachedOpstate(S&& sndr) :
        opstate(Connect(std::forward<S>(sndr), (DetachedReceiver<S>&)*this))
    {
        Start(opstate);
    }
};

}

struct StartDetachedT;

inline constexpr struct StartDetachedT {
    template <Sender S>
    void operator()(S&& sndr) const
    {
        return dse_TagInvoke(*this, std::forward<S>(sndr));
    }

    template <typename S>
    friend void dse_TagInvoke(StartDetachedT, S&& sndr)
    {
        new impl::DetachedOpstate<S>(std::forward<S>(sndr));
    }
} StartDetached;

}

#endif // UTIL_IMPL_EXEC_START_DETACHED_H
