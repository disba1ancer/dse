/*
 * dse_obscon.h
 *
 *  Created on: 9 мар. 2019 г.
 *      Author: Anton
 */

#ifndef DSE_OBSCON_H_
#define DSE_OBSCON_H_

#include <stdexcept>
#include "obsbase.h"
#include "util.h"

namespace dse {
namespace core {

template <typename Owner>
class Connection;

template <>
class Connection<void> : public IConBase {
	void* observer;
	IObservableLinkController* linkController;
public:
	Connection();
	Connection(void* observer, IObservableLinkController* linkController);
	~Connection();
	Connection(const Connection&) = delete;
	Connection(Connection&& src);
	Connection& operator =(const Connection&) = delete;
	Connection& operator =(Connection&& src);
	void unlink() override;
	void move(IObservableLinkController* dst) override;
	void* getObserver() override;
	template <typename Owner>
	friend class Connection;
};

template <typename Owner>
class Connection : public IConBase, private dse::util::OwnerStore<Owner, Connection<Owner>> {
	friend class dse::util::OwnerStore<Owner, Connection<Owner>>;
	IObservableLinkController* linkController;
public:
	Connection();
	Connection(Connection<void>&& con);
	~Connection();
	Connection(const Connection&) = delete;
	Connection(Connection&& src);
	Connection& operator =(const Connection&) = delete;
	Connection& operator =(Connection&& src);
	void unlink();
	void move(IObservableLinkController* dst);
	void* getObserver();
};

template <typename Owner>
Connection<Owner>::Connection() : dse::util::OwnerStore<Owner, Connection>(nullptr), linkController(nullptr) {}

template <typename Owner>
Connection<Owner>::Connection(Connection<void>&& con) : dse::util::OwnerStore<Owner, Connection>(static_cast<Owner*>(con.observer)), linkController(con.linkController) {
	linkController->move(this, &con);
	con.linkController = nullptr;
	con.observer = nullptr;
}

template <typename Owner>
Connection<Owner>::~Connection() {
	unlink();
}

template <typename Owner>
Connection<Owner>::Connection(Connection&& src) : dse::util::OwnerStore<Owner, Connection<Owner>>(src), linkController(src.linkController) {
	linkController->move(this, &src);
	src.linkController = nullptr;
}

template <typename Owner>
Connection<Owner>& Connection<Owner>::operator =(Connection&& src) {
	if (linkController) throw std::runtime_error("Couldn't link to linked connection");
	linkController = src.linkController;
	src.linkController = nullptr;
	linkController->move(this, &src);
	return *this;
}

template <typename Owner>
void Connection<Owner>::unlink() {
	if (linkController) {
		linkController->unlink(this);
		linkController = nullptr;
	}
}

template <typename Owner>
void Connection<Owner>::move(IObservableLinkController* dst) {
	linkController = dst;
}

template <typename Owner>
void* Connection<Owner>::getObserver() {
	return this->getOwner();
}

inline Connection<void>::Connection() : observer(nullptr), linkController(nullptr) {}

inline Connection<void>::Connection(void* observer, IObservableLinkController* linkController) : observer(observer), linkController(linkController) {}

inline Connection<void>::~Connection() {
	unlink();
}

inline Connection<void>::Connection(Connection&& src) : observer(src.observer), linkController(src.linkController) {
	linkController = nullptr;
	observer = nullptr;
}

inline Connection<void>& Connection<void>::operator =(Connection&& src) {
	if (linkController) linkController->unlink(this);
	linkController = src.linkController;
	observer = src.observer;
	src.linkController = nullptr;
	src.observer = nullptr;
	linkController->move(this, &src);
	return *this;
}

inline void Connection<void>::unlink() {
	if (linkController) {
		linkController->unlink(this);
		linkController = nullptr;
	}
}

inline void Connection<void>::move(IObservableLinkController* dst) {
	linkController = dst;
}

inline void* Connection<void>::getObserver() {
	return this->observer;
}

}
}

#endif /* DSE_OBSCON_H_ */
