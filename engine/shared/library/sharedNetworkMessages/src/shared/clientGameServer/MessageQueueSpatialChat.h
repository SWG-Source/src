//======================================================================
//
// MessageQueueSpatialChat.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_MessageQueueSpatialChat_H
#define INCLUDED_MessageQueueSpatialChat_H

//======================================================================

#include "sharedFoundation/MessageQueue.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;

//-----------------------------------------------------------------------

/**
* A spatial chat message is issued on an server object's MessageQueue if the object
* observes 'source' speaking to 'target'.
*
* A spatial chat message is issued on an client object's MessageQueue if it
* wants to speak.
*
*/

class MessageQueueSpatialChat : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:
	
	enum Flags
	{
		F_isPrivate       = 0x0001,  // message text is intended for target only.
		F_skipTarget      = 0x0002,
		F_skipSource      = 0x0004,
		F_targetOnly      = 0x0008,
		F_targetGroupOnly = 0x0010,
		F_shipPilot       = 0x0020,
		F_shipOperations  = 0x0040,
		F_shipGunner      = 0x0080,
		F_ship            = F_shipPilot|F_shipOperations|F_shipGunner,
		F_targetAndSourceGroup = 0x01000,
	};

	MessageQueueSpatialChat (const NetworkId & sourceId,
		const NetworkId &       targetId,
		const Unicode::String & text,
		uint16                  volume,
		uint16                  chatType,
		uint16                  moodType,
		uint32                  flags,
		uint32                  language,
		const Unicode::String & outOfBand,
		const Unicode::String & sourceName = Unicode::emptyString);
	
	MessageQueueSpatialChat  (const MessageQueueSpatialChat&);
	~MessageQueueSpatialChat ();

	const NetworkId &             getSourceId     () const;
	const NetworkId &             getTargetId     () const;
	const Unicode::String &       getText         () const;
	const uint16                  getVolume       () const;
	const uint16                  getChatType     () const;
	const uint16                  getMoodType     () const;
	const uint32                  getFlags        () const;
	const uint32                  getLanguage     () const;
	const Unicode::String &       getOutOfBand    () const;
	const Unicode::String &       getSourceName   () const;

private:

	MessageQueueSpatialChat();
	MessageQueueSpatialChat& operator= (const MessageQueueSpatialChat&);

private:

	Unicode::String  m_text; 
	NetworkId        m_sourceId;
	NetworkId        m_targetId;
	uint32           m_flags;
	uint16           m_volume;
	uint16           m_chatType;
	uint16           m_moodType;
	uint8            m_language;
	Unicode::String  m_outOfBand;
	Unicode::String  m_sourceName;
};

//----------------------------------------------------------------------

inline const NetworkId &       MessageQueueSpatialChat::getSourceId     () const { return m_sourceId; }
inline const NetworkId &       MessageQueueSpatialChat::getTargetId     () const { return m_targetId; }
inline const Unicode::String & MessageQueueSpatialChat::getText         () const { return m_text; }
inline const uint16            MessageQueueSpatialChat::getVolume       () const { return m_volume; }
inline const uint16            MessageQueueSpatialChat::getChatType     () const { return m_chatType; }
inline const uint16            MessageQueueSpatialChat::getMoodType     () const { return m_moodType; }
inline const uint32            MessageQueueSpatialChat::getFlags        () const { return m_flags; }
inline const Unicode::String & MessageQueueSpatialChat::getOutOfBand    () const { return m_outOfBand; }
inline const uint32            MessageQueueSpatialChat::getLanguage     () const { return static_cast<uint32> (m_language); }
inline const Unicode::String & MessageQueueSpatialChat::getSourceName   () const { return m_sourceName; }

//======================================================================

#endif
