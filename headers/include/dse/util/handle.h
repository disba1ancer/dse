#ifndef DSE_UTIL_HANDLE_H
#define DSE_UTIL_HANDLE_H

#include "dse/util/evtmgr.h"
#include <utility>

namespace dse::util {

template <class T>
struct handle_traits;
// {
//     using sender = <event source type>;
//     static void kill_handle(sender&, T);
// };

namespace handle_owner_impl {

template<class H, class S, void(*k)(S&, H)>
struct handle_owner
{
    using handle = H;
    using sender = S;
    handle_owner() : s(nullptr), h(NullHandle) {}
    handle_owner(sender& s, handle h) : s(std::addressof(s)), h(h) {}
    handle_owner(handle_owner&& oth) noexcept : handle_owner()
    {
        *this = oth;
    }
    handle_owner(const handle_owner&) = delete;
    handle_owner& operator=(handle_owner&& oth) noexcept
    {
        using std::swap;
        swap(s, oth.s);
        swap(h, oth.h);
        return *this;
    }
    handle_owner& operator=(const handle_owner&) = delete;
    ~handle_owner()
    {
        reset();
    }
    void reset()
    {
        if (h != NullHandle)
        {
            k(*s, h);
        }
        s = nullptr;
        h = NullHandle;
    }
    void reset(sender& newSender, handle newHandle)
    {
        reset();
        s = std::addressof(newSender);
        h = newHandle;
    }
    auto detach() -> handle
    {
        return std::exchange(h, NullHandle);
    }
private:
    static constexpr auto NullHandle = handle{};
    sender* s;
    handle h;
};

} // namespace handle_owner_impl

template<class T>
requires std::is_scoped_enum_v<T>
using handle_owner = handle_owner_impl::handle_owner<T, typename handle_traits<T>::sender, handle_traits<T>::kill_handle>;

template <class S, class EventTypeEnum>
struct basic_subscribable
{
    using handler_id = typename util::event_manager<EventTypeEnum>::handler_id;
    enum class event_handle : handler_id {};
private:
    static void kill_handle(S& s, event_handle h)
    {
        s.UnsubscribeEvent(std::to_underlying(h));
    }
public:
    using handle_owner = util::handle_owner_impl::handle_owner<event_handle, S, kill_handle>;

    template <EventTypeEnum type>
    auto SubscribeEvent(util::function_ptr<typename util::event_traits<type>::handler> handler) -> handle_owner
    {
        return {*static_cast<S*>(this), event_handle{static_cast<S*>(this)->SubscribeEvent(type, handler.get_object_ptr(), reinterpret_cast<void(*)()>(handler.get_function()))}};
    }
};

} // namespace dse::util

#endif // DSE_UTIL_HANDLE_H
