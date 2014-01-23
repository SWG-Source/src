//===================================================================
//
// WorldSnapshotReaderWriter.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#include "sharedUtility/FirstSharedUtility.h"
#include "sharedUtility/WorldSnapshotReaderWriter.h"

#include "sharedDebug/PerformanceTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/CrcString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedMath/Quaternion.h"
#include "sharedUtility/LocalMachineOptionManager.h"

#include <algorithm>
#include <map>
#include <vector>
#include <unordered_map>

//===================================================================

namespace
{
	const Tag TAG_NODE = TAG (N,O,D,E);
	const Tag TAG_NODS = TAG (N,O,D,S);
	const Tag TAG_OTNL = TAG (O,T,N,L);
	const Tag TAG_WSNP = TAG (W,S,N,P);

	float ms_detailLevelBias;
	WorldSnapshotReaderWriter::Node::DetailLevelChangedFunction ms_detailLevelChangedFunction;

#ifdef _DEBUG
	int ms_loadSingleNetworkId;
	float ms_overrideUpdateRadius = 0.f;
#endif
}

//===================================================================
// STATIC PUBLIC WorldSnapshotReaderWriter::Node
//===================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL (WorldSnapshotReaderWriter::Node, false, 128, 0, 0);

//-------------------------------------------------------------------

void WorldSnapshotReaderWriter::Node::install ()
{
	installMemoryBlockManager ();

	LocalMachineOptionManager::registerOption (ms_detailLevelBias, "SharedUtility/WorldSnapshot", "detailLevelBias");

#ifdef _DEBUG
	ms_loadSingleNetworkId = ConfigFile::getKeyInt("SharedUtility/WorldSnapshot", "loadSingleNetworkId", ms_loadSingleNetworkId);
	ms_overrideUpdateRadius = ConfigFile::getKeyFloat("SharedUtility/WorldSnapshot", "overrideUpdateRadius", ms_overrideUpdateRadius);
#endif

	ExitChain::add (remove, "WorldSnapshotReaderWriter::Node::install");
}

//-------------------------------------------------------------------

float WorldSnapshotReaderWriter::Node::getDetailLevelBias ()
{
	return ms_detailLevelBias;
}

//-------------------------------------------------------------------

void WorldSnapshotReaderWriter::Node::setDetailLevelBias (const float detailLevelBias)
{
	ms_detailLevelBias = detailLevelBias;

	if (ms_detailLevelChangedFunction)
		ms_detailLevelChangedFunction ();
}

//-------------------------------------------------------------------

void WorldSnapshotReaderWriter::Node::setDetailLevelChangedFunction (DetailLevelChangedFunction detailLevelChangedFunction)
{
	ms_detailLevelChangedFunction = detailLevelChangedFunction;
}

//===================================================================
// PUBLIC WorldSnapshotReaderWriter::Node
//===================================================================

WorldSnapshotReaderWriter::Node::Node () :
	m_deleted (false),
	m_networkIdInt (0),
	m_containedByNetworkIdInt (0),
	m_objectTemplateNameIndex (0),
	m_cellIndex (0),
	m_transform_p (),
	m_radius (0.f),
	m_portalLayoutCrc (0),
	m_parent (0),
	m_eventName (),
	m_nodeList (0),
	m_spatialSubdivisionHandle (0),
	m_distanceSquaredTo (0.f),
	m_inWorld (false)
{
}

//-------------------------------------------------------------------

WorldSnapshotReaderWriter::Node::~Node ()
{
	m_parent = 0;

	if (m_nodeList)
	{
		IGNORE_RETURN (std::for_each (m_nodeList->begin (), m_nodeList->end (), PointerDeleter ()));
		m_nodeList->clear ();
		delete m_nodeList;
		m_nodeList = 0;
	}

	m_spatialSubdivisionHandle = 0;
}

//-------------------------------------------------------------------

void WorldSnapshotReaderWriter::Node::setDeleted ()
{
	m_spatialSubdivisionHandle = 0;
	m_deleted = true;
	m_networkIdInt = 0;
}

//-------------------------------------------------------------------

void WorldSnapshotReaderWriter::Node::setNetworkIdInt (const int64 networkIdInt)
{
	m_networkIdInt = networkIdInt;
}

//-------------------------------------------------------------------

void WorldSnapshotReaderWriter::Node::setContainedByNetworkIdInt (const int64 containedByNetworkIdInt)
{
	m_containedByNetworkIdInt = containedByNetworkIdInt;
}

