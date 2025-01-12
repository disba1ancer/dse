#ifndef DSE_CORE_DETAIL_WINDOW_H
#define DSE_CORE_DETAIL_WINDOW_H

#include <dse/util/evtmgr.h>

namespace dse::core {

enum class WindowEvent : unsigned {
    Close,
    Resize,
    Key,
    MouseMove,
    System = 0x10000
};

inline constexpr auto operator+(WindowEvent a, unsigned b) -> WindowEvent
{
    return WindowEvent{std::to_underlying(a) + b};
}

inline constexpr auto operator-(WindowEvent a, WindowEvent b) -> unsigned
{
    return std::to_underlying(a) - std::to_underlying(b);
}

enum class KeyboardKeyState {
	UP,
	DOWN,
	PRESSED
};

}

namespace dse::util {

template <>
struct event_traits<core::WindowEvent::Close> {
    using handler = void();
};

template <>
struct event_traits<core::WindowEvent::Resize> {
    using handler = void();
};

template <>
struct event_traits<core::WindowEvent::Key> {
    using handler = void(core::KeyboardKeyState, int);
};

template <>
struct event_traits<core::WindowEvent::MouseMove> {
    using handler = void(int x, int y);
};

}

namespace dse::core {

enum class WindowShowCommand {
	Hide,
	Show,
	ShowMinimized,
	ShowNormal,
	ShowMaximized,
	ShowRestored,
	ShowFullScreen
};

enum class WindowFrameStyle {
	None,
	Fixed,
	Sizable
};

}

#endif // DSE_CORE_DETAIL_WINDOW_H
