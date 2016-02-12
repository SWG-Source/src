// ======================================================================
//
// SharedImageDesignerManager.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/SharedImageDesignerManager.h"

#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Os.h"
#include "sharedGame/ConfigSharedGame.h"
#include "sharedGame/CustomizationManager.h"
#include "sharedObject/PaletteColorCustomizationVariable.h"


// ======================================================================

std::string const SharedImageDesignerManager::cms_bodySkillModName = "body";
std::string const SharedImageDesignerManager::cms_faceSkillModName = "face";
std::string const SharedImageDesignerManager::cms_markingsSkillModName = "markings";
std::string const SharedImageDesignerManager::cms_hairSkillModName = "hair";

// ======================================================================

namespace SharedImageDesignerManagerNamespace
{
	std::map<NetworkId, SharedImageDesignerManager::Session> ms_sessions;
	bool ms_installed = false;
	
	const char * const cms_physicalType = "physical";
}

using namespace SharedImageDesignerManagerNamespace;

// ======================================================================

SharedImageDesignerManager::SkillMods::SkillMods()
: faceSkillMod(0),
  bodySkillMod(0),
  markingsSkillMod(0),
  hairSkillMod(0)
{
}

//----------------------------------------------------------------------

SharedImageDesignerManager::SkillMods::SkillMods(int const bodySkillMod, int const faceSkillMod, int const markingsSkillMod, int const hairSkillMod)
: faceSkillMod(faceSkillMod),
  bodySkillMod(bodySkillMod),
  markingsSkillMod(markingsSkillMod),
  hairSkillMod(hairSkillMod)
{
}

//----------------------------------------------------------------------

int SharedImageDesignerManager::SkillMods::getSkillMod(std::string const & skillModName) const
{
	if(skillModName == cms_bodySkillModName)
		return bodySkillMod;
	else if(skillModName == cms_faceSkillModName)
		return faceSkillMod;
	else if(skillModName == cms_markingsSkillModName)
		return markingsSkillMod;
	else if(skillModName == cms_hairSkillModName)
		return hairSkillMod;
	else
	{
		DEBUG_REPORT_PRINT(ConfigSharedGame::getImageDesignerVerboseOutput(), ("Unknown skill mod [%s]", skillModName.c_str()));
		return -1;
	}
}

// ======================================================================

SharedImageDesignerManagerSession::SharedImageDesignerManagerSession()
: designerId(),
  recipientId(),
  terminalId(),
  startingTime(0),
  designType(ImageDesignChangeMessage::DT_COSMETIC),
  newHairSet(false),
  newHairAsset(),
  hairCustomizationData(),
  designerRequiredCredits(0),
  recipientPaidCredits(0),
  accepted(false),
  morphChanges(),
  indexChanges(),
  holoEmote(),
  bodySkillMod(0),
  faceSkillMod(0),
  markingsSkillMod(0),
  hairSkillMod(0)
{
}

//----------------------------------------------------------------------

SharedImageDesignerManagerSession::SharedImageDesignerManagerSession(ImageDesignChangeMessage const & message)
: designerId(message.getDesignerId()),
  recipientId(message.getRecipientId()),
  terminalId(message.getTerminalId()),
  startingTime(message.getStartingTime()),
  designType(message.getDesignType()),
  newHairSet(message.getNewHairSet()),
  newHairAsset(message.getNewHairAsset()),
  hairCustomizationData(message.getHairCustomizationData()),
  designerRequiredCredits(message.getDesignerRequiredCredits()),
  recipientPaidCredits(message.getRecipientPaidCredits()),
  accepted(message.getAccepted()),
  morphChanges(message.getMorphParameterChanges()),
  indexChanges(message.getIndexParameterChanges()),
  holoEmote(message.getHoloEmote()),
  bodySkillMod(message.getBodySkillMod()),
  faceSkillMod(message.getFaceSkillMod()),
  markingsSkillMod(message.getMarkingsSkillMod()),
  hairSkillMod(message.getHairSkillMod())
{
}

// ======================================================================

void SharedImageDesignerManager::install ()
{
	InstallTimer const installTimer("SharedImageDesignerManager::install ");

	DEBUG_FATAL(ms_installed, ("Already Installed"));
	ms_installed = true;

	ExitChain::add(SharedImageDesignerManager::remove, "SharedImageDesignerManager::remove", 0, false);
}

//----------------------------------------------------------------------

void SharedImageDesignerManager::remove ()
{
	DEBUG_FATAL(!ms_installed, ("Not Installed"));

	ms_installed = false;
}

