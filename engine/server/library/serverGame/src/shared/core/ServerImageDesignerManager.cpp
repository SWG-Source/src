// ============================================================================
//
// ServerImageDesignerManager.cpp
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ServerImageDesignerManager.h"

#include "serverGame/Chat.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerObjectTemplate.h"
#include "serverGame/ServerWorld.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedGame/CustomizationManager.h"
#include "sharedGame/CustomizationManager_MorphParameter.h"
#include "sharedGame/ProsePackage.h"
#include "sharedGame/SharedStringIds.h"
#include "sharedNetworkMessages/ImageDesignChangeMessage.h"
#include "sharedObject/Controller.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/CustomizationVariable.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/Object.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/PaletteColorCustomizationVariable.h"
#include "sharedObject/RangedIntCustomizationVariable.h"
#include "sharedObject/SlotId.h"
#include "sharedObject/SlotIdManager.h"
#include "sharedObject/SlottedContainer.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptFunctionTable.h"
#include "serverScript/ScriptParameters.h"
#include "sharedUtility/DataTable.h"

// ============================================================================

// ============================================================================

namespace ServerImageDesignerManagerNamespace
{
	class MyCallback : public MessageDispatch::Callback
	{
	public:
		MyCallback ()
		: MessageDispatch::Callback ()
		{
			connect (*this, &MyCallback::onScaleFactorChanged, static_cast<CustomizationManager::MorphParameter::Messages::ScaleFactorChanged *>(0));
		}

		void onScaleFactorChanged (CustomizationManager::MorphParameter::Messages::ScaleFactorChanged::Payload const & payload)
		{
			NetworkId const & nid = payload.first;
			Object * const o = NetworkIdManager::getObjectById(nid);
			ServerObject * const so = o ? o->asServerObject() : nullptr;
			CreatureObject * const creature = so ? so->asCreatureObject() : nullptr;
			if (creature)
			{
				ObjectTemplate const * const tmp = creature->getSharedTemplate();
				SharedObjectTemplate const * const tmpl = dynamic_cast<SharedObjectTemplate const *>(tmp);
				if(tmpl)
				{
					float const denormalized = CustomizationManager::MorphParameter::denormalizeScale (tmpl, payload.second);
					creature->setScaleFactor (denormalized);
				}
			}
		}//lint !e1762 function could be const (no it couldn't, overriding base function)
	};

	struct HairData
	{
		std::string hairTemplate;
		std::string characterTemplate;
		int requiredHairSkillMod;
	};

	namespace Filenames
	{
		char const * const cms_hairstyleAssetTable = "datatables/customization/hair_assets_skill_mods.iff";
		char const * const cms_allowBaldTable = "datatables/customization/allow_bald.iff";
	}
	
	namespace HairAssetSkillModsColumnNames
	{
		char const * const hairServerTemplate = "SERVER_TEMPLATE";
		char const * const playerServerTemplate = "SERVER_PLAYER_TEMPLATE";
		char const * const skillModValue = "SKILL_MOD_VALUE";
	}

	namespace AllowBaldColumnNames
	{
		char const * const speciesGender = "SPECIES_GENDER";
		char const * const allowBald = "ALLOW_BALD";
	}

	bool ms_installed = false;
	MyCallback * ms_callback = 0;

	std::string const cms_hairCustomizationName("hair");
	char const * const cms_hairSlotName = "hair";

	std::map<int, std::string> ms_hairIndexMap;
	std::multimap<NetworkId, ServerImageDesignerManager::GradualCustomizationData> ms_gradualCustomizationMap;
	std::map<std::string, HairData> ms_hairNameToHairData;
	std::map<std::string, bool> ms_genderSpeciesToAllowBald;
}

using namespace ServerImageDesignerManagerNamespace;

// ============================================================================

