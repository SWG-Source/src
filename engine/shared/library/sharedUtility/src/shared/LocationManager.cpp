// ======================================================================
//
// LocationManager.cpp
// asommers
//
// copyright 2003, sony online entertainment
//
// ======================================================================

#include "sharedUtility/FirstSharedUtility.h"
#include "sharedUtility/LocationManager.h"

#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedMath/Rectangle2d.h"
#include "sharedMath/SphereTree.h"
#include "sharedMath/Vector2d.h"
#include "sharedUtility/BakedTerrain.h"
#include "sharedUtility/BakedTerrainReader.h"
#include "sharedUtility/ConfigSharedUtility.h"

#include <algorithm>
#include <map>
#include <string>

// ======================================================================
// LocationManagerNamespace
// ======================================================================

namespace LocationManagerNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class Node
	{
		MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

	public:

		static void install ();
		static void remove ();

	public:

		explicit Node (NetworkId const & networkId);
		~Node ();

		Sphere const getSphere () const;
		void setSphere (int x, int z, int radius);

		SpatialSubdivisionHandle * getSpatialSubdivisionHandle ();
		void setSpatialSubdivisionHandle (SpatialSubdivisionHandle * spatialSubdivisionHandle);

	private:

		Node ();
		Node (Node const & rhs);
		Node & operator= (Node const & rhs);

	private:

		NetworkId m_networkId;
		int m_x;
		int m_z;
		int m_radius;
		SpatialSubdivisionHandle * m_spatialSubdivisionHandle;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class NodeSphereExtentAccessor: public BaseSphereTreeAccessor<Node *, NodeSphereExtentAccessor>
	{
	public:

		static Sphere const getExtent (Node const * const node)
		{
			return node ? node->getSphere () : Sphere::zero;
		}
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef SphereTree<Node *, NodeSphereExtentAccessor> NodeSphereTree;
	typedef std::vector<Node *> NodeVector;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	std::string ms_planetName;

	NodeSphereTree ms_nodeSphereTree;

	typedef std::map<NetworkId, Node *> NodeMap;
	NodeMap ms_nodeMap;

	BakedTerrain * ms_bakedTerrain;

	typedef std::vector<std::pair<int, int> > OffsetList;
	OffsetList ms_offsetList;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void remove ();
	bool intersectsObject (float x, float z, float radius);
	void clearSphereTree (NodeSphereTree & sphereTree);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}

using namespace LocationManagerNamespace;

// ======================================================================
// STATIC PUBLIC LocationManager::Node
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL (Node, true, 0, 0, 0);

// ----------------------------------------------------------------------

void Node::install ()
{
	installMemoryBlockManager ();
}

// ----------------------------------------------------------------------

void Node::remove ()
{
	removeMemoryBlockManager ();
}

// ======================================================================
// PUBLIC LocationManagerNamespace::Node
// ======================================================================

Node::Node (NetworkId const & networkId) :
	m_networkId (networkId),
	m_x (0),
	m_z (0),
	m_radius (0),
	m_spatialSubdivisionHandle (0)
{
}

// ----------------------------------------------------------------------

Node::~Node ()
{
	m_spatialSubdivisionHandle = 0;
}

// ----------------------------------------------------------------------

Sphere const Node::getSphere () const
{
	return Sphere (Vector (static_cast<float> (m_x), 0.f, static_cast<float> (m_z)), static_cast<float> (m_radius));
}

// ----------------------------------------------------------------------

void Node::setSphere (int const x, int const z, int const radius)
{
	m_x = x;
	m_z = z;
	m_radius = radius;
}

// ----------------------------------------------------------------------

SpatialSubdivisionHandle * Node::getSpatialSubdivisionHandle ()
{
	return m_spatialSubdivisionHandle;
}

// ----------------------------------------------------------------------

void Node::setSpatialSubdivisionHandle (SpatialSubdivisionHandle * const spatialSubdivisionHandle)
{
	m_spatialSubdivisionHandle = spatialSubdivisionHandle;
}

// ======================================================================
// STATIC PUBLIC LocationManager
// ======================================================================

void LocationManager::install ()
{
	Node::install ();

	ExitChain::add (remove, "LocationManager::remove");
}

// ----------------------------------------------------------------------

void LocationManager::setPlanetName (char const * const planetName)
{
	ms_planetName = planetName;

	char fileName [256];
	sprintf (fileName, "terrain/%s.trn", planetName);

	delete ms_bakedTerrain;

	ms_bakedTerrain = BakedTerrainReader::create (fileName);
}

// ----------------------------------------------------------------------

void LocationManager::updateObject (NetworkId const & networkId, int const x, int const z, int const radius)
{
	Node * node = 0;

	NodeMap::iterator iter = ms_nodeMap.find (networkId);
	if (iter == ms_nodeMap.end ())
	{
		//-- add
		node = new Node (networkId);
		IGNORE_RETURN (ms_nodeMap.insert (std::make_pair (networkId, node)));
	}
	else
	{
		//-- update
		node = NON_NULL (iter->second);
		ms_nodeSphereTree.removeObject (node->getSpatialSubdivisionHandle ());
	}

	node->setSphere (x, z, radius);
	node->setSpatialSubdivisionHandle (ms_nodeSphereTree.addObject (node));
}

// ----------------------------------------------------------------------

void LocationManager::removeObject (NetworkId const & networkId)
{
	NodeMap::iterator iter = ms_nodeMap.find (networkId);
	if (iter != ms_nodeMap.end ())
	{
		Node * const node = NON_NULL (iter->second);
		ms_nodeSphereTree.removeObject (node->getSpatialSubdivisionHandle ());
		delete node;

		ms_nodeMap.erase (iter);
	}
}

// ----------------------------------------------------------------------

bool LocationManager::requestLocation (float const searchX, float const searchZ, float const searchRadius, float const locationRadius, bool const checkWater, bool const checkSlope, float & resultX, float & resultZ)
{
	Rectangle2d const searchArea (searchX - searchRadius, searchZ - searchRadius, searchX + searchRadius, searchZ + searchRadius);

	int const chunkSize = ConfigSharedUtility::getChunkSize();

	ms_offsetList.clear ();
	{
		//-- generate the offset list
		int const logicalWidth = static_cast<int> (searchArea.getWidth () / chunkSize);
		int const logicalHeight = static_cast<int> (searchArea.getHeight () / chunkSize);
		int const logicalRadius = static_cast<int> (searchRadius / chunkSize);
		int const centerX = logicalWidth / 2;
		int const centerZ = logicalHeight / 2;
		for (int i = 0; i < logicalWidth; ++i)
			for (int j = 0; j < logicalHeight; ++j)
			{
				int const logicalX = i - centerX;
				int const logicalZ = j - centerZ;
				if (sqr (logicalX) + sqr (logicalZ) < sqr (logicalRadius))
					ms_offsetList.push_back (std::make_pair (logicalX, logicalZ));
			}

		//-- shuffle offset list to randomize rasterization
		std::random_shuffle (ms_offsetList.begin (), ms_offsetList.end ());
	}

	bool found = false;
	{
		for (OffsetList::iterator iter = ms_offsetList.begin (); iter != ms_offsetList.end (); ++iter)
		{
			//-- start in the upper left corner of the search area
			Rectangle2d currentArea (0.f, 0.f, locationRadius * 2.f, locationRadius * 2.f);
			currentArea.translate (static_cast<float> (iter->first * chunkSize) + searchX, static_cast<float> (iter->second * chunkSize) + searchZ);

			//-- check the current area for water
			if (checkWater && ms_bakedTerrain && ms_bakedTerrain->getWater (currentArea))
				continue;

			//-- check the current area for slope
			if (checkSlope && ms_bakedTerrain && ms_bakedTerrain->getSlope (currentArea))
				continue;

			//-- check the current area for objects
			float const x = currentArea.getCenter ().x;
			float const z = currentArea.getCenter ().y;
			if (intersectsObject (x, z, locationRadius))
				continue;

			//-- we found a location
			found = true;
			resultX = x;
			resultZ = z;
		}
	}

	return found;
}

// ======================================================================
// LocationManagerNamespace
// ======================================================================

void LocationManagerNamespace::remove ()
{
	//-- delete the baked terrain
	if (ms_bakedTerrain)
	{
		delete ms_bakedTerrain;
		ms_bakedTerrain = 0;
	}

	//-- clear the sphere tree
	clearSphereTree (ms_nodeSphereTree);

	//-- clear the node map
	IGNORE_RETURN (std::for_each (ms_nodeMap.begin (), ms_nodeMap.end (), PointerDeleterPairSecond ()));
	ms_nodeMap.clear ();

	Node::remove ();
}

// ----------------------------------------------------------------------

bool LocationManagerNamespace::intersectsObject (float const x, float const z, float const radius)
{
	NodeVector result;
	ms_nodeSphereTree.findInRange (Vector (x, 0.f, z), radius, result);
	return !result.empty ();
}

// ----------------------------------------------------------------------

void LocationManagerNamespace::clearSphereTree (NodeSphereTree & sphereTree)
{
	typedef std::vector<std::pair<Node *, Sphere> > NodeSphereList;
	NodeSphereList nodeSphereList;
	sphereTree.dumpSphereTreeObjs (nodeSphereList);

	NodeSphereList::const_iterator end = nodeSphereList.end ();
	for (NodeSphereList::const_iterator iter = nodeSphereList.begin (); iter != end; ++iter)
	{
		Node * const node = iter->first;
		if (node)
		{
			sphereTree.removeObject (node->getSpatialSubdivisionHandle ());
			node->setSpatialSubdivisionHandle (0);

			delete node;
		}
	}
}

// ======================================================================

