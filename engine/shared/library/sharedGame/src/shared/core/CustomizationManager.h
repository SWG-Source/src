// ======================================================================
//
// CustomizationManager.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CustomizationManager_H
#define INCLUDED_CustomizationManager_H

// ======================================================================

class CustomizationData;
class CustomizationVariable;
class Object;
class NetworkId;

//-----------------------------------------------------------------------

//for forward declaration purposed
class CustomizationManagerPaletteColumns
{
public:
	int palette;
	int creationColumns;
	int creationIndexes;
	int noviceColumns;
	int noviceIndexes;
	int level1Columns;
	int level1Indexes;
	int level2Columns;
	int level2Indexes;
	int level3Columns;
	int level3Indexes;
	int level4Columns;
	int level4Indexes;
	int masterColumns;
	int masterIndexes;
};

class CustomizationManager
{
public:
	struct MorphParameter;

	struct Customization
	{
		std::string name;
		std::string type;
		std::string variables;
		bool isScale;
		bool reverse;
		std::string colorLinked;
		std::string colorLinkedToSelf0;
		std::string colorLinkedToSelf1;
		float cameraYaw;
		bool discrete;
		bool randomizable;
		bool randomizableGroup;
		bool isVarHairColor;
		std::string skillMod;
		int skillModValue;
		std::string modificationType;
	};

	struct CustomizationGroupSharedData
	{
		std::string name;
		bool isMarking;
		bool isHair;
	};

	struct CustomizationGroupInstanceData
	{
		std::string species_gender;
		std::string groupName;
		bool isColorLinkedToHair;
		bool isColorLinkedToBody;
		std::string lookAtBone;
		float zoomFactor;
	};

	enum PathType
	{
		PT_none,
		PT_shared,
		PT_private
	};

	typedef CustomizationManagerPaletteColumns PaletteColumns;

	static std::string const cms_shared_owner;
	static std::string const cms_shared_owner_no_slash;
	static std::string const cms_priv;

	typedef std::vector<Customization> CustomizationVector;
	typedef std::map<std::string, CustomizationVector> CustomizationSpeciesMap;

	static void install();
	static void remove();

	static bool getAllowBald(std::string const & species_gender);
	static CustomizationSpeciesMap const & getCustomizationSpeciesMap(std::string const & species_gender);
	static CustomizationGroupSharedData const & getCustomizationGroupSharedData(std::string const & customizationGroupName);
	static CustomizationGroupInstanceData const & getCustomizationGroupInstanceData(std::string const & species_gender, std::string const & customizationGroupName);
	static bool getCustomization(std::string const & species_gender, std::string const & customizationName, CustomizationManager::Customization & /*OUT*/ result);
	static std::string getServerSpeciesGender(Object const & creature);
	static std::string getSharedSpeciesGender(Object const & creature);
	static CustomizationVariable * findVariable(CustomizationData & cdata, std::string const & partialName, PathType & type);
	static CustomizationVariable * findVariable(CustomizationData & cdata, std::string const & partialName);
	static std::string getCustomizationNameForVariables(std::string const & species_gender, std::string const & variables, bool isOnHair);
	static std::string convertSharedHairTemplateNameToServerHairTemplateName(std::string const &sharedHairTemplateName);
	static std::string convertServerHairTemplateNameToSharedHairTemplateName(std::string const &serverHairTemplateName);
	static PaletteColumns getPaletteColumnDataShortName(std::string const & paletteName);
	static PaletteColumns getPaletteColumnDataLongName(std::string const & paletteName);
	static std::map<std::string, PaletteColumns> const & getPaletteColumnData();
	static bool isHoloemoteName(std::string const & holoemoteName);
	static int const getRequiredHairSkillForHoloemote(std::string const & holoemoteName);

private:
	//disabled
	CustomizationManager (CustomizationManager const & rhs);
	CustomizationManager & operator= (CustomizationManager const & rhs);

private:
	static void clearData();
	static std::string shortenPaletteName(std::string const & longPaletteName);
};

// ======================================================================

#endif