void ServerImageDesignerManager::install()
{
	DEBUG_FATAL(ms_installed, ("Already Installed"));
	ms_installed = true;

	ms_callback = new MyCallback();

	ms_hairNameToHairData.clear();
	ms_genderSpeciesToAllowBald.clear();
	DataTable dataTable;
	Iff hairMappingIff(Filenames::cms_hairstyleAssetTable);
	dataTable.load(hairMappingIff);
	HairData hairData;
	for (int i = 0; i < dataTable.getNumRows(); ++i)
	{
		hairData.hairTemplate = dataTable.getStringValue(HairAssetSkillModsColumnNames::hairServerTemplate, i);
		hairData.characterTemplate = dataTable.getStringValue(HairAssetSkillModsColumnNames::playerServerTemplate, i);
		hairData.requiredHairSkillMod = dataTable.getIntValue(HairAssetSkillModsColumnNames::skillModValue, i);
		ms_hairNameToHairData[hairData.hairTemplate] = hairData;
	}
	Iff allowBaldIff(Filenames::cms_allowBaldTable);
	DataTable allowBaldDataTable;
	allowBaldDataTable.load(allowBaldIff);
	for (int j = 0; j < allowBaldDataTable.getNumRows(); ++j)
	{
		std::string const & genderSpecies = allowBaldDataTable.getStringValue(AllowBaldColumnNames::speciesGender, j);
		int const allowBald = allowBaldDataTable.getIntValue(AllowBaldColumnNames::allowBald, j);
		ms_genderSpeciesToAllowBald[genderSpecies] = (allowBald != 0) ? true : false;
	}

	ExitChain::add(ServerImageDesignerManager::remove, "ServerImageDesignerManager::remove", 0, false);
}

//-----------------------------------------------------------------------------

void ServerImageDesignerManager::remove()
{
	DEBUG_FATAL(!ms_installed, ("Not Installed"));
	ms_installed = false;
	
	ms_hairNameToHairData.clear();
	ms_genderSpeciesToAllowBald.clear();

	delete ms_callback;
	ms_callback = nullptr;
}

//-----------------------------------------------------------------------------

