// ======================================================================
//
// ObjectTableBuffer.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "ObjectTableBuffer.h"

#include <vector>
#include <map>
#include <set>

#include "SwgDatabaseServer/DeleteDemandLoadedContainerQuery.h"
#include "SwgDatabaseServer/Schema.h"
#include "serverDatabase/DatabaseProcess.h"
#include "serverDatabase/GetContentsList.h"
#include "serverNetworkMessages/GetMoneyFromOfflineObjectMessage.h"
#include "serverNetworkMessages/UpdateObjectPositionMessage.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedLog/Log.h"
#include "sharedMath/Quaternion.h"
#include "sharedNetworkMessages/MessageQueueDataTransform.h"
#include "sharedNetworkMessages/ObjectChannelMessages.h"

// ======================================================================

namespace ObjectTableBufferNamespace
{
	static const int s_numPositions = 20;
	static const int s_positionMasks[20] = {1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768,65536,131072,262144,524288};
}
using namespace ObjectTableBufferNamespace;

// ======================================================================

ObjectTableBuffer::ObjectTableBuffer(DB::ModeQuery::Mode mode) :
		IndexedNetworkTableBuffer<DBSchema::ObjectBufferRow, DBSchema::ObjectRow, DBQuery::ObjectsTableQuery, DBQuery::ObjectsTableQuerySelect>(mode)
{
}

// ======================================================================

ObjectTableBuffer::~ObjectTableBuffer()
{
}

// ----------------------------------------------------------------------



/**
 * Called when a new object is created.
 *
 * Sets up the data for the object in the Objects table.
 */

void ObjectTableBuffer::newObject(const NetworkId &objectId, int templateId, Tag typeId)
{
	DBSchema::ObjectBufferRow *row=addEmptyRow(objectId);
    row->object_template_id.setValue(templateId);
	row->type_id.setValue(static_cast<long>(typeId)); // cast because tags are unsigned
	row->deleted.setValue(0);
	row->controller_type.setValue(0);
}

// ----------------------------------------------------------------------

bool ObjectTableBuffer::handleUpdateObjectPosition(const UpdateObjectPositionMessage &message)
{
	DBSchema::ObjectBufferRow *row=dynamic_cast<DBSchema::ObjectBufferRow*>(findRowByIndex(message.getNetworkId()));
	if (row==0)
	{
		row=addEmptyRow(message.getNetworkId());
	}
	
	Vector v = message.getTransform().getPosition_p(); 
	row->x = static_cast<double>(v.x);
	row->y = static_cast<double>(v.y);
	row->z = static_cast<double>(v.z);

	Quaternion q(message.getTransform());

	Transform t;
	q.getTransform(&t);

	row->quaternion_w = static_cast<double>(q.w);
	row->quaternion_x = static_cast<double>(q.x);
	row->quaternion_y = static_cast<double>(q.y);
	row->quaternion_z = static_cast<double>(q.z);

	row->node_x=roundToNode(v.x); 
	row->node_y=roundToNode(v.y);
	row->node_z=roundToNode(v.z);

	row->contained_by = message.getContainerId();
	row->slot_arrangement = message.getSlotArrangement();
	row->load_with = message.getLoadWith();

	return true;
}

// ----------------------------------------------------------------------

/** 
 * Process a delete message for an object.
 *
 * Sets the "deleted" column to "true".
 *
 * Note:  even if the object is a new object, leaves it in the buffer
 * (but filters it out in save().)  This is to avoid a possible bug
 * where some delta comes to the object after it is deleted, and we end
 * up creating a new row that doesn't have "deleted" set.
 */

