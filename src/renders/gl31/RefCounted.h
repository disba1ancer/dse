#ifndef DSE_RENDERS_GL31_REFCOUNTED_H
#define DSE_RENDERS_GL31_REFCOUNTED_H

#include <cstddef>

namespace dse::renders::gl31 {

class RefCounted
{
	std::size_t refcount;
public:
	void AddRef();
	void Release();
	bool isNoRefs();
public:
    RefCounted();
};

} // namespace gl31::renders::dse

#endif // DSE_RENDERS_GL31_REFCOUNTED_H