bool ServerImageDesignerManager::makeChanges(SharedImageDesignerManager::Session const & session)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("ServerImageDesignerManager not installed"));
		return false;
	}

	NetworkId const & recipientId = session.recipientId;
	Object * const recipientObj = NetworkIdManager::getObjectById(recipientId);
	ServerObject * const recipientServerObj = recipientObj ? recipientObj->asServerObject() : nullptr;
	CreatureObject * const recipient = recipientServerObj ? recipientServerObj->asCreatureObject() : nullptr;
	NetworkId const & designerId = session.designerId;
	Object * const designerObj = NetworkIdManager::getObjectById(designerId);
	ServerObject * const designerServerObj = designerObj ? designerObj->asServerObject() : nullptr;
	CreatureObject * const designer = designerServerObj ? designerServerObj->asCreatureObject() : nullptr;
	if(designer && recipient)
	{

		//validate that any selected hair can go on this character
		if(session.newHairSet)
		{
			//check new hair
			if(!session.newHairAsset.empty())
			{
				bool newHairOk = false;
				std::map<std::string, HairData>::iterator a = ms_hairNameToHairData.find(session.newHairAsset);
				if(a != ms_hairNameToHairData.end())
				{
					if(strcmp(recipient->getTemplateName(), a->second.characterTemplate.c_str()) == 0)
					{
						SharedImageDesignerManager::SkillMods const mods = getSkillModsForDesigner(designer->getNetworkId());
						if(mods.hairSkillMod >= a->second.requiredHairSkillMod)
							newHairOk = true;
					}
				}
				if(!newHairOk)
				{
					Chat::sendSystemMessage(*designer, SharedStringIds::imagedesigner_hair_not_valid, Unicode::emptyString);
					if(designer != recipient)
						Chat::sendSystemMessage(*recipient, SharedStringIds::imagedesigner_hair_not_valid, Unicode::emptyString);
					return false;
				}
			}
			//they want bald, is this okay for this gender/species?
			else
			{
				bool baldOk = false;
				std::string const & speciesGender = CustomizationManager::getServerSpeciesGender(*recipient);
				std::map<std::string, bool>::iterator b = ms_genderSpeciesToAllowBald.find(speciesGender);
				if(b != ms_genderSpeciesToAllowBald.end())
				{
					if(b->second)
						baldOk = true;
				}
				if(!baldOk)
				{
					Chat::sendSystemMessage(*designer, SharedStringIds::imagedesigner_bald_not_valid, Unicode::emptyString);
					if(designer != recipient)
						Chat::sendSystemMessage(*recipient, SharedStringIds::imagedesigner_bald_not_valid, Unicode::emptyString);
					return false;
				}
			}
		}

		//ensure designer is going to get paid enough
		if(designer != recipient)
		{
			if(session.designerRequiredCredits > session.recipientPaidCredits)
			{
				Chat::sendSystemMessage(*designer, SharedStringIds::imagedesigner_not_paying_enough, Unicode::emptyString);
				if(designer != recipient)
					Chat::sendSystemMessage(*recipient, SharedStringIds::imagedesigner_not_paying_enough, Unicode::emptyString);
				ServerImageDesignerManager::cancelSession(designer->getNetworkId(), recipient->getNetworkId());
				return false;
			}

			if(session.designerRequiredCredits > recipient->getTotalMoney())
			{
				Chat::sendSystemMessage(*designer, SharedStringIds::imagedesigner_not_paying_enough, Unicode::emptyString);
				if(designer != recipient)
					Chat::sendSystemMessage(*recipient, SharedStringIds::imagedesigner_not_paying_enough, Unicode::emptyString);
				ServerImageDesignerManager::cancelSession(designer->getNetworkId(), recipient->getNetworkId());
				return false;
			}

			bool result = false;
			//pay the designer
			if(session.recipientPaidCredits > 0)
			{
				int moneyToMove = session.recipientPaidCredits;
				int const recipientCash = recipient->getCashBalance();
				if(moneyToMove <= recipientCash)
				{
					//just move cash if we can
					result = recipient->transferCashTo(session.designerId, moneyToMove);
					if(!result)
					{
						WARNING(true, ("ImageDesigner: could not transfer [%d] cash credits from nid[%s] to nid[%s] for services rendered", moneyToMove, recipient->getNetworkId().getValueString().c_str(), session.designerId.getValueString().c_str()));
						Chat::sendSystemMessage(*designer, SharedStringIds::imagedesigner_not_paying_enough, Unicode::emptyString);
						if(designer != recipient)
							Chat::sendSystemMessage(*recipient, SharedStringIds::imagedesigner_not_paying_enough, Unicode::emptyString);
						ServerImageDesignerManager::cancelSession(designer->getNetworkId(), recipient->getNetworkId());
						return false;
					}
					//we've moved all the necessary money
				}
				else
				{
					if(recipientCash > 0)
					{
						//move all their cash, and pull the rest from the bank
						result = recipient->transferCashTo(session.designerId, recipientCash);
						if(!result)
						{
							WARNING(true, ("ImageDesigner: could not transfer [%d] cash credits from nid[%s] to nid[%s] for services rendered", recipientCash, recipient->getNetworkId().getValueString().c_str(), session.designerId.getValueString().c_str()));
							Chat::sendSystemMessage(*designer, SharedStringIds::imagedesigner_not_paying_enough, Unicode::emptyString);
							if(designer != recipient)
								Chat::sendSystemMessage(*recipient, SharedStringIds::imagedesigner_not_paying_enough, Unicode::emptyString);
							ServerImageDesignerManager::cancelSession(designer->getNetworkId(), recipient->getNetworkId());
							return false;
						}
						moneyToMove -= recipientCash;
					}

					//get the rest from the bank if we can
					result = recipient->transferBankCreditsTo(session.designerId, moneyToMove);
					if(!result)
					{
						WARNING(true, ("ImageDesigner: could not transfer [%d] bank credits from nid[%s] to nid[%s] for services rendered", moneyToMove, recipient->getNetworkId().getValueString().c_str(), session.designerId.getValueString().c_str()));
						Chat::sendSystemMessage(*designer, SharedStringIds::imagedesigner_not_paying_enough, Unicode::emptyString);
						if(designer != recipient)
							Chat::sendSystemMessage(*recipient, SharedStringIds::imagedesigner_not_paying_enough, Unicode::emptyString);
						ServerImageDesignerManager::cancelSession(designer->getNetworkId(), recipient->getNetworkId());
						return false;
					}
				}

				//tell recipient that they paid
				ProsePackage prosePackage;
				prosePackage.stringId = SharedStringIds::imagedesigner_recip_paid;
				prosePackage.digitInteger = session.recipientPaidCredits;
				Chat::sendSystemMessage(*recipient, prosePackage);

				//tell designer that were they paid
				prosePackage.stringId = SharedStringIds::imagedesigner_designer_was_paid;
				Chat::sendSystemMessage(*designer, prosePackage);
			}
		}

		//OK, everything checks out, make all the changes NOW!

		for(std::map<std::string, float>::const_iterator i = session.morphChanges.begin(); i != session.morphChanges.end(); ++i)
		{
			setMorphCustomization(*recipient, i->first, i->second);
		}

		for(std::map<std::string, int>::const_iterator j = session.indexChanges.begin(); j != session.indexChanges.end(); ++j)
		{
			setIndexCustomization(*recipient, j->first, j->second);
		}

		if(session.newHairSet)
		{
			SlottedContainer * const slotted = ContainerInterface::getSlottedContainer(*recipient);
			if(slotted)
			{
				SlotId const slot = SlotIdManager::findSlotId (ConstCharCrcLowerString (cms_hairSlotName));
				Container::ContainerErrorCode tmp = Container::CEC_Success;
				Object * const originalHairObject = slotted->getObjectInSlot(slot, tmp).getObject();
				ServerObject * const originalHairServerObject = originalHairObject ? originalHairObject->asServerObject() : nullptr;
				TangibleObject * const orignalHair = originalHairServerObject ? originalHairServerObject->asTangibleObject() : nullptr;
				std::string originalHairCustomizationData;
				if(orignalHair)
				{
					//first get the color of the old hair, so it can be applied to the new hair
					CustomizationData const * const originalHairData = orignalHair->fetchCustomizationData ();
					if(originalHairData)
					{
						originalHairCustomizationData = originalHairData->writeLocalDataToString();
						originalHairData->release();
					}
						//destroy the old hair
					IGNORE_RETURN(orignalHair->permanentlyDestroy(DeleteReasons::Script));
				}

				if(!session.newHairAsset.empty())
				{
					GradualCustomizationData gradualCustomization;
					gradualCustomization.customizationName = cms_hairCustomizationName;
					gradualCustomization.templateName = session.newHairAsset;
					gradualCustomization.customizationString = originalHairCustomizationData;
					gradualCustomization.customizationData = getHairCustomizations(session);
					gradualCustomization.countdown = 20;
					IGNORE_RETURN(ms_gradualCustomizationMap.insert(std::make_pair(recipient->getNetworkId(), gradualCustomization)));
				}
			}
		}
		sendSessionToScript(session, session.designerId, static_cast<int>(Scripting::TRIG_IMAGE_DESIGN_COMPLETED));
	}
	return true;
}

