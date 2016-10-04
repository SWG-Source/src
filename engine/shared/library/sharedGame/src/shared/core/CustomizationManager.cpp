// ======================================================================
//
// CustomizationManager.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/CustomizationManager.h"

#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedMessageDispatch/Receiver.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/Object.h"
#include "sharedObject/RangedIntCustomizationVariable.h"
#include "sharedUtility/DataTable.h"

#include <algorithm>
#include <map>
#include <vector>

// ======================================================================

//allow long container names
#if WIN32
#pragma warning (disable:4503)
#endif


// ======================================================================

std::string const CustomizationManager::cms_shared_owner          = "/shared_owner/";
std::string const CustomizationManager::cms_shared_owner_no_slash = "/shared_owner";
std::string const CustomizationManager::cms_priv                  = "/private/";

// ======================================================================

namespace CustomizationManagerNamespace
{
	namespace AllowBaldColumnNames
	{
		char const * const species_gender = "SPECIES_GENDER";
		char const * const allowBald = "ALLOW_BALD";
	}

	namespace CustomizationGroupSharedColumnNames
	{
		char const * const customizationGroup = "CUSTOMIZATION_GROUP";
		char const * const isMarking = "IS_MARKING";
		char const * const isHair = "IS_HAIR";
	}

	namespace CustomizationGroupInstanceColumnNames
	{
		char const * const species_gender = "SPECIES_GENDER";
		char const * const customizationGroup = "CUSTOMIZATION_GROUP";
		char const * const isColorLinkedToHair = "IS_COLOR_LINKED_TO_HAIR";
		char const * const isColorLinkedToBody = "IS_COLOR_LINKED_TO_BODY";
		char const * const lookAtBone = "LOOK_AT_BONE";
		char const * const zoomFactor = "ZOOM_FACTOR";
	}

	namespace CustomizationDataColumnNames
	{
		char const * const species_gender = "SPECIES_GENDER";
		char const * const customizationGroup = "CUSTOMIZATION_GROUP";
		char const * const type = "TYPE";
		char const * const customizationName = "CUSTOMIZATION_NAME";
		char const * const variables = "VARIABLES";
		char const * const isScale = "IS_SCALE";
		char const * const reverse = "REVERSE";
		char const * const colorLinked = "COLOR_LINKED";
		char const * const colorLinkedToSelf0 = "COLOR_LINKED_TO_SELF_0";
		char const * const colorLinkedToSelf1 = "COLOR_LINKED_TO_SELF_1";
		char const * const cameraYaw = "CAMERA_YAW";
		char const * const discrete = "DISCRETE";
		char const * const randomizable = "RAMDOMIZABLE";
		char const * const randomizableGroup = "RANDOMIZABLE_GROUP";
		char const * const isVarHairColor = "IS_VAR_HAIR_COLOR";
		char const * const imagedesignSkillMod = "IMAGEDESIGN_SKILLMOD";
		char const * const skillModValue = "SKILLMOD_VALUE";
		char const * const modificationType = "MODIFICATION_TYPE";
	}

	namespace PaletteDataColumnNames
	{
		char const * const palette = "PALETTE";
		char const * const creationColumns = "CREATION_COLUMNS";
		char const * const creationIndexes = "CREATION_INDEXES";
		char const * const noviceColumns = "ID_NOVICE_COLUMNS";
		char const * const noviceIndexes = "ID_NOVICE_INDEXES";
		char const * const level1Columns = "ID_LEVEL1_COLUMNS";
		char const * const level1Indexes = "ID_LEVEL1_INDEXES";
		char const * const level2Columns = "ID_LEVEL2_COLUMNS";
		char const * const level2Indexes = "ID_LEVEL2_INDEXES";
		char const * const level3Columns = "ID_LEVEL3_COLUMNS";
		char const * const level3Indexes = "ID_LEVEL3_INDEXES";
		char const * const level4Columns = "ID_LEVEL4_COLUMNS";
		char const * const level4Indexes = "ID_LEVEL4_INDEXES";
		char const * const masterColumns = "ID_MASTER_COLUMNS";
		char const * const masterIndexes = "ID_MASTER_INDEXES";
	}

