// ======================================================================
//
// SwgSnapshot.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "SwgDatabaseServer/CommoditiesSnapshot.h"

#include "Archive/AutoDeltaSet.h"
#include "Archive/AutoDeltaVector.h"
#include "serverDatabase/ConfigServerDatabase.h"
#include "serverDatabase/DatabaseProcess.h" //TODO:  remove after cleaning up authority transfer stuff
#include "serverDatabase/GameServerConnection.h"
#include "serverDatabase/ImmediateDeleteCustomPersistStep.h"
#include "serverDatabase/MessageToManager.h"
#include "serverDatabase/ObjectLocator.h"
#include "serverDatabase/Persister.h"
#include "serverNetworkMessages/CMCreateAuctionMessage.h"
#include "serverNetworkMessages/CMCreateAuctionBidMessage.h"
#include "serverNetworkMessages/CMCreateLocationMessage.h"
#include "serverNetworkMessages/CMDeleteAuctionMessage.h"
#include "serverNetworkMessages/CMDeleteLocationMessage.h"
#include "serverNetworkMessages/CMUpdateAuctionMessage.h"
#include "serverNetworkMessages/CMUpdateLocationMessage.h"
#include "serverUtility/MissionLocation.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedFoundation/Os.h"
#include "sharedUtility/Location.h"
#include "sharedGame/MatchMakingId.h"
#include "sharedLog/Log.h"
#include "sharedMath/Quaternion.h"
#include "sharedMathArchive/TransformArchive.h"
#include "sharedMathArchive/VectorArchive.h"
#include "swgSharedUtility/Attributes.def"
#include "SwgDatabaseServer/ObjectTableBuffer.h"
#include "unicodeArchive/UnicodeArchive.h"

#include "sharedFoundation/CrcConstexpr.hpp"

// ======================================================================

/**
 * Construct a commodities snapshot.
 */
CommoditiesSnapshot::CommoditiesSnapshot(DB::ModeQuery::Mode mode) :
		Snapshot(mode, false),
		m_deleteMarketAuctionsBuffer(),
		m_deleteAuctionLocationsBuffer(DB::ModeQuery::mode_DELETE),
		m_createMarketAuctionsBuffer(),
		m_createAuctionLocationsBuffer(DB::ModeQuery::mode_INSERT),
		m_createMarketAuctionBidsBuffer(DB::ModeQuery::mode_INSERT),
		m_updateMarketAuctionsBuffer(),
		m_updateAuctionLocationsBuffer(DB::ModeQuery::mode_UPDATE),
		doLoadAfterSaveComplete(false)
{
	;
}

// ----------------------------------------------------------------------

CommoditiesSnapshot::~CommoditiesSnapshot()
{
}

// ----------------------------------------------------------------------

/**
 * Called by a worker thread.  Saves the snapshot to the database
 * @todo:  would it be better to have SwgSnapshot add a CustomStep for
 * the objectTableBuffer and not override this function?
 */

bool CommoditiesSnapshot::saveToDB(DB::Session *session)
{
	if (Snapshot::getTimeStamp()!=0)
	{
		if (! (Snapshot::saveTimestamp(session)))
			return false;
	}

	LOG("CommoditiesServer", ("start save CommoditiesSnapshot"));
	// artificial delay can be added here to help debug timing issues
	if (ConfigServerDatabase::getWriteDelay() > 0)
	{
		LOG("CommoditiesSnapshot::saveToDB",("Delaying writing of snapshot for %i seconds",ConfigServerDatabase::getWriteDelay()));
		Os::sleep(ConfigServerDatabase::getWriteDelay()*1000);
	}
	DEBUG_REPORT_LOG(true, ("Snapshot calling MarketAuctionsBuffer.save().\n"));

	session->setAutoCommitMode(false);
	
	// If we want to use database rferential integrity (and we do),
	// then these must be executed in order!
	if (! (m_deleteAuctionLocationsBuffer.save(session)))
		return false;
	if (! (m_deleteMarketAuctionsBuffer.save(session)))
		return false;
	
	if (! (m_createAuctionLocationsBuffer.save(session)))
		return false;
	if (! (m_createMarketAuctionsBuffer.save(session)))
		return false;
	if (! (m_createMarketAuctionBidsBuffer.save(session)))
		return false;
	
	if (! (m_updateAuctionLocationsBuffer.save(session)))
		return false;
	if (! (m_updateMarketAuctionsBuffer.save(session)))
		return false;

	session->commitTransaction();
	LOG("CommoditiesServer", ("finish save CommoditiesSnapshot"));
	return true;
}