//----------------------------------------------------------------------

void ServerImageDesignerManager::setMorphCustomization(CreatureObject & creature, std::string const & customizationName, float const value)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("ServerImageDesignerManager not installed"));
		return;
	}

	CustomizationManager::Customization customization;
	bool const result = CustomizationManager::getCustomization(CustomizationManager::getServerSpeciesGender(creature), customizationName, customization);
	if(result)
	{
		CustomizationData * const customizationData = fetchCustomizationDataForCustomization(creature, customization);
		if(customizationData)
		{
			CustomizationManager::MorphParameter param(*customizationData, customization);
			param.update(creature, value);
			customizationData->release();
		}
	}
}

//----------------------------------------------------------------------

void ServerImageDesignerManager::setIndexCustomization(CreatureObject & creature, std::string const & customizationName, int const value)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("ServerImageDesignerManager not installed"));
		return;
	}

	CustomizationManager::Customization customization;
	bool const result = CustomizationManager::getCustomization(CustomizationManager::getServerSpeciesGender(creature), customizationName, customization);
	if(result)
	{
		CustomizationData * const customizationData = fetchCustomizationDataForCustomization(creature, customization);
		if(customizationData)
		{
			CustomizationVariable * const cv = CustomizationManager::findVariable (*customizationData, customization.variables);
			PaletteColorCustomizationVariable * const paletteVariable = dynamic_cast<PaletteColorCustomizationVariable *>(cv);
			if(paletteVariable)
			{
				int rangeMin = 0;
				int rangeMax = 0;
				paletteVariable->getRange(rangeMin, rangeMax);
				int const finalValue = clamp(rangeMin, value, rangeMax);
				paletteVariable->setValue(finalValue);
			}
			customizationData->release();
		}
	}
}