//----------------------------------------------------------------------

void SharedImageDesignerManager::startSession(SharedImageDesignerManager::Session const & session)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("SharedImageDesignerManager not installed"));
		return; //lint !e527 unreachable (reachable in release)
	}

	NetworkId const & designerId = session.designerId;
	if(isCurrentlyDesigning(designerId))
	{
		endSession(designerId);
	}
	ms_sessions[designerId] = session;
}

//----------------------------------------------------------------------

void SharedImageDesignerManager::updateSession(SharedImageDesignerManager::Session const & session)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("SharedImageDesignerManager not installed"));
		return; //lint !e527 unreachable (reachable in release)
	}

	ms_sessions[session.designerId] = session;
}

//----------------------------------------------------------------------

bool SharedImageDesignerManager::getSession(NetworkId const & designerId, SharedImageDesignerManager::Session & /*OUT*/ session)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("SharedImageDesignerManager not installed"));
		return false; //lint !e527 unreachable (reachable in release)
	}

	std::map<NetworkId, SharedImageDesignerManager::Session>::const_iterator i = ms_sessions.find(designerId);
	if(i != ms_sessions.end())
	{
		session = i->second;
		return true;
	}
	else 
		return false;
}

//----------------------------------------------------------------------

bool SharedImageDesignerManager::isCurrentlyDesigning(NetworkId const & designerId)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("SharedImageDesignerManager not installed"));
		return false; //lint !e527 unreachable (reachable in release)
	}

	std::map<NetworkId, SharedImageDesignerManager::Session>::const_iterator i = ms_sessions.find(designerId);
	return i != ms_sessions.end();
}

//----------------------------------------------------------------------

bool SharedImageDesignerManager::isCurrentlyBeingDesigned(NetworkId const & recipientId)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("SharedImageDesignerManager not installed"));
		return false; //lint !e527 unreachable (reachable in release)
	}

	for(std::map<NetworkId, SharedImageDesignerManager::Session>::const_iterator i = ms_sessions.begin(); i != ms_sessions.end(); ++i)
	{
		if(i->second.recipientId == recipientId)
			return true;
	}
	return false;
}

//----------------------------------------------------------------------

void SharedImageDesignerManager::endSession(NetworkId const & designerId)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("SharedImageDesignerManager not installed"));
		return; //lint !e527 unreachable (reachable in release)
	}

	if(!isCurrentlyDesigning(designerId))
		return;

	IGNORE_RETURN(ms_sessions.erase(designerId));
}

//----------------------------------------------------------------------

time_t SharedImageDesignerManager::getTimeRemaining(NetworkId const & designerId)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("SharedImageDesignerManager not installed"));
		return 0; //lint !e527 unreachable (reachable in release)
	}

	if(!isCurrentlyDesigning(designerId))
		return 0;

	Session session;
	bool const result = getSession(designerId, session);
	if(!result)
		return 0;

	if(session.startingTime == 0)
		return 0;

	SkillMods const skillMods(session.bodySkillMod, session.faceSkillMod, session.markingsSkillMod, session.hairSkillMod);
	ImageDesignerPaletteLevel const designerLevel = getAvailablePaletteLevels(skillMods);

	time_t const elapsedTime = Os::getRealSystemTime() - session.startingTime;

	time_t endTime = 0;
	if(session.designType == ImageDesignChangeMessage::DT_COSMETIC)
	{
		int minimumCosmeticTime = ConfigSharedGame::getImageDesignerCosmeticSessionTimeSeconds();
		//cut time in half if we have a terminal
		if(session.terminalId != NetworkId::cms_invalid || designerLevel == IDPL_MASTER)
		{
			minimumCosmeticTime /= 2;
		}
		if(session.terminalId != NetworkId::cms_invalid && designerLevel == IDPL_MASTER)
		{
			minimumCosmeticTime = 0;
		}

		endTime = session.startingTime + minimumCosmeticTime;
	}
	else if(session.designType == ImageDesignChangeMessage::DT_PHYSICAL)
	{
		int minimumPhysicalTime = ConfigSharedGame::getImageDesignerPhysicalSessionTimeSeconds();
		//cut time in half if we have a terminal
		if(session.terminalId != NetworkId::cms_invalid || designerLevel == IDPL_MASTER)
		{
			minimumPhysicalTime /= 2;
		}
		if(session.terminalId != NetworkId::cms_invalid && designerLevel == IDPL_MASTER)
		{
			minimumPhysicalTime = 0;
		}

		endTime = session.startingTime + minimumPhysicalTime;
	}
	else
	{
		//this was for stat migration, and shouldn't be called anymore
		endTime = session.startingTime;
	}

	time_t timeLeft = (endTime - session.startingTime) - elapsedTime;
	if(timeLeft < 0)
		timeLeft = 0;
	return timeLeft;
}

