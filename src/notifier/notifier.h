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

namespace dse {
namespace notifier {

template <typename T>
class connection;

template <typename T>
class notifier;

template <typename R, typename ... Args>
class notifier<R(Args...)> {
public:
	typedef std::function<R(Args...)> callback;
	typedef connection<R(Args...)> connection;
private:
	std::list<std::pair<connection*, callback>> callbacks;
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
		auto iter = callbacks.emplace(callbacks.end(), std::make_pair<connection*>(nullptr, std::move(c)));
		return connection(this, iter);
	}

	void unsubscribe(connection* c) {
		callbacks.erase(c->it);
		c->notif = nullptr;
	}

	void notify(Args...args) {
		for (auto& c : callbacks) {
			c.second(args...);
		}
	}

	void unsubscribeAll() {
		for (auto& c : callbacks) {
			unsubscribe(c.first);
		}
	}
};

template <typename R, typename ... Args>
class connection<R(Args...)> {
	typedef notifier<R(Args...)> notifier;
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
	}

	static void swap(connection& a, connection& b) {
		std::swap(a.notif, b.notif);
		std::swap(a.it, b.it);
		a.it->first = &a;
		b.it->first = &b;
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
