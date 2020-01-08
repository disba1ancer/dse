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
 * GLImportedFunction.h
 *
 *  Created on: 16 сент. 2018 г.
 *      Author: Anton
 */

#ifndef IMPORTEDFUNCTION_H_
#define IMPORTEDFUNCTION_H_

#include <stdexcept>
#ifdef _WIN32
#include "../../os/win32.h"
#endif

template <typename Callback> class GLImportedFunction;

template <typename RetVal, typename ... Args>
class GLImportedFunction<RetVal(APIENTRY *)(Args...)> {
	using Callback = RetVal(APIENTRY *)(Args...);
	const char *functionName;
	Callback glFunc;
public:
	constexpr GLImportedFunction(const char *functionName) : functionName(functionName), glFunc(nullptr) {}
	void load() {
		if (!glFunc) {
			glFunc = reinterpret_cast<Callback>(
					wglGetProcAddress(functionName)
			);
			if (!glFunc) {
				throw std::runtime_error("Couldn't load OpenGL function");
			}
		}
	}

	void reset() {
		glFunc = 0;
	}

	RetVal operator()(Args...args) {
		load();
		return glFunc(args...);
	}

	operator bool() {
		return glFunc != nullptr;
	}
};

#define IMPLEMENT_GL_FUNCTION(_name, _type) GLImportedFunction<PFN ## _type ## PROC> _name(#_name)
#define IMPORT_GL_FUNCTION(_name, _type) extern GLImportedFunction<PFN ## _type ## PROC> _name

#endif /* IMPORTEDFUNCTION_H_ */