void ObjectTableBuffer::handleDeleteMessage(const NetworkId &objectId, int reasonCode, bool demandLoadedContainer, bool cascadeDeleteReason)
{
	DEBUG_FATAL(reasonCode==0,("Programmer bug:  The gameserver sent 0 as the reason code when deleting object %s, but 0 indicates the object is not deleted.\n",objectId.getValueString().c_str()));
	
	DBSchema::ObjectBufferRow *row=dynamic_cast<DBSchema::ObjectBufferRow*>(findRowByIndex(objectId));
	if (row==0)
		row=addEmptyRow(objectId);

	row->deleted.setValue(reasonCode);

	if (demandLoadedContainer)
	{
		if (cascadeDeleteReason)
			m_demandLoadedContainersToDelete.push_back(std::make_pair(objectId, reasonCode));
		else
			m_demandLoadedContainersToDelete.push_back(std::make_pair(objectId, 0));
	}
}

// ----------------------------------------------------------------------

int ObjectTableBuffer::roundToNode(real coordinate)
{
	int c=static_cast<int>(coordinate);
	// (c/chunksize)*chunksize : ... blah
	return (c >= 0) ? (c/100)*100 : (((c-100)+1)/100)*100;
}

// ----------------------------------------------------------------------

// Topological sort
struct DFSNode
{
	NetworkId m_oid;
	NetworkId m_containedBy;
	bool m_visited;
};

// ----------------------------------------------------------------------

void ObjectTableBuffer::getTags(DB::TagSet &tags) const
{
	for (IndexType::const_iterator r=m_rows.begin(); r!=m_rows.end(); ++r)
		tags.insert(r->second->type_id.getValue());
}

// ----------------------------------------------------------------------

void ObjectTableBuffer::getObjectList(std::vector<NetworkId> &theList) const
{
	typedef std::map <NetworkId, DFSNode> NodesType;
	NodesType nodes;
	std::vector <DFSNode*> nodeStack;

	// Copy the rows to our temporary space for sorting
	for (IndexType::const_iterator r=m_rows.begin(); r!=m_rows.end(); ++r)
	{
		DFSNode &newNode = nodes[r->second->object_id.getValue()];
		newNode.m_oid = r->second->object_id.getValue();
		newNode.m_containedBy = r->second->contained_by.getValue();
		newNode.m_visited = false;
	}

	// Depth-first search, outputting the nodes as we recurse *out* of them
	for (NodesType::iterator i=nodes.begin(); i!=nodes.end(); ++i)
	{
		if (!i->second.m_visited)
		{
			nodeStack.push_back(&i->second);
			i->second.m_visited = true;
			while (!nodeStack.empty())
			{
				DFSNode *theNode = nodeStack.back();
				bool noContainer = true;
				if (theNode->m_containedBy!=NetworkId::cms_invalid)
				{
					NodesType::iterator j=nodes.find(theNode->m_containedBy);
					if (j!=nodes.end() && !(j->second.m_visited))
					{
						nodeStack.push_back(&(j->second));
						j->second.m_visited = true;
						noContainer=false;
					}
				}

				if (noContainer)
				{
					nodeStack.pop_back();
					theList.push_back(theNode->m_oid);
				}
			}
		}
	}
}

// ----------------------------------------------------------------------


