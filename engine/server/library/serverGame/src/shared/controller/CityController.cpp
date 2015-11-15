// ======================================================================
//
// CityController.cpp
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/CityController.h"

#include "serverGame/CityObject.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"

// ======================================================================

CityController::CityController(CityObject *newOwner) :
	UniverseController(newOwner)
{
}

// ----------------------------------------------------------------------

CityController::~CityController()
{
}

// ----------------------------------------------------------------------

void CityController::handleMessage(const int message, const float value, const MessageQueue::Data* const data, const uint32 flags)
{
	CityObject *owner = dynamic_cast<CityObject *>(getOwner());
	NOT_NULL(owner);

	switch (message)
	{
	case CM_cityCreate:
		{
			MessageQueueGenericValueType<std::pair<int, CityInfo> > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<int, CityInfo> > const *>(data);
			const CityInfo & ci = msg->getValue().second;
			IGNORE_RETURN(owner->createCity(ci.getCityName(),
				ci.getCityHallId(),
				ci.getPlanet(),
				ci.getX(),
				ci.getZ(),
				ci.getRadius(),
				ci.getLeaderId(),
				ci.getIncomeTax(),
				ci.getPropertyTax(),
				ci.getSalesTax(),
				ci.getTravelLoc(),
				ci.getTravelCost(),
				ci.getTravelInterplanetary(),
				ci.getCloneLoc(),
				ci.getCloneRespawn(),
				ci.getCloneRespawnCell(),
				ci.getCloneId(),
				msg->getValue().first));
		}
		break;
	case CM_citySetName:
		{
			MessageQueueGenericValueType<std::pair<int, std::string> > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<int, std::string> > const *>(data);
			owner->setCityName(msg->getValue().first, msg->getValue().second);
		}
		break;
	case CM_citySetCityHall:
		{
			MessageQueueGenericValueType<std::pair<int, NetworkId> > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<int, NetworkId> > const *>(data);
			owner->setCityHall(msg->getValue().first, msg->getValue().second);
		}
		break;
	case CM_citySetLocation:
		{
			MessageQueueGenericValueType<std::pair<int, std::pair<std::string, std::pair<int, int> > > > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<int, std::pair<std::string, std::pair<int, int> > > > const *>(data);
			owner->setCityLocation(
				msg->getValue().first,
				msg->getValue().second.first,
				msg->getValue().second.second.first,
				msg->getValue().second.second.second);
		}
		break;
	case CM_citySetRadius:
		{
			MessageQueueGenericValueType<std::pair<int, int> > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<int, int> > const *>(data);
			owner->setCityRadius(msg->getValue().first, msg->getValue().second);
		}
		break;
	case CM_citySetFaction:
		{
			MessageQueueGenericValueType<std::pair<int, std::pair<uint32, bool> > > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<int, std::pair<uint32, bool> > > const *>(data);
			owner->setCityFaction(msg->getValue().first, msg->getValue().second.first, msg->getValue().second.second);
		}
		break;
	case CM_citySetGcwDefenderRegion:
		{
			MessageQueueGenericValueType<std::pair<std::pair<int, std::string>, std::pair<int, bool> > > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<std::pair<int, std::string>, std::pair<int, bool> > > const *>(data);
			owner->setCityGcwDefenderRegion(msg->getValue().first.first, msg->getValue().first.second, msg->getValue().second.first, msg->getValue().second.second);
		}
		break;
	case CM_citySetCreationTime:
		{
			MessageQueueGenericValueType<std::pair<int, int> > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<int, int> > const *>(data);
			owner->setCityCreationTime(msg->getValue().first, msg->getValue().second);
		}
		break;
	case CM_citySetLeader:
		{
			MessageQueueGenericValueType<std::pair<int, NetworkId> > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<int, NetworkId> > const *>(data);
			owner->setCityLeader(msg->getValue().first, msg->getValue().second);
		}
		break;
	case CM_citySetIncomeTax:
		{
			MessageQueueGenericValueType<std::pair<int, int> > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<int, int> > const *>(data);
			owner->setCityIncomeTax(msg->getValue().first, msg->getValue().second);
		}
		break;
	case CM_citySetPropertyTax:
		{
			MessageQueueGenericValueType<std::pair<int, int> > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<int, int> > const *>(data);
			owner->setCityPropertyTax(msg->getValue().first, msg->getValue().second);
		}
		break;
	case CM_citySetSalesTax:
		{
			MessageQueueGenericValueType<std::pair<int, int> > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<int, int> > const *>(data);
			owner->setCitySalesTax(msg->getValue().first, msg->getValue().second);
		}
		break;
	case CM_citySetTravelInfo:
		{
			MessageQueueGenericValueType<std::pair<int, std::pair<Vector, std::pair<int, bool> > > > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<int, std::pair<Vector, std::pair<int, bool> > > > const *>(data);
			owner->setCityTravelInfo(
				msg->getValue().first,
				msg->getValue().second.first,
				msg->getValue().second.second.first,
				msg->getValue().second.second.second);
		}
		break;
	case CM_citySetCloneInfo:
		{
			MessageQueueGenericValueType<std::pair<int, std::pair<Vector, std::pair<Vector, std::pair<NetworkId, NetworkId> > > > > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<int, std::pair<Vector, std::pair<Vector, std::pair<NetworkId, NetworkId> > > > > const *>(data);
			owner->setCityCloneInfo(
				msg->getValue().first,
				msg->getValue().second.first,
				msg->getValue().second.second.first,
				msg->getValue().second.second.second.first,
				msg->getValue().second.second.second.second);
		}
		break;
	case CM_cityRemove:
		{
			MessageQueueGenericValueType<int> const *msg = safe_cast<MessageQueueGenericValueType<int> const *>(data);
			owner->removeCity(msg->getValue());
		}
		break;
	case CM_citySetCitizen:
		{
			MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<std::string, std::pair<NetworkId, int> > > > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<std::string, std::pair<NetworkId, int> > > > const *>(data);
			owner->setCitizen(msg->getValue().first.first, msg->getValue().first.second, msg->getValue().second.first, msg->getValue().second.second.first, msg->getValue().second.second.second);
		}
		break;
	case CM_citySetCitizenProfessionInfo:
		{
			MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<std::string, int> > > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<std::string, int> > > const *>(data);
			owner->setCitizenProfessionInfo(msg->getValue().first.first, msg->getValue().first.second, msg->getValue().second.first, msg->getValue().second.second);
		}
		break;
	case CM_citySetCitizenAddRank:
		{
			MessageQueueGenericValueType<std::pair<std::string, std::pair<NetworkId, int> > > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<std::string, std::pair<NetworkId, int> > > const *>(data);
			owner->addCitizenRank(msg->getValue().second.second, msg->getValue().second.first, msg->getValue().first);
		}
		break;
	case CM_citySetCitizenRemoveRank:
		{
			MessageQueueGenericValueType<std::pair<std::string, std::pair<NetworkId, int> > > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<std::string, std::pair<NetworkId, int> > > const *>(data);
			owner->removeCitizenRank(msg->getValue().second.second, msg->getValue().second.first, msg->getValue().first);
		}
		break;
	case CM_citySetCitizenTitle:
		{
			MessageQueueGenericValueType<std::pair<std::string, std::pair<NetworkId, int> > > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<std::string, std::pair<NetworkId, int> > > const *>(data);
			owner->setCitizenTitle(msg->getValue().second.second, msg->getValue().second.first, msg->getValue().first);
		}
		break;
	case CM_cityRemoveCitizen:
		{
			MessageQueueGenericValueType<std::pair<int, NetworkId> > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<int, NetworkId> > const *>(data);
			owner->removeCitizen(msg->getValue().first, msg->getValue().second);
		}
		break;
	case CM_citySetStructure:
		{
			MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<int, bool> > > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<int, bool> > > const *>(data);
			owner->setCityStructure(msg->getValue().first.first, msg->getValue().first.second, msg->getValue().second.first, msg->getValue().second.second);
		}
		break;
	case CM_cityRemoveStructure:
		{
			MessageQueueGenericValueType<std::pair<int, NetworkId> > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<int, NetworkId> > const *>(data);
			owner->removeCityStructure(msg->getValue().first, msg->getValue().second);
		}
		break;
	case CM_adjustPgcRating:
		{
			MessageQueueGenericValueType<std::pair<std::string, std::pair<NetworkId, int> > > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<std::string, std::pair<NetworkId, int> > > const *>(data);
			owner->adjustPgcRating(msg->getValue().second.first, msg->getValue().first, msg->getValue().second.second);
		}
		break;
	case CM_adjustPgcRatingData:
		{
			MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<std::string, int> > > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<std::string, int> > > const *>(data);
			owner->adjustPgcRatingData(msg->getValue().first.second, msg->getValue().second.first, msg->getValue().first.first, msg->getValue().second.second);
		}
		break;
	case CM_setPgcRatingData:
		{
			MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<std::string, int> > > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<std::pair<int, NetworkId>, std::pair<std::string, int> > > const *>(data);
			owner->setPgcRatingData(msg->getValue().first.second, msg->getValue().second.first, msg->getValue().first.first, msg->getValue().second.second);
		}
		break;
	case CM_fixPgcChroniclerName:
		{
			MessageQueueGenericValueType<std::pair<std::string, NetworkId> > const *msg = safe_cast<MessageQueueGenericValueType<std::pair<std::string, NetworkId> > const *>(data);
			owner->verifyPgcChroniclerName(msg->getValue().second, msg->getValue().first);
		}
		break;
	default:
		UniverseController::handleMessage(message, value, data, flags);
		break;
	}
}

// ======================================================================