//----------------------------------------------------------------------

bool SharedImageDesignerManager::isSessionValid(SharedImageDesignerManager::Session const & session, SharedImageDesignerManager::SkillMods const & designerSkills, std::string const & recipientSpeciesGender, CustomizationData * const customizationDataCreature, CustomizationData * const customizationDataHair)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("SharedImageDesignerManager not installed"));
		return false; //lint !e527 unreachable (reachable in release)
	}

	if(!customizationDataCreature)
		return false;

	//check time left
	time_t const timeLeft = getTimeRemaining(session.designerId);
	if(timeLeft > 0)
	{
		DEBUG_REPORT_PRINT(ConfigSharedGame::getImageDesignerVerboseOutput(), ("ImageDesigner: Designer[%s] can't change Recipient[%s], not enough time has elapsed", session.designerId.getValueString().c_str(), session.recipientId.getValueString().c_str()));
		return false;
	}

	CustomizationManager::Customization customization;

	//check morph changes
	for(std::map<std::string, float>::const_iterator i = session.morphChanges.begin(); i != session.morphChanges.end(); ++i)
	{
		if(!isMorphChangeValid(i->first, i->second, designerSkills, recipientSpeciesGender))
		{
			DEBUG_REPORT_PRINT(ConfigSharedGame::getImageDesignerVerboseOutput(), ("ImageDesigner: Designer[%s] can't change Recipient[%s]'s [%s] to [%f] as requested", session.designerId.getValueString().c_str(), session.recipientId.getValueString().c_str(), i->first.c_str(), i->second));
			return false;
		}
	}

	//check index changes
	for(std::map<std::string, int>::const_iterator j = session.indexChanges.begin(); j != session.indexChanges.end(); ++j)
	{
		bool const result = CustomizationManager::getCustomization(recipientSpeciesGender, j->first, customization);
		if(!result)
			return false;

		//check against the hair customization data if necessary
		CustomizationData * customizationDataForThisCustomization = customizationDataCreature;
		if(customization.isVarHairColor)
		{
			if(customizationDataHair)
				customizationDataForThisCustomization = customizationDataHair;
			else
				customizationDataForThisCustomization = nullptr;
		}

		if(customizationDataForThisCustomization)
		{
			if(!isIndexChangeValid(j->first, j->second, designerSkills, recipientSpeciesGender, customizationDataForThisCustomization) && !customization.isVarHairColor)
			{
				DEBUG_REPORT_PRINT(ConfigSharedGame::getImageDesignerVerboseOutput(), ("ImageDesigner: Designer[%s] can't change Recipient[%s]'s [%s] to [%d] as requested", session.designerId.getValueString().c_str(), session.recipientId.getValueString().c_str(), j->first.c_str(), j->second));
				return false;
			}
		}
	}

	//check hair change
	if(session.newHairSet)
	{
		//if bald, see if that's alright for this avatar
		if(session.newHairAsset.empty())
		{
			//bald doens't have a skill level, if bald is ok for the recipient, then the designer can do it
			bool const baldOk = CustomizationManager::getAllowBald(recipientSpeciesGender);
			if(!baldOk)
			{
				DEBUG_REPORT_PRINT(ConfigSharedGame::getImageDesignerVerboseOutput(), ("ImageDesigner: Designer[%s] can't change Recipient[%s]'s hair to bald, bald is not okay for that species/gender", session.designerId.getValueString().c_str(), session.recipientId.getValueString().c_str()));
				return false;
			}
			//TODO check that the recipient hair 1)can go on that species_gender, and 2)the designer has the skill to make the change
		}
	}

	//check holoemote
	if(!session.holoEmote.empty())
	{
		bool const result = hasSkillForHoloemote(session.holoEmote, designerSkills);
		if(!result)
		{
			DEBUG_REPORT_PRINT(ConfigSharedGame::getImageDesignerVerboseOutput(), ("ImageDesigner: Designer[%s] can't give Recipient[%s] the [%s] holoemote, they don't have the skill", session.designerId.getValueString().c_str(), session.recipientId.getValueString().c_str(), session.holoEmote.c_str()));
			return false;
		}
	}

	return true;
}

