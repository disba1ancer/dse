/*
 * Object.h
 *
 *  Created on: 18 янв. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SCN_OBJECT_H_
#define SCN_OBJECT_H_

namespace dse {
namespace scn {

class Object {
public:
	Object();
	~Object();
	Object(const Object &other) = default;
	Object(Object &&other) = default;
	Object& operator=(const Object &other) = default;
	Object& operator=(Object &&other) = default;
};

} /* namespace scn */
} /* namespace dse */

#endif /* SCN_OBJECT_H_ */