	namespace HoloemoteColumnNames
	{
		char const * const holoemoteName = "HOLOEMOTE";
		char const * const requiredHairSkillMod = "REQUIRED_HAIR_SKILLMOD";
	}

	std::map<std::string, bool>  ms_allowBald;
	///all the customization data, indexed by species_gender, then by groupName
	std::map<std::string, CustomizationManager::CustomizationSpeciesMap> ms_customizationsBySpeciesGenderAndGroup;
	///all the customization data, indexed by species_gender only
	std::map<std::string, CustomizationManager::CustomizationVector> ms_customizationsBySpeciesGender;
	std::map<std::string, CustomizationManager::CustomizationGroupSharedData> ms_customizationGroupsShared;
	std::map<std::pair<std::string, std::string>, CustomizationManager::CustomizationGroupInstanceData> ms_customizationGroupsInstance;
	std::map<std::string, CustomizationManager::PaletteColumns> ms_paletteColumnData;
	std::map<std::string, int> ms_holoemoteHairSkillRequired;

	bool ms_installed = false;
}

using namespace CustomizationManagerNamespace;

// ======================================================================

void CustomizationManager::install ()
{
	InstallTimer const installTimer("CustomizationManager::install ");

	DEBUG_FATAL(ms_installed, ("Already Installed"));
	ms_installed = true;
	clearData();

	//load in allow bald table
	DataTable allowBaldDataTable;
	Iff allowBaldIff("datatables/customization/allow_bald.iff");
	allowBaldDataTable.load(allowBaldIff);
	std::string species_gender;
	int allowBaldInt = 0;
	for (int i = 0; i < allowBaldDataTable.getNumRows(); ++i)
	{
		species_gender = allowBaldDataTable.getStringValue(AllowBaldColumnNames::species_gender, i);
		allowBaldInt = allowBaldDataTable.getIntValue(AllowBaldColumnNames::allowBald, i);
		ms_allowBald[species_gender] = (allowBaldInt == 1) ? true : false;
	}

	//load customization_group_shared table
	DataTable customizationGroupSharedDataTable;
	Iff customizationGroupSharedIff("datatables/customization/customization_group_shared.iff");
	customizationGroupSharedDataTable.load(customizationGroupSharedIff);
	int isMarkingInt = 0;
	int isHairInt = 0;
	for (int j = 0; j < customizationGroupSharedDataTable.getNumRows(); ++j)
	{
		CustomizationGroupSharedData group;
		group.name = customizationGroupSharedDataTable.getStringValue(CustomizationGroupSharedColumnNames::customizationGroup, j);
		isMarkingInt = customizationGroupSharedDataTable.getIntValue(CustomizationGroupSharedColumnNames::isMarking, j);
		isHairInt = customizationGroupSharedDataTable.getIntValue(CustomizationGroupSharedColumnNames::isHair, j);
		group.isMarking = (isMarkingInt == 1) ? true : false;
		group.isHair = (isHairInt == 1) ? true : false;
		ms_customizationGroupsShared[group.name] = group;
	}

	//load customization_group_instance table
	DataTable customizationGroupInstanceDataTable;
	Iff customizationGroupInstanceIff("datatables/customization/customization_group_instance.iff");
	customizationGroupInstanceDataTable.load(customizationGroupInstanceIff);
	int isColorLinkedToHairInt = 0;
	int isColorLinkedToBodyInt = 0;
	for (int k = 0; k < customizationGroupInstanceDataTable.getNumRows(); ++k)
	{
		CustomizationGroupInstanceData data;
		data.species_gender = customizationGroupInstanceDataTable.getStringValue(CustomizationGroupInstanceColumnNames::species_gender, k);
		data.groupName = customizationGroupInstanceDataTable.getStringValue(CustomizationGroupInstanceColumnNames::customizationGroup, k);
		isColorLinkedToHairInt = customizationGroupInstanceDataTable.getIntValue(CustomizationGroupInstanceColumnNames::isColorLinkedToHair, k);
		data.isColorLinkedToHair = (isColorLinkedToHairInt == 1) ? true : false;
		isColorLinkedToBodyInt = customizationGroupInstanceDataTable.getIntValue(CustomizationGroupInstanceColumnNames::isColorLinkedToBody, k);
		data.isColorLinkedToBody = (isColorLinkedToBodyInt == 1) ? true : false;
		data.lookAtBone = customizationGroupInstanceDataTable.getStringValue(CustomizationGroupInstanceColumnNames::lookAtBone, k);
		data.zoomFactor = customizationGroupInstanceDataTable.getFloatValue(CustomizationGroupInstanceColumnNames::zoomFactor, k);
		ms_customizationGroupsInstance[std::make_pair(data.species_gender, data.groupName)] = data;
	}

	//load customization_data table
	DataTable customizationDataTable;
	Iff customizationIff("datatables/customization/customization_data.iff");
	customizationDataTable.load(customizationIff);
	std::string customizationGroup;
	int isScaleInt = 0;
	int reverseInt = 0;
	int discreteInt = 0;
	int randomizableInt = 0;
	int randomizableGroupInt = 0;
	int isVarHairColorInt = 0;
	for (int l = 0; l < customizationDataTable.getNumRows(); ++l)
	{
		species_gender = customizationDataTable.getStringValue(CustomizationDataColumnNames::species_gender, l);
		customizationGroup = customizationDataTable.getStringValue(CustomizationDataColumnNames::customizationGroup, l);

		Customization cust;
		cust.name = customizationDataTable.getStringValue(CustomizationDataColumnNames::customizationName, l);
		cust.type = customizationDataTable.getStringValue(CustomizationDataColumnNames::type, l);
		cust.variables = customizationDataTable.getStringValue(CustomizationDataColumnNames::variables, l);
		isScaleInt = customizationDataTable.getIntValue(CustomizationDataColumnNames::isScale, l);
		cust.isScale = (isScaleInt == 1) ? true : false;
		reverseInt = customizationDataTable.getIntValue(CustomizationDataColumnNames::reverse, l);
		cust.reverse = (reverseInt == 1) ? true : false;
		cust.colorLinked = customizationDataTable.getStringValue(CustomizationDataColumnNames::colorLinked, l);
		cust.colorLinkedToSelf0= customizationDataTable.getStringValue(CustomizationDataColumnNames::colorLinkedToSelf0, l);
		cust.colorLinkedToSelf1= customizationDataTable.getStringValue(CustomizationDataColumnNames::colorLinkedToSelf1, l);
		cust.cameraYaw = customizationDataTable.getFloatValue(CustomizationDataColumnNames::cameraYaw, l);
		discreteInt = customizationDataTable.getIntValue(CustomizationDataColumnNames::discrete, l);
		cust.discrete = (discreteInt == 1) ? true : false;
		randomizableInt = customizationDataTable.getIntValue(CustomizationDataColumnNames::randomizable, l);
		cust.randomizable = (randomizableInt == 1) ? true : false;
		randomizableGroupInt = customizationDataTable.getIntValue(CustomizationDataColumnNames::randomizableGroup, l);
		cust.randomizableGroup = (randomizableGroupInt == 1) ? true : false;
		isVarHairColorInt = customizationDataTable.getIntValue(CustomizationDataColumnNames::isVarHairColor, l);
		cust.isVarHairColor = (isVarHairColorInt == 1) ? true : false;
		cust.skillMod = customizationDataTable.getStringValue(CustomizationDataColumnNames::imagedesignSkillMod, l);
		cust.skillModValue = customizationDataTable.getIntValue(CustomizationDataColumnNames::skillModValue, l);
		cust.modificationType = customizationDataTable.getStringValue(CustomizationDataColumnNames::modificationType, l);

		//we specifically want a CustomizationSpeciesMap created if we don't already have one 
		CustomizationSpeciesMap customizationMap = ms_customizationsBySpeciesGenderAndGroup[species_gender];
		//we specifically want a CustomizationVector created if we don't already have one 
		CustomizationVector customizationVector = customizationMap[customizationGroup];
		customizationVector.push_back(cust);
		customizationMap[customizationGroup] = customizationVector;
		ms_customizationsBySpeciesGenderAndGroup[species_gender] = customizationMap;

		customizationVector = ms_customizationsBySpeciesGender[species_gender];
		customizationVector.push_back(cust);
		ms_customizationsBySpeciesGender[species_gender] = customizationVector;
	}

	//load palette data
	DataTable paletteDataTable;
	Iff paletteDataIff("datatables/customization/palette_columns.iff");
	paletteDataTable.load(paletteDataIff);
	for (int m = 0; m < paletteDataTable.getNumRows(); ++m)
	{
		PaletteColumns paletteData;
		std::string const & palette = paletteDataTable.getStringValue(PaletteDataColumnNames::palette, m);
		paletteData.creationColumns = paletteDataTable.getIntValue(PaletteDataColumnNames::creationColumns, m);
		paletteData.creationIndexes = paletteDataTable.getIntValue(PaletteDataColumnNames::creationIndexes, m);
		paletteData.noviceColumns = paletteDataTable.getIntValue(PaletteDataColumnNames::noviceColumns, m);
		paletteData.noviceIndexes = paletteDataTable.getIntValue(PaletteDataColumnNames::noviceIndexes, m);
		paletteData.level1Columns = paletteDataTable.getIntValue(PaletteDataColumnNames::level1Columns, m);
		paletteData.level1Indexes = paletteDataTable.getIntValue(PaletteDataColumnNames::level1Indexes, m);
		paletteData.level2Columns = paletteDataTable.getIntValue(PaletteDataColumnNames::level2Columns, m);
		paletteData.level2Indexes = paletteDataTable.getIntValue(PaletteDataColumnNames::level2Indexes, m);
		paletteData.level3Columns = paletteDataTable.getIntValue(PaletteDataColumnNames::level3Columns, m);
		paletteData.level3Indexes = paletteDataTable.getIntValue(PaletteDataColumnNames::level3Indexes, m);
		paletteData.level4Columns = paletteDataTable.getIntValue(PaletteDataColumnNames::level4Columns, m);
		paletteData.level4Indexes = paletteDataTable.getIntValue(PaletteDataColumnNames::level4Indexes, m);
		paletteData.masterColumns = paletteDataTable.getIntValue(PaletteDataColumnNames::masterColumns, m);
		paletteData.masterIndexes = paletteDataTable.getIntValue(PaletteDataColumnNames::masterIndexes, m);
		ms_paletteColumnData[palette] = paletteData;
	}

	//load holoemotes table
	DataTable holoemoteDataTable;
	Iff holoemoteDataIff("datatables/customization/holoemotes.iff");
	holoemoteDataTable.load(holoemoteDataIff);
	for (int n = 0; n < holoemoteDataTable.getNumRows(); ++n)
	{
		std::string const & holoemoteName = holoemoteDataTable.getStringValue(HoloemoteColumnNames::holoemoteName, n);
		int const requiredHairSkillMod = holoemoteDataTable.getIntValue(HoloemoteColumnNames::requiredHairSkillMod, n);
		ms_holoemoteHairSkillRequired[holoemoteName] = requiredHairSkillMod;
	}


	ExitChain::add(CustomizationManager::remove, "CustomizationManager::remove", 0, false);
}

