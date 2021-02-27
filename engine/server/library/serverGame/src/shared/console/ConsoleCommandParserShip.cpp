//======================================================================
//
// ConsoleCommandParserShip.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ConsoleCommandParserShip.h"

#include "UnicodeUtils.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/ServerWorld.h"
#include "serverGame/ShipComponentDataManager.h"
#include "serverGame/ShipInternalDamageOverTime.h"
#include "serverGame/ShipInternalDamageOverTimeManager.h"
#include "serverGame/ShipObject.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedGame/ShipChassis.h"
#include "sharedGame/ShipChassisSlot.h"
#include "sharedGame/ShipComponentAttachmentManager.h"
#include "sharedGame/ShipComponentData.h"
#include "sharedGame/ShipComponentDescriptor.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedNetworkMessages/ShipDamageMessage.h"
#include "sharedObject/Controller.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplateList.h"

#include <cstdio>

// ======================================================================

//lint -e641

namespace
{
	namespace CommandNames
	{
#define MAKE_COMMAND(a) const char * const a = #a
		MAKE_COMMAND(getSlotList);
		MAKE_COMMAND(install);
		MAKE_COMMAND(pseudoInstall);
		MAKE_COMMAND(uninstall);
		MAKE_COMMAND(purge);
		MAKE_COMMAND(getComponentDescriptor);
		MAKE_COMMAND(pseudoDamageShip);
		MAKE_COMMAND(idotList);
		MAKE_COMMAND(idotSet);
		MAKE_COMMAND(idotRemove);
#undef MAKE_COMMAND
	}

	const CommandParser::CmdInfo cmds[] =
	{
		{CommandNames::getSlotList,            0, "[slot name] [ship OID]",                   "Lists slots on a ship"},
		{CommandNames::install,                2, "<component id> <slot> [ship OID]",         "Installs a component in a ship."},
		{CommandNames::pseudoInstall,          2, "<component name> <slot> [ship OID]",       "Installs a psuedo component in a ship."},
		{CommandNames::uninstall,              1, "<slot> [target container Id] [ship OID]",  "Uninstalls a component from a ship."},
		{CommandNames::purge,                  1, "<slot> [ship OID]",                        "Purges a component from a ship."},
		{CommandNames::getComponentDescriptor, 0, "[component OID]",                          "View component descriptor info for a component"},
		{CommandNames::pseudoDamageShip,       0, "",                                         "Send a hit message to your piloted ship by the lookAt target."},
		{CommandNames::idotList,               0, "[networkId]",                              "List all active IDOTS."},
		{CommandNames::idotSet,                3, "<slot> <damage rate> <damage threshold> [networkId]",  "Set an active IDOT."},
		{CommandNames::idotRemove,             1, "<slot> [networkId]",                                   "Remove an active IDOT."},

		{"", 0, "", ""} // this must be last
	};

	//----------------------------------------------------------------------

	TangibleObject * findTangible (const NetworkId & defaultId, const CommandParser::StringVector_t & argv, int index)
	{
		if (index < 0)
			return 0;

		NetworkId oid = defaultId;

		if (argv.size () > static_cast<size_t>(index))
			oid = NetworkId (Unicode::wideToNarrow (argv [static_cast<size_t>(index)]));

		return dynamic_cast<TangibleObject *>(NetworkIdManager::getObjectById(oid));
	}
}

//-----------------------------------------------------------------

ConsoleCommandParserShip::ConsoleCommandParserShip (void) :
CommandParser ("ship", 0, "...", "Ship related commands.", 0)
{
	createDelegateCommands (cmds);
}

//----------------------------------------------------------------------