//-------------------------------------------------------------------

void WorldSnapshotReaderWriter::Node::setObjectTemplateNameIndex (const int objectTemplateNameIndex)
{
	m_objectTemplateNameIndex = objectTemplateNameIndex;
}

//-------------------------------------------------------------------

void WorldSnapshotReaderWriter::Node::setCellIndex (const int cellIndex)
{
	m_cellIndex = cellIndex;
}

//-------------------------------------------------------------------

void WorldSnapshotReaderWriter::Node::setTransform_p (const Transform& transform_p)
{
	m_transform_p = transform_p;
}

//-------------------------------------------------------------------

void WorldSnapshotReaderWriter::Node::setRadius (const float radius)
{
	#ifdef _DEBUG
	m_radius = ms_overrideUpdateRadius > radius ? ms_overrideUpdateRadius : radius;
	#else
	m_radius = radius;
	#endif
}

//-------------------------------------------------------------------

void WorldSnapshotReaderWriter::Node::setPortalLayoutCrc (const uint32 portalLayoutCrc)
{
	m_portalLayoutCrc = portalLayoutCrc;
}

//-------------------------------------------------------------------

void WorldSnapshotReaderWriter::Node::setEventName(const std::string & eventName)
{
	m_eventName = eventName;
}

//-------------------------------------------------------------------

bool WorldSnapshotReaderWriter::Node::isDeleted () const
{
	return m_deleted;
}

//-------------------------------------------------------------------

int64 WorldSnapshotReaderWriter::Node::getNetworkIdInt () const
{
	return m_networkIdInt;
}

//-------------------------------------------------------------------

int64 WorldSnapshotReaderWriter::Node::getContainedByNetworkIdInt () const
{
	return m_containedByNetworkIdInt;
}

//-------------------------------------------------------------------

int WorldSnapshotReaderWriter::Node::getObjectTemplateNameIndex () const
{
	return m_objectTemplateNameIndex;
}

//-------------------------------------------------------------------

int WorldSnapshotReaderWriter::Node::getCellIndex () const
{
	return m_cellIndex;
}

//-------------------------------------------------------------------

const Transform& WorldSnapshotReaderWriter::Node::getTransform_p () const
{
	return m_transform_p;
}

//-------------------------------------------------------------------

float WorldSnapshotReaderWriter::Node::getRadius () const
{
	return m_radius;
}

//-------------------------------------------------------------------

uint32 WorldSnapshotReaderWriter::Node::getPortalLayoutCrc () const
{
	return m_portalLayoutCrc;
}

//-------------------------------------------------------------------

const std::string & WorldSnapshotReaderWriter::Node::getEventName() const
{
	return m_eventName;
}

//-------------------------------------------------------------------

int WorldSnapshotReaderWriter::Node::getNumberOfNodes () const
{
	return m_nodeList ? static_cast<int> (m_nodeList->size ()) : 0;
}

//-------------------------------------------------------------------

const WorldSnapshotReaderWriter::Node* WorldSnapshotReaderWriter::Node::getNode (const int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, index, getNumberOfNodes ());
	NOT_NULL (m_nodeList);
	return (*m_nodeList) [static_cast<uint> (index)];
}

//-------------------------------------------------------------------

const WorldSnapshotReaderWriter::Node* WorldSnapshotReaderWriter::Node::getParent () const
{
	return m_parent;
}

//-------------------------------------------------------------------

int WorldSnapshotReaderWriter::Node::getTotalNumberOfNodes () const
{
	int numberOfNodes = 1;

	int i;
	for (i = 0; i < getNumberOfNodes (); ++i)
		numberOfNodes += getNode (i)->getTotalNumberOfNodes ();

	return numberOfNodes;
}

//-------------------------------------------------------------------

const Sphere WorldSnapshotReaderWriter::Node::getSphere () const
{
	const float minimum = getRadius ();
	const float maximum = 512.f;
	const float radius = minimum >= maximum ? getRadius () : linearInterpolate (minimum, maximum, clamp (0.f, ms_detailLevelBias, 1.f));

	return Sphere (m_transform_p.getPosition_p (), radius);
}

//-------------------------------------------------------------------

SpatialSubdivisionHandle* WorldSnapshotReaderWriter::Node::getSpatialSubdivisionHandle () const
{
	return m_spatialSubdivisionHandle;
}  //lint !e1763  //-- function indirectly modifies class

