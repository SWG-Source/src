//========================================================================
//
// MessageQueueActivateInstallation.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================


#ifndef INCLUDED_MessageQueueActivateInstallation_H
#define INCLUDED_MessageQueueActivateInstallation_H

#include "sharedFoundation/MessageQueue.h"

// ======================================================================

/**
 * Message sent to a installation to tell it to activate itself.
 */

class MessageQueueActivateInstallation : public MessageQueue::Data
{
public:
	MessageQueueActivateInstallation();
	virtual ~MessageQueueActivateInstallation();
	
	MessageQueueActivateInstallation&	operator=	(const MessageQueueActivateInstallation & source);
	MessageQueueActivateInstallation(const MessageQueueActivateInstallation & source);
	
private:
};

//----------------------------------------------------------------------

#endif	// INCLUDED_MessageQueueActivateInstallation_H
