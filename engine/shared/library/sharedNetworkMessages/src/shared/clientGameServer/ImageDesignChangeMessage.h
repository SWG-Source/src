// ImageDesignChangeMessage.h
// Copyright 2004, Sony Online Entertainment Inc., all rights reserved. 

#ifndef	_INCLUDED_ImageDesignChangeMessage_H
#define	_INCLUDED_ImageDesignChangeMessage_H

//-----------------------------------------------------------------------

#include "sharedFoundation/MessageQueue.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;

//-----------------------------------------------------------------------

class ImageDesignChangeMessage : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	enum DesignType
	{
		DT_COSMETIC,
		DT_PHYSICAL,
		DT_STAT_MIGRATION,
		DT_UNKNOWN
	};

	enum Origin
	{
		O_DESIGNER,
		O_RECIPIENT,
		O_SERVER,
		O_UNKNOWN
	};

public:
	ImageDesignChangeMessage();
	ImageDesignChangeMessage & operator=(const ImageDesignChangeMessage & rhs);

	//accessors
	NetworkId const & getDesignerId() const;
	NetworkId const & getRecipientId() const;
	NetworkId const & getTerminalId() const;
	bool getNewHairSet() const;
	std::string const & getNewHairAsset() const;
	std::string const & getHairCustomizationData() const;
	DesignType getDesignType() const;
	time_t getStartingTime() const;
	int getDesignerRequiredCredits() const;
	int getRecipientPaidCredits() const;
	bool getAccepted() const;
	Origin getOrigin() const;
	std::map<std::string, float> const & getMorphParameterChanges () const;
	std::map<std::string, int> const & getIndexParameterChanges () const;
	std::string const & getHoloEmote() const;
	int getBodySkillMod() const;
	int getFaceSkillMod() const;
	int getMarkingsSkillMod() const;
	int getHairSkillMod() const;

	//mutators
	void setDesignerId(NetworkId const & designerId);
	void setRecipientId(NetworkId const & recipientId);
	void setTerminalId(NetworkId const & recipientId);
	void setNewHairSet(bool changed);
	void setNewHairAsset(std::string const & newAsset);
	void setHairCustomizationData(std::string const & hairCustomizationData);
	void setDesignType(DesignType const & designType);
	void setStartingTime(time_t startingTime);
	void setDesignerRequiredCredits(int credits);
	void setRecipientPaidCredits(int credits);
	void setAccepted(bool accepted);
	void setOrigin(Origin const & origin);
	void addMorphParameterChange(std::string const & name, float value);
	void addIndexParameterChange(std::string const & name, int value);
	void setMorphParameterChanges(std::map<std::string, float> const & morphChanges);
	void setIndexParameterChanges(std::map<std::string, int> const & indexChanges);
	void setHoloEmote(std::string const & holoEmote);
	void setBodySkillMod(int skillMod);
	void setFaceSkillMod(int skillMod);
	void setMarkingsSkillMod(int skillMod);
	void setHairSkillMod(int skillMod);

private:
	//disabled
	ImageDesignChangeMessage(ImageDesignChangeMessage const & source);

private:
	NetworkId m_designerId;
	NetworkId m_recipientId;
	NetworkId m_terminalId;
	bool m_newHairSet;
	std::string m_newHairAsset;
	std::string m_hairCustomizationData;
	DesignType m_designType;
	time_t m_startingTime;
	int m_designerRequiredCredits;
	int m_recipientPaidCredits;
	bool m_accepted;
	Origin m_origin;
	std::map<std::string, float> m_morphChanges;
	std::map<std::string, int> m_indexChanges;
	std::string m_holoEmote;
	int m_bodySkillMod;
	int m_faceSkillMod;
	int m_markingsSkillMod;
	int m_hairSkillMod;
};

//-----------------------------------------------------------------------

inline NetworkId const & ImageDesignChangeMessage::getDesignerId() const
{
	return m_designerId;
}

//-----------------------------------------------------------------------

inline NetworkId const & ImageDesignChangeMessage::getRecipientId() const
{
	return m_recipientId;
}

//-----------------------------------------------------------------------

inline NetworkId const & ImageDesignChangeMessage::getTerminalId() const
{
	return m_terminalId;
}

//-----------------------------------------------------------------------

inline bool ImageDesignChangeMessage::getNewHairSet() const
{
	return m_newHairSet;
}

//-----------------------------------------------------------------------

inline std::string const & ImageDesignChangeMessage::getNewHairAsset() const
{
	return m_newHairAsset;
}

//-----------------------------------------------------------------------

inline std::string const & ImageDesignChangeMessage::getHairCustomizationData() const
{
	return m_hairCustomizationData;
}

//-----------------------------------------------------------------------

inline ImageDesignChangeMessage::DesignType ImageDesignChangeMessage::getDesignType() const
{
	return m_designType;
}

//-----------------------------------------------------------------------