//-------------------------------------------------------------------

void WorldSnapshotReaderWriter::Node::setSpatialSubdivisionHandle (SpatialSubdivisionHandle* spatialSubdivisionHandle) const
{
	m_spatialSubdivisionHandle = spatialSubdivisionHandle;
}

//-------------------------------------------------------------------

void WorldSnapshotReaderWriter::Node::computeDistanceSquaredTo (const Vector& position_p) const
{
	m_distanceSquaredTo = position_p.magnitudeBetweenSquared (m_transform_p.getPosition_p ()) - sqr (getRadius ());
}

//-------------------------------------------------------------------

float WorldSnapshotReaderWriter::Node::getDistanceSquaredTo () const
{
	return m_distanceSquaredTo;
}

//-------------------------------------------------------------------

bool WorldSnapshotReaderWriter::Node::isInWorld () const
{
	return m_inWorld;
}

//-------------------------------------------------------------------

void WorldSnapshotReaderWriter::Node::addToWorld () const
{
	m_inWorld = true;
}

//-------------------------------------------------------------------

void WorldSnapshotReaderWriter::Node::removeFromWorld () const
{
	m_inWorld = false;

	int i;
	for (i = 0; i < getNumberOfNodes (); ++i)
		getNode (i)->removeFromWorld ();
}

//===================================================================
// STATIC PRIVATE WorldSnapshotReaderWriter::Node
//===================================================================

void WorldSnapshotReaderWriter::Node::remove ()
{
	removeMemoryBlockManager ();
}

//===================================================================
// PRIVATE WorldSnapshotReaderWriter::Node
//===================================================================

void WorldSnapshotReaderWriter::Node::setParent (Node* const node)
{
	m_parent = node;
}

//-------------------------------------------------------------------

void WorldSnapshotReaderWriter::Node::addNode (Node* const node)
{
	if (!m_nodeList)
		m_nodeList = new NodeList;

	m_nodeList->push_back (node);
	node->setParent (this);
}

//-------------------------------------------------------------------

void WorldSnapshotReaderWriter::Node::load (Iff& iff)
{
	iff.enterForm (TAG_NODE);

		switch (iff.getCurrentName ())
		{
		case TAG_0000:
			load_0000 (iff);
			break;

		default:
			{
				char tagBuffer [5];
				ConvertTagToString (iff.getCurrentName (), tagBuffer);

				char buffer [128];
				iff.formatLocation (buffer, sizeof (buffer));
				DEBUG_FATAL (true, ("WorldSnapshotReaderWriter::load invalid version %s/%s", buffer, tagBuffer));
			}
			break;
		}

	iff.exitForm (TAG_NODE);
}

//-------------------------------------------------------------------

void WorldSnapshotReaderWriter::Node::save (Iff& iff) const
{
	iff.insertForm (TAG_NODE);
		iff.insertForm (TAG_0000);

			//-- save node data
			iff.insertChunk (TAG_DATA);

				iff.insertChunkData ((int32)getNetworkIdInt ());
				iff.insertChunkData ((int32)getContainedByNetworkIdInt ());
				iff.insertChunkData (getObjectTemplateNameIndex ());
				iff.insertChunkData (getCellIndex ());

				const Quaternion q (getTransform_p ());
				iff.insertChunkFloatQuaternion (q);
				iff.insertChunkFloatVector (getTransform_p ().getPosition_p ());
				iff.insertChunkData (getRadius ());
				iff.insertChunkData (getPortalLayoutCrc ());

			iff.exitChunk (TAG_DATA);

			//-- save children
			int i;
			for (i = 0; i < getNumberOfNodes (); ++i)
				getNode (i)->save (iff);

		iff.exitForm (TAG_0000);
	iff.exitForm (TAG_NODE);
}

//-------------------------------------------------------------------

