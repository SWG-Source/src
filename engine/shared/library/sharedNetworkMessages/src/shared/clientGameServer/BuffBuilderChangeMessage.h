// BuffBuilderChangeMessage.h
// Copyright 2006, Sony Online Entertainment Inc., all rights reserved. 

#ifndef	_INCLUDED_BuffBuilderChangeMessage_H
#define	_INCLUDED_BuffBuilderChangeMessage_H

//-----------------------------------------------------------------------

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"
class MemoryBlockManager;

//-----------------------------------------------------------------------

class BuffBuilderChangeMessage : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	enum Origin
	{
		O_BUFFER,
		O_RECIPIENT,
		O_SERVER,
		O_UNKNOWN
	};

public:
	BuffBuilderChangeMessage();
	BuffBuilderChangeMessage & operator=(const BuffBuilderChangeMessage & rhs);

	//accessors
	NetworkId const & getBufferId() const;
	NetworkId const & getRecipientId() const;
	time_t getStartingTime() const;
	int getBufferRequiredCredits() const;
	bool getAccepted() const;
	Origin getOrigin() const;
	std::map<std::string, std::pair<int,int> > const & getBuffComponents () const;

	//mutators
	void setBufferId(NetworkId const & bufferId);
	void setRecipientId(NetworkId const & recipientId);
	void setStartingTime(time_t startingTime);
	void setBufferRequiredCredits(int credits);
	void setAccepted(bool accepted);
	void setOrigin(Origin const & origin);
	void addBuffComponent(std::string const & name, int expertiseModifier);
	void setBuffComponents(std::map<std::string, std::pair<int,int> > const & indexChanges);

private:
	//disabled
	BuffBuilderChangeMessage(BuffBuilderChangeMessage const & source);

private:
	NetworkId m_bufferId;
	NetworkId m_recipientId;
	time_t m_startingTime;
	int m_bufferRequiredCredits;
	bool m_accepted;
	Origin m_origin;
	std::map<std::string, std::pair<int,int> > m_buffComponents;
};

//-----------------------------------------------------------------------

inline NetworkId const & BuffBuilderChangeMessage::getBufferId() const
{
	return m_bufferId;
}

//-----------------------------------------------------------------------

inline NetworkId const & BuffBuilderChangeMessage::getRecipientId() const
{
	return m_recipientId;
}

//-----------------------------------------------------------------------

inline time_t BuffBuilderChangeMessage::getStartingTime() const
{
	return m_startingTime;
}

//-----------------------------------------------------------------------

inline int BuffBuilderChangeMessage::getBufferRequiredCredits() const
{
	return m_bufferRequiredCredits;
}

//-----------------------------------------------------------------------

inline bool BuffBuilderChangeMessage::getAccepted() const
{
	return m_accepted;
}

//-----------------------------------------------------------------------

inline BuffBuilderChangeMessage::Origin BuffBuilderChangeMessage::getOrigin() const
{
	return m_origin;
}

//-----------------------------------------------------------------------

inline void BuffBuilderChangeMessage::setBufferId(NetworkId const & bufferId)
{
	m_bufferId = bufferId;
}

//-----------------------------------------------------------------------

inline void BuffBuilderChangeMessage::setRecipientId(NetworkId const & recipientId)
{
	m_recipientId = recipientId;
}

//-----------------------------------------------------------------------

inline void BuffBuilderChangeMessage::setStartingTime(time_t const startingTime)
{
	m_startingTime = startingTime;
}

//-----------------------------------------------------------------------

inline void BuffBuilderChangeMessage::setBufferRequiredCredits(int const credits)
{
	if(credits < 0)
		m_bufferRequiredCredits = 0;
	m_bufferRequiredCredits = credits;
}

//-----------------------------------------------------------------------

inline void BuffBuilderChangeMessage::setAccepted(bool const accepted)
{
	m_accepted = accepted;
}

//-----------------------------------------------------------------------

inline void BuffBuilderChangeMessage::setOrigin(BuffBuilderChangeMessage::Origin const & origin)
{
	m_origin = origin;
}

//-----------------------------------------------------------------------

inline std::map<std::string, std::pair<int,int> > const & BuffBuilderChangeMessage::getBuffComponents() const
{
	return m_buffComponents;
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_BuffBuilderChangeMessage_H
