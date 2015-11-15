// FactoryController.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_FactoryController_H
#define	_INCLUDED_FactoryController_H

//-----------------------------------------------------------------------

#include "serverGame/TangibleController.h"

class FactoryObject;

//-----------------------------------------------------------------------

class FactoryController : public TangibleController
{
public:
	explicit FactoryController(FactoryObject * owner);
	~FactoryController();

protected:
	virtual void handleMessage (int message, float value, const MessageQueue::Data* data, uint32 flags);

private:
	FactoryController & operator = (const FactoryController & rhs);
	FactoryController(const FactoryController & source);

};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_FactoryController_H
