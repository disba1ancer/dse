#ifndef DSE_CORE_DETAIL_STATICMOD_H
#define DSE_CORE_DETAIL_STATICMOD_H

#include "impexp.h"

namespace dse::core {

extern "C" {
void API_DSE_CORE dse_core_RegisterModule(const char8_t* name, void (*(*getProc)(const char8_t*))()) noexcept;
}

}

#endif // DSE_CORE_DETAIL_STATICMOD_H