/**
 * Called when a message is received to update an object's transform and container.
 *
 * Identifies what the message changes, and puts it in the appropriate
 * table buffer.
 */

void CommoditiesSnapshot::handleUpdateObjectPosition(const UpdateObjectPositionMessage &message)
{
	;
}

// ----------------------------------------------------------------------

/**
 * Called when a message is received to delete an object.
 *
 * Tells the objectTableBuffer to set the deleted column to true.
 */

void CommoditiesSnapshot::handleDeleteMessage(const NetworkId &objectId, int reasonCode, bool immediate, bool demandLoadedContainer, bool cascadeReason)
{
	;
}

// ----------------------------------------------------------------------

void CommoditiesSnapshot::handleMessageTo(const MessageToPayload &data)
{
	;	
}

// ----------------------------------------------------------------------

void CommoditiesSnapshot::handleMessageToAck(const MessageToId &messageId)
{
	;	
}

// ----------------------------------------------------------------------

/**
 * Load the objects.
 *
 * This function will load all the objects listed in m_oidList.
 */
bool CommoditiesSnapshot::load(DB::Session *session)
{
	return true;
}

// ----------------------------------------------------------------------

/**
 * Send the snapshot to the appropriate server.
 *
 * This function will send messages to the appropriate gameServer to
 * create a proxy for the objects in the snapshot, then it will send a
 * message to central to relinquish authority for the objects.
 *
 * @return true if the objects were sent, false if they were not.
 */
bool CommoditiesSnapshot::send(GameServerConnection *connection) const
{
	return true;
}

// ----------------------------------------------------------------------

void CommoditiesSnapshot::saveCompleted()
{
	Snapshot::saveCompleted();
	if (doLoadAfterSaveComplete)
		Persister::getInstance().startLoadCommodities();
}

// ----------------------------------------------------------------------

void CommoditiesSnapshot::newObject(NetworkId const & objectId, int templateId, Tag typeId)
{
	WARNING(true,("Attempt to create object %s.  It has unknown type %i.",objectId.getValueString().c_str(),typeId));
}

// ----------------------------------------------------------------------

void CommoditiesSnapshot::handleCommoditiesDataMessage(const MessageDispatch::MessageBase & message)
{
	const uint32 messageType = message.getType();
	
	switch(messageType) {
		case constcrc("CMCreateAuctionMessage") :
		{
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			CMCreateAuctionMessage msg(ri);
			handleCreateAuctionMessage(msg);
			break;
		}
		case constcrc("CMCreateLocationMessage") :
		{
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			CMCreateLocationMessage msg(ri);
			handleCreateLocationMessage(msg);
			break;
		}
		case constcrc("CMCreateAuctionBidMessage") :
		{
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			CMCreateAuctionBidMessage msg(ri);
			handleCreateAuctionBidMessage(msg);
			break;
		}
		case constcrc("CMUpdateAuctionMessage") :
		{
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			CMUpdateAuctionMessage msg(ri);
			handleUpdateAuctionMessage(msg);
			break;
		}
		case constcrc("CMUpdateLocationMessage") :
		{
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			CMUpdateLocationMessage msg(ri);
			handleUpdateLocationMessage(msg);
			break;
		}
		case constcrc("CMDeleteAuctionMessage") :
		{
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			CMDeleteAuctionMessage msg(ri);
			handleDeleteAuctionMessage(msg);
			break;
		}
		case constcrc("CMDeleteLocationMessage") :
		{
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			CMDeleteLocationMessage msg(ri);
			handleDeleteLocationMessage(msg);
			break;
		}
	}
}