//----------------------------------------------------------------------

SharedImageDesignerManager::SkillMods ServerImageDesignerManager::getSkillModsForDesigner(NetworkId const & designerId)
{
	SharedImageDesignerManager::SkillMods skillMods;

	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("ServerImageDesignerManager not installed"));
		return skillMods;
	}

	Object const * const o = NetworkIdManager::getObjectById(designerId);
	ServerObject const * const so = o ? o->asServerObject() : nullptr;
	CreatureObject const * const designer = so ? so->asCreatureObject() : nullptr;
	if(designer)
	{
		skillMods.bodySkillMod = designer->getModValue(SharedImageDesignerManager::cms_bodySkillModName);
		skillMods.faceSkillMod = designer->getModValue(SharedImageDesignerManager::cms_faceSkillModName);
		skillMods.markingsSkillMod = designer->getModValue(SharedImageDesignerManager::cms_markingsSkillModName);
		skillMods.hairSkillMod = designer->getModValue(SharedImageDesignerManager::cms_hairSkillModName);
	}
	return skillMods;
}

//----------------------------------------------------------------------

void ServerImageDesignerManager::updateGradualCustomizations(CreatureObject * target)
{
	if(!target)
		return;

	//move certain customizations over time
	std::pair<std::multimap<NetworkId, GradualCustomizationData>::iterator, std::multimap<NetworkId, GradualCustomizationData>::iterator> p = ms_gradualCustomizationMap.equal_range(target->getNetworkId());
	if(p.first != ms_gradualCustomizationMap.end())
	{
		for(std::multimap<NetworkId, GradualCustomizationData>::iterator i = p.first; i != p.second;)
		{
			if(!--(i->second.countdown))
			{
				if(i->second.customizationName == cms_hairCustomizationName)
				{
					//do this one immediately
					SlotId const slot = SlotIdManager::findSlotId (ConstCharCrcLowerString (cms_hairCustomizationName.c_str()));
					ServerObject * const hair = ServerWorld::createNewObject(i->second.templateName, *target, slot, true);
					TangibleObject * const tangibleHair = hair ? hair->asTangibleObject() : nullptr;
					if(tangibleHair)
					{
						//first set hair color to colors from old hair (if any)
						if(!i->second.customizationString.empty())
						{
							CustomizationData * const data = tangibleHair->fetchCustomizationData();
							if(data)
							{
								data->loadLocalDataFromString(i->second.customizationString);
								data->release();
							}
						}
						//override hair color with any customizations made in this session
						for(std::map<std::string, int>::iterator j = i->second.customizationData.begin(); j != i->second.customizationData.end(); ++j)
						{
							setIndexCustomization(*target, j->first, j->second);
						}
					}
					else
					{
						DEBUG_WARNING(true, ("Could not create hair %s\n", i->second.templateName.c_str()));
					}
				}
				i = ms_gradualCustomizationMap.erase(i);
			}
			else
			{
				++i;
			}
		}
	}
}

