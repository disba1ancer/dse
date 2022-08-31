/*
 * Context.h
 *
 *  Created on: 8 янв. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SUBSYS_GL_CONTEXT_H_
#define SUBSYS_GL_CONTEXT_H_

#ifdef _WIN32
#include <dse/core/win32.h>
#endif
#include <type_traits>
#include "gl.h"
#include "TrvMvOnlyRes.h"
#include <dse/core/Window.h>
#include <dse/util/enum_bitwise.h>

namespace dse::ogl31rbe::glwrp {

enum class ContextVersion {
	legacy,
	gl10,
	gl11,
	gl12,
	gl13,
	gl14,
	gl15,
	gl20,
	gl21,
	gl30,
	gl31,
	gl32,
	gl33,
	gl40,
	gl41,
	gl42,
	gl43,
	gl44,
	gl45,
	gl46
};

enum class ContextFlags {
	Debug = WGL_CONTEXT_DEBUG_BIT_ARB,
	ForwardCompatible = WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
};

} namespace dse::util {
template <> struct enable_enum_bitwise<dse::ogl31rbe::glwrp::ContextFlags> :
	std::true_type {};
} namespace dse::ogl31rbe::glwrp {

class Context {
#ifdef _WIN32
	struct ContextOwner : public TrvMvOnlyRes<HGLRC, true> {
		ContextOwner() noexcept;
		ContextOwner(HGLRC glrc) noexcept;
		ContextOwner(const ContextOwner&) = delete;
		ContextOwner(ContextOwner&&) noexcept = default;
		ContextOwner& operator=(const ContextOwner&) = delete;
		ContextOwner& operator=(ContextOwner&&) noexcept = default;
		~ContextOwner();
	};
	swal::WindowDC dc;
	ContextOwner glrc;
	int vsync = 0;
	static thread_local Context* currentContext;
#endif
public:
	Context(core::Window& wnd, ContextVersion ver, ContextFlags flags);
	~Context();
	Context(const Context &other) = delete;
	Context(Context &&other) = delete;
	Context& operator=(const Context &other) = delete;
	Context& operator=(Context &&other) = delete;
	void SwapBuffers();
	void enableVSync(int val);
	static auto getProcAddress(const char* name) -> void(*)();
	static auto makeLegacyContext(swal::WindowDC& dc) -> ContextOwner;
	static auto makeContext(swal::WindowDC& dc, ContextVersion ver, ContextFlags flags) -> ContextOwner;
	void MakeCurrent(core::Window& wnd);
	void MakeCurrent();
	static void MakeCurrentEmpty();
private:
	void MakeCurrentInternal();
};

} /* namespace dse::ogl31rbe::glwrp */

#endif /* SUBSYS_GL_CONTEXT_H_ */
