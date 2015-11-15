//========================================================================
//
// MessageQueueDeactivateInstallation.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================


#ifndef INCLUDED_MessageQueueDeactivateInstallation_H
#define INCLUDED_MessageQueueDeactivateInstallation_H

#include "sharedFoundation/MessageQueue.h"

//----------------------------------------------------------------------

/**
 * Message sent to a installation to tell it to deactivate itself.
 */

class MessageQueueDeactivateInstallation : public MessageQueue::Data
{
  public:
	MessageQueueDeactivateInstallation();
	virtual ~MessageQueueDeactivateInstallation();
	
	MessageQueueDeactivateInstallation&	operator=	(const MessageQueueDeactivateInstallation & source);
	MessageQueueDeactivateInstallation(const MessageQueueDeactivateInstallation & source);
	
  private:
};

//----------------------------------------------------------------------


#endif