bool ObjectTableBuffer::save(DB::Session *session)
{
	int updates=0;
	int deletes=0;
	int onlyPositionOrDeletes=0;

 	DBQuery::ObjectsTableQuery qry;

	if (getMode()==DB::ModeQuery::mode_INSERT)
		qry.insertMode();
	else
		qry.updateMode();

	if (!qry.setupData(session))
		return false;

	for (IndexType::iterator i=m_rows.begin(); i!=m_rows.end(); ++i)
	{
		if (i->second->object_template_id.isNull() && 
			i->second->type_id.isNull() &&
			i->second->scene_id.isNull() &&
			i->second->controller_type.isNull() &&
			i->second->object_name.isNull() &&
			i->second->volume.isNull() &&
			i->second->slot_arrangement.isNull() &&
			i->second->player_controlled.isNull() &&
			i->second->cache_version.isNull() &&
			i->second->load_contents.isNull() &&
			i->second->cash_balance.isNull() &&
			i->second->bank_balance.isNull() &&
			i->second->complexity.isNull() &&
			i->second->name_string_table.isNull() &&
			i->second->name_string_text.isNull())
		{
			++onlyPositionOrDeletes;
		}

		if (!i->second->deleted.isNull() && i->second->deleted.getValue()!=0)
			++deletes;

		
		if (!(getMode()==DB::ModeQuery::mode_INSERT && !i->second->deleted.isNull() && i->second->deleted.getValue()!=0))
		{
			++updates;
			qry.setData(*(i->second));
#if 0  // Enable this to debug load_with problems
			DEBUG_REPORT_LOG(true,("Save object %s ",i->second->object_id.getValue().getValueString().c_str()));
			if (i->second->contained_by.isNull())
				DEBUG_REPORT_LOG(true, ("contained_by nullptr "));
			else					
				DEBUG_REPORT_LOG(true, ("contained_by %s ",i->second->contained_by.getValue().getValueString().c_str()));
			if (i->second->load_with.isNull())
				DEBUG_REPORT_LOG(true,("load_with nullptr\n"));
			else
				DEBUG_REPORT_LOG(true,("load_with %s\n",i->second->load_with.getValue().getValueString().c_str()));
#endif			

			if (!qry.addData((i->second)))
				return false;

			if (qry.getNumItems() == ObjectTableBufferNamespace::ms_maxItemsPerExec)
			{	
				if (! (session->exec(&qry)))
					return false;
				qry.clearData();
			}

			
		}
	}
	
	if (qry.getNumItems() != 0)
		if (! (session->exec(&qry)))
			return false;

	qry.done();
	qry.freeData();

	if (!m_demandLoadedContainersToDelete.empty())
	{
		DBQuery::DeleteDemandLoadedContainerQuery ddlcq;
		if (!ddlcq.setupData(*session))
			return false;

		for (std::vector<std::pair<NetworkId, int> >::const_iterator i=m_demandLoadedContainersToDelete.begin(); i!=m_demandLoadedContainersToDelete.end(); ++i)
		{
			if (!ddlcq.addData(i->first, i->second))
				return false;
			if (ddlcq.getCount() == ObjectTableBufferNamespace::ms_maxItemsPerExec)
			{
				if (! (session->exec(&ddlcq)))
					return false;
				ddlcq.clearData();
			}
		}
		if (ddlcq.getCount() != 0)
		{
			if (! (session->exec(&ddlcq)))
				return false;
			ddlcq.clearData();
		}
		ddlcq.done();
		ddlcq.freeData();
	}
	
	if (getMode()==DB::ModeQuery::mode_INSERT)
		LOG("SaveCounts",("Object table new objects:  %i (out of %i) total, %i only position or delete, %i delete\n",updates,m_rows.size(),onlyPositionOrDeletes,deletes));
	else
		LOG("SaveCounts",("Object table updates:  %i total, %i only position or delete, %i delete\n",updates,onlyPositionOrDeletes,deletes));
	return true;
}

// ----------------------------------------------------------------------

