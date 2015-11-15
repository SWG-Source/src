//========================================================================
//
// MessageQueueSelectProfessionTemplate.h
//
// copyright 2005 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================


#ifndef INCLUDED_MessageQueueSelectProfessionTemplate_H
#define INCLUDED_MessageQueueSelectProfessionTemplate_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;

class MessageQueueSelectProfessionTemplate : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	         MessageQueueSelectProfessionTemplate(std::string const & professionTemplate);
	virtual ~MessageQueueSelectProfessionTemplate();

	std::string const & getProfessionTemplate() const;

private:

	std::string m_professionTemplate;

private:
	MessageQueueSelectProfessionTemplate();
	MessageQueueSelectProfessionTemplate(const MessageQueueSelectProfessionTemplate &);
	MessageQueueSelectProfessionTemplate & operator=(const MessageQueueSelectProfessionTemplate &);
};

//----------------------------------------------------------------------

inline std::string const & MessageQueueSelectProfessionTemplate::getProfessionTemplate() const
{
	return m_professionTemplate;
}

//----------------------------------------------------------------------

#endif	// INCLUDED_MessageQueueSelectProfessionTemplate_H
