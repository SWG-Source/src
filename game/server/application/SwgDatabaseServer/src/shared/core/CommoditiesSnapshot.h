// ======================================================================
//
// CommoditiesSnapshot.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CommoditiesSnapshot_H
#define INCLUDED_CommoditiesSnapshot_H

// ======================================================================

#include "Archive/ByteStream.h"
#include "serverDatabase/Snapshot.h"
#include "serverDatabase/TableBuffer.h"
#include "sharedFoundation/Tag.h"
#include "SwgDatabaseServer/AuctionLocationsBuffer.h"
#include "SwgDatabaseServer/MarketAuctionsBuffer.h"
#include "SwgDatabaseServer/MarketAuctionBidsBuffer.h"

class CMCreateAuctionMessage;
class CMCreateAuctionBidMessage;
class CMCreateLocationMessage;
class CMDeleteAuctionMessage;
class CMDeleteLocationMessage;
class CMUpdateAuctionMessage;
class CMUpdateLocationMessage;

namespace DB
{
	class Session;
}

// ======================================================================

/**
 * A Snapshot customized to Star Wars Galaxies Commodities Server.
 *
 * @todo:  there are a lot of public functions that could be made private
 */
class CommoditiesSnapshot : public Snapshot
{
public:
	CommoditiesSnapshot(DB::ModeQuery::Mode mode);
	~CommoditiesSnapshot();

	// Network Functions:
	virtual void handleUpdateObjectPosition(const UpdateObjectPositionMessage &message);
	virtual void handleDeleteMessage   (const NetworkId &objectID, int reasonCode, bool immediate, bool demandLoadedContainer, bool cascadeReason);
	virtual void handleMessageTo       (const MessageToPayload &data);
	virtual void handleMessageToAck    (const MessageToId &messageId);
	virtual void handleAddResourceTypeMessage (const AddResourceTypeMessage &) { ;}
	virtual void handleBountyHunterTargetMessage (const BountyHunterTargetMessage &) { ;}
	virtual bool send                  (GameServerConnection *connection) const;

	// Saving and Loading:
	virtual bool saveToDB              (DB::Session *session);
	virtual bool load                  (DB::Session *session);
	virtual void saveCompleted         ();
	virtual void getWorldContainers    (std::vector<NetworkId> &containers) const;
	
	virtual void handleCommoditiesDataMessage(const MessageDispatch::MessageBase & message);
	virtual void startLoadAfterSaveComplete();

	// Functions for object creation:
	virtual void newObject(NetworkId const & objectId, int templateId, Tag typeId);
	
protected:
	MarketAuctionsBufferDelete m_deleteMarketAuctionsBuffer;
	AuctionLocationsBuffer  m_deleteAuctionLocationsBuffer;
	MarketAuctionsBufferCreate m_createMarketAuctionsBuffer;
	AuctionLocationsBuffer  m_createAuctionLocationsBuffer;
	MarketAuctionBidsBuffer m_createMarketAuctionBidsBuffer;
	MarketAuctionsBufferUpdate m_updateMarketAuctionsBuffer;
	AuctionLocationsBuffer  m_updateAuctionLocationsBuffer;
	
	bool doLoadAfterSaveComplete;

private:
	void handleCreateAuctionMessage(CMCreateAuctionMessage &msg);
	void handleCreateAuctionBidMessage(CMCreateAuctionBidMessage &msg);
	void handleCreateLocationMessage(CMCreateLocationMessage &msg);
	void handleUpdateAuctionMessage(CMUpdateAuctionMessage &msg);
	void handleUpdateLocationMessage(CMUpdateLocationMessage &msg);
	void handleDeleteAuctionMessage(CMDeleteAuctionMessage &msg);
	void handleDeleteLocationMessage(CMDeleteLocationMessage &msg);

	virtual void decodeServerData(NetworkId const & objectId, Tag typeId, uint16 index, Archive::ReadIterator &bs, bool isBaseline);
	virtual void decodeSharedData(NetworkId const & objectId, Tag typeId, uint16 index, Archive::ReadIterator &bs, bool isBaseline);
	virtual void decodeClientData(NetworkId const & objectId, Tag typeId, uint16 index, Archive::ReadIterator &bs, bool isBaseline);
	virtual void decodeParentClientData(NetworkId const & objectId, Tag typeId, uint16 index, Archive::ReadIterator &bs, bool isBaseline);
	
	virtual bool encodeParentClientData(NetworkId const & objectId, Tag typeId, std::vector<BatchBaselinesMessageData> &baselines) const;
	virtual bool encodeClientData(NetworkId const & objectId, Tag typeId, std::vector<BatchBaselinesMessageData> &baselines) const;
	virtual bool encodeServerData(NetworkId const & objectId, Tag typeId, std::vector<BatchBaselinesMessageData> &baselines) const;
	virtual bool encodeSharedData(NetworkId const & objectId, Tag typeId, std::vector<BatchBaselinesMessageData> &baselines) const;

private:
	CommoditiesSnapshot(const CommoditiesSnapshot&); //disable
	CommoditiesSnapshot &operator=(const CommoditiesSnapshot&); //disable
};

// ======================================================================
#endif