// ----------------------------------------------------------------------
	
void CommoditiesSnapshot::startLoadAfterSaveComplete()
{
	doLoadAfterSaveComplete = true;
}

// ----------------------------------------------------------------------

void CommoditiesSnapshot::handleCreateAuctionMessage(CMCreateAuctionMessage &msg)
{
	DEBUG_REPORT_LOG(true, ("CommoditiesSnapshot Received CMCreateAuctionMessage\n"));
	LOG("CommoditiesServer", ("Handling CMCreateAuctionMessage for item %s", msg.GetItemId().getValueString().c_str()));
	m_createMarketAuctionsBuffer.setMarketAuctions(
		msg.GetItemId(), 
		msg.GetOwnerId(), 
		msg.GetCreatorId(), 
		msg.GetLocationId(), 
		msg.GetMinimumBid(), 
		msg.GetBuyNowPrice(), 
		msg.GetAuctionTimer(), 
		msg.GetAttributes(), 
		msg.GetUserDescription(), 
		msg.GetItemType(), 
		msg.GetItemTemplateId(), 
		msg.GetItemName(), 
		msg.GetExpireTimer(),
		msg.GetFlags(),
		msg.GetItemSize());
}

void CommoditiesSnapshot::handleCreateAuctionBidMessage(CMCreateAuctionBidMessage &msg)
{
	DEBUG_REPORT_LOG(true, ("CommoditiesSnapshot Received CMCreateAuctionBidMessage\n"));
	m_createMarketAuctionBidsBuffer.setMarketAuctionBids(
		msg.GetItemId(),
		msg.GetBidderId(),
		msg.GetBid(),
		msg.GetMaxProxyBid());
}

void CommoditiesSnapshot::handleCreateLocationMessage(CMCreateLocationMessage &msg)
{
	DEBUG_REPORT_LOG(true, ("CommoditiesSnapshot Received CMCreateLocationMessage\n"));
	m_createAuctionLocationsBuffer.setAuctionLocations(
		msg.GetLocationId(),
		msg.GetLocationString(),
		msg.GetOwnerId(),
		msg.GetSalesTax(),
		msg.GetBankId(),
		msg.GetEmptyDate(),
		msg.GetLastAccessDate(),
		msg.GetInactiveDate(),
		msg.GetStatus(),
		msg.GetSearchEnabled(),
		msg.GetEntranceCharge());
}

void CommoditiesSnapshot::handleUpdateAuctionMessage(CMUpdateAuctionMessage &msg)
{
	DEBUG_REPORT_LOG(true, ("CommoditiesSnapshot Received CMUpdateAuctionMessage\n"));
	DBSchema::MarketAuctionsRow *row =  m_createMarketAuctionsBuffer.findRowByIndex(msg.GetItemId());
	if (row)
	{
		m_createMarketAuctionsBuffer.setMarketAuctions(msg.GetItemId(), msg.GetOwnerId(), msg.GetFlags());
	}
	else
	{
		m_updateMarketAuctionsBuffer.setMarketAuctions(msg.GetItemId(), msg.GetOwnerId(), msg.GetFlags());
	}
}

