// CellController.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved.
// Author: Justin Randall

#ifndef	_INCLUDED_CellController_H
#define	_INCLUDED_CellController_H

//-----------------------------------------------------------------------

#include "serverGame/ServerController.h"

class CellObject;

//-----------------------------------------------------------------------

class CellController : public ServerController
{
public:
	explicit CellController(CellObject * newOwner);
	~CellController();

protected:
	virtual void handleMessage(int message, float value, const MessageQueue::Data* data, uint32 flags);
	virtual void setGoal(Transform const & newGoal, ServerObject * goalCellObject, bool teleport);

private:
	CellController & operator = (const CellController & rhs);
	CellController(const CellController & source);
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_CellController_H