//----------------------------------------------------------------------

bool SharedImageDesignerManager::isMorphChangeValid(std::string const & customizationName, float const newValue, SharedImageDesignerManager::SkillMods const & designerSkills, std::string const & recipientSpeciesGender)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("SharedImageDesignerManager not installed"));
		return false; //lint !e527 unreachable (reachable in release)
	}

	UNREF(newValue);
	std::string requiredSkillModName;
	int requiredSkillModValue = 0;
	int designerSkillModValue = 0;
	CustomizationManager::Customization customization;
	//check that the morph change 1)is a valid customization variable for the recipient, 2)the designer has the skill
	bool const result = CustomizationManager::getCustomization(recipientSpeciesGender, customizationName, customization);
	if(!result)
	{
		//unknown customization name, fail
		DEBUG_REPORT_PRINT(ConfigSharedGame::getImageDesignerVerboseOutput(), ("ImageDesigner: Unknown customization [%s] for [%s]", customizationName.c_str(), recipientSpeciesGender.c_str()));
		return false;
	}
	requiredSkillModName = customization.skillMod;
	if(!isASkillModName(requiredSkillModName))
	{
		//unknown skill modname, fail
		DEBUG_REPORT_PRINT(ConfigSharedGame::getImageDesignerVerboseOutput(), ("ImageDesigner: Unknown skill mod name [%s]", requiredSkillModName.c_str()));
		return false;
	}

	requiredSkillModValue = customization.skillModValue;
	designerSkillModValue = designerSkills.getSkillMod(requiredSkillModName);
	if(designerSkillModValue < requiredSkillModValue)
	{
		//designer doesn't have enough skill, fail
		return false;
	}
	return true;
}

//----------------------------------------------------------------------

bool SharedImageDesignerManager::isIndexChangeValid(std::string const & customizationName, int const newValue, SharedImageDesignerManager::SkillMods const & designerSkills, std::string const & recipientSpeciesGender, CustomizationData * const customizationData)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("SharedImageDesignerManager not installed"));
		return false; //lint !e527 unreachable (reachable in release)
	}

	if(newValue < 0)
		return false;

	if(!customizationData)
		return false;

	std::string requiredSkillModName;
	int requiredSkillModValue = 0;
	int designerSkillModValue = 0;
	CustomizationManager::Customization customization;
	//check that the index change 1)is a valid customization variable for the recipient, 2)the designer has the skill,
		//and 3)is within the right area of the palette for the designer's skill
	bool const result = CustomizationManager::getCustomization(recipientSpeciesGender, customizationName, customization);
	if(!result)
	{
		//unknown customization name, fail
		DEBUG_REPORT_PRINT(ConfigSharedGame::getImageDesignerVerboseOutput(), ("ImageDesigner: Unknown customization [%s] for [%s]", customizationName.c_str(), recipientSpeciesGender.c_str()));
		return false;
	}
	requiredSkillModName = customization.skillMod;
	if(!isASkillModName(requiredSkillModName))
	{
		//unknown skill modname, fail
		DEBUG_REPORT_PRINT(ConfigSharedGame::getImageDesignerVerboseOutput(), ("ImageDesigner: Unknown skill mod name [%s]", requiredSkillModName.c_str()));
		return false;
	}

	requiredSkillModValue = customization.skillModValue;
	designerSkillModValue = designerSkills.getSkillMod(requiredSkillModName);
	if(designerSkillModValue < requiredSkillModValue)
	{
		//designer doesn't have enough skill, fail
		return false;
	}

	CustomizationVariable * const cv = CustomizationManager::findVariable (*customizationData, customization.variables);
	PaletteColorCustomizationVariable * const paletteVariable = dynamic_cast<PaletteColorCustomizationVariable *>(cv);
	if(!paletteVariable)
		return false;
	PaletteArgb const * const palette = paletteVariable->fetchPalette();
	if(!palette)
		return false;
	else
	{
		PaletteValues const values = getPaletteValuesForPaletteAndSkillMods(palette, designerSkills);
		palette->release();
		//if the requested palette value is outside the range the designer can use, fail
		if(values.maxIndex < newValue)
			return false;
	}
	return true;
}

//----------------------------------------------------------------------