void CommoditiesSnapshot::handleUpdateLocationMessage(CMUpdateLocationMessage &msg)
{
	DEBUG_REPORT_LOG(true, ("CommoditiesSnapshot Received CMUpdateLocationMessage\n"));
	DBSchema::AuctionLocationsRow *row =  m_createAuctionLocationsBuffer.findRowByIndex(msg.GetLocationId());
	if (row)
	{
		m_createAuctionLocationsBuffer.setAuctionLocations(
			msg.GetLocationId(),
			msg.GetLocationString(),
			msg.GetOwnerId(),
			msg.GetSalesTax(),
			msg.GetBankId(),
			msg.GetEmptyDate(),
			msg.GetLastAccessDate(),
			msg.GetInactiveDate(),
			msg.GetStatus(),
			msg.GetSearchEnabled(),
			msg.GetEntranceCharge());
	}
	else
	{
		m_updateAuctionLocationsBuffer.setAuctionLocations(
			msg.GetLocationId(),
			msg.GetLocationString(),
			msg.GetOwnerId(),
			msg.GetSalesTax(),
			msg.GetBankId(),
			msg.GetEmptyDate(),
			msg.GetLastAccessDate(),
			msg.GetInactiveDate(),
			msg.GetStatus(),
			msg.GetSearchEnabled(),
			msg.GetEntranceCharge());
	}
}

void CommoditiesSnapshot::handleDeleteAuctionMessage(CMDeleteAuctionMessage &msg)
{
	DEBUG_REPORT_LOG(true, ("CommoditiesSnapshot Received CMDeleteAuctionMessage\n"));
	m_createMarketAuctionsBuffer.removeMarketAuctions(msg.GetItemId());
	m_updateMarketAuctionsBuffer.removeMarketAuctions(msg.GetItemId());
	m_createMarketAuctionBidsBuffer.removeMarketAuctionBids(msg.GetItemId());
	m_deleteMarketAuctionsBuffer.setMarketAuctions(msg.GetItemId());
}

void CommoditiesSnapshot::handleDeleteLocationMessage(CMDeleteLocationMessage &msg)
{
	DEBUG_REPORT_LOG(true, ("CommoditiesSnapshot Received CMDeleteLocationMessage\n"));
	m_createAuctionLocationsBuffer.removeAuctionLocations(msg.GetLocationId());
	m_updateAuctionLocationsBuffer.removeAuctionLocations(msg.GetLocationId());
	m_deleteAuctionLocationsBuffer.setAuctionLocations(msg.GetLocationId());
}

// ----------------------------------------------------------------------
/**
 * For gold mode, return a list of all the objects in the world that may be
 * ordinary (non-demand-load) containers.  The live database will be checked
 * for objects inside these objects.
 */
void CommoditiesSnapshot::getWorldContainers(std::vector<NetworkId> &containers) const
{
}
// ----------------------------------------------------------------------

void CommoditiesSnapshot::decodeServerData(NetworkId const & objectId, Tag typeId, uint16 index, Archive::ReadIterator &bs, bool isBaseline)
{
}

void CommoditiesSnapshot::decodeSharedData(NetworkId const & objectId, Tag typeId, uint16 index, Archive::ReadIterator &bs, bool isBaseline)
{
}

void CommoditiesSnapshot::decodeClientData(NetworkId const & objectId, Tag typeId, uint16 index, Archive::ReadIterator &bs, bool isBaseline)
{
}

void CommoditiesSnapshot::decodeParentClientData(NetworkId const & objectId, Tag typeId, uint16 index, Archive::ReadIterator &bs, bool isBaseline)
{
}

bool CommoditiesSnapshot::encodeSharedData(NetworkId const & objectId, Tag typeId, std::vector<BatchBaselinesMessageData> &baselines) const
{
	return false;
}

bool CommoditiesSnapshot::encodeServerData(NetworkId const & objectId, Tag typeId, std::vector<BatchBaselinesMessageData> &baselines) const
{
	return false;
}

bool CommoditiesSnapshot::encodeClientData(NetworkId const & objectId, Tag typeId, std::vector<BatchBaselinesMessageData> &baselines) const
{
	return false;
}

bool CommoditiesSnapshot::encodeParentClientData(NetworkId const & objectId, Tag typeId, std::vector<BatchBaselinesMessageData> &baselines) const
{
	return false;
}