inline time_t ImageDesignChangeMessage::getStartingTime() const
{
	return m_startingTime;
}

//-----------------------------------------------------------------------

inline int ImageDesignChangeMessage::getDesignerRequiredCredits() const
{
	return m_designerRequiredCredits;
}

//-----------------------------------------------------------------------

inline int ImageDesignChangeMessage::getRecipientPaidCredits() const
{
	return m_recipientPaidCredits;
}

//-----------------------------------------------------------------------

inline bool ImageDesignChangeMessage::getAccepted() const
{
	return m_accepted;
}

//-----------------------------------------------------------------------

inline ImageDesignChangeMessage::Origin ImageDesignChangeMessage::getOrigin() const
{
	return m_origin;
}

//-----------------------------------------------------------------------

inline std::string const & ImageDesignChangeMessage::getHoloEmote() const
{
	return m_holoEmote;
}


//-----------------------------------------------------------------------

inline int ImageDesignChangeMessage::getBodySkillMod() const
{
	return m_bodySkillMod;
}

//-----------------------------------------------------------------------

inline int ImageDesignChangeMessage::getFaceSkillMod() const
{
	return m_faceSkillMod;
}

//-----------------------------------------------------------------------

inline int ImageDesignChangeMessage::getMarkingsSkillMod() const
{
	return m_markingsSkillMod;
}

//-----------------------------------------------------------------------

inline int ImageDesignChangeMessage::getHairSkillMod() const
{
	return m_hairSkillMod;
}

//-----------------------------------------------------------------------

inline void ImageDesignChangeMessage::setDesignerId(NetworkId const & designerId)
{
	m_designerId = designerId;
}

//-----------------------------------------------------------------------

inline void ImageDesignChangeMessage::setRecipientId(NetworkId const & recipientId)
{
	m_recipientId = recipientId;
}

//-----------------------------------------------------------------------

inline void ImageDesignChangeMessage::setTerminalId(NetworkId const & terminalId)
{
	m_terminalId = terminalId;
}

//-----------------------------------------------------------------------

inline void ImageDesignChangeMessage::setNewHairSet(bool const changed)
{
	m_newHairSet = changed;
}

//-----------------------------------------------------------------------

inline void ImageDesignChangeMessage::setNewHairAsset(std::string const & newAsset)
{
	m_newHairAsset = newAsset;
}

//-----------------------------------------------------------------------

inline void ImageDesignChangeMessage::setHairCustomizationData(std::string const & hairCustomizationData)
{
	m_hairCustomizationData = hairCustomizationData;
}

//-----------------------------------------------------------------------

inline void ImageDesignChangeMessage::setDesignType(DesignType const & designType)
{
	m_designType = designType;
}

//-----------------------------------------------------------------------

inline void ImageDesignChangeMessage::setStartingTime(time_t const startingTime)
{
	m_startingTime = startingTime;
}

//-----------------------------------------------------------------------

inline void ImageDesignChangeMessage::setDesignerRequiredCredits(int const credits)
{
	if(credits < 0)
		m_designerRequiredCredits = 0;
	m_designerRequiredCredits = credits;
}

//-----------------------------------------------------------------------

inline void ImageDesignChangeMessage::setRecipientPaidCredits(int const credits)
{
	if(credits < 0)
		m_recipientPaidCredits = 0;
	m_recipientPaidCredits = credits;
}

//-----------------------------------------------------------------------

inline void ImageDesignChangeMessage::setAccepted(bool const accepted)
{
	m_accepted = accepted;
}

//-----------------------------------------------------------------------

inline void ImageDesignChangeMessage::setOrigin(ImageDesignChangeMessage::Origin const & origin)
{
	m_origin = origin;
}

//-----------------------------------------------------------------------

inline void ImageDesignChangeMessage::setHoloEmote(std::string const & holoEmote)
{
	m_holoEmote = holoEmote;
}

//-----------------------------------------------------------------------

inline void ImageDesignChangeMessage::setBodySkillMod(int const skillMod)
{
	m_bodySkillMod = skillMod;
}

//-----------------------------------------------------------------------

inline void ImageDesignChangeMessage::setFaceSkillMod(int const skillMod)
{
	m_faceSkillMod = skillMod;
}

//-----------------------------------------------------------------------

inline void ImageDesignChangeMessage::setMarkingsSkillMod(int const skillMod)
{
	m_markingsSkillMod = skillMod;
}

//-----------------------------------------------------------------------

inline void ImageDesignChangeMessage::setHairSkillMod(int const skillMod)
{
	m_hairSkillMod = skillMod;
}

//-----------------------------------------------------------------------

inline std::map<std::string, float> const & ImageDesignChangeMessage::getMorphParameterChanges() const
{
	return m_morphChanges;
}

//-----------------------------------------------------------------------

inline std::map<std::string, int> const & ImageDesignChangeMessage::getIndexParameterChanges() const
{
	return m_indexChanges;
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ImageDesignChangeMessage_H
