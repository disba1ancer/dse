/*
 * com.h
 *
 *  Created on: 12 сент. 2020 г.
 *      Author: disba1ancer
 */

#ifndef WIN32_COM_H_
#define WIN32_COM_H_

#include <windows.h>

namespace swal {

struct com_deleter {
	void operator ()(void* ptr) const { CoTaskMemFree(ptr); }
};

}

#endif /* WIN32_COM_H_ */
