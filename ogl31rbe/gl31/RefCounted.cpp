#include "RefCounted.h"

namespace dse::ogl31rbe::gl31 {

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

} // namespace dse::ogl31rbe::gl31