//----------------------------------------------------------------------

void CustomizationManager::remove ()
{
	clearData();
	DEBUG_FATAL(!ms_installed, ("Not Installed"));
	ms_installed = false;
}

//----------------------------------------------------------------------

void CustomizationManager::clearData()
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("CustomizationManager not installed"));
		return; //lint !e527 unreachable (reachable in release)
	}

	ms_allowBald.clear();
	ms_customizationsBySpeciesGenderAndGroup.clear();
	ms_customizationsBySpeciesGender.clear();
	ms_customizationGroupsShared.clear();
	ms_customizationGroupsInstance.clear();
	ms_paletteColumnData.clear();
	ms_holoemoteHairSkillRequired.clear();
}

//----------------------------------------------------------------------

bool CustomizationManager::getAllowBald(std::string const & species_gender)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("CustomizationManager not installed"));
		return false; //lint !e527 unreachable (reachable in release)
	}

	std::map<std::string, bool>::const_iterator i = ms_allowBald.find(species_gender);
	if(i != ms_allowBald.end())
	{
		return i->second;
	}
	return false;
}

//----------------------------------------------------------------------

CustomizationManager::CustomizationSpeciesMap const & CustomizationManager::getCustomizationSpeciesMap(std::string const & species_gender)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("CustomizationManager not installed"));
	}

	return ms_customizationsBySpeciesGenderAndGroup[species_gender];
}

