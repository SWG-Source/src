// ======================================================================
//
// Snapshot.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_Snapshot_H
#define INCLUDED_Snapshot_H

// ======================================================================

#include "serverNetworkMessages/MessageToPayload.h"
#include "sharedDatabaseInterface/DbModeQuery.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/BaselinesMessage.h"
#include "sharedNetworkMessages/BatchBaselinesMessage.h"
#include "sharedNetworkMessages/DeltasMessage.h"

namespace DB
{
	class Session;
}

class AbstractTableBuffer;
class AddResourceTypeMessage;
class BountyHunterTargetMessage;
class CustomPersistStep;
class GameServerConnection;
class ObjectLocator;
class UpdateObjectPositionMessage;

// ======================================================================

/**
 * A Snapshot is a collection of TableBuffers.
 *
 * This class is an abstract base class for Snapshots, to define the
 * interface.  It is not game-specific.
 */
class Snapshot
{
public:
	Snapshot(DB::ModeQuery::Mode mode, bool useGoldDatabase);
	virtual ~Snapshot();
	
	virtual void handleUpdateObjectPosition(const UpdateObjectPositionMessage &msg) =0;
	virtual void handleDeleteMessage   (const NetworkId & objectID, int reasonCode, bool immediate, bool demandLoadedContainer, bool cascadeReason) =0;
	virtual void handleMessageTo       (const MessageToPayload &data) =0;
	virtual void handleMessageToAck    (const MessageToId &messageId) =0;
	virtual void handleAddResourceTypeMessage (const AddResourceTypeMessage &message) =0;
	virtual void handleBountyHunterTargetMessage (const BountyHunterTargetMessage &message) =0;

	virtual void getWorldContainers    (std::vector<NetworkId> &containers) const = 0;

	void handleDeltasMessage           (NetworkId & objectId, const DeltasMessage &msg);
	void handleBaselinesMessage        (NetworkId & objectId, const BaselinesMessage &msg);
	
	virtual bool saveToDB              (DB::Session *session) =0;
	virtual bool load                  (DB::Session *session) =0;
	virtual bool send                  (GameServerConnection *connection) const =0;
	virtual void saveCompleted         ();

	void takeTimestamp();

	virtual void newObject             (const NetworkId & objectId, int templateId, Tag typeId) =0;
//	virtual void addObjectIdForLoad    (NetworkId & objectId)=0; //TODO:  the load list could be moved into Snapshot, intead of being in the derived class

	void addLocator                    (ObjectLocator *newLocator);
	void addCustomPersistStep          (CustomPersistStep *newStep);
	int  getLocatorCount               () const;
	bool getUseGoldDatabase            () const;

	DB::ModeQuery::Mode                getMode() const;
	
	void setUniverseAuthHack();

	virtual void handleCommoditiesDataMessage(const MessageDispatch::MessageBase & message) =0;
	virtual void startLoadAfterSaveComplete() =0;

  protected:
	typedef std::vector<ObjectLocator *> LocatorListType;
	typedef std::vector<CustomPersistStep*> CustomStepListType;
	
    /**
	 * This is a list of buffers.  They will likely be members of a derived Snapshot.
	 * The base class deals with the buffers using only the interface of
	 * AbstractTableBuffer.
	 */
	LocatorListType m_locatorList;       ///< List of ObjectLocators that find objects in the database
	CustomStepListType m_customStepList; ///< List of extra things to do while saving this snapshot (e.g. associating characters with accounts)

	bool m_universeAuthorityHack;  //TODO:  remove after cleaning up authority transfer code
	
	int getTimeStamp   () const;
	bool saveTimestamp (DB::Session *session);
	
  private:
	bool m_useGoldDatabase;
	DB::ModeQuery::Mode m_mode;
	int m_timestamp;

public:
	bool m_isBeingSaved;
	bool getIsBeingSaved();
	static int getCreationCount() { return ms_creationCount; }
	static int getDeletionCount() { return ms_deletionCount; }
	static int getPendingCount() { return ms_creationCount - ms_deletionCount; }
private:
	static int ms_creationCount;
	static int ms_deletionCount;

  protected:
	virtual void decodeServerData(const NetworkId & objectId, Tag typeId, uint16 index, Archive::ReadIterator &bs, bool isBaseline) = 0;
	virtual void decodeSharedData(const NetworkId & objectId, Tag typeId, uint16 index, Archive::ReadIterator &bs, bool isBaseline) = 0;
	virtual void decodeClientData(const NetworkId & objectId, Tag typeId, uint16 index, Archive::ReadIterator &bs, bool isBaseline) = 0;
	virtual void decodeParentClientData(const NetworkId & objectId, Tag typeId, uint16 index, Archive::ReadIterator &bs, bool isBaseline) = 0;
	
	virtual bool encodeParentClientData(NetworkId const & objectId, Tag typeId, std::vector<BatchBaselinesMessageData> &baselines) const = 0;
	virtual bool encodeClientData(NetworkId const & objectId, Tag typeId, std::vector<BatchBaselinesMessageData> &baselines) const = 0;
	virtual bool encodeServerData(NetworkId const & objectId, Tag typeId, std::vector<BatchBaselinesMessageData> &baselines) const = 0;
	virtual bool encodeSharedData(NetworkId const & objectId, Tag typeId, std::vector<BatchBaselinesMessageData> &baselines) const = 0;

  private:
	Snapshot(const Snapshot&); //disable
	Snapshot &operator=(const Snapshot&); //disable
};

// ----------------------------------------------------------------------

inline bool Snapshot::getIsBeingSaved()
{
	return m_isBeingSaved;
}

inline void Snapshot::setUniverseAuthHack()
{
	m_universeAuthorityHack=true;
}

// ----------------------------------------------------------------------

inline bool Snapshot::getUseGoldDatabase() const
{
	return m_useGoldDatabase;
}

// ----------------------------------------------------------------------

inline DB::ModeQuery::Mode Snapshot::getMode() const
{
	return m_mode;
}

// ----------------------------------------------------------------------

inline int Snapshot::getTimeStamp() const
{
	return m_timestamp;
}

// ======================================================================
#endif
