// ======================================================================
//
// ImageDesignStartMessage.h
//
// Copyright 2002-2004 Sony Online Entertainment
//
// ======================================================================

#ifndef	_ImageDesignStartMessage_H
#define	_ImageDesignStartMessage_H

//-----------------------------------------------------------------------

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

//-----------------------------------------------------------------------

class ImageDesignStartMessage : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:
	ImageDesignStartMessage(NetworkId const & designerId, NetworkId const & recipientId, NetworkId const & terminalId, std::string const & currentHoloEmote);

public:
	NetworkId const & getDesignerId() const;
	NetworkId const & getRecipientId() const;
	NetworkId const & getTerminalId() const;
	std::string const & getCurrentHoloEmote() const;

private:
//disabled
	ImageDesignStartMessage(ImageDesignStartMessage const & source);

private:
	NetworkId m_designerId;
	NetworkId m_recipientId;
	NetworkId m_terminalId;
	std::string m_currentHoloEmote;
};

// ----------------------------------------------------------------------

inline NetworkId const & ImageDesignStartMessage::getDesignerId() const
{
	return m_designerId;
}

// ----------------------------------------------------------------------

inline NetworkId const & ImageDesignStartMessage::getRecipientId() const
{
	return m_recipientId;
}

// ----------------------------------------------------------------------

inline NetworkId const & ImageDesignStartMessage::getTerminalId() const
{
	return m_terminalId;
}

// ----------------------------------------------------------------------

inline std::string const & ImageDesignStartMessage::getCurrentHoloEmote() const
{
	return m_currentHoloEmote;
}

// ----------------------------------------------------------------------

#endif // _ImageDesignStartMessage_H

