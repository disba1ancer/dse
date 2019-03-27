/*******************************************************************************
 * DSE - disba1ancer's (graphic) engine.
 *
 * Copyright (c) 2019 disba1ancer.
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *******************************************************************************/
/*
 * dse_obstore.h
 *
 *  Created on: 9 мар. 2019 г.
 *      Author: disba1ancer
 */

#ifndef DSE_OBSTORE_H_
#define DSE_OBSTORE_H_

#include <map>

#include "obsbase.h"

namespace dse {
namespace core {

template <typename M>
class ObserverStore;

template <typename ... Args>
class ObserverStore<void(Args...)> : public IObservableLinkController {
	/*struct Observer {
		void* object;
		void (*call)(void*, Args...);
	};*/

	std::map<IConBase*, void (*)(void*, Args...)> observers;
public:
	ObserverStore() = default;
	ObserverStore(const ObserverStore&) = delete;
	ObserverStore(ObserverStore&& src);
	~ObserverStore();
	ObserverStore& operator =(const ObserverStore&) = delete;
	ObserverStore& operator =(ObserverStore&& src);
	Connection<void> add(void* object, void (*method)(void*, Args...));
	void unlink(IConBase* con) override;
	void move(IConBase* dst, IConBase* src) override;
	void operator ()(Args ... args);
};

template <typename ... Args>
ObserverStore<void(Args...)>::ObserverStore(ObserverStore&& src) : observers(std::move(src.observers)) {
	for (auto pair : observers) {
		pair.first->move(this);
	}
}

template <typename ... Args>
ObserverStore<void(Args...)>::~ObserverStore() {
	for (auto pair : observers) {
		pair.first->unlink();
	}
}

template <typename ... Args>
ObserverStore<void(Args...)>& ObserverStore<void(Args...)>::operator =(ObserverStore&& src) {
	for (auto pair : observers) {
		pair.first->unlink();
	}
	observers = std::move(src.observers);
	for (auto pair : observers) {
		pair.first->move(this);
	}
}

template <typename ... Args>
Connection<void> ObserverStore<void(Args...)>::add(void* object, void (*method)(void*, Args...)) {
	Connection<void> con(object, this);
	observers.insert(typename decltype(observers)::value_type(&con, method));
	return con;
}

template <typename ... Args>
void ObserverStore<void(Args...)>::unlink(IConBase* con) {
	observers.erase(observers.find(con));
}

template <typename ... Args>
void ObserverStore<void(Args...)>::move(IConBase* dst, IConBase* src) {
	auto iter = observers.find(src);
	auto pair = *iter;
	//pair.first = dst;
	observers.erase(iter);
	observers.insert(typename decltype(observers)::value_type(dst, pair.second));
}

template <typename ... Args>
void ObserverStore<void(Args...)>::operator ()(Args ... args) {
	for (auto pair : observers) {
		pair.second(pair.first->getObserver(), args...);
	}
}

}
}

#endif /* DSE_OBSTORE_H_ */
