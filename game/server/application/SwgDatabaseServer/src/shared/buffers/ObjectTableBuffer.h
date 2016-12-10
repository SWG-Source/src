// ======================================================================
//
// ObjectTableBuffer.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ObjectTableBuffer_H
#define INCLUDED_ObjectTableBuffer_H

// ======================================================================

#include "IndexedNetworkTableBuffer.h"

#include "SwgDatabaseServer/ObjectsTableQuery.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedFoundation/Tag.h"

// ======================================================================

class NetworkId;
class UpdateObjectPositionMessage;

namespace DB
{
	class Session;
}
namespace DBSchema
{
	struct ObjectRow;
}
namespace ObjectTableBufferNamespace
{
	const int ms_maxItemsPerExec = 10000;
}

using namespace ObjectTableBufferNamespace;


// ======================================================================

class ObjectTableBuffer : public IndexedNetworkTableBuffer<DBSchema::ObjectBufferRow, DBSchema::ObjectRow, DBQuery::ObjectsTableQuery, DBQuery::ObjectsTableQuerySelect>
{
public:
	explicit ObjectTableBuffer(DB::ModeQuery::Mode mode);
	virtual ~ObjectTableBuffer();
	
//	virtual void loadRow(DB::Session *session, const NetworkId &primaryKey);

	virtual bool handleUpdateObjectPosition(const UpdateObjectPositionMessage &message);
	void handleDeleteMessage(const NetworkId &objectId, int reasonCode, bool demandLoadedContainer, bool cascadeDeleteReason);
	void decodeObjVarFreeFlags(const NetworkId &objectId, int freePositions);
	int encodeObjVarFreeFlags(const NetworkId & objectId) const;
	void handlePackedObjvar(const NetworkId &objectId, int position, const std::string &objvarName, int typeId, const std::string &packedValue);
	void getObjvarsForObject(const NetworkId &objectId, std::vector<DynamicVariableList::MapType::Command> &commands) const;
	bool getMoneyFromOfflineObject(uint32 replyServer,NetworkId const & sourceObject, int amount, NetworkId const & replyTo, std::string const & successCallback, std::string const & failCallback, std::vector<int8> const & packedDictionary);
		
	void newObject(const NetworkId &objectId, int templateId, Tag typeId);

//	std::vector<NetworkId> queueForLoadWithContents(const std::vector<NetworkId> &oidList, DB::Session *session);

	void getObjectList(std::vector<NetworkId> &theList) const;
	void getWorldContainers(std::vector<NetworkId> &containers) const;
	void getTags(DB::TagSet &tags) const;
	
	virtual bool save(DB::Session *session);

  private:
	static int roundToNode(real coordinate);
	std::vector<std::pair<NetworkId,int> > m_demandLoadedContainersToDelete;

  private:
	ObjectTableBuffer(); // disable	
};

// ======================================================================

#endif
