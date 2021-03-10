/*
 * notifier.h
 *
 *  Created on: 28 дек. 2019 г.
 *      Author: disba1ancer
 */

#ifndef NOTIFIER_H_
#define NOTIFIER_H_

#include <list>
#include <functional>
#include "util/emptylock.h"
#include <mutex>

namespace dse {
namespace notifier {

template <typename T, typename Lockable = util::emptylock>
class connection;

template <typename T, typename Lockable = util::emptylock>
class notifier;

template <typename R, typename ... Args, typename Lockable>
class notifier<R(Args...), Lockable> {
public:
	typedef std::function<R(Args...)> callback;
	typedef connection<R(Args...), Lockable> connection;
private:
	std::list<std::pair<connection*, callback>> callbacks;
	Lockable lock;
public:
	notifier() = default;
	notifier(const notifier&) = delete;
	notifier& operator =(const notifier&) = delete;
	notifier(notifier&&) = delete;
	notifier& operator =(notifier&&) = delete;

	~notifier() {
		unsubscribeAll();
	}

	connection subscribe(callback&& c) {
		std::lock_guard lock(this->lock);
		auto iter = callbacks.emplace(callbacks.end(), std::make_pair<connection*>(nullptr, std::move(c)));
		return connection(this, iter);
	}

	void unsubscribe(connection* c) {
		std::lock_guard lock(this->lock);
		unsubscribe_lockless(c);
	}
private:
	void unsubscribe_lockless(connection* c) {
		callbacks.erase(c->it);
		c->notif = nullptr;
	}
public:
	void notify(Args...args) {
		std::lock_guard lock(this->lock);
		for (auto& c : callbacks) {
			c.second(args...);
		}
	}

	void unsubscribeAll() {
		std::lock_guard lock(this->lock);
		typename connection::iterator it;
		while ((it = callbacks.begin()) != callbacks.end()) {
			unsubscribe_lockless(it->first);
		}
	}
};

template <typename R, typename ... Args, typename Lockable>
class connection<R(Args...), Lockable> {
	typedef notifier<R(Args...), Lockable> notifier;
	typedef typename notifier::callback callback;
	friend notifier;
	typedef typename std::list<std::pair<connection*, callback>>::iterator iterator;

	notifier* notif;
	iterator it;

	connection(notifier* n, iterator it) : notif(n), it(it) {
		it->first = this;
	}
public:
	connection() : notif(nullptr) {}
	connection(const connection&) = delete;
	connection& operator =(const connection&) = delete;

	connection(connection&& con) : notif(con.notif), it(con.it) {
		if (notif) {
			con.notif = nullptr;
			it->first = this;
		}
	}

	connection& operator =(connection&& con) {
		swap(*this, con);
		return *this;
	}

	static void swap(connection& a, connection& b) {
		std::swap(a.notif, b.notif);
		std::swap(a.it, b.it);
		if (a.notif) {
			a.it->first = &a;
		}
		if (b.notif) {
			b.it->first = &b;
		}
	}

	~connection() {
		unsubscribe();
	}

	bool connected() const {
		return notif;
	}

	void unsubscribe() {
		if (notif) {
			notif->unsubscribe(this);
		}
	}
};

} // namespace notifier
} // namespace dse

#endif /* NOTIFIER_H_ */