void WorldSnapshotReaderWriter::Node::load_0000 (Iff& iff)
{
	iff.enterForm (TAG_0000);

		//-- read node data
		iff.enterChunk (TAG_DATA);

			setNetworkIdInt (iff.read_int32 ());
			setContainedByNetworkIdInt (iff.read_int32 ());
			setObjectTemplateNameIndex (iff.read_int32 ());
			setCellIndex (iff.read_int32 ());

			const Quaternion q = iff.read_floatQuaternion ();
			const Vector v = iff.read_floatVector ();

			Transform t;
			q.getTransform (&t);
			t.setPosition_p (v);

			setTransform_p (t);
			setRadius (iff.read_float ());
			setPortalLayoutCrc (iff.read_uint32 ());

		iff.exitChunk (TAG_DATA);

		//-- read children
		while (iff.getNumberOfBlocksLeft ())
		{
			Node* const node = new Node;
			node->load (iff);
			addNode (node);
		}  //lint !e429  //-- custodial pointer is not freed or returned

		//-- verify children
		{
			if (getPortalLayoutCrc () != 0)
			{
				int i;
				for (i = 1; i <= getNumberOfNodes (); ++i)
				{
					int j;
					for (j = 0; j < getNumberOfNodes (); ++j)
						if (getNode (j)->getCellIndex () == i)
							break;

				}

			}
		}

	iff.exitForm (TAG_0000);
}

//===================================================================
// PUBLIC WorldSnapshotReaderWriter
//===================================================================

WorldSnapshotReaderWriter::WorldSnapshotReaderWriter () :
	m_nodeList (new NodeList),
	m_objectTemplateNameList (new ObjectTemplateNameList),
	m_objectTemplateCrcMap (new ObjectTemplateCrcMap),
	m_networkIdNodeMap (new NetworkIdNodeMap)
{
}

//-------------------------------------------------------------------

WorldSnapshotReaderWriter::~WorldSnapshotReaderWriter ()
{
	clear ();

	delete m_nodeList;
	delete m_objectTemplateNameList;
	delete m_objectTemplateCrcMap;
	delete m_networkIdNodeMap;
}

//-------------------------------------------------------------------

bool WorldSnapshotReaderWriter::load (const char* sceneName)
{
	char filename[256];
	IGNORE_RETURN(snprintf(filename, sizeof(filename)-1, "snapshot/%s.ws", sceneName));
	filename[sizeof(filename)-1] = '\0';

	Iff iff;
	if (iff.open (filename, true))
	{
		PerformanceTimer timer;
		timer.start ();

		//-- clear
		clear ();

		timer.stop ();
		//DEBUG_REPORT_LOG (true, ("clear %1.2f\n", timer.getElapsedTime ()));
		timer.start ();

		//-- load
		load (iff);

		timer.stop ();
		//DEBUG_REPORT_LOG (true, ("load %1.2f\n", timer.getElapsedTime ()));
		timer.start ();

		//-- wander through the nodes adding them to the node map
		{
			NodeList nodeStack;

			//-- push all root nodes on the node stack
			{
				int i;
				for (i = 0; i < getNumberOfNodes (); ++i)
					nodeStack.push_back (const_cast<Node*> (getNode (i)));
			}

			//--
			while (!nodeStack.empty ())
			{
				Node* const node = nodeStack.back ();
				nodeStack.pop_back ();

				std::pair<NetworkIdNodeMap::iterator, bool> result = m_networkIdNodeMap->insert (std::make_pair (node->getNetworkIdInt (), node));
				UNREF(result);
				DEBUG_FATAL (!result.second, ("WorldSnapshotReaderWriter::load: could not insert %i into networkIdNodeMap", node->getNetworkIdInt ()));

				int i;
				for (i = 0; i < node->getNumberOfNodes (); ++i)
					nodeStack.push_back (const_cast<Node*> (node->getNode (i)));
			}
		}

		timer.stop ();
		//DEBUG_REPORT_LOG (true, ("process %1.2f\n", timer.getElapsedTime ()));

		return true;
	}

	return false;
}

//-------------------------------------------------------------------

void WorldSnapshotReaderWriter::load (Iff& iff)
{
	if (iff.enterForm (TAG_WSNP, true))
	{
		switch (iff.getCurrentName ())
		{
		case TAG_0000:
			break;

		case TAG_0001:
			load_0001 (iff);
			break;

		default:
			{
				char tagBuffer [5];
				ConvertTagToString (iff.getCurrentName (), tagBuffer);

				char buffer [128];
				iff.formatLocation (buffer, sizeof (buffer));
				DEBUG_FATAL (true, ("WorldSnapshotReaderWriter::load invalid version %s/%s", buffer, tagBuffer));
			}
			break;
		}

		iff.exitForm (TAG_WSNP, true);
	}
}

//-------------------------------------------------------------------

bool WorldSnapshotReaderWriter::save (const char* filename) const
{
	Iff iff (65536);
	save (iff);

	return iff.write (filename, true);
}

//-------------------------------------------------------------------

