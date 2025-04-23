#ifndef DSE_UTIL_EVTMGR_H
#define DSE_UTIL_EVTMGR_H

#include <type_traits>
#include "functional.h"
#include <set>

namespace dse::util {

namespace evtmgr_impl {

using handler_id = std::size_t;

}

template <auto en>
requires std::is_scoped_enum_v<decltype(en)>
struct event_traits;

template <class EventEnum>
requires std::is_scoped_enum_v<EventEnum>
struct event_manager {
    using event_id = EventEnum;
    using handler_id = evtmgr_impl::handler_id;
private:
    struct Key {
        handler_id prev;
        handler_id next;
        union {
            void* observer;
            event_id event;
        };
        void(*callback)();
    };
    template <event_id event>
    using handler = typename event_traits<event>::handler;
    Key& get_handler(handler_id id)
    {
        return handlers[id - 1];
    }
    auto allocate_handler() -> handler_id
    {
        if (freeHandlersHead == 0) {
            handler_id newid = handlers.size() + 1;
            handlers.resize(newid);
            return newid;
        }
        handler_id newid = freeHandlersHead;
        freeHandlersHead = get_handler(newid).next;
        return newid;
    }
    void free_handler(handler_id id)
    {
        auto& handler = get_handler(id);
        handler.callback = nullptr;
        handler.next = freeHandlersHead;
        freeHandlersHead = id;
    }
    void insert_handler(handler_id id, handler_id before)
    {
        auto& next = get_handler(before);
        auto& prev = get_handler(next.prev);
        auto& handler = get_handler(id);
        handler.prev = next.prev;
        handler.next = before;
        prev.next = id;
        next.prev = id;
    }
    void erase_handler(handler_id id)
    {
        auto& handler = get_handler(id);
        auto& next = get_handler(handler.next);
        auto& prev = get_handler(handler.prev);
        prev.next = handler.next;
        next.prev = handler.prev;
    }
    void insert_handler_into_chain(event_id chain, handler_id id)
    {
        auto& chainHead = handlerChains[chain];
        if (chainHead == 0) {
            chainHead = allocate_handler();
            auto &head = get_handler(chainHead);
            head.callback = nullptr;
            head.event = chain;
            head.prev = chainHead;
            head.next = chainHead;
        }
        insert_handler(id, chainHead);
    }
    void erase_handler_with_chain(handler_id id)
    {
        auto& handler = get_handler(id);
        erase_handler(id);
        if (handler.prev != handler.next) {
            return;
        }
        auto &head = get_handler(handler.next);
        handlerChains.erase(head.event);
        free_handler(handler.next);
    }
public:
    auto register_e(event_id event, void* object, void(*callback)()) -> handler_id
    {
        if (callback == nullptr) {
            return 0;
        }
        auto newid = allocate_handler();
        auto& handler = get_handler(newid);
        handler.observer = object;
        handler.callback = callback;
        insert_handler_into_chain(event, newid);
        return newid;
    }
    void unregister(handler_id id)
    {
        if (id == 0 || id > handlers.size()) {
            return;
        }
        auto& handler = get_handler(id);
        if (handler.callback == nullptr) {
            return;
        }
        erase_handler_with_chain(id);
        free_handler(id);
    }
    template <event_id event>
    bool register_e(const function_ptr<handler<event>>& f)
    {
        return register_e(event, f.get_object_ptr(), reinterpret_cast<void(*)()>(f.get_function()));
    }
    template <class H, class ... Args>
    bool send(event_id event, Args&& ... args)
    {
        auto it = handlerChains.find(event);
        if (it == handlerChains.end()) {
            return false;
        }
        auto startId = it->second;
        auto id = get_handler(startId).next;
        while(id != startId) {
            auto& handler = get_handler(id);
            using handler_t = function_ptr<H>;
            auto callback = reinterpret_cast<typename handler_t::sfn*>(handler.callback);
            handler_t f{handler.observer, callback};
            f(std::forward<Args>(args)...);
            id = handler.next;
        }
        return true;
    }
    template <event_id event, class ... Args>
    bool send(Args&& ... args)
    {
        return send<handler<event>>(event, std::forward<Args>(args)...);
    }
private:
    std::vector<Key> handlers;
    std::unordered_map<event_id, handler_id> handlerChains;
    handler_id freeHandlersHead = 0;
};

template<class T>
struct handler_owner
{
    using handler_id = evtmgr_impl::handler_id;
    handler_owner(T& observable, handler_id id) : observable(observable), id(id) {}
    handler_owner(handler_owner&&) = delete;
    handler_owner(const handler_owner&) = delete;
    handler_owner& operator=(handler_owner&&) = delete;
    handler_owner& operator=(const handler_owner&) = delete;
    ~handler_owner()
    {
        observable.unregister(id);
    }
    void detach()
    {
        id = 0;
    }
private:
    T& observable;
    handler_id id;
};

} // namespace dse::util

#endif // DSE_UTIL_EVTMGR_H