//----------------------------------------------------------------------

void ServerImageDesignerManager::sendSessionToScriptForValidation(SharedImageDesignerManager::Session const & session)
{
	NetworkId const & designerId = session.designerId;
	NetworkId const & recipientId = session.recipientId;
	Object const * const designerObj = NetworkIdManager::getObjectById(designerId);
	ServerObject const * const designerServer = designerObj ? designerObj->asServerObject() : nullptr;
	CreatureObject const * const designer = designerServer ? designerServer->asCreatureObject() : nullptr;
	Object * const recipientObj = NetworkIdManager::getObjectById(recipientId);
	ServerObject * const recipientServer = recipientObj ? recipientObj->asServerObject() : nullptr;
	CreatureObject * const recipient = recipientServer ? recipientServer->asCreatureObject() : nullptr;
	if(designer && recipient)
	{
		sendSessionToScript(session, session.designerId, static_cast<int>(Scripting::TRIG_IMAGE_DESIGN_VALIDATE));
	}
}

//----------------------------------------------------------------------

void ServerImageDesignerManager::sendSessionToScript(SharedImageDesignerManager::Session const & session, NetworkId const & objectToTriggerId, int const trigger)
{
	Object * const objectToTrigger = NetworkIdManager::getObjectById(objectToTriggerId);
	ServerObject * const serverObjectToTrigger = objectToTrigger ? objectToTrigger->asServerObject() : nullptr;
	if(serverObjectToTrigger)
	{
		GameScriptObject * const scriptObject = serverObjectToTrigger->getScriptObject();
		if (scriptObject)
		{
			ScriptParams params;
			params.addParam(session.designerId, "designerId");
			params.addParam(session.recipientId, "recipientId");
			params.addParam(session.terminalId, "terminalId");
			params.addParam(static_cast<int>(session.startingTime), "startingTime");
			params.addParam(static_cast<int>(session.designType), "designType");
			params.addParam(session.newHairSet, "newHairSet");
			params.addParam(session.newHairAsset.c_str(), "newHairAsset");
			params.addParam(session.hairCustomizationData.c_str(), "hairCustomizationData");
			params.addParam(session.designerRequiredCredits, "designerRequiredCredits");
			params.addParam(session.recipientPaidCredits, "recipientPaidCredits");
			params.addParam(session.accepted, "accepted");
			std::vector<const char *> morphKeys;
			std::vector<float> morphValues;
			std::vector<const char *> indexKeys;
			std::vector<int> indexValues;
			for(std::map<std::string, float>::const_iterator i = session.morphChanges.begin(); i != session.morphChanges.end(); ++i)
			{
				morphKeys.push_back(i->first.c_str());
				morphValues.push_back(i->second);
			}
			params.addParam(morphKeys, "morphChangesKeys");
			params.addParam(morphValues, "morphChangesValues");
			for(std::map<std::string, int>::const_iterator j = session.indexChanges.begin(); j != session.indexChanges.end(); ++j)
			{
				indexKeys.push_back(j->first.c_str());
				indexValues.push_back(j->second);
			}
			params.addParam(indexKeys, "indexChangesKeys");
			params.addParam(indexValues, "indexChangesValues");
			params.addParam(session.holoEmote.c_str(), "holoEmote");

			IGNORE_RETURN(scriptObject->trigAllScripts(static_cast<Scripting::TrigId>(trigger), params));
		}
	}
}

//----------------------------------------------------------------------