//----------------------------------------------------------------------

CustomizationManager::CustomizationGroupSharedData const & CustomizationManager::getCustomizationGroupSharedData(std::string const & customizationGroupName)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("CustomizationManager not installed"));
	}

	return ms_customizationGroupsShared[customizationGroupName];
}

//----------------------------------------------------------------------

CustomizationManager::CustomizationGroupInstanceData const & CustomizationManager::getCustomizationGroupInstanceData(std::string const & species_gender, std::string const & customizationGroupName)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("CustomizationManager not installed"));
	}

	return ms_customizationGroupsInstance[std::make_pair(species_gender, customizationGroupName)];
}

//----------------------------------------------------------------------

bool CustomizationManager::getCustomization(std::string const & species_gender, std::string const & customizationName, CustomizationManager::Customization & /*OUT*/ result)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("CustomizationManager not installed"));
		return false; //lint !e527 unreachable (reachable in release)
	}

	std::map<std::string, CustomizationVector>::const_iterator i = ms_customizationsBySpeciesGender.find(species_gender);
	if(i != ms_customizationsBySpeciesGender.end())
	{
		CustomizationVector customizations = i->second;
		for(CustomizationVector::const_iterator j = customizations.begin(); j != customizations.end(); ++j)
		{
			if(j->name == customizationName)
			{
				result = *j;
				return true;
			}
		}
	}
	return false;
}

