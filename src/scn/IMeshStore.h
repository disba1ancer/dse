/*
 * IMeshStore.h
 *
 *  Created on: 4 мар. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SCN_IMESHSTORE_H_
#define SCN_IMESHSTORE_H_

#include "IMesh.h"
#include <map>

namespace dse {
namespace scn {

class IMeshStore: public IMesh {
	std::map<void*, void*> store;
public:
	virtual void storeCustomValue(void *owner, void *value) override {
		if (value == nullptr) {
			store.erase(owner);
		} else {
			store[owner] = value;
		}
	}
	virtual void* getCustomValue(void *owner) override {
		auto it = store.find(owner);
		if (it != store.end()) {
			return it->second;
		}
		return nullptr;
	}
};

} /* namespace scn */
} /* namespace dse */

#endif /* SCN_IMESHSTORE_H_ */
