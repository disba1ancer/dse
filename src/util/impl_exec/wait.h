#ifndef UTIL_IMPL_EXEC_WAIT_H
#define UTIL_IMPL_EXEC_WAIT_H

#include <mutex>
#include <condition_variable>
#include "base.h"

namespace dse::util {

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

} // namespace dse::util

#endif // UTIL_IMPL_EXEC_WAIT_H