void ObjectTableBuffer::decodeObjVarFreeFlags(const NetworkId &objectId, int freePositions)
{
//	DEBUG_REPORT_LOG(true,("OBJVAR_DEBUG:  decodeObjVarFreeFlags(%s,%i)\n",objectId.getValueString().c_str(),freePositions));
	
	DBSchema::ObjectBufferRow *row=dynamic_cast<DBSchema::ObjectBufferRow*>(findRowByIndex(objectId));
	if (row==0)
		row=addEmptyRow(objectId);

	for (int i=0; i<s_numPositions; ++i)
	{
		bool set=((s_positionMasks[i] & freePositions)!=0);
		if (set)
		{
			switch (i)
			{
				case 0:
					row->objvar_0_type = -1;
					break;
				case 1:
					row->objvar_1_type = -1;
					break;
				case 2:
					row->objvar_2_type = -1;
					break;
				case 3:
					row->objvar_3_type = -1;
					break;
				case 4:
					row->objvar_4_type = -1;
					break;
				case 5:
					row->objvar_5_type = -1;
					break;
				case 6:
					row->objvar_6_type = -1;
					break;
				case 7:
					row->objvar_7_type = -1;
					break;
				case 8:
					row->objvar_8_type = -1;
					break;
				case 9:
					row->objvar_9_type = -1;
					break;
				case 10:
					row->objvar_10_type = -1;
					break;
				case 11:
					row->objvar_11_type = -1;
					break;
				case 12:
					row->objvar_12_type = -1;
					break;
				case 13:
					row->objvar_13_type = -1;
					break;
				case 14:
					row->objvar_14_type = -1;
					break;
				case 15:
					row->objvar_15_type = -1;
					break;
				case 16:
					row->objvar_16_type = -1;
					break;
				case 17:
					row->objvar_17_type = -1;
					break;
				case 18:
					row->objvar_18_type = -1;
					break;
				case 19:
					row->objvar_19_type = -1;
					break;
				default:
					DEBUG_FATAL(true,("ObjectId %s had freePositions flag %i, which is not valid.",objectId.getValueString().c_str(),freePositions));
			}
		}
	}
}

// ----------------------------------------------------------------------

void ObjectTableBuffer::handlePackedObjvar(const NetworkId &objectId, int position, const std::string &objvarName, int typeId, const std::string &packedValue)
{
//	DEBUG_REPORT_LOG(true,("OBJVAR_DEBUG:  handlePackedObjvar(%s,%i,%s,%i,%s)\n",objectId.getValueString().c_str(),position,objvarName.c_str(), typeId, packedValue.c_str()));
	
   	DBSchema::ObjectBufferRow *row=dynamic_cast<DBSchema::ObjectBufferRow*>(findRowByIndex(objectId));
	if (row==0)
		row=addEmptyRow(objectId);

	switch(position)
	{
		case 0:
			row->objvar_0_name = objvarName;
			row->objvar_0_type = typeId;
			row->objvar_0_value = packedValue;
			break;
		case 1:
			row->objvar_1_name = objvarName;
			row->objvar_1_type = typeId;
			row->objvar_1_value = packedValue;
			break;
		case 2:
			row->objvar_2_name = objvarName;
			row->objvar_2_type = typeId;
			row->objvar_2_value = packedValue;
			break;
		case 3:
			row->objvar_3_name = objvarName;
			row->objvar_3_type = typeId;
			row->objvar_3_value = packedValue;
			break;
		case 4:
			row->objvar_4_name = objvarName;
			row->objvar_4_type = typeId;
			row->objvar_4_value = packedValue;
			break;
		case 5:
			row->objvar_5_name = objvarName;
			row->objvar_5_type = typeId;
			row->objvar_5_value = packedValue;
			break;
		case 6:
			row->objvar_6_name = objvarName;
			row->objvar_6_type = typeId;
			row->objvar_6_value = packedValue;
			break;
		case 7:
			row->objvar_7_name = objvarName;
			row->objvar_7_type = typeId;
			row->objvar_7_value = packedValue;
			break;
		case 8:
			row->objvar_8_name = objvarName;
			row->objvar_8_type = typeId;
			row->objvar_8_value = packedValue;
			break;
		case 9:
			row->objvar_9_name = objvarName;
			row->objvar_9_type = typeId;
			row->objvar_9_value = packedValue;
			break;
		case 10:
			row->objvar_10_name = objvarName;
			row->objvar_10_type = typeId;
			row->objvar_10_value = packedValue;
			break;
		case 11:
			row->objvar_11_name = objvarName;
			row->objvar_11_type = typeId;
			row->objvar_11_value = packedValue;
			break;
		case 12:
			row->objvar_12_name = objvarName;
			row->objvar_12_type = typeId;
			row->objvar_12_value = packedValue;
			break;
		case 13:
			row->objvar_13_name = objvarName;
			row->objvar_13_type = typeId;
			row->objvar_13_value = packedValue;
			break;
		case 14:
			row->objvar_14_name = objvarName;
			row->objvar_14_type = typeId;
			row->objvar_14_value = packedValue;
			break;
		case 15:
			row->objvar_15_name = objvarName;
			row->objvar_15_type = typeId;
			row->objvar_15_value = packedValue;
			break;
		case 16:
			row->objvar_16_name = objvarName;
			row->objvar_16_type = typeId;
			row->objvar_16_value = packedValue;
			break;
		case 17:
			row->objvar_17_name = objvarName;
			row->objvar_17_type = typeId;
			row->objvar_17_value = packedValue;
			break;
		case 18:
			row->objvar_18_name = objvarName;
			row->objvar_18_type = typeId;
			row->objvar_18_value = packedValue;
			break;
		case 19:
			row->objvar_19_name = objvarName;
			row->objvar_19_type = typeId;
			row->objvar_19_value = packedValue;
			break;
		default:
			WARNING_STRICT_FATAL(true,("ObjectId %s had objvar %s with postion %i, which is not a valid position.",objectId.getValueString().c_str(),objvarName.c_str(), position));
	}
}