bool ConsoleCommandParserShip::performParsing (const NetworkId & userId, const StringVector_t & argv, const String_t & , String_t & result, const CommandParser *)
{			
	CreatureObject const * const user = dynamic_cast<const CreatureObject *>(NetworkIdManager::getObjectById (userId));

	if (!user)
		return false;

    CreatureObject * const playerObject = dynamic_cast<CreatureObject *>(ServerWorld::findObjectByNetworkId(userId));
    if (!playerObject)
    {
        WARNING_STRICT_FATAL(true, ("Console command executed on invalid player object %s", userId.getValueString().c_str()));
        return false;
    }

    if (!playerObject->getClient()->isGod()) {
        return false;
    }

	// ----------------------------------------------------------------
	
	if (isCommand (argv[0], CommandNames::getSlotList))
	{
		ShipObject * const ship = dynamic_cast<ShipObject *>(findTangible (user->getLookAtTarget (), argv, 2));

		std::string slotNameToken;
		if (argv.size () > 1)
		{
			slotNameToken = Unicode::wideToNarrow (argv [1]);
			if (!slotNameToken.empty () && slotNameToken [0] == '*')
				slotNameToken.clear ();
		}

		if (ship == nullptr)
		{
			result += getErrorMessage (argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		uint32 const chassisType = ship->getChassisType    ();

		ShipChassis const * const shipChassis = ShipChassis::findShipChassisByCrc    (chassisType);

		if (shipChassis == nullptr)
		{
			result += Unicode::narrowToWide ("No chassis");
			return true;
		}

		ShipChassis::SlotVector const & slotVector = shipChassis->getSlots ();

		char buf [1024];
		const size_t buf_size = sizeof (buf);

		snprintf (buf, buf_size, 
			"Chassis type [%s] (%lu)\n"
			"       Slot count [%d], Hitpoints [%5.2f/%5.2f], Mass [%5.2f/%5.2f]\n",
			shipChassis->getName ().getString (), shipChassis->getCrc (),
			static_cast<int>(slotVector.size ()),
			ship->getCurrentChassisHitPoints(), ship->getMaximumChassisHitPoints(),
			ship->getChassisComponentMassCurrent(), ship->getChassisComponentMassMaximum());

		result += Unicode::narrowToWide (buf);

		for (int chassisSlot = ShipChassisSlotType::SCST_first; chassisSlot < ShipChassisSlotType::SCST_num_types; ++chassisSlot)
		{
			int effectiveCompatibilitySlot = chassisSlot;
			// weapon slots beyond 7 wrap back to slot 0 for the purposes of compatibility checking on captial ships
			if (ship->isCapitalShip() && chassisSlot >= ShipChassisSlotType::SCST_num_explicit_types && chassisSlot <= ShipChassisSlotType::SCST_weapon_last)
				effectiveCompatibilitySlot = ShipChassisSlotType::SCST_weapon_first+((chassisSlot-ShipChassisSlotType::SCST_weapon_first)&7);

			ShipChassisSlot const * const slot = shipChassis->getSlot(static_cast<ShipChassisSlotType::Type>(effectiveCompatibilitySlot));

			if (!slot)
				continue;

			if (!slotNameToken.empty() && ShipChassisSlotType::getNameFromType(slot->getSlotType ()) != slotNameToken)
				continue;

			snprintf (buf, buf_size,
				"        \\#pcontrast1 [SLOT %s (%d)]\\#.\n"
				"                Hitweight [%d], compats: ",
				ShipChassisSlotType::getNameFromType(static_cast<ShipChassisSlotType::Type>(chassisSlot)).c_str(),
				chassisSlot,
				slot->getHitWeight());

			result += Unicode::narrowToWide (buf);
			result += Unicode::narrowToWide(slot->getCompatibilityString());
			result.push_back ('\n');

			ShipComponentData * shipComponentData = 0;

			if (ship->isSlotInstalled(chassisSlot))
				shipComponentData = ship->createShipComponentData(chassisSlot);

			if (shipComponentData == nullptr)
			{
				result += Unicode::narrowToWide ("                Loaded NONE\n");
			}
			else
			{
				result += Unicode::narrowToWide ("                Loaded:\n");
				shipComponentData->printDebugString (result, Unicode::narrowToWide ("                     "));
				ShipComponentAttachmentManager::TemplateHardpointPairVector const & thpv = 
					ShipComponentAttachmentManager::getAttachmentsForShip (chassisType, shipComponentData->getDescriptor ().getCrc(), chassisSlot);

				if (!thpv.empty ())
				{
					result += Unicode::narrowToWide ("                   Attachments: ");

					for (ShipComponentAttachmentManager::TemplateHardpointPairVector::const_iterator thpv_it = thpv.begin (); thpv_it != thpv.end (); ++thpv_it)
					{
						uint32 const objectTemplateCrc  = (*thpv_it).first;
						CrcString const & hardpointName = (*thpv_it).second;

						CrcString const & objectTemplateCrcString = ObjectTemplateList::lookUp(objectTemplateCrc);

						result += Unicode::narrowToWide (objectTemplateCrcString.getString ());
						result.push_back (':');
						result += Unicode::narrowToWide (hardpointName.getString ());
						result.push_back (',');
						result.push_back (' ');
					}

					result.push_back ('\n');
				}

				delete shipComponentData;

			}
		}
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv[0], CommandNames::getComponentDescriptor))
	{
		TangibleObject * const component = findTangible (user->getLookAtTarget (), argv, 1);

		if (component == nullptr)
		{
			result += getErrorMessage (argv[0], ERR_INVALID_OBJECT);
			return true;
		}

		ShipComponentData * const shipComponentData = ShipComponentDataManager::create (*component);

		if (shipComponentData == nullptr)
		{
			result += Unicode::narrowToWide ("Not a ship component");
			return true;
		}

		shipComponentData->printDebugString (result, Unicode::emptyString);

		delete shipComponentData;
		return true;

	}

	//----------------------------------------------------------------------

	else if (isCommand (argv[0], CommandNames::install))
	{
		ShipObject * const ship          = dynamic_cast<ShipObject *>(findTangible (user->getLookAtTarget (), argv, 3));
		TangibleObject * const component = findTangible (NetworkId::cms_invalid, argv, 1);
		std::string const & slotName     = Unicode::wideToNarrow (argv [2]);

		if (ship == nullptr)
		{
			result += Unicode::narrowToWide ("no ship");
			return true;
		}

		if (component == nullptr)
		{
			result += Unicode::narrowToWide ("no component");
			return true;
		}

		ShipChassisSlotType::Type shipChassisSlotType = ShipChassisSlotType::getTypeFromName (slotName);

		if (shipChassisSlotType == ShipChassisSlotType::SCST_num_types)
		{
			result += Unicode::narrowToWide ("not a valid slot name");
			return true;
		}

		if (ship->isSlotInstalled (shipChassisSlotType))
		{
			result += Unicode::narrowToWide ("That slot is already filled");
			return true;
		}

		ShipChassis const * const shipChassis = ShipChassis::findShipChassisByCrc (ship->getChassisType ());
		if (shipChassis == nullptr)
		{
			result += Unicode::narrowToWide ("Ship chassis invalid");
			return true;
		}

		ShipChassisSlot const * const slot = shipChassis->getSlot (shipChassisSlotType);
		if (slot == nullptr)
		{
			result += Unicode::narrowToWide ("Ship chassis does not support that slot");
			return true;
		}
		
		ShipComponentData * const shipComponentData = ShipComponentDataManager::create (*component);

		if (shipComponentData == nullptr)
		{
			result += Unicode::narrowToWide ("Not a ship component");
			return true;
		}

		if (!slot->canAcceptComponent (shipComponentData->getDescriptor ()))
		{
			result += Unicode::narrowToWide ("Component cannot be installed here, may not be compatible.");
			delete shipComponentData;
			return true;
		}

		if (!ship->installComponent (user->getNetworkId(), static_cast<int>(shipChassisSlotType), *component))
			result += Unicode::narrowToWide ("Installation failed.  Check warnings");
		else
			result += Unicode::narrowToWide ("Installation Success");
		

		delete shipComponentData;
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv[0], CommandNames::pseudoInstall))
	{
		ShipObject * const ship                = dynamic_cast<ShipObject *>(findTangible (user->getLookAtTarget (), argv, 3));
		std::string const & componentName      = Unicode::wideToNarrow (argv [1]);
		std::string const & slotName           = Unicode::wideToNarrow (argv [2]);
		
		if (ship == nullptr)
		{
			result += Unicode::narrowToWide ("no ship");
			return true;
		}
		
		uint32 const componentCrc = Crc::normalizeAndCalculate (componentName.c_str ());

		ShipComponentDescriptor const * const shipComponentDescriptor = ShipComponentDescriptor::findShipComponentDescriptorByCrc (componentCrc);
		if (shipComponentDescriptor == nullptr)
		{
			result += Unicode::narrowToWide ("Invalid component name");
			return true;
		}

		ShipChassisSlotType::Type const shipChassisSlotType = ShipChassisSlotType::getTypeFromName (slotName);

		if (shipChassisSlotType == ShipChassisSlotType::SCST_num_types)
		{
			result += Unicode::narrowToWide ("not a valid slot name");
			return true;
		}

		if (!ship->pseudoInstallComponent (static_cast<int>(shipChassisSlotType), componentCrc))
			result += Unicode::narrowToWide ("Installation Failed... check warnings");
		else		
			result += Unicode::narrowToWide ("Installation Success");

		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv[0], CommandNames::uninstall))
	{
		ShipObject * const ship                = dynamic_cast<ShipObject *>(findTangible (user->getLookAtTarget (), argv, 3));
		TangibleObject * const targetContainer = findTangible (user->getInventory ()->getNetworkId (), argv, 2);
		std::string const & slotName           = Unicode::wideToNarrow (argv [1]);

		if (ship == nullptr)
		{
			result += Unicode::narrowToWide ("no ship");
			return true;
		}

		if (targetContainer == nullptr)
		{
			result += Unicode::narrowToWide ("no target container");
			return true;
		}

		ShipChassisSlotType::Type shipChassisSlotType = ShipChassisSlotType::getTypeFromName (slotName);

		if (shipChassisSlotType == ShipChassisSlotType::SCST_num_types)
		{
			result += Unicode::narrowToWide ("not a valid slot name");
			return true;
		}

		if (!ship->isSlotInstalled (shipChassisSlotType))
		{
			result += Unicode::narrowToWide ("That slot is not filled");
			return true;
		}

		if (!ship->uninstallComponent (user->getNetworkId(), static_cast<int>(shipChassisSlotType), *targetContainer))
			result += Unicode::narrowToWide ("Uninstallation failed.  Check warnings");
		else
			result += Unicode::narrowToWide ("Uninstallation Success");
		
		return true;
	}
	
	//----------------------------------------------------------------------
	
	else if (isCommand (argv[0], CommandNames::purge))
	{
		ShipObject * const ship                = dynamic_cast<ShipObject *>(findTangible (user->getLookAtTarget (), argv, 2));
		std::string const & slotName           = Unicode::wideToNarrow (argv [1]);
		
		if (ship == nullptr)
		{
			result += Unicode::narrowToWide ("no ship");
			return true;
		}
		
		ShipChassisSlotType::Type shipChassisSlotType = ShipChassisSlotType::getTypeFromName (slotName);
		
		if (shipChassisSlotType == ShipChassisSlotType::SCST_num_types)
		{
			result += Unicode::narrowToWide ("not a valid slot name");
			return true;
		}
		
		if (!ship->isSlotInstalled (shipChassisSlotType))
		{
			result += Unicode::narrowToWide ("That slot is not filled");
			return true;
		}
		
		ship->purgeComponent (static_cast<int>(shipChassisSlotType));
		result += Unicode::narrowToWide ("Purge Success");
		
		return true;
	}
	
	//----------------------------------------------------------------------
	
	else if (isCommand (argv[0], CommandNames::pseudoDamageShip))
	{
		ShipObject const * const victimShipObject = user->getPilotedShip();
		if (victimShipObject == nullptr)
		{
			result += Unicode::narrowToWide ("You are not piloting a ship.");
			return true;
		}
		
		ShipObject const * const attackerShipObject = dynamic_cast<ShipObject *>(findTangible (user->getLookAtTarget (), argv, 2));
		if (attackerShipObject == nullptr)
		{
			result += Unicode::narrowToWide ("You don't have attacker ship targeted.");
			return true;
		}
		
		Controller * const victimShipController = const_cast<Controller * const>(victimShipObject->getController());
		if (victimShipController)
		{
			ShipDamageMessage shipDamage(attackerShipObject->getNetworkId(),
											attackerShipObject->getPosition_w(),
											1.0f );

			MessageQueueGenericValueType<ShipDamageMessage> * const damageMessage = new MessageQueueGenericValueType<ShipDamageMessage>(shipDamage);
			
			victimShipController->appendMessage(CM_shipDamageMessage, 0.0f, damageMessage, 
												GameControllerMessageFlags::SEND | 
												GameControllerMessageFlags::RELIABLE | 
												GameControllerMessageFlags::DEST_AUTH_CLIENT);
		}

		return true;
	}
	
	//----------------------------------------------------------------------

	else if (isCommand (argv[0], CommandNames::idotList))
	{
		ShipObject * const ship                = dynamic_cast<ShipObject *>(findTangible (user->getLookAtTarget (), argv, 1));

		ShipInternalDamageOverTimeManager::IdotVector const & idotVector = ShipInternalDamageOverTimeManager::getActiveShipInternalDamageOverTime();

		char buf[1024];
		size_t const buf_size = sizeof(buf);

		int displayCount = 0;

		for (ShipInternalDamageOverTimeManager::IdotVector::const_iterator it = idotVector.begin(); it != idotVector.end(); ++it)
		{
			ShipInternalDamageOverTime const & idot = *it;

			ShipObject const * const idotShip = idot.getShipObject();

			if (ship != nullptr && ship != idotShip)
				continue;

			uint32 const chassisType = idotShip->getChassisType();
			ShipChassis const * const chassis = ShipChassis::findShipChassisByCrc(chassisType);			
			std::string const chassisTypeName = (chassis != nullptr) ? chassis->getName().getString() : "<NO VALID CHASSIS>";

			float hpCur = 0.0f;
			float hpMax = 0.0f;
	
			IGNORE_RETURN(idot.checkValidity(hpCur, hpMax));

			float hpPercent = (hpMax > 0.0f) ? hpCur / hpMax : -1.0f;

			snprintf(buf, buf_size, 
				"[%15s] [%15s]:[%-10s] rate=[%3.1f], threshold=[%3.1f], hp cur/max/pct=[%5.1f/%5.1f/%3.3f\n",
				idotShip->getNetworkId().getValueString().c_str(), chassisTypeName.c_str(), 
				ShipChassisSlotType::getNameFromType(static_cast<ShipChassisSlotType::Type>(idot.getChassisSlot())).c_str(),
				idot.getDamageRate(), idot.getDamageThreshold(), hpCur, hpMax, hpPercent); 
			buf[buf_size-1] = 0;
			result += Unicode::narrowToWide(buf);
			++displayCount;
		}

		snprintf(buf, buf_size, "%d/%d displayed", displayCount, idotVector.size());
		buf[buf_size-1] = 0;
		result += Unicode::narrowToWide(buf);
		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv[0], CommandNames::idotSet))
	{
		ShipObject * const ship                = dynamic_cast<ShipObject *>(findTangible (user->getLookAtTarget (), argv, 4));

		if (ship == nullptr)
		{
			result += Unicode::narrowToWide ("no ship");
			return true;
		}

		std::string const & slotName           = Unicode::wideToNarrow (argv [1]);		
		ShipChassisSlotType::Type const shipChassisSlotType = ShipChassisSlotType::getTypeFromName (slotName);
		
		float const damageRate = static_cast<float>(atof(Unicode::wideToNarrow(argv[2]).c_str()));
		float const damageThreshold = static_cast<float>(atof(Unicode::wideToNarrow(argv[3]).c_str()));

		if (!ShipInternalDamageOverTimeManager::setEntry(*ship, static_cast<int>(shipChassisSlotType), damageRate, damageThreshold))
			result += Unicode::narrowToWide("failed to set the IDOT entry");

		return true;
	}

	//----------------------------------------------------------------------

	else if (isCommand (argv[0], CommandNames::idotRemove))
	{
		ShipObject * const ship                = dynamic_cast<ShipObject *>(findTangible (user->getLookAtTarget (), argv, 4));

		if (ship == nullptr)
		{
			result += Unicode::narrowToWide ("no ship");
			return true;
		}

		std::string const & slotName           = Unicode::wideToNarrow (argv [1]);		
		ShipChassisSlotType::Type const shipChassisSlotType = ShipChassisSlotType::getTypeFromName (slotName);

		if (!ShipInternalDamageOverTimeManager::removeEntry(*ship, static_cast<int>(shipChassisSlotType)))
			result += Unicode::narrowToWide("failed to remove the IDOT entry");

		return true;
	}

	//----------------------------------------------------------------------
		
	return false;
}
	
//----------------------------------------------------------------------
