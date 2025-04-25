#ifndef DSE_UTIL_HANDLE_H
#define DSE_UTIL_HANDLE_H

#include <utility>

namespace dse::util {

template <class T>
struct handle_traits;
// {
//     using sender = <event source type>;
//     static void kill_handle(sender&, T);
// };

template<class T>
requires std::is_scoped_enum_v<T>
struct handle_owner
{
    using handle = T;
    using sender = typename handle_traits<T>::sender;
    handle_owner() : s(nullptr), h(NullHandle) {}
    handle_owner(sender& s, handle h) : s(std::addressof(s)), h(h) {}
    handle_owner(handle_owner&&) = delete;
    handle_owner(const handle_owner&) = delete;
    handle_owner& operator=(handle_owner&&) = delete;
    handle_owner& operator=(const handle_owner&) = delete;
    ~handle_owner()
    {
        reset();
    }
    void reset()
    {
        if (h != NullHandle)
        {
            handle_traits<T>::kill_handle(*s, h);
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

} // namespace dse::util

#endif // DSE_UTIL_HANDLE_H
