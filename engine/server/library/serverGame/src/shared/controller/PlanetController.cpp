//========================================================================
//
// PlanetController.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/PlanetController.h"
#include "serverGame/PlanetObject.h"
#include "serverGame/GameServer.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedNetworkMessages/ObjectChannelMessages.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedObject/AlterResult.h"
#include "sharedObject/Object.h"
#include "swgServerNetworkMessages/MessageQueuePlanetAddTravelPoint.h"

//-----------------------------------------------------------------------

PlanetController::PlanetController(PlanetObject * newOwner) :
		UniverseController(newOwner)
{
}

//-----------------------------------------------------------------------

PlanetController::~PlanetController()
{
}

//-----------------------------------------------------------------------

void PlanetController::handleMessage (const int message, const float value, const MessageQueue::Data* const data, const uint32 flags)
{
	PlanetObject* const owner = dynamic_cast<PlanetObject *>(getOwner());
	NOT_NULL(owner);

    switch(message)
    {
		case CM_planetAddTravelPoint:
			{
				const MessageQueuePlanetAddTravelPoint* const message = safe_cast<const MessageQueuePlanetAddTravelPoint*> (data);
				if (message)
					owner->addTravelPoint (message->getTravelPointName (), message->getPosition_w (), message->getCost (), message->getInterplanetary (), static_cast<TravelPoint::TravelPointType>(message->getType()));
			}
			break;

		case CM_planetRemoveTravelPoint:
			{
				const MessageQueueGenericValueType<std::string>* const message = safe_cast<const MessageQueueGenericValueType<std::string>*> (data);
				if (message)
					owner->removeTravelPoint (message->getValue ());
			}
			break;

		case CM_setWeather:
			{
				const MessageQueueGenericValueType<std::pair<int, Vector> >* const message = safe_cast<const MessageQueueGenericValueType<std::pair<int, Vector> >*> (data);
				if (message != nullptr)
				{
					owner->setWeather(
						message->getValue().first, 
						message->getValue().second.x, 
						message->getValue().second.y, 
						message->getValue().second.z);
				}
			}
			break;

		case CM_serverRemovePlanetMapLocation:
			{
				const MessageQueueGenericValueType<NetworkId> * const message = safe_cast<const MessageQueueGenericValueType<NetworkId> *> (data);
				if (message)
					owner->removeMapLocation (message->getValue ());
			}
			break;

		case CM_serverAddPlanetMapLocation:
			{
				const MessageQueueGenericValueType<std::pair<MapLocation, std::pair<int, int> > > * const message = safe_cast<const MessageQueueGenericValueType<std::pair<MapLocation, std::pair<int, int> > > *> (data);
				if (message)
					owner->addMapLocation (message->getValue ().first, message->getValue ().second.first, (message->getValue ().second.second ? true : false));
			}
			break;

		case CM_collectionRequestServerFirst:
			{
				const MessageQueueGenericValueType<std::pair<std::pair<std::string, NetworkId>, std::pair<Unicode::String, std::string> > > * const message = safe_cast<const MessageQueueGenericValueType<std::pair<std::pair<std::string, NetworkId>, std::pair<Unicode::String, std::string> > > *> (data);
				if (message)
				{
					CollectionsDataTable::CollectionInfoCollection const * const collectionInfo = CollectionsDataTable::getCollectionByName (message->getValue ().first.first);
					if (collectionInfo)
						owner->setCollectionServerFirst (*collectionInfo, message->getValue ().first.second, message->getValue ().second.first, message->getValue ().second.second);
				}
			}
			break;

		case CM_collectionRevokeServerFirst:
			{
				const MessageQueueGenericValueType<std::string> * const message = safe_cast<const MessageQueueGenericValueType<std::string> *> (data);
				if (message)
				{
					CollectionsDataTable::CollectionInfoCollection const * const collectionInfo = CollectionsDataTable::getCollectionByName (message->getValue ());
					if (collectionInfo)
						owner->removeCollectionServerFirst (*collectionInfo);
				}
			}
			break;

		case CM_setConnectedCharacterLfgData:
			{
				const MessageQueueGenericValueType<std::pair<NetworkId, LfgCharacterData> > * const message = safe_cast<const MessageQueueGenericValueType<std::pair<NetworkId, LfgCharacterData> > *> (data);
				if (message)
				{
					owner->setConnectedCharacterLfgData(message->getValue().first, message->getValue().second);
				}
			}
			break;

		case CM_removeConnectedCharacterData:
			{
				const MessageQueueGenericValueType<NetworkId> * const message = safe_cast<const MessageQueueGenericValueType<NetworkId> *> (data);
				if (message)
				{
					owner->removeConnectedCharacterData(message->getValue());
				}
			}
			break;

		case CM_setConnectedCharacterGroupData:
			{
				const MessageQueueGenericValueType<std::pair<NetworkId, NetworkId> > * const message = safe_cast<const MessageQueueGenericValueType<std::pair<NetworkId, NetworkId> > *> (data);
				if (message)
				{
					owner->setConnectedCharacterGroupData(message->getValue().first, message->getValue().second);
				}
			}
			break;

		case CM_setConnectedCharacterPlanetData:
			{
				const MessageQueueGenericValueType<std::pair<std::string, NetworkId> > * const message = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, NetworkId> > *> (data);
				if (message)
				{
					owner->setConnectedCharacterPlanetData(message->getValue().second, message->getValue().first);
				}
			}
			break;

		case CM_setConnectedCharacterRegionData:
			{
				const MessageQueueGenericValueType<std::pair<std::string, NetworkId> > * const message = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, NetworkId> > *> (data);
				if (message)
				{
					owner->setConnectedCharacterRegionData(message->getValue().second, message->getValue().first);
				}
			}
			break;

		case CM_setConnectedCharacterPlayerCityData:
			{
				const MessageQueueGenericValueType<std::pair<std::string, NetworkId> > * const message = safe_cast<const MessageQueueGenericValueType<std::pair<std::string, NetworkId> > *> (data);
				if (message)
				{
					owner->setConnectedCharacterPlayerCityData(message->getValue().second, message->getValue().first);
				}
			}
			break;

		case CM_setConnectedCharacterBiographyData:
			{
				const MessageQueueGenericValueType<std::pair<Unicode::String, NetworkId> > * const message = safe_cast<const MessageQueueGenericValueType<std::pair<Unicode::String, NetworkId> > *> (data);
				if (message)
				{
					owner->setConnectedCharacterBiographyData(message->getValue().second, message->getValue().first);
				}
			}
			break;

		case CM_removeConnectedCharacterBiographyData:
			{
				const MessageQueueGenericValueType<NetworkId> * const message = safe_cast<const MessageQueueGenericValueType<NetworkId> *> (data);
				if (message)
				{
					owner->removeConnectedCharacterBiographyData(message->getValue());
				}
			}
			break;

		case CM_setConnectedCharacterLevelData:
			{
				const MessageQueueGenericValueType<std::pair<NetworkId, int> > * const message = safe_cast<const MessageQueueGenericValueType<std::pair<NetworkId, int> > *> (data);
				if (message)
				{
					owner->setConnectedCharacterLevelData(message->getValue().first, static_cast<int16>(message->getValue().second));
				}
			}
			break;

		case CM_setConnectedCharacterProfessionData:
			{
				const MessageQueueGenericValueType<std::pair<NetworkId, int> > * const message = safe_cast<const MessageQueueGenericValueType<std::pair<NetworkId, int> > *> (data);
				if (message)
				{
					owner->setConnectedCharacterProfessionData(message->getValue().first, static_cast<LfgCharacterData::Profession>(message->getValue().second));
				}
			}
			break;

		case CM_setConnectedCharacterSearchableByCtsSourceGalaxyData:
			{
				const MessageQueueGenericValueType<std::pair<NetworkId, int> > * const message = safe_cast<const MessageQueueGenericValueType<std::pair<NetworkId, int> > *> (data);
				if (message)
				{
					owner->setConnectedCharacterSearchableByCtsSourceGalaxyData(message->getValue().first, (message->getValue().second == 0) ? false : true);
				}
			}
			break;

		case CM_setConnectedCharacterDisplayLocationInSearchResultsData:
			{
				const MessageQueueGenericValueType<std::pair<NetworkId, int> > * const message = safe_cast<const MessageQueueGenericValueType<std::pair<NetworkId, int> > *> (data);
				if (message)
				{
					owner->setConnectedCharacterDisplayLocationInSearchResultsData(message->getValue().first, (message->getValue().second == 0) ? false : true);
				}
			}
			break;

		case CM_setConnectedCharacterAnonymousData:
			{
				const MessageQueueGenericValueType<std::pair<NetworkId, int> > * const message = safe_cast<const MessageQueueGenericValueType<std::pair<NetworkId, int> > *> (data);
				if (message)
				{
					owner->setConnectedCharacterAnonymousData(message->getValue().first, (message->getValue().second == 0) ? false : true);
				}
			}
			break;

		case CM_setConnectedCharacterInterestsData:
			{
				const MessageQueueGenericValueType<std::pair<NetworkId, BitArray> > * const message = safe_cast<const MessageQueueGenericValueType<std::pair<NetworkId, BitArray> > *> (data);
				if (message)
				{
					owner->setConnectedCharacterInterestsData(message->getValue().first, message->getValue().second);
				}
			}
			break;

		case CM_addUniverseEvent:
			{
				const MessageQueueGenericValueType<std::string> * const message = safe_cast<const MessageQueueGenericValueType<std::string> *> (data);
				if (message)
				{
					owner->addUniverseEvent(message->getValue());
				}
			}
			break;

		case CM_removeUniverseEvent:
			{
				const MessageQueueGenericValueType<std::string> * const message = safe_cast<const MessageQueueGenericValueType<std::string> *> (data);
				if (message)
				{
					owner->removeUniverseEvent(message->getValue());
				}
			}
			break;

		case CM_adjustGcwImperialScore:
			{
				const MessageQueueGenericValueType<std::map<std::string, int64> > * const message = safe_cast<const MessageQueueGenericValueType<std::map<std::string, int64> > *> (data);
				if (message)
				{
					const std::map<std::string, int64> & data = message->getValue();

					for (std::map<std::string, int64>::const_iterator iter = data.begin(); iter != data.end(); ++iter)
						owner->adjustGcwImperialScore("CM_adjustGcwImperialScore", nullptr, iter->first, iter->second);
				}
			}
			break;

		case CM_adjustGcwRebelScore:
			{
				const MessageQueueGenericValueType<std::map<std::string, int64> > * const message = safe_cast<const MessageQueueGenericValueType<std::map<std::string, int64> > *> (data);
				if (message)
				{
					const std::map<std::string, int64> & data = message->getValue();

					for (std::map<std::string, int64>::const_iterator iter = data.begin(); iter != data.end(); ++iter)
						owner->adjustGcwRebelScore("CM_adjustGcwRebelScore", nullptr, iter->first, iter->second);
				}
			}
			break;

		default:
			UniverseController::handleMessage(message, value, data, flags);
            break;
    }
}

// ======================================================================
