/*
 * dse_obsbase.h
 *
 *  Created on: 9 мар. 2019 г.
 *      Author: Anton
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