//----------------------------------------------------------------------

std::string CustomizationManager::getServerSpeciesGender(Object const & creature)
{
	std::string result;

	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("CustomizationManager not installed"));
		return result; //lint !e527 unreachable (reachable in release)
	}

	std::string const templateName = creature.getObjectTemplateName();
	std::string::size_type serverPos = templateName.find_last_of("/");
	std::string::size_type const iffPos = templateName.find(".iff", 0);
	if (serverPos != static_cast<unsigned int>(std::string::npos) && iffPos != static_cast<unsigned int>(std::string::npos))
	{
		serverPos += strlen("/");
		result = templateName.substr(serverPos, iffPos - serverPos);
	}
	return result;
}

//----------------------------------------------------------------------

std::string CustomizationManager::getSharedSpeciesGender(Object const & creature)
{
	std::string result;

	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("CustomizationManager not installed"));
		return result; //lint !e527 unreachable (reachable in release)
	}

	std::string const templateName = creature.getObjectTemplateName();
	std::string::size_type sharedPos = templateName.find("shared_", 0);
	std::string::size_type const iffPos = templateName.find(".iff", 0);
	if (sharedPos != static_cast<unsigned int>(std::string::npos) && iffPos != static_cast<unsigned int>(std::string::npos))
	{
		sharedPos += strlen("shared_");
		result = templateName.substr(sharedPos, iffPos - sharedPos);
	}
	return result;
}

//----------------------------------------------------------------------

CustomizationVariable * CustomizationManager::findVariable (CustomizationData & cdata, std::string const & partialName)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("CustomizationManager not installed"));
		return nullptr; //lint !e527 unreachable (reachable in release)
	}

	PathType type = PT_none;
	return findVariable (cdata, partialName, type);
}

//----------------------------------------------------------------------

CustomizationVariable * CustomizationManager::findVariable (CustomizationData & cdata, std::string const & partialName, PathType & type)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("CustomizationManager not installed"));
		return nullptr; //lint !e527 unreachable (reachable in release)
	}

	type = PT_none;
	CustomizationVariable * cv = 0;
	cv = cdata.findVariable (CustomizationManager::cms_shared_owner + partialName);

	RangedIntCustomizationVariable const * const rangedCV = dynamic_cast<RangedIntCustomizationVariable *>(cv);
	if(rangedCV)
	{
		//if we have the ranged customization variable is a dependent variable, ignore it (there will be another that controls it)
		if(rangedCV->getIsDependentVariable())
			cv = nullptr;
	}

	if (!cv)
	{
		cv = cdata.findVariable (CustomizationManager::cms_priv + partialName);
		if (cv)
			type = PT_private;
	}
	else
		type = PT_shared;

	return cv;
}

//----------------------------------------------------------------------

