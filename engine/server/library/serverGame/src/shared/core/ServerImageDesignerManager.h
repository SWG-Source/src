// ============================================================================
// 
// ServerImageDesignerManager.h
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#ifndef INCLUDED_ServerImageDesignerManager_H
#define INCLUDED_ServerImageDesignerManager_H

//-----------------------------------------------------------------------------

#include "sharedGame/CustomizationManager.h"
#include "sharedGame/SharedImageDesignerManager.h"

//-----------------------------------------------------------------------------

class ImageDesignChangeMessage;
class CreatureObject;
class CustomizationData;
class ScriptParams;

//-----------------------------------------------------------------------------

class ServerImageDesignerManager
{
public:
	struct GradualCustomizationData
	{
		std::string customizationName;
		std::string templateName;
		std::map<std::string, int> customizationData;
		std::string customizationString;
		int countdown;
	};

	static void install();
	static void remove();

	static bool makeChanges(SharedImageDesignerManager::Session const & session);
	static SharedImageDesignerManager::SkillMods getSkillModsForDesigner(NetworkId const & designerId);
	static void updateGradualCustomizations(CreatureObject* target);
	static void sendSessionToScriptForValidation(SharedImageDesignerManager::Session const & session);
	static void convertSessionToScriptParams(SharedImageDesignerManager::Session const & session, ScriptParams & /*IN-OUT*/ params);
	static CustomizationData * fetchCustomizationDataForCustomization(CreatureObject & creature, CustomizationManager::Customization const & customization);
	static void cancelSession(NetworkId const & designerId, NetworkId const & recipientId);
	static std::map<std::string, int> getHairCustomizations(SharedImageDesignerManager::Session const & session);

private:
	// Disable
	ServerImageDesignerManager();
	ServerImageDesignerManager(ServerImageDesignerManager const &);
	ServerImageDesignerManager &operator =(ServerImageDesignerManager const &);

private:
	static void sendSessionToScript(SharedImageDesignerManager::Session const & session, NetworkId const & objectToTriggerId, int trigger);

private:
	static void setMorphCustomization(CreatureObject & creature, std::string const & customizationName, float value);
	static void setIndexCustomization(CreatureObject & creature, std::string const & customizationName, int value);
};

// ============================================================================

#endif // INCLUDED_ServerImageDesignerManager_H
