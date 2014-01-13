// ImageDesignChangeMessage.cpp
// Copyright 2004, Sony Online Entertainment Inc., all rights reserved. 

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ImageDesignChangeMessage.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedNetworkMessages/ControllerMessageFactory.h"

#include <string>

//-----------------------------------------------------------------------

CONTROLLER_MESSAGE_ALLOW_FROM_CLIENT_IMPLEMENTATION(ImageDesignChangeMessage, CM_imageDesignerChange);

//-----------------------------------------------------------------------

ImageDesignChangeMessage::ImageDesignChangeMessage() :
MessageQueue::Data(),
m_designerId(),
m_recipientId(),
m_terminalId(),
m_newHairSet(false),
m_newHairAsset(),
m_hairCustomizationData(),
m_designType(DT_COSMETIC),
m_startingTime(0),
m_designerRequiredCredits(0),
m_recipientPaidCredits(0),
m_accepted(false),
m_origin(O_UNKNOWN),
m_morphChanges(),
m_indexChanges(),
m_holoEmote(),
m_bodySkillMod(0),
m_faceSkillMod(0),
m_markingsSkillMod(0),
m_hairSkillMod(0)
{
}

//-----------------------------------------------------------------------

ImageDesignChangeMessage & ImageDesignChangeMessage::operator=(const ImageDesignChangeMessage & rhs)
{
	if (this == &rhs)
		return *this;

	m_designerId = rhs.m_designerId;
	m_recipientId = rhs.m_recipientId;
	m_terminalId = rhs.m_terminalId;
	m_newHairSet = rhs.m_newHairSet;
	m_newHairAsset = rhs.m_newHairAsset;
	m_hairCustomizationData = rhs.m_hairCustomizationData;
	m_designType = rhs.m_designType;
	m_startingTime = rhs.m_startingTime;
	m_designerRequiredCredits = rhs.m_designerRequiredCredits;
	m_recipientPaidCredits = rhs.m_recipientPaidCredits;
	m_accepted = rhs.m_accepted;
	m_origin = rhs.m_origin;
	m_morphChanges = rhs.m_morphChanges;
	m_indexChanges = rhs.m_indexChanges;
	m_holoEmote = rhs.m_holoEmote;
	m_bodySkillMod = rhs.m_bodySkillMod;
	m_faceSkillMod = rhs.m_faceSkillMod;
	m_markingsSkillMod = rhs.m_markingsSkillMod;
	m_hairSkillMod = rhs.m_hairSkillMod;

	return *this;
}

//-----------------------------------------------------------------------

void ImageDesignChangeMessage::addMorphParameterChange(std::string const & name, float const value)
{
	m_morphChanges[name] = value;
}

//-----------------------------------------------------------------------

void ImageDesignChangeMessage::addIndexParameterChange(std::string const & name, int const value)
{
	m_indexChanges[name] = value;
}

//-----------------------------------------------------------------------

void ImageDesignChangeMessage::setMorphParameterChanges(std::map<std::string, float> const & morphChanges)
{
	m_morphChanges = morphChanges;
}

//-----------------------------------------------------------------------

void ImageDesignChangeMessage::setIndexParameterChanges(std::map<std::string, int> const & indexChanges)
{
	m_indexChanges = indexChanges;
}

//-----------------------------------------------------------------------

void ImageDesignChangeMessage::pack(MessageQueue::Data const * const data, Archive::ByteStream & target)
{
	const ImageDesignChangeMessage* const msg = safe_cast<const ImageDesignChangeMessage*> (data);

	if (msg)
	{
		Archive::put(target, msg->getDesignerId());
		Archive::put(target, msg->getRecipientId());
		Archive::put(target, msg->getTerminalId());
		Archive::put(target, msg->getNewHairSet());
		Archive::put(target, msg->getNewHairAsset());
		Archive::put(target, msg->getHairCustomizationData());
		Archive::put(target, msg->getDesignType());
		Archive::put(target, msg->getStartingTime());
		Archive::put(target, msg->getDesignerRequiredCredits());
		Archive::put(target, msg->getRecipientPaidCredits());
		Archive::put(target, msg->getAccepted());
		Archive::put(target, msg->getOrigin());
		Archive::put(target, msg->getBodySkillMod());
		Archive::put(target, msg->getFaceSkillMod());
		Archive::put(target, msg->getMarkingsSkillMod());
		Archive::put(target, msg->getHairSkillMod());

		std::map<std::string, float> const & morphs = msg->getMorphParameterChanges();
		int size = morphs.size();
		Archive::put(target, size);
		for(std::map<std::string, float>::const_iterator i = morphs.begin(); i != morphs.end(); ++i)
		{
			Archive::put(target, i->first);
			Archive::put(target, i->second);
		}

		std::map<std::string, int> const & indexes = msg->getIndexParameterChanges();
		size = indexes.size();
		Archive::put(target, size);
		for(std::map<std::string, int>::const_iterator j = indexes.begin(); j != indexes.end(); ++j)
		{
			Archive::put(target, j->first);
			Archive::put(target, j->second);
		}
		Archive::put(target, msg->getHoloEmote());
	}
}

//-----------------------------------------------------------------------

MessageQueue::Data* ImageDesignChangeMessage::unpack(Archive::ReadIterator & source)
{
	ImageDesignChangeMessage * msg = new ImageDesignChangeMessage;

	NetworkId tempId;
	bool tempBool = false;
	std::string tempStr;
	int tempInt = 0;
	time_t tempTime = 0;

	Archive::get(source, tempId);
	msg->setDesignerId(tempId);
	Archive::get(source, tempId);
	msg->setRecipientId(tempId);
	Archive::get(source, tempId);
	msg->setTerminalId(tempId);
	Archive::get(source, tempBool);
	msg->setNewHairSet(tempBool);
	Archive::get(source, tempStr);
	msg->setNewHairAsset(tempStr);
	Archive::get(source, tempStr);
	msg->setHairCustomizationData(tempStr);
	Archive::get(source, tempInt);
	DesignType const designType = static_cast<DesignType>(tempInt);
	msg->setDesignType(designType);
	Archive::get(source, tempTime);
	msg->setStartingTime(tempTime);
	Archive::get(source, tempInt);
	msg->setDesignerRequiredCredits(tempInt);
	Archive::get(source, tempInt);
	msg->setRecipientPaidCredits(tempInt);
	Archive::get(source, tempBool);
	msg->setAccepted(tempBool);
	Archive::get(source, tempInt);
	Origin const origin = static_cast<Origin>(tempInt);
	msg->setOrigin(origin);
	Archive::get(source, tempInt);
	msg->setBodySkillMod(tempInt);
	Archive::get(source, tempInt);
	msg->setFaceSkillMod(tempInt);
	Archive::get(source, tempInt);
	msg->setMarkingsSkillMod(tempInt);
	Archive::get(source, tempInt);
	msg->setHairSkillMod(tempInt);

	int size = 0;
	Archive::get(source, size);
	std::string name;
	float value = 0;
	for(int i = 0; i < size; ++i)
	{
		Archive::get(source, name);
		Archive::get(source, value);
		msg->addMorphParameterChange(name, value);
	}

	Archive::get(source, size);
	int value2 = 0;
	for(int j = 0; j < size; ++j)
	{
		Archive::get(source, name);
		Archive::get(source, value2);
		msg->addIndexParameterChange(name, value2);
	}
	Archive::get(source, tempStr);
	msg->setHoloEmote(tempStr);
	return msg;
}


//======================================================================
