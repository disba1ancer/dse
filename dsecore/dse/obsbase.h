/*******************************************************************************
 * DSE - disba1ancer's (graphic) engine.
 *
 * Copyright (c) 2019 ${user}.
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
 * dse_obsbase.h
 *
 *  Created on: 9 мар. 2019 г.
 *      Author: disba1ancer
 */

#ifndef DSE_OBSBASE_H_
#define DSE_OBSBASE_H_

namespace dse {
namespace core {

struct IObservableLinkController;

struct IConBase{
	virtual void* getObserver() = 0;
	virtual void unlink() = 0;
	virtual void move(IObservableLinkController* dst) = 0;
protected:
	~IConBase() = default;
};

struct IObservableLinkController {
	virtual void unlink(IConBase* con) = 0;
	virtual void move(IConBase* dst, IConBase* src) = 0;
protected:
	~IObservableLinkController() = default;
};

}
}

#endif /* DSE_OBSBASE_H_ */
