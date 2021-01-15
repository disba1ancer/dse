/*
 * com.h
 *
 *  Created on: 12 сент. 2020 г.
 *      Author: disba1ancer
 */

#ifndef SWAL_COM_H
#define SWAL_COM_H

#include "win_headers.h"

namespace swal {

struct com_deleter {
	void operator ()(void* ptr) const { CoTaskMemFree(ptr); }
};

}

#endif /* SWAL_COM_H */