std::string CustomizationManager::getCustomizationNameForVariables(std::string const & species_gender, std::string const & variables, bool isOnHair)
{
	std::string result;
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("CustomizationManager not installed"));
		return result; //lint !e527 unreachable (reachable in release)
	}

	std::map<std::string, CustomizationManager::CustomizationVector>::const_iterator i = ms_customizationsBySpeciesGender.find(species_gender);
	if(i != ms_customizationsBySpeciesGender.end())
	{
		for(CustomizationManager::CustomizationVector::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			Customization const & customization = *j;
			if(customization.variables == variables)
			{
				//a given variable might exist on both the creature AND their hair object, make sure we get the right customization
				if((!customization.isVarHairColor && !isOnHair) ||
				   (customization.isVarHairColor && isOnHair))
					return customization.name;
			}
		}
	}

	return result;
}

//----------------------------------------------------------------------

std::string CustomizationManager::convertSharedHairTemplateNameToServerHairTemplateName(std::string const &sharedHairTemplateName)
{
	static std::string const shared("shared_");
	std::string::size_type pos = sharedHairTemplateName.find(shared);
	std::string result;
	if(pos != sharedHairTemplateName.npos) //lint !e737 size_type and npos mismatch bogosity
	{
		result = sharedHairTemplateName.substr(0, pos);
		result += sharedHairTemplateName.substr(pos + shared.size());
	}
	return result;
}

//----------------------------------------------------------------------

std::string CustomizationManager::convertServerHairTemplateNameToSharedHairTemplateName(std::string const &serverHairTemplateName)
{
	static std::string const shared("shared_");
	std::string result;
	std::string::size_type pos = serverHairTemplateName.find_last_of("/");
	if(pos != serverHairTemplateName.npos) //lint !e737 size_type and npos mismatch bogosity
	{
		result = serverHairTemplateName.substr(0, pos + 1);
		result += shared;
		result += serverHairTemplateName.substr(pos + 1);
	}
	return result;
}

//----------------------------------------------------------------------

/** Get the palette column data for a given palette.
    The paletteName should be of the format "pn_hum_eyes"
*/
CustomizationManager::PaletteColumns CustomizationManager::getPaletteColumnDataShortName(std::string const & paletteName)
{
	return ms_paletteColumnData[paletteName]; 
}

//----------------------------------------------------------------------

/** Get the palette column data for a given palette.
    The paletteName should be of the format "palette/pn_hum_eyes.pal"
*/
CustomizationManager::PaletteColumns CustomizationManager::getPaletteColumnDataLongName(std::string const & paletteName)
{
	std::string const & shortPaletteName = shortenPaletteName(paletteName);
	return ms_paletteColumnData[shortPaletteName]; 
}

//----------------------------------------------------------------------

std::map<std::string, CustomizationManager::PaletteColumns> const & CustomizationManager::getPaletteColumnData()
{
	return ms_paletteColumnData;
}

//----------------------------------------------------------------------

std::string CustomizationManager::shortenPaletteName(std::string const & longPaletteName)
{
	std::string result;
	std::string::size_type startingPos = longPaletteName.find_last_of("/");
	if(startingPos != longPaletteName.npos) //lint !e737 size_type and npos mismatch bogosity
	{
		++startingPos;
		std::string::size_type const endingPos = longPaletteName.find(".pal", startingPos);
		if(endingPos != longPaletteName.npos) //lint !e737 size_type and npos mismatch bogosity
			if(endingPos > startingPos)
				result = longPaletteName.substr(startingPos, endingPos - startingPos);
	}
	return result;
}

//----------------------------------------------------------------------

bool CustomizationManager::isHoloemoteName(std::string const & holoemoteName)
{
	std::map<std::string, int>::const_iterator i = ms_holoemoteHairSkillRequired.find(holoemoteName);
	return (i != ms_holoemoteHairSkillRequired.end());
}

//----------------------------------------------------------------------

int const CustomizationManager::getRequiredHairSkillForHoloemote(std::string const & holoemoteName)
{
	if(holoemoteName.empty())
		return 0;
	std::map<std::string, int>::const_iterator i = ms_holoemoteHairSkillRequired.find(holoemoteName);
	if (i != ms_holoemoteHairSkillRequired.end())
	{
		return i->second;
	}
	else
		return 10000;
}

// =====================================================================