// ----------------------------------------------------------------------

void ObjectTableBuffer::getObjvarsForObject(const NetworkId &objectId, std::vector<DynamicVariableList::MapType::Command> &commands) const
{
	const DBSchema::ObjectBufferRow *row=safe_cast<const DBSchema::ObjectBufferRow*>(findConstRowByIndex(objectId));
	if (row==0)
	{
		WARNING_STRICT_FATAL(true,("Loaded OBJECT_VARIABLES for %s, but had no OBJECTS row.",objectId.getValueString().c_str()));
		return;
	}

	DynamicVariableList::MapType::Command c;
	c.cmd = DynamicVariableList::MapType::Command::ADD;

	if (row->objvar_0_type.getValue() >= 0)
	{
		row->objvar_0_name.getValue(c.key);
		c.value.load(0, row->objvar_0_type.getValue(), row->objvar_0_value.getValue());
		commands.push_back(c);
	}
	if (row->objvar_1_type.getValue() >= 0)
	{
		row->objvar_1_name.getValue(c.key);
		c.value.load(1, row->objvar_1_type.getValue(), row->objvar_1_value.getValue());
		commands.push_back(c);
	}
	if (row->objvar_2_type.getValue() >= 0)
	{
		row->objvar_2_name.getValue(c.key);
		c.value.load(2, row->objvar_2_type.getValue(), row->objvar_2_value.getValue());
		commands.push_back(c);
	}
	if (row->objvar_3_type.getValue() >= 0)
	{
		row->objvar_3_name.getValue(c.key);
		c.value.load(3, row->objvar_3_type.getValue(), row->objvar_3_value.getValue());
		commands.push_back(c);
	}
	if (row->objvar_4_type.getValue() >= 0)
	{
		row->objvar_4_name.getValue(c.key);
		c.value.load(4, row->objvar_4_type.getValue(), row->objvar_4_value.getValue());
		commands.push_back(c);
	}
	if (row->objvar_5_type.getValue() >= 0)
	{
		row->objvar_5_name.getValue(c.key);
		c.value.load(5, row->objvar_5_type.getValue(), row->objvar_5_value.getValue());
		commands.push_back(c);
	}
	if (row->objvar_6_type.getValue() >= 0)
	{
		row->objvar_6_name.getValue(c.key);
		c.value.load(6, row->objvar_6_type.getValue(), row->objvar_6_value.getValue());
		commands.push_back(c);
	}
	if (row->objvar_7_type.getValue() >= 0)
	{
		row->objvar_7_name.getValue(c.key);
		c.value.load(7, row->objvar_7_type.getValue(), row->objvar_7_value.getValue());
		commands.push_back(c);
	}
	if (row->objvar_8_type.getValue() >= 0)
	{
		row->objvar_8_name.getValue(c.key);
		c.value.load(8, row->objvar_8_type.getValue(), row->objvar_8_value.getValue());
		commands.push_back(c);
	}
	if (row->objvar_9_type.getValue() >= 0)
	{
		row->objvar_9_name.getValue(c.key);
		c.value.load(9, row->objvar_9_type.getValue(), row->objvar_9_value.getValue());
		commands.push_back(c);
	}
	if (row->objvar_10_type.getValue() >= 0)
	{
		row->objvar_10_name.getValue(c.key);
		c.value.load(10, row->objvar_10_type.getValue(), row->objvar_10_value.getValue());
		commands.push_back(c);
	}
	if (row->objvar_11_type.getValue() >= 0)
	{
		row->objvar_11_name.getValue(c.key);
		c.value.load(11, row->objvar_11_type.getValue(), row->objvar_11_value.getValue());
		commands.push_back(c);
	}
	if (row->objvar_12_type.getValue() >= 0)
	{
		row->objvar_12_name.getValue(c.key);
		c.value.load(12, row->objvar_12_type.getValue(), row->objvar_12_value.getValue());
		commands.push_back(c);
	}
	if (row->objvar_13_type.getValue() >= 0)
	{
		row->objvar_13_name.getValue(c.key);
		c.value.load(13, row->objvar_13_type.getValue(), row->objvar_13_value.getValue());
		commands.push_back(c);
	}
	if (row->objvar_14_type.getValue() >= 0)
	{
		row->objvar_14_name.getValue(c.key);
		c.value.load(14, row->objvar_14_type.getValue(), row->objvar_14_value.getValue());
		commands.push_back(c);
	}
	if (row->objvar_15_type.getValue() >= 0)
	{
		row->objvar_15_name.getValue(c.key);
		c.value.load(15, row->objvar_15_type.getValue(), row->objvar_15_value.getValue());
		commands.push_back(c);
	}
	if (row->objvar_16_type.getValue() >= 0)
	{
		row->objvar_16_name.getValue(c.key);
		c.value.load(16, row->objvar_16_type.getValue(), row->objvar_16_value.getValue());
		commands.push_back(c);
	}
	if (row->objvar_17_type.getValue() >= 0)
	{
		row->objvar_17_name.getValue(c.key);
		c.value.load(17, row->objvar_17_type.getValue(), row->objvar_17_value.getValue());
		commands.push_back(c);
	}
	if (row->objvar_18_type.getValue() >= 0)
	{
		row->objvar_18_name.getValue(c.key);
		c.value.load(18, row->objvar_18_type.getValue(), row->objvar_18_value.getValue());
		commands.push_back(c);
	}
	if (row->objvar_19_type.getValue() >= 0)
	{
		row->objvar_19_name.getValue(c.key);
		c.value.load(19, row->objvar_19_type.getValue(), row->objvar_19_value.getValue());
		commands.push_back(c);
	}
}