bool SharedImageDesignerManager::isASkillModName(std::string const & skillModName)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("SharedImageDesignerManager not installed"));
		return false; //lint !e527 unreachable (reachable in release)
	}

	return skillModName == cms_bodySkillModName ||
	       skillModName == cms_faceSkillModName ||
	       skillModName == cms_markingsSkillModName ||
	       skillModName == cms_hairSkillModName;
}

//----------------------------------------------------------------------

void SharedImageDesignerManager::populateChangeMessage(SharedImageDesignerManager::Session const & session, ImageDesignChangeMessage & /*OUT*/ msg)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("SharedImageDesignerManager not installed"));
		return; //lint !e527 unreachable (reachable in release)
	}

	msg.setDesignerId(session.designerId);
	msg.setRecipientId(session.recipientId);
	msg.setTerminalId(session.terminalId);
	msg.setStartingTime(session.startingTime);
	msg.setDesignType(session.designType);
	msg.setNewHairSet(session.newHairSet);
	msg.setOrigin(ImageDesignChangeMessage::O_DESIGNER);
	msg.setNewHairAsset(session.newHairAsset);
	msg.setHairCustomizationData(session.hairCustomizationData);
	msg.setDesignerRequiredCredits(session.designerRequiredCredits);
	msg.setRecipientPaidCredits(session.recipientPaidCredits);
	msg.setAccepted(session.accepted);
	msg.setMorphParameterChanges(session.morphChanges);
	msg.setIndexParameterChanges(session.indexChanges);
	msg.setHoloEmote(session.holoEmote);
	msg.setBodySkillMod(session.bodySkillMod);
	msg.setFaceSkillMod(session.faceSkillMod);
	msg.setMarkingsSkillMod(session.markingsSkillMod);
	msg.setHairSkillMod(session.hairSkillMod);
}

//----------------------------------------------------------------------

SharedImageDesignerManager::ImageDesignerPaletteLevel SharedImageDesignerManager::getAvailablePaletteLevels(SharedImageDesignerManager::SkillMods const & designerSkills)
{
	ImageDesignerPaletteLevel maxLevel = IDPL_NOT_A_DESIGNER;
	if(designerSkills.faceSkillMod >= 4 || designerSkills.bodySkillMod >= 1 || designerSkills.hairSkillMod >= 5 || designerSkills.markingsSkillMod >= 2)
		maxLevel = IDPL_NOVICE;
	if(designerSkills.faceSkillMod >= 5 || designerSkills.bodySkillMod >= 2 || designerSkills.hairSkillMod >= 6 || designerSkills.markingsSkillMod >= 2)
		maxLevel = IDPL_LEVEL1;
	if(designerSkills.faceSkillMod >= 6 || designerSkills.bodySkillMod >= 3 || designerSkills.hairSkillMod >= 8 || designerSkills.markingsSkillMod >= 3)
		maxLevel = IDPL_LEVEL2;
	if(designerSkills.faceSkillMod >= 8 || designerSkills.bodySkillMod >= 5 || designerSkills.hairSkillMod >= 10 || designerSkills.markingsSkillMod >= 5)
		maxLevel = IDPL_LEVEL3;
	if(designerSkills.faceSkillMod >= 10 || designerSkills.bodySkillMod >= 7 || designerSkills.hairSkillMod >= 12 || designerSkills.markingsSkillMod >= 7)
		maxLevel = IDPL_LEVEL4;
	if(designerSkills.faceSkillMod >= 12 || designerSkills.bodySkillMod >= 9 || designerSkills.hairSkillMod >= 13 || designerSkills.markingsSkillMod >= 9)
		maxLevel = IDPL_MASTER;
	return maxLevel;
}

//----------------------------------------------------------------------

SharedImageDesignerManager::PaletteValues SharedImageDesignerManager::getPaletteValuesForPaletteAndSkillMods(PaletteArgb const * const palette, SharedImageDesignerManager::SkillMods const & designerSkills)
{
	SharedImageDesignerManager::ImageDesignerPaletteLevel const level = SharedImageDesignerManager::getAvailablePaletteLevels(designerSkills);
	return getPaletteValuesForPaletteAndLevel(palette, level);
}

//----------------------------------------------------------------------

