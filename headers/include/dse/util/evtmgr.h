#ifndef DSE_UTIL_EVTMGR_H
#define DSE_UTIL_EVTMGR_H

#include <type_traits>
#include "functional.h"
#include <vector>
#include <unordered_map>

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
    struct Key;
    struct KeyIter {
        KeyIter& operator++()
        {
            handler += handler->next;
            return *this;
        }
        bool operator==(const KeyIter& oth) const
        {
            return handler == oth.handler;
        }
        Key& operator*()
        {
            return *handler;
        }
        Key* handler;
    };
    struct Key {
        union {
            std::ptrdiff_t prev;
            event_id event2;
        };
        std::ptrdiff_t next;
        union {
            void* observer;
            event_id event;
        };
        void(*callback)();
        KeyIter begin()
        {
            return {this + next};
        }
        KeyIter end()
        {
            return {this + prev};
        }
    };
    template <event_id event>
    using handler = typename event_traits<event>::handler;
    Key& get_handler(handler_id id)
    {
        return handlers[id - 1];
    }
    Key* to_ptr(std::ptrdiff_t index) {
        return handlers.data() + index;
    }
    auto to_index(Key* handler) -> std::ptrdiff_t {
        return handler - handlers.data();
    }
    Key* allocate_handler()
    {
        if (freeHandlersHead == -1) {
            auto size = handlers.size();
            handlers.push_back({});
            return to_ptr(size);
        }
        Key* newHandler = to_ptr(freeHandlersHead);
        freeHandlersHead += newHandler->next;
        return newHandler;
    }
    void free_handler(Key* handler)
    {
        handler->callback = nullptr;
        handler->next = freeHandlersHead - to_index(handler);
        freeHandlersHead = to_index(handler);
    }
    void insert_handler(Key* handler, Key* before)
    {
        auto after = before + before->prev;
        handler->prev = after - handler;
        handler->next = before - handler;
        after->next = -handler->prev;
        before->prev = -handler->next;
    }
    void erase_handler(Key* handler)
    {
        auto next = handler + handler->next;
        auto prev = handler + handler->prev;;
        prev->next = next - prev;
        next->prev = prev - next;
    }
    Key* make_multihandler_chain(event_id chain)
    {
        auto head = allocate_handler();
        head->callback = nullptr;
        head->event = chain;
        head->prev = 0;
        head->next = 0;
        return head;
    }
    bool is_single_handler_chain(Key* chainHead)
    {
        return chainHead->next == 0;
    }
    void insert_handler_into_chain(event_id chain, Key* handler)
    {
        auto it = handlerChains.find(chain);
        if (it == handlerChains.end()) {
            handlerChains[chain] = to_index(handler);
            return;
        }
        auto chainHead = to_ptr(it->second);
        if (is_single_handler_chain(chainHead)) {
            auto singleHandler = chainHead;
            chainHead = make_multihandler_chain(chain);
            it->second = to_index(chainHead);
            insert_handler(singleHandler, chainHead);
        }
        insert_handler(handler, chainHead);
    }
    void erase_handler_with_chain(Key* handler)
    {
        if (is_single_handler_chain(handler)) {
            handlerChains.erase(handler->event2);
            return;
        }
        erase_handler(handler);
        if (handler->prev != handler->next) {
            return;
        }
        auto head = handler + handler->next;
        handlerChains.erase(head->event);
        free_handler(head);
    }
    template <class H, class ... Args>
    void call_handler(Key* handler, Args&& ... args)
    {
        using handler_t = function_ptr<H>;
        auto callback = reinterpret_cast<typename handler_t::sfn*>(handler->callback);
        handler_t f{handler->observer, callback};
        f(std::forward<Args>(args)...);
    }
public:
    auto subscribe(event_id event, void* object, void(*callback)()) -> handler_id
    {
        if (callback == nullptr) {
            return {};
        }
        auto handler = allocate_handler();
        handler->observer = object;
        handler->callback = callback;
        handler->event2 = event;
        handler->next = 0;
        insert_handler_into_chain(event, handler);
        return to_index(handler) + 1;
    }
    void unsubscribe(handler_id id)
    {
        if ((id -= 1) >= handlers.size()) {
            return;
        }
        auto handler = to_ptr(id);
        if (handler->callback == nullptr) {
            return;
        }
        erase_handler_with_chain(handler);
        free_handler(handler);
    }
    template <event_id event>
    auto subscribe(const function_ptr<handler<event>>& f) -> handler_id
    {
        return subscribe(event, f.get_object_ptr(), reinterpret_cast<void(*)()>(f.get_function()));
    }
    template <class H, class ... Args>
    bool send(event_id event, Args&& ... args)
    {
        auto it = handlerChains.find(event);
        if (it == handlerChains.end()) {
            return false;
        }
        auto chainIndex = it->second;
        auto chainHead = to_ptr(chainIndex);
        if (is_single_handler_chain(chainHead)) {
            call_handler<H>(chainHead, std::forward<Args>(args)...);
            return true;
        }
        for(auto& handler : *chainHead) {
            call_handler<H>(&handler, std::forward<Args>(args)...);
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
    std::ptrdiff_t freeHandlersHead = -1;
};

} // namespace dse::util

#endif // DSE_UTIL_EVTMGR_H