// ----------------------------------------------------------------------

int ObjectTableBuffer::encodeObjVarFreeFlags(const NetworkId & objectId) const
{
	const DBSchema::ObjectBufferRow *row=findConstRowByIndex(objectId);
	WARNING_STRICT_FATAL(row==nullptr,("Loading object %s, no ObjectRow in the buffer\n",objectId.getValueString().c_str()));
	if (!row)
		return 0;

	int positionFlags=0;
	
	for (int i=0; i<s_numPositions; ++i)
	{
		bool free = false;
		switch(i)
		{
			case 0:
				free = (row->objvar_0_type.getValue() < 0);
				break;
			case 1:
				free = (row->objvar_1_type.getValue() < 0);
				break;
			case 2:
				free = (row->objvar_2_type.getValue() < 0);
				break;
			case 3:
				free = (row->objvar_3_type.getValue() < 0);
				break;
			case 4:
				free = (row->objvar_4_type.getValue() < 0);
				break;
			case 5:
				free = (row->objvar_5_type.getValue() < 0);
				break;
			case 6:
				free = (row->objvar_6_type.getValue() < 0);
				break;
			case 7:
				free = (row->objvar_7_type.getValue() < 0);
				break;
			case 8:
				free = (row->objvar_8_type.getValue() < 0);
				break;
			case 9:
				free = (row->objvar_9_type.getValue() < 0);
				break;
			case 10:
				free = (row->objvar_10_type.getValue() < 0);
				break;
			case 11:
				free = (row->objvar_11_type.getValue() < 0);
				break;
			case 12:
				free = (row->objvar_12_type.getValue() < 0);
				break;
			case 13:
				free = (row->objvar_13_type.getValue() < 0);
				break;
			case 14:
				free = (row->objvar_14_type.getValue() < 0);
				break;
			case 15:
				free = (row->objvar_15_type.getValue() < 0);
				break;
			case 16:
				free = (row->objvar_16_type.getValue() < 0);
				break;
			case 17:
				free = (row->objvar_17_type.getValue() < 0);
				break;
			case 18:
				free = (row->objvar_18_type.getValue() < 0);
				break;
			case 19:
				free = (row->objvar_19_type.getValue() < 0);
				break;
			default:
				DEBUG_FATAL(true,("Iterated through too many positions.  WTF?"));
		}

		if (free)
			positionFlags += s_positionMasks[i];
	}

	return positionFlags;
}

