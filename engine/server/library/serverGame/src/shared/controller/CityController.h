// ======================================================================
//
// CityController.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	INCLUDED_CityController_H
#define	INCLUDED_CityController_H

// ======================================================================

#include "serverGame/UniverseController.h"

// ======================================================================

class CityObject;

// ======================================================================

class CityController: public UniverseController
{
public:
	explicit CityController(CityObject *newOwner);
	~CityController();

protected:
	virtual void handleMessage(int message, float value, const MessageQueue::Data* data, uint32 flags);

private:
	CityController();
	CityController(CityController const &);
	CityController& operator=(CityController const &);
};

// ======================================================================

#endif

