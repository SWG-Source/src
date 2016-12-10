//===================================================================
//
// WorldSnapshotReaderWriter.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_WorldSnapshotReaderWriter_H
#define INCLUDED_WorldSnapshotReaderWriter_H

//===================================================================

#include "sharedMath/Sphere.h"
#include "sharedMath/Transform.h"
#include "sharedFoundation/MemoryBlockManagerMacros.h"
#include <string>

class CrcString;
class Iff;
class MemoryBlockManager;
class SpatialSubdivisionHandle;

//===================================================================

class WorldSnapshotReaderWriter
{
public:

	class Node
	{
		friend class WorldSnapshotReaderWriter;

		MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

	public:

		static void      install ();

		static float     getDetailLevelBias ();
		static void      setDetailLevelBias (float detailLevelBias);

		typedef void (*DetailLevelChangedFunction) ();
		static void setDetailLevelChangedFunction (DetailLevelChangedFunction detailLevelChangedFunction);

	public:

		Node ();
		~Node ();

		void             setDeleted ();
		void             setNetworkIdInt (int64 networkIdInt);
		void             setContainedByNetworkIdInt (int64 containedByNetworkIdInt);
		void             setObjectTemplateNameIndex (int objectTemplateNameIndex);
		void             setCellIndex (int cellIndex);
		void             setTransform_p (const Transform& transform_p); 
		void             setRadius (float radius);
		void             setPortalLayoutCrc (uint32 portalLayoutCrc);
		void             setEventName(const std::string & eventName);

		bool             isDeleted () const;
		int64            getNetworkIdInt () const;
		int64            getContainedByNetworkIdInt () const;
		int              getObjectTemplateNameIndex () const;
		int              getCellIndex () const;
		const Transform& getTransform_p () const;
		float            getRadius () const;
		uint32           getPortalLayoutCrc () const;
		const std::string& getEventName() const; 

		int              getNumberOfNodes () const;
		const Node*      getNode (int nodeIndex) const;
		const Node*      getParent () const;

		int              getTotalNumberOfNodes () const;

		const Sphere     getSphere () const;
		SpatialSubdivisionHandle* getSpatialSubdivisionHandle () const;
		void             setSpatialSubdivisionHandle (SpatialSubdivisionHandle* spatialSubdivisionHandle) const;

		//-- used during run-time operation
		void             computeDistanceSquaredTo (const Vector& position_p) const;
		float            getDistanceSquaredTo () const;

		bool             isInWorld () const;
		void             addToWorld () const;
		void             removeFromWorld () const;

	private:

		static void      remove ();

	private:

		void             setParent (Node* node);
		void             addNode (Node* node);

		void             load (Iff& iff);
		void             save (Iff& iff) const;

		void             load_0000 (Iff& iff);

	private:

		Node (const Node&);
		Node& operator= (const Node&);

	private:

		bool             m_deleted;
		int64            m_networkIdInt;
		int64            m_containedByNetworkIdInt;
		int              m_objectTemplateNameIndex;
		int              m_cellIndex;
		Transform        m_transform_p;
		float            m_radius;
		uint32           m_portalLayoutCrc;
		Node*            m_parent;
		std::string      m_eventName;

		typedef std::vector<Node*> NodeList;
		NodeList*        m_nodeList;

		mutable SpatialSubdivisionHandle* m_spatialSubdivisionHandle;
		mutable float m_distanceSquaredTo;

		mutable bool m_inWorld;
	};

public:

	WorldSnapshotReaderWriter ();
	~WorldSnapshotReaderWriter ();

	bool             load (const char* filename);
	void             load (Iff& iff);
	bool             save (const char* filename) const;
	void             save (Iff& iff) const;

	//-- creation interface
	void             clear ();
	const Node*      addObject (int64 networkIdInt, int64 containedByNetworkIdInt, CrcString const &objectTemplateName, int cellIndex, const Transform& transform_p, float radius, uint32 portalLayoutCrc, const std::string & eventName = "");

	//-- query interface
	int              getNumberOfNodes () const;
	const Node*      getNode (int nodeIndex) const;
	int              getNumberOfObjectTemplateNames () const;
	const char*      getObjectTemplateName (int objectTemplateNameIndex) const;

	int              getTotalNumberOfNodes () const;

	Node*            find (int64 networkIdInt);
	const Node*      find (int64 networkIdInt) const;
	void             removeNode (int64 networkIdInt);

	void             removeFromWorld ();

private:

	void             load_0001 (Iff& iff);

	void             preSave () const;

private:

	WorldSnapshotReaderWriter (const WorldSnapshotReaderWriter&);
	WorldSnapshotReaderWriter& operator= (const WorldSnapshotReaderWriter&);

private:

	typedef std::vector<Node*> NodeList;
	NodeList* const m_nodeList;

	typedef std::vector<char*> ObjectTemplateNameList;
	ObjectTemplateNameList* const m_objectTemplateNameList;

	typedef std::unordered_map<uint32, uint> ObjectTemplateCrcMap;
	ObjectTemplateCrcMap* const m_objectTemplateCrcMap; // map from crc to index in name list

	typedef std::map<int64, Node*> NetworkIdNodeMap;
	NetworkIdNodeMap* const m_networkIdNodeMap;
};

//===================================================================

#endif
