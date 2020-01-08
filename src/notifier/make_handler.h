/*
 * make_handler.h
 *
 *  Created on: 8 янв. 2020 г.
 *      Author: disba1ancer
 */

#ifndef NOTIFIER_MAKE_HANDLER_H_
#define NOTIFIER_MAKE_HANDLER_H_

namespace dse {
namespace notifier {

template <auto T>
class make_handler;

template <typename Ret, typename Obj, typename ... Args, Ret(Obj::*method)(Args...)>
class make_handler<method> {
	Obj* obj;
public:
	make_handler(Obj* obj) : obj(obj) {}
	make_handler(Obj& obj) : obj(&obj) {}
	Ret operator()(Args ... args) { return (obj->*method)(args...); }
};

} // namespace notifier
} // namespace dse

#endif /* NOTIFIER_MAKE_HANDLER_H_ */