void WorldSnapshotReaderWriter::save (Iff& iff) const
{
	//-- save
	iff.insertForm (TAG_WSNP);
		iff.insertForm (TAG_0001);

			//-- insert the node list
			iff.insertForm (TAG_NODS);

				{
					NOT_NULL (m_nodeList);

					uint i;
					for (i = 0; i < m_nodeList->size (); ++i)
						(*m_nodeList) [i]->save (iff);
				}

			iff.exitForm (TAG_NODS);

			//-- insert the object template name table
			iff.insertChunk (TAG_OTNL);

				{
					NOT_NULL (m_objectTemplateNameList);

					iff.insertChunkData (static_cast<int> (m_objectTemplateNameList->size ()));

					ObjectTemplateNameList::iterator iter = m_objectTemplateNameList->begin ();
					for (; iter != m_objectTemplateNameList->end (); ++iter)
						iff.insertChunkString (*iter);
				}

			iff.exitChunk (TAG_OTNL);

		iff.exitForm (TAG_0001);
	iff.exitForm (TAG_WSNP);
}

//-------------------------------------------------------------------

void WorldSnapshotReaderWriter::clear ()
{
	const bool deleteNetworkIdNodeMap = m_nodeList->empty ();

	IGNORE_RETURN (std::for_each (m_nodeList->begin (), m_nodeList->end (), PointerDeleter ()));
	m_nodeList->clear ();

	IGNORE_RETURN (std::for_each (m_objectTemplateNameList->begin (), m_objectTemplateNameList->end (), ArrayPointerDeleter ()));
	m_objectTemplateNameList->clear ();
	m_objectTemplateCrcMap->clear();

	if (deleteNetworkIdNodeMap)
		IGNORE_RETURN (std::for_each (m_networkIdNodeMap->begin (), m_networkIdNodeMap->end (), PointerDeleterPairSecond ()));

	m_networkIdNodeMap->clear ();
}

//-------------------------------------------------------------------

WorldSnapshotReaderWriter::Node const *WorldSnapshotReaderWriter::addObject (
	const int64 networkIdInt,
	const int64 containedByNetworkIdInt,
	CrcString const &objectTemplateName,
	const int cellIndex,
	const Transform& transform_p,
	const float radius,
	const uint32 portalLayoutCrc,
	const std::string & eventName )
{
	NOT_NULL (m_objectTemplateNameList);
	NOT_NULL (m_networkIdNodeMap);

	//-- find objectTemplateNameIndex
	uint objectTemplateNameIndex = 0;

	ObjectTemplateCrcMap::const_iterator i = m_objectTemplateCrcMap->find(objectTemplateName.getCrc());
	if (i != m_objectTemplateCrcMap->end())
		objectTemplateNameIndex = (*i).second;
	else
	{
		objectTemplateNameIndex = m_objectTemplateNameList->size();
		(*m_objectTemplateCrcMap)[objectTemplateName.getCrc()] = objectTemplateNameIndex;
		m_objectTemplateNameList->push_back(DuplicateString(objectTemplateName.getString()));
	}

	Node * const node = new Node;
	node->setNetworkIdInt            (networkIdInt);
	node->setContainedByNetworkIdInt (containedByNetworkIdInt);
	node->setObjectTemplateNameIndex (static_cast<int> (objectTemplateNameIndex));
	node->setCellIndex               (cellIndex);
	node->setTransform_p             (transform_p);
	node->setRadius                  (radius);
	node->setPortalLayoutCrc         (portalLayoutCrc);
	node->setEventName               (eventName);

	std::pair<NetworkIdNodeMap::iterator, bool> result = m_networkIdNodeMap->insert (std::make_pair (networkIdInt, node));

	//////////////////////////////////////////////////////////////////////////
	// if cellIndex == -1 then this should be a server-only object,
	// but it might be in the client buildout file, so we just want to ignore
	// the error if we try to add it a second time.
	UNREF(result);
	DEBUG_FATAL( cellIndex >= 0 && !result.second, ("Failed to add existing object [%d.%d] to node map",
		(int)( -networkIdInt >> 48),
		(int)networkIdInt ) );

	if (containedByNetworkIdInt != 0)
	{
		NetworkIdNodeMap::iterator parentIter = m_networkIdNodeMap->find(containedByNetworkIdInt);
		FATAL(parentIter == m_networkIdNodeMap->end(), ("WorldSnapshotReaderWriter::addObject: Contained object %d with unknown container %d!", (int)networkIdInt, (int)containedByNetworkIdInt));
		Node * const parent = parentIter->second;
		parent->addNode(node);
	}
	else
		m_nodeList->push_back(node);

	return node;
}

