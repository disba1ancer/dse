#ifndef DSE_UTIL_EVTMGR_H
#define DSE_UTIL_EVTMGR_H

#include <type_traits>
#include <compare>
#include "functional.h"
#include <set>

namespace dse::util {

template <auto en>
requires std::is_scoped_enum_v<decltype(en)>
struct event_traits;

template <class EventEnum>
requires std::is_scoped_enum_v<EventEnum>
class event_manager {
    using EventId = EventEnum;
    struct Key {
        std::underlying_type_t<EventId> id;
        void* observer;
        void(*callback)();
    };
    struct KeyCompare {
        using is_transparent = void;
        bool operator()(const Key& a, const Key& b) const
        {
            auto ord = a.id <=> b.id;
            if (ord != 0) {
                return ord < 0;
            }
            auto pA = reinterpret_cast<std::uintptr_t>(a.observer);
            auto pB = reinterpret_cast<std::uintptr_t>(b.observer);
            if ((ord = pA <=> pB) != 0) {
                return ord < 0;
            }
            pA = reinterpret_cast<std::uintptr_t>(reinterpret_cast<void*>(a.callback));
            pB = reinterpret_cast<std::uintptr_t>(reinterpret_cast<void*>(b.callback));
            return pA <=> pB < 0;
        }
        constexpr bool operator()(const EventId& a, const Key& b) const
        {
            return std::to_underlying(a) < b.id;
        }
        constexpr bool operator()(const Key& a, const EventId& b) const
        {
            return a.id < std::to_underlying(b);
        }
    };
    template <EventId event>
    using handler = typename event_traits<event>::handler;
public:
    bool register_e(EventId event, void* object, void(*callback)())
    {
        auto [it, r] = handlers.emplace(std::to_underlying(event), object, callback);
        return r;
    }
    void unregister(EventId event, void* object, void(*callback)())
    {
        handlers.erase(Key{std::to_underlying(event), object, callback});
    }
    template <EventId event>
    bool register_e(const function_ptr<handler<event>>& f)
    {
        return register_e(event, f.get_object_ptr(), reinterpret_cast<void(*)()>(f.get_function()));
    }
    template <EventId event>
    void unregister(const function_ptr<handler<event>>& f)
    {
        unregister(event, f.get_object_ptr(), reinterpret_cast<void(*)()>(f.get_function()));
    }
    template <typename H, typename ... Args>
    bool send(EventId event, Args&& ... args)
    {
        auto [begin, end] = handlers.equal_range(event);
        if (begin == end) {
            return false;
        }
        for (auto& hndlr : std::ranges::subrange{begin, end}) {
            using handler_t = function_ptr<H>;
            auto callback = reinterpret_cast<typename handler_t::sfn*>(hndlr.callback);
            handler_t f{hndlr.observer, callback};
            f(std::forward<Args>(args)...);
        }
        return true;
    }
    template <EventId event, typename ... Args>
    bool send(Args&& ... args)
    {
        return send<handler<event>>(event, std::forward<Args>(args)...);
    }
private:
    std::set<Key, KeyCompare> handlers;
};

} // namespace dse::util

#endif // DSE_UTIL_EVTMGR_H