SharedImageDesignerManager::PaletteValues SharedImageDesignerManager::getPaletteValuesForPaletteAndLevel(PaletteArgb const * const palette, SharedImageDesignerManager::ImageDesignerPaletteLevel const & level)
{
	PaletteValues paletteValues;
	paletteValues.columns = 0;
	paletteValues.maxIndex = 10000;

	if(palette)
	{
		CrcString const & paletteNameCrc = palette->getName();
		char const * const paletteName = paletteNameCrc.getString();
		CustomizationManager::PaletteColumns const paletteColumnData = CustomizationManager::getPaletteColumnDataLongName(paletteName);
		if(level == SharedImageDesignerManager::IDPL_NOT_A_DESIGNER)
		{
			paletteValues.columns = paletteColumnData.creationColumns;
			paletteValues.maxIndex = paletteColumnData.creationIndexes;
		}
		else if(level == SharedImageDesignerManager::IDPL_NOVICE)
		{
			paletteValues.columns = paletteColumnData.noviceColumns;
			paletteValues.maxIndex = paletteColumnData.noviceIndexes;
		}
		else if(level == SharedImageDesignerManager::IDPL_LEVEL1)
		{
			paletteValues.columns = paletteColumnData.level1Columns;
			paletteValues.maxIndex = paletteColumnData.level1Indexes;
		}
		else if(level == SharedImageDesignerManager::IDPL_LEVEL2)
		{
			paletteValues.columns = paletteColumnData.level2Columns;
			paletteValues.maxIndex = paletteColumnData.level2Indexes;
		}
		else if(level == SharedImageDesignerManager::IDPL_LEVEL3)
		{
			paletteValues.columns = paletteColumnData.level3Columns;
			paletteValues.maxIndex = paletteColumnData.level3Indexes;
		}
		else if(level == SharedImageDesignerManager::IDPL_LEVEL4)
		{
			paletteValues.columns = paletteColumnData.level4Columns;
			paletteValues.maxIndex = paletteColumnData.level4Indexes;
		}
		else if(level == SharedImageDesignerManager::IDPL_MASTER)
		{
			paletteValues.columns = paletteColumnData.masterColumns;
			paletteValues.maxIndex = paletteColumnData.masterIndexes;
		}
		else
		{
			DEBUG_FATAL(true, ("Unknown SharedImageDesignerManager::ImageDesignerPaletteLevel type %d found", level));
		}
	}
	return paletteValues;
}

//----------------------------------------------------------------------

bool SharedImageDesignerManager::hasSkillForHoloemote(std::string const & holoemoteName, SharedImageDesignerManager::SkillMods const & designerSkills)
{
	if(!CustomizationManager::isHoloemoteName(holoemoteName))
		return false;
	int const requiredHairSkill = CustomizationManager::getRequiredHairSkillForHoloemote(holoemoteName);
	if(designerSkills.hairSkillMod >= requiredHairSkill)
		return true;
	else
		return false;
}

//----------------------------------------------------------------------

/** Look through each customization change.
    If any of the changes are "physical", the session's overall change type is "physical".
		If stat migration is set, then the type is "stta migration"
*/
void SharedImageDesignerManager::updateDesignType(SharedImageDesignerManager::Session & /*IN OUT*/ session, std::string const & recipientSpeciesGender)
{
	CustomizationManager::Customization customization;
	bool result = false;

	session.designType = ImageDesignChangeMessage::DT_COSMETIC;

	for(std::map<std::string, float>::iterator i = session.morphChanges.begin(); i != session.morphChanges.end(); ++i)
	{
		result = CustomizationManager::getCustomization(recipientSpeciesGender, i->first, customization);
		if(result)
		{
			if(customization.modificationType == cms_physicalType)
			{
				session.designType = ImageDesignChangeMessage::DT_PHYSICAL;
				return;
			}
		}
	}

	for(std::map<std::string, int>::iterator j = session.indexChanges.begin(); j != session.indexChanges.end(); ++j)
	{
		result = CustomizationManager::getCustomization(recipientSpeciesGender, j->first, customization);
		if(result)
		{
			if(customization.modificationType == cms_physicalType)
			{
				session.designType = ImageDesignChangeMessage::DT_PHYSICAL;
				return;
			}
		}
	}
}

//----------------------------------------------------------------------

bool SharedImageDesignerManager::hasSkillForCustomization(std::string const & customizationName, std::string const & recipientSpeciesGender, SkillMods const & designerSkills)
{
	CustomizationManager::Customization customization;
	bool const result = CustomizationManager::getCustomization(recipientSpeciesGender, customizationName, customization);
	if(result)
	{
		int const requiredSkillModValue = customization.skillModValue;
		int const designerSkillModValue = designerSkills.getSkillMod(customization.skillMod);
		if(designerSkillModValue >= requiredSkillModValue)
		{
			return true;
		}
	}
	return false;
}

// =====================================================================
