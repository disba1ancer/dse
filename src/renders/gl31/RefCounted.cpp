#include "RefCounted.h"

namespace dse {
namespace renders {
namespace gl31 {

RefCounted::RefCounted() :
	refcount(0)
{}

void RefCounted::AddRef()
{
	++refcount;
}

void RefCounted::Release()
{
	if (refcount) {
		--refcount;
	}
}

bool RefCounted::isNoRefs()
{
	return refcount == 0;
}

} // namespace gl31
} // namespace renders
} // namespace dse