CustomizationData * ServerImageDesignerManager::fetchCustomizationDataForCustomization(CreatureObject & creature, CustomizationManager::Customization const & customization)
{
	//use the hair if it's the one to be queried
	TangibleObject * objectToQuery = &creature;
	if(customization.isVarHairColor)
	{
		ServerObject * const hair = creature.getHair();
		TangibleObject * const tangibleHair = hair ? hair->asTangibleObject() : nullptr;
		if(tangibleHair)
			objectToQuery = tangibleHair;
		else
			return nullptr;
	}
	return objectToQuery->fetchCustomizationData();
}

//----------------------------------------------------------------------

void ServerImageDesignerManager::cancelSession(NetworkId const & designerId, NetworkId const & recipientId)
{
	//send the cancel message to the designer
	Object * const designerObject = NetworkIdManager::getObjectById(designerId);
	Controller * const designerController = designerObject ? designerObject->getController() : nullptr;
	if(designerController)
	{
		ImageDesignChangeMessage * outMsg = new ImageDesignChangeMessage();

		outMsg->setDesignerId(designerId);
		outMsg->setRecipientId(recipientId);
		outMsg->setOrigin(ImageDesignChangeMessage::O_SERVER);
		designerController->appendMessage(static_cast<int>(CM_imageDesignerCancel), 0.0f, outMsg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
	}

	//send the cancel message to the recipient
	Object * const recipientObject = NetworkIdManager::getObjectById(recipientId);
	Controller * const recipientController = recipientObject ? recipientObject->getController() : nullptr;
	if(recipientController && designerController != recipientController)
	{
		ImageDesignChangeMessage * outMsg = new ImageDesignChangeMessage();

		outMsg->setDesignerId(designerId);
		outMsg->setRecipientId(recipientId);
		outMsg->setOrigin(ImageDesignChangeMessage::O_SERVER);
		recipientController->appendMessage(static_cast<int>(CM_imageDesignerCancel), 0.0f, outMsg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
	}

	//send cancel trigger to designer player
	ServerObject * const designerServerObject = designerObject ? designerObject->asServerObject() : nullptr;
	if(designerServerObject)
	{
		GameScriptObject * const scriptObject = designerServerObject->getScriptObject();
		if (scriptObject)
		{
			ScriptParams params;
			IGNORE_RETURN(scriptObject->trigAllScripts(Scripting::TRIG_IMAGE_DESIGN_CANCELED, params));
		}
	}

	//send cancel trigger to recipient player
	ServerObject * const recipientServerObject = recipientObject ? recipientObject->asServerObject() : nullptr;
	if(recipientServerObject && recipientServerObject != designerServerObject)
	{
		GameScriptObject * const scriptObject = recipientServerObject->getScriptObject();
		if (scriptObject)
		{
			ScriptParams params;
			IGNORE_RETURN(scriptObject->trigAllScripts(Scripting::TRIG_IMAGE_DESIGN_CANCELED, params));
		}
	}

	SharedImageDesignerManager::endSession(designerId);
}

//----------------------------------------------------------------------

/** For a given session, pick out all the customization changes that would affect the player's hair
*/
std::map<std::string, int> ServerImageDesignerManager::getHairCustomizations(SharedImageDesignerManager::Session const & session)
{
	Object * const o = NetworkIdManager::getObjectById(session.recipientId);
	ServerObject * const so = o ? o->asServerObject() : nullptr;
	CreatureObject * const recipient = so ? so->asCreatureObject() : nullptr;
	CustomizationManager::Customization customization;
	bool result = false;
	std::map<std::string, int> hairCustomizations;
	if (recipient)
	{
		//all hair customizations will be in the index changes
		for(std::map<std::string, int>::const_iterator i = session.indexChanges.begin(); i != session.indexChanges.end(); ++i)
		{
			result = CustomizationManager::getCustomization(CustomizationManager::getServerSpeciesGender(*recipient), i->first, customization);
			if(result)
			{
				if(customization.isVarHairColor)
					hairCustomizations[i->first] = i->second;
			}
		}
	}
	return hairCustomizations;
}

// ============================================================================