//-------------------------------------------------------------------

int WorldSnapshotReaderWriter::getNumberOfNodes () const
{
	NOT_NULL (m_nodeList);
	return static_cast<int> (m_nodeList->size ());
}

//-------------------------------------------------------------------

const WorldSnapshotReaderWriter::Node* WorldSnapshotReaderWriter::getNode (const int nodeIndex) const
{
	NOT_NULL (m_nodeList);
	return (*m_nodeList) [static_cast<uint> (nodeIndex)];
}

//-------------------------------------------------------------------

int WorldSnapshotReaderWriter::getTotalNumberOfNodes () const
{
	int numberOfNodes = 0;

	int i;
	for (i = 0; i < getNumberOfNodes (); ++i)
		numberOfNodes += getNode (i)->getTotalNumberOfNodes ();

	return numberOfNodes;
}

//-------------------------------------------------------------------

int WorldSnapshotReaderWriter::getNumberOfObjectTemplateNames () const
{
	return static_cast<int>(m_objectTemplateNameList->size());
}

//-------------------------------------------------------------------

const char* WorldSnapshotReaderWriter::getObjectTemplateName (const int objectTemplateNameIndex) const
{
	NOT_NULL (m_objectTemplateNameList);
	return (*m_objectTemplateNameList) [static_cast<uint> (objectTemplateNameIndex)];
}

//-------------------------------------------------------------------

WorldSnapshotReaderWriter::Node* WorldSnapshotReaderWriter::find (const int64 networkIdInt)
{
	NOT_NULL (m_networkIdNodeMap);
	NetworkIdNodeMap::iterator iter = m_networkIdNodeMap->find (networkIdInt);
	if (iter != m_networkIdNodeMap->end () && !iter->second->isDeleted ())
		return iter->second;

	return 0;
}

//-------------------------------------------------------------------

const WorldSnapshotReaderWriter::Node* WorldSnapshotReaderWriter::find (const int64 networkIdInt) const
{
	NOT_NULL (m_networkIdNodeMap);
	NetworkIdNodeMap::iterator iter = m_networkIdNodeMap->find (networkIdInt);
	if (iter != m_networkIdNodeMap->end () && !iter->second->isDeleted ())
		return iter->second;

	return 0;
}

//-------------------------------------------------------------------

void WorldSnapshotReaderWriter::removeNode (const int64 networkIdInt)
{
	NOT_NULL (m_networkIdNodeMap);
	NetworkIdNodeMap::iterator iter = m_networkIdNodeMap->find (networkIdInt);
	if (iter != m_networkIdNodeMap->end ())
	{
		iter->second->setDeleted ();
		m_networkIdNodeMap->erase(iter);
	}
}

//-------------------------------------------------------------------

void WorldSnapshotReaderWriter::removeFromWorld ()
{
	for (int i = 0; i < getNumberOfNodes (); ++i)
		getNode (i)->removeFromWorld ();
}

//===================================================================
// PRIVATE WorldSnapshotReaderWriter
//===================================================================

void WorldSnapshotReaderWriter::load_0001 (Iff& iff)
{
	iff.enterForm (TAG_0001);

		//-- insert the node list
		iff.enterForm (TAG_NODS);

			//-- read children
			while (iff.getNumberOfBlocksLeft ())
			{
				Node* const node = new Node;
				node->load (iff);

#ifdef _DEBUG
				if (ms_loadSingleNetworkId && node->getNetworkIdInt() != ms_loadSingleNetworkId)
					delete node;
				else
#endif
					m_nodeList->push_back(node);
			}  //lint !e429  //-- custodial pointer is not freed or returned

		iff.exitForm (TAG_NODS);

		//-- insert the object template name table
		iff.enterChunk (TAG_OTNL);

			{
				const int n = iff.read_int32 ();
				int i;
				for (i = 0; i < n; ++i)
				{
					char * const objectTemplateName = iff.read_string();
					(*m_objectTemplateCrcMap)[Crc::calculate(objectTemplateName)] = m_objectTemplateNameList->size();
					m_objectTemplateNameList->push_back(objectTemplateName);
				}
			}

		iff.exitChunk (TAG_OTNL);

	iff.exitForm (TAG_0001);
}

//===================================================================