// ----------------------------------------------------------------------

void ObjectTableBuffer::getWorldContainers(std::vector<NetworkId> &containers) const
{
	for (IndexType::const_iterator r=m_rows.begin(); r!=m_rows.end(); ++r)
	{
		if ((r->second->contained_by.getValue()==NetworkId::cms_invalid) && (r->second->load_contents.getValue()))
			containers.push_back(r->second->object_id.getValue());
	}
}

// ----------------------------------------------------------------------

bool ObjectTableBuffer::getMoneyFromOfflineObject(uint32 replyServer, NetworkId const & sourceObject, int amount, NetworkId const & replyTo, std::string const & successCallback, std::string const & failCallback, std::vector<int8> const & packedDictionary)
{
	DBSchema::ObjectBufferRow * const row=findRowByIndex(sourceObject);
	if (!row)
		return false;

	// We might have buffered values for bank balance, cash balance, both, or neither
	// We can only change the ones for which we have a value.
	bool const definiteBankBalance = !row->bank_balance.isNull();
	bool const definiteCashBalance = !row->cash_balance.isNull();
	
	int const previousBankBalance = definiteBankBalance ? row->bank_balance.getValue() : 0;
	int const previousCashBalance = definiteCashBalance ? row->cash_balance.getValue() : 0;

	if (previousBankBalance + previousCashBalance < amount)
	{
		if (definiteBankBalance && definiteCashBalance)
		{
			// send failure message
			GetMoneyFromOfflineObjectMessage replyMessage(sourceObject, amount, replyTo, successCallback, failCallback, packedDictionary, false);
			DatabaseProcess::getInstance().sendToGameServer(replyServer, replyMessage);
		}
		else
			return false; // can't be handled in memory, but there's a possibility it could be handled in the DB
	}

	// Has enough money to handle this in memory
	
	int newBankBalance = previousBankBalance - amount;
	int newCashBalance = previousCashBalance;
	if (newBankBalance < 0)
	{
		int const remainingAmount = -newBankBalance;
		newBankBalance = 0;
		newCashBalance -= remainingAmount;
		DEBUG_FATAL(newCashBalance < 0,("Programmer bug:  newCashBalance ended up negative, which should not be possible.  Indicates bad logic in getMoneyFromOfflineObject()"));
	}

	if (definiteBankBalance)
		row->bank_balance=newBankBalance;
	if (definiteCashBalance)
		row->cash_balance=newCashBalance;

	// Send success message
	GetMoneyFromOfflineObjectMessage replyMessage(sourceObject, amount, replyTo, successCallback, failCallback, packedDictionary, true);
	DatabaseProcess::getInstance().sendToGameServer(replyServer, replyMessage);
	
	return true;
}

// ======================================================================
