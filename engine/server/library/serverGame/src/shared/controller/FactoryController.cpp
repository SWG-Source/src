// FactoryController.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverGame/FirstServerGame.h"
#include "serverGame/FactoryController.h"
#include "serverGame/FactoryObject.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedObject/NetworkIdManager.h"

//-----------------------------------------------------------------------

FactoryController::FactoryController(FactoryObject * o) :
TangibleController(o)
{
}

//-----------------------------------------------------------------------

FactoryController::~FactoryController()
{
}

//-----------------------------------------------------------------------

void FactoryController::handleMessage (int message, float value, const MessageQueue::Data* data, uint32 flags)
{
	FactoryObject * owner = safe_cast<FactoryObject *>(getOwner());
	if(! owner)
	{
		TangibleController::handleMessage(message, value, data, flags);
		return;
	}

//	switch(message)
//	{
//	default:
		TangibleController::handleMessage(message, value, data, flags);
//		break;
//	}
}

//-----------------------------------------------------------------------

