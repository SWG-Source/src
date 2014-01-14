// ======================================================================
//
// SharedImageDesignerManager.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SharedImageDesignerManager_H
#define INCLUDED_SharedImageDesignerManager_H


// ======================================================================

#include "sharedFoundation/NetworkId.h"

#include "sharedMath/PaletteArgb.h"
#include "sharedNetworkMessages/ImageDesignChangeMessage.h"

#include <map>

//-----------------------------------------------------------------------

class CustomizationData;


//-----------------------------------------------------------------------

//for forward declaration purposed
class SharedImageDesignerManagerSession
{
public:
	NetworkId designerId;
	NetworkId recipientId;
	NetworkId terminalId;
	time_t startingTime;
	ImageDesignChangeMessage::DesignType designType;
	bool newHairSet;
	std::string newHairAsset;
	std::string hairCustomizationData;
	int designerRequiredCredits;
	int recipientPaidCredits;
	bool accepted;
	std::map<std::string, float> morphChanges;
	std::map<std::string, int> indexChanges;
	std::string holoEmote;
	int bodySkillMod;
	int faceSkillMod;
	int markingsSkillMod;
	int hairSkillMod;

	SharedImageDesignerManagerSession();
	SharedImageDesignerManagerSession(ImageDesignChangeMessage const & message);
};

class SharedImageDesignerManager
{
public:
	static std::string const cms_bodySkillModName;
	static std::string const cms_faceSkillModName;
	static std::string const cms_markingsSkillModName;
	static std::string const cms_hairSkillModName;

	typedef SharedImageDesignerManagerSession Session;

	struct SkillMods
	{
		int faceSkillMod;
		int bodySkillMod;
		int markingsSkillMod;
		int hairSkillMod;

		int getSkillMod(std::string const & skillModName) const;
		SkillMods();
		SkillMods(int bodySkillMod, int faceSkillMod, int markingsSkillMod, int hairSkillMod);
	};

	enum ImageDesignerPaletteLevel
	{
		IDPL_NOT_A_DESIGNER,
		IDPL_NOVICE,
		IDPL_LEVEL1,
		IDPL_LEVEL2,
		IDPL_LEVEL3,
		IDPL_LEVEL4,
		IDPL_MASTER
	};

	struct PaletteValues
	{
		int columns;
		int maxIndex;
	};

public:
	static void install();
	static void startSession(Session const & session);
	static void updateSession(Session const & session);
	static void endSession(NetworkId const & designerId);
	static bool getSession(NetworkId const & designerId, Session & /*OUT*/ session);
	static bool isCurrentlyDesigning(NetworkId const & designerId);
	static bool isCurrentlyBeingDesigned(NetworkId const & recipientId);
	static time_t getTimeRemaining(NetworkId const & designerId);
	static bool isSessionValid(Session const & session, SkillMods const & designerSkills, std::string const & recipientSpeciesGender, CustomizationData * customizationDataCreature, CustomizationData * customizationDataHair);
	static bool isMorphChangeValid(std::string const & customizationName, float const newValue, SkillMods const & designerSkills, std::string const & recipientSpeciesGender);
	static bool isIndexChangeValid(std::string const & customizationName, int const newValue, SkillMods const & designerSkills, std::string const & recipientSpeciesGender, CustomizationData * customizationData);
	static bool isASkillModName(std::string const & skillModName);
	static void populateChangeMessage(Session const & session, ImageDesignChangeMessage & /*OUT*/ msg);
	static ImageDesignerPaletteLevel getAvailablePaletteLevels(SkillMods const & designerSkills);
	static PaletteValues getPaletteValuesForPaletteAndSkillMods(PaletteArgb const * palette, SkillMods const & designerSkills);
	static PaletteValues getPaletteValuesForPaletteAndLevel(PaletteArgb const * const palette, ImageDesignerPaletteLevel const & level);
	static bool hasSkillForHoloemote(std::string const & holoemoteName, SkillMods const & designerSkills);
	static void updateDesignType(Session & session, std::string const & recipientSpeciesGender);
	static bool hasSkillForCustomization(std::string const & customizationName, std::string const & recipientSpeciesGender, SkillMods const & designerSkills);

private:
	//disabled
	SharedImageDesignerManager (SharedImageDesignerManager const & rhs);
	SharedImageDesignerManager & operator= (SharedImageDesignerManager const & rhs);

private:
	static void remove();
};

// ======================================================================

#endif
