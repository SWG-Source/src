//========================================================================
//
// MessageQueueInstallationHarvest.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================


#ifndef INCLUDED_MessageQueueInstallationHarvest_H
#define INCLUDED_MessageQueueInstallationHarvest_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;
/**
 * Sent by:  anything
 * Sent to:  installation
 * Action:  Causes the installation to place all the resources/objects it has gathered
 * so far into its "hopper"
 */

class MessageQueueInstallationHarvest : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:
	MessageQueueInstallationHarvest();
	virtual ~MessageQueueInstallationHarvest();
	
	MessageQueueInstallationHarvest&	operator=	(const MessageQueueInstallationHarvest & source);
	MessageQueueInstallationHarvest(const MessageQueueInstallationHarvest & source);
	
private:
};

// ======================================================================

#endif
