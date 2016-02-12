// ======================================================================
//
// TerrainQuadTree.cpp
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedTerrain/FirstSharedTerrain.h"
#include "sharedTerrain/TerrainQuadTree.h"

#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedMath/Vector2d.h"
#include "sharedTerrain/ConfigSharedTerrain.h"
#include "sharedTerrain/ProceduralTerrainAppearance.h"

// ======================================================================
/**
* Quadtree iteration orders depend on the following quadrant layout:
* 0 = northeast
* 1 = northwest
* 2 = southwest
* 3 = southeast
*
* like mathematical quadrants:
* +-----------
* | 1  | 0  |
* |____|____|
* | 2  | 3  |
* |____|____|
*
* -1 represents the current node processing
*/

const int TerrainQuadTree::IteratorConstants::ORDER_PRE[4]	= { 0, 1, 2, 3 };

const int TerrainQuadTree::IteratorConstants::ORDER_ENE[4]	= { 0, 3, 1, 2 };
const int TerrainQuadTree::IteratorConstants::ORDER_NNE[4]	= { 0, 1, 3, 2 };
const int TerrainQuadTree::IteratorConstants::ORDER_NNW[4]	= { 1, 0, 2, 3 };
const int TerrainQuadTree::IteratorConstants::ORDER_WNW[4]	= { 1, 2, 0, 3 };
const int TerrainQuadTree::IteratorConstants::ORDER_WSW[4]	= { 2, 1, 3, 0 };
const int TerrainQuadTree::IteratorConstants::ORDER_SSW[4]	= { 2, 3, 1, 0 };
const int TerrainQuadTree::IteratorConstants::ORDER_SSE[4]	= { 3, 2, 0, 1 };
const int TerrainQuadTree::IteratorConstants::ORDER_ESE[4]	= { 3, 0, 2, 1 };

/**
* 
*
*/

const int * const TerrainQuadTree::IteratorConstants::HALF_QUADRANT_ORDERS[8] =
{
	TerrainQuadTree::IteratorConstants::ORDER_ENE,
	TerrainQuadTree::IteratorConstants::ORDER_NNE,
	TerrainQuadTree::IteratorConstants::ORDER_NNW,
	TerrainQuadTree::IteratorConstants::ORDER_WNW,
	TerrainQuadTree::IteratorConstants::ORDER_WSW,
	TerrainQuadTree::IteratorConstants::ORDER_SSW,
	TerrainQuadTree::IteratorConstants::ORDER_SSE,
	TerrainQuadTree::IteratorConstants::ORDER_ESE
};

const int TerrainQuadTree::Node::REVERSE_DIRECTIONS[4] =
{
	TerrainQuadTree::Node::SOUTH,
	TerrainQuadTree::Node::EAST,
	TerrainQuadTree::Node::NORTH,
	TerrainQuadTree::Node::WEST
};

const int TerrainQuadTree::Node::OPPOSITE_QUADRANTS[4] =
{
	TerrainQuadTree::Node::SOUTHWEST,
	TerrainQuadTree::Node::SOUTHEAST,
	TerrainQuadTree::Node::NORTHEAST,
	TerrainQuadTree::Node::NORTHWEST
};

const int TerrainQuadTree::Node::OPPOSITE_HALFQUADRANTS[8] =
{
	TerrainQuadTree::Node::WSW,
	TerrainQuadTree::Node::SSW,
	TerrainQuadTree::Node::SSE,
	TerrainQuadTree::Node::ESE,
	TerrainQuadTree::Node::ENE,
	TerrainQuadTree::Node::NNE,
	TerrainQuadTree::Node::NNW,
	TerrainQuadTree::Node::WNW
};

// ======================================================================

struct Wombat
{
	int outer  [2]; // neighbors outside the parent quad
	int inner  [2]; // neighbors inside the parent quad
	int child  [2];
};

//-----------------------------------------------------------------

static const Wombat s_wombats[4] =
{
	{
		{TerrainQuadTree::Node::NORTH,TerrainQuadTree::Node::EAST},
		{TerrainQuadTree::Node::SOUTH,TerrainQuadTree::Node::WEST},
		{TerrainQuadTree::Node::SOUTHEAST,TerrainQuadTree::Node::NORTHWEST},
	},
	{
		{TerrainQuadTree::Node::NORTH,TerrainQuadTree::Node::WEST},
		{TerrainQuadTree::Node::SOUTH,TerrainQuadTree::Node::EAST},
		{TerrainQuadTree::Node::SOUTHWEST,TerrainQuadTree::Node::NORTHEAST},
	},
	{
		{TerrainQuadTree::Node::SOUTH,TerrainQuadTree::Node::WEST},
		{TerrainQuadTree::Node::NORTH,TerrainQuadTree::Node::EAST},
		{TerrainQuadTree::Node::NORTHWEST,TerrainQuadTree::Node::SOUTHEAST},
	},
	{
		{TerrainQuadTree::Node::SOUTH,TerrainQuadTree::Node::EAST},
		{TerrainQuadTree::Node::NORTH,TerrainQuadTree::Node::WEST},
		{TerrainQuadTree::Node::NORTHEAST,TerrainQuadTree::Node::SOUTHWEST},
	}
};

// ======================================================================
// STATIC PUBLIC TerrainQuadTree::Node
// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL (TerrainQuadTree::Node, false, 256, 8, 0);

//-------------------------------------------------------------------

static bool ms_useDistance2d = false;

void TerrainQuadTree::Node::install ()
{
	installMemoryBlockManager ();
}

//-------------------------------------------------------------------

void TerrainQuadTree::Node::remove ()
{
	removeMemoryBlockManager ();
}

// ======================================================================
// PUBLIC TerrainQuadTree::Node
// ======================================================================
/**
* Create a node.
* @param newX the bottom left corner of the node in chunkspace.
* @param newY the bottom left corner of the node in chunkspace.
* @param newSize the width/height of the node in chunkspace.
*/
TerrainQuadTree::Node::Node (const int x, const int z, const int size, const float minChunkWidthInMeters) : 
	m_x(static_cast<short>(x)), 
	m_z(static_cast<short>(z)),
	m_size(static_cast<short>(size)), 
	m_childIndex (0), 
	m_worldExtentInitialized (false),
	m_selectedForRender (false),
	m_outsideBuildRange (false),
	m_minChunkWidthInMeters (minChunkWidthInMeters),
	m_chunk (0), 
	m_parent (0), 
	m_numberOfChunks (0),
	m_worldExtent ()
{
	DEBUG_FATAL(x>=32768 || x<-32768, ("Chunkspace coordinates are limited to +/-32768"));
	DEBUG_FATAL(z>=32768 || z<-32768, ("Chunkspace coordinates are limited to +/-32768"));
	DEBUG_FATAL(m_size >= MAX_SIZE, ("Cannot create a node bigger than or equal to %d\n", MAX_SIZE));
	DEBUG_FATAL(m_size < 1, ("Minimum size of quad node is 1!\n"));

	m_subNodes[0] = m_subNodes[1] = m_subNodes[2] = m_subNodes[3] = 0;
	clearHasLargerNeighbors ();

	Vector v;
	v.x = getX() * m_minChunkWidthInMeters;
	v.z = getZ() * m_minChunkWidthInMeters;
	v.y = 0;

	m_worldExtent.setMin (v);

	v.x += (getSize() * m_minChunkWidthInMeters);
	v.z += (getSize() * m_minChunkWidthInMeters);
	v.y = 0;

	m_worldExtent.setMax (v);
	m_worldExtent.calculateCenterAndRadius ();

	m_neighbors [0] = m_neighbors [1] = m_neighbors [2] = m_neighbors [3] = 0;
}

//-----------------------------------------------------------------
/**
* The destructor deletes any chunk that is still on the node.
*/

TerrainQuadTree::Node::~Node ()
{
	delete m_chunk;

	m_chunk = 0;
}


//-----------------------------------------------------------------
/**
* destroyNode simply calls removeNode for the specified node, and then deletes the specified node.
* @param node The node to be removed.  It is removed and deleted.  This should be a leaf node without chunks, otherwise you will have a memory leak or worse.
*/
void TerrainQuadTree::Node::destroyNode (TerrainQuadTree::Node * const node)
{
	removeNode (node);
	delete node;
}

//-----------------------------------------------------------------
/**
* removeNode detaches a node from the tree.  Nothing is deleted.
* @param node The node to be detached.  Node must be a direct child of this.
*/
void TerrainQuadTree::Node::removeNode (TerrainQuadTree::Node * const node)
{
	DEBUG_FATAL (node->m_parent != this, ("Can only call removeNode for direct parent-child.\n"));

	for (int i = 0; i < 4; ++i)
	{
		if (node->m_neighbors [i])
		{
			node->m_neighbors [i]->m_neighbors [REVERSE_DIRECTIONS [i]] = 0;
			node->m_neighbors [i] = 0;
		}
	}

	Node * p = this;

	while (p)
	{
		p->m_numberOfChunks -= node->m_numberOfChunks;
		p = p->m_parent;
	}

	m_subNodes [node->m_childIndex]	= 0;
	node->m_numberOfChunks				= 0;
	node->m_parent				= 0;
}

//-----------------------------------------------------------------
/**
* attach a node to the tree at the specified position and quadrant.
* @param node the node to attach
* @param quadrant an index in [0-3].
*/
void TerrainQuadTree::Node::addNode (TerrainQuadTree::Node * const node, const int quadrant)
{
	NOT_NULL (node);
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, quadrant, 4);
	DEBUG_FATAL(!nodeEncloses (node->getX (), node->getZ ()), ("attempt to add invalid subnode %d, %d {%d}\n", node->getX (), node->getZ (), node->getSize ()));
	DEBUG_FATAL(quadrant<0 || quadrant>4, ("Illegal child index."));

	node->m_childIndex    = static_cast<unsigned>(quadrant);
	node->m_parent        = this;
	m_subNodes [quadrant] = node;
	
	const Wombat & wb = s_wombats [quadrant];
	
	for (int i = 0; i < 2; ++i)
	{
		const int outer = wb.outer [i];
		const int inner = wb.inner [i];

		node->m_neighbors [outer] = 
			this->m_neighbors [outer] ? this->m_neighbors [outer]->getSubNode (wb.child [i]) : 0;

		if (node->m_neighbors [outer])
		{
			const int rev = REVERSE_DIRECTIONS [outer];
			DEBUG_FATAL (node->m_neighbors [outer]->m_neighbors [rev], ("error inserting terrain node\n"));
			node->m_neighbors [outer]->m_neighbors [rev] = node;
		}

		node->m_neighbors [wb.inner [i]] = this->getSubNode (wb.child [i]);

		if (node->m_neighbors [inner])
		{
			const int rev = REVERSE_DIRECTIONS [inner];
			node->m_neighbors [inner]->m_neighbors [rev] = node;
		}
	}
}

//-----------------------------------------------------------------
/**
* Add a chunk to the tree in the appropriate position, creating any nodes and subnodes as necessary to contain it.
* @param newChunk the chunk to add to the tree.  Currently all chunks are assumed to be of size 1.
*/
TerrainQuadTree::Node * TerrainQuadTree::Node::addChunk (ProceduralTerrainAppearance::Chunk * const chunk, const int size)
{
	NOT_NULL (chunk);

	DEBUG_FATAL (!nodeEncloses (chunk->getChunkX (), chunk->getChunkZ ()), ("Error adding chunk to node -- chunk %d,%d does not fit in the tree!!\n", chunk->getChunkX (), chunk->getChunkZ ()));
	
	if (m_worldExtentInitialized)
		m_worldExtent.grow (chunk->getBoxExtent ());
	else
	{
		// make sure the box extent of this quad node is at least as
		// big as the node in 2d, with the appropriate y values

		Vector v = m_worldExtent.getMin ();
		v.y = chunk->getBoxExtent ().getMin ().y;

		m_worldExtent.setMin (v);

		v = m_worldExtent.getMax ();
		v.y = chunk->getBoxExtent ().getMax ().y;

		m_worldExtent.setMax (v);

		m_worldExtent.calculateCenterAndRadius ();

		m_worldExtentInitialized = true;
	}

	m_numberOfChunks++;

	//-- hit the bottom, add the chunk
	if (getSize() == size)
	{
		
		DEBUG_FATAL ( m_chunk, ("Error adding chunk to node -- chunk is already in the tree!!\n"));
		m_chunk = chunk;
		return this;
	}

	int which = getQuadrant (chunk->getChunkX (), chunk->getChunkZ ());

	// add the node in the appropriate part of the array and the list
	if (m_subNodes [which] == 0)
	{
		const int half = getSize() / 2;

		Node * fresh = new Node (
								 (which == 2 || which == 1) ? getX() : getX() + half,
								 (which == 2 || which == 3) ? getZ() : getZ() + half,
								half,
								m_minChunkWidthInMeters);

		addNode (fresh, which);
	}
	
	return m_subNodes [which]->addChunk (chunk, size);
}

//-----------------------------------------------------------------
/**
* Remove all subtrees of this node.  All subnodes are deleted.  All chunks in the subtrees are deleted if indicated.
* The parents of this subnode DO NOT have their chunk counts updated apporpriately.
* @param deleteChunks if true, delete any chunks found in the tree.
*/
int TerrainQuadTree::Node::internalRemoveSubNodes (const bool deleteChunks)
{
	int num_chunks_removed = 0;

	for (int i = 0; i < 4; ++i )
	{
		Node * const snode = m_subNodes [i];

		if (snode)
		{
			if (snode->m_chunk)
			{
				if (deleteChunks)
					delete snode->m_chunk;

				snode->m_chunk = 0;
				num_chunks_removed++;
			}
			
			num_chunks_removed += snode->internalRemoveSubNodes (deleteChunks);
			
			for (int j = 0; j < 4; ++j)
			{
				if (snode->m_neighbors [j])
				{
					snode->m_neighbors [j]->m_neighbors [REVERSE_DIRECTIONS [j]] = 0;
					snode->m_neighbors [j] = 0;
				}
			}

			delete m_subNodes [i];
			m_subNodes [i] = 0;
		}
	}

	return num_chunks_removed;
}
//-----------------------------------------------------------------
/**
* Remove all subtrees of this node.  All subnodes are deleted.  All chunks in the subtrees are deleted if indicated.
* The parents of this chunk have their chunk counts updated appropriately.
* @param deleteChunks if true, delete any chunks found in the tree.
*/
	
int TerrainQuadTree::Node::removeSubNodes (const bool deleteChunks)
{
	if (getSize() == 1)
		return 0;

	int num_chunks_removed = internalRemoveSubNodes (deleteChunks);

	//-- update m_numberOfChunks for this node and all its' parental units
	Node * p = this;

	while (p)
	{
		p->m_numberOfChunks -= num_chunks_removed;
		p = p->m_parent;
	}

	return num_chunks_removed;
}

//-----------------------------------------------------------------
	
int TerrainQuadTree::Node::internalRemoveSubNodeChunks (const bool deleteChunks, const bool recurse) const
{
	int num_chunks_removed = 0;

	for (int i = 0; i < 4; ++i )
	{
		if (m_subNodes[i])
		{
			if (m_subNodes[i]->m_chunk)
			{
				if (deleteChunks)
					delete m_subNodes[i]->m_chunk;

				m_subNodes[i]->m_chunk = 0;
				num_chunks_removed++;
			}

			if (recurse)
				num_chunks_removed += m_subNodes [i]->internalRemoveSubNodeChunks (deleteChunks, recurse);
		}
	}

	return num_chunks_removed;
}

//-----------------------------------------------------------------

int TerrainQuadTree::Node::removeSubNodeChunks (const bool deleteChunks, const bool recurse)
{
	if (getSize() == 1)
		return 0;
	
	int num_chunks_removed = internalRemoveSubNodeChunks (deleteChunks, recurse);

	//-- update m_numberOfChunks for this node and all its' parental units
	Node * p = this;

	while (p)
	{
		p->m_numberOfChunks -= num_chunks_removed;
		p = p->m_parent;
	}

	return num_chunks_removed;

}

//-----------------------------------------------------------------
/**
* remove a chunk from the tree.  It's former node is not deleted.  uses findChunkNode.
* @param aChunk which chunk to remove
* @param deleteChunk delete the chunk or not
*/
void TerrainQuadTree::Node::removeChunk (ProceduralTerrainAppearance::Chunk * const chunk, const bool deleteChunk)
{
	NOT_NULL (chunk);

	if (m_chunk == chunk)
	{
		if (deleteChunk)
			delete m_chunk;
		m_chunk = 0;

		Node * p = this;

		while (p)
		{
			p->m_numberOfChunks--;
			p = p->m_parent;
		}

		return;
	}

	Node * node = findChunkNode (chunk, 0, 0, 0);
	NOT_NULL (node);

	node->removeChunk (chunk, deleteChunk);
}

//-----------------------------------------------------------------
/**
* pruneTree removes all subtrees which do not contain any chunks.  All subtree nodes are deleted. 
*/
void TerrainQuadTree::Node::pruneTree ()
{
	if (m_numberOfChunks == 0)
	{
		IGNORE_RETURN (removeSubNodes (false));
		return;
	}

	for (int i = 0; i < 4; ++i)
	{
		if (m_subNodes[i])
		{
			if (m_subNodes[i]->m_numberOfChunks == 0)
				IGNORE_RETURN (m_subNodes[i]->removeSubNodes (false));
			else
				m_subNodes[i]->pruneTree ();
		}
	}
}

//-----------------------------------------------------------------
/**
* Find the leaf node which directly contains this chunk.
*
* @param aChunk the chunk to be located.  If non nullptr, the parameters ax and az are ignored.
* @param ax the chunkspace coordinate of the chunk to be located.  Only used if param aChunk is nullptr
* @param az the chunkspace coordinate of the chunk to be located.  Only used if param aChunk is nullptr
*/
TerrainQuadTree::Node * TerrainQuadTree::Node::findChunkNode (const ProceduralTerrainAppearance::Chunk * const chunk, int x, int z, int size)
{
	Node * node = this;

	if (chunk)
	{
		x = chunk->getChunkX ();
		z = chunk->getChunkZ ();
		size = static_cast <int> (chunk->getChunkWidthInMeters () / m_minChunkWidthInMeters);
	}

	if (!node->nodeEncloses (x, z))
		return 0;

	while (node)
	{
		if (node->getSize() == size && node->getX() == x && node->getZ() == z)
			return node;
		
		const int which = node->getQuadrant (x, z);
		node = node->m_subNodes [which];
	}

	return 0;
}

TerrainQuadTree::Node * TerrainQuadTree::Node::findFirstRenderablePeerNeighbor (const TerrainQuadTree::Node * const otherNode, const int dir)
{
	NOT_NULL (otherNode);
	
	const int dir_offsets[4][2] =
	{		
		// north
		{ 0,						otherNode->getSize () },	
		// west
		{ -otherNode->getSize (),	0},		
		// south
		{ 0,						-otherNode->getSize () },
		// east
		{ otherNode->getSize (),	0 },
	};
	
	Node * neighbor = findFirstRenderableNode (	otherNode->getX() + dir_offsets[dir][0],
												otherNode->getZ() + dir_offsets[dir][1]);
	
	if (neighbor && neighbor->nodeEncloses (otherNode->getX(), otherNode->getZ()))
		neighbor = 0;

	return neighbor;
}

//-----------------------------------------------------------------

TerrainQuadTree::Node * TerrainQuadTree::Node::findFirstRenderableLowerNeighbor (const TerrainQuadTree::Node * const otherNode, const int dir)
{
	NOT_NULL (otherNode);
	
	const int halfSize = otherNode->getSize () / 2;
	
	const int dir_offsets[4][2][2] =
	{		
		// north
		{
			{ halfSize,					otherNode->getSize () },
			{ 0,						otherNode->getSize () }
		},	
		// west
		{
			{ -halfSize,				halfSize },
			{ -halfSize,				0 }
		},		
		// south
		{
			{ 0,						-halfSize },
			{ halfSize,					-halfSize }
		},
		// east
		{
			{ otherNode->getSize (),	0 },
			{ otherNode->getSize (),	halfSize }
		},
	};
	
	Node * neighbors [2];
	
	neighbors [0] = findFirstRenderableNode (	otherNode->getX() + dir_offsets[dir][0][0],
												otherNode->getZ() + dir_offsets[dir][0][1]);
	
	if (neighbors [0] && neighbors [0]->nodeEncloses (otherNode->getX(), otherNode->getZ()))
		neighbors [0] = 0;
	
	neighbors [1] = findFirstRenderableNode (	otherNode->getX() + dir_offsets[dir][1][0],
												otherNode->getZ() + dir_offsets[dir][1][1]);
	
	if (neighbors [1] && neighbors [1]->nodeEncloses (otherNode->getX(), otherNode->getZ()))
		neighbors [1] = 0;
	
	if (neighbors [0] && (neighbors [1] == 0 || neighbors [0]->getSize () < neighbors [1]->getSize ()))
		return neighbors [0];
	else
		return neighbors [1];
}

//-----------------------------------------------------------------

TerrainQuadTree::Node * TerrainQuadTree::Node::findNode (const int ax, const int az, const int asize)
{
	DEBUG_FATAL (asize > getSize(), ("findNode whaa???"));

	if (!nodeEncloses (ax, az))
		return 0;

	Node * node = this;

	while (node)
	{
		if (node->getSize () == asize)
		{
			break;
		}

		node = node->getSubNode (node->getQuadrant (ax, az));
	}

	return node;
}

//-----------------------------------------------------------------
/**
* Obtain the first renderable chunk which contains the specified position
*/
const TerrainQuadTree::Node * TerrainQuadTree::Node::findFirstRenderableNode (const Vector& position) const
{
	const int ax = static_cast <int> ((position.x >= 0 ? position.x : (position.x - m_minChunkWidthInMeters)) / m_minChunkWidthInMeters);
	const int az = static_cast <int> ((position.z >= 0 ? position.z : (position.z - m_minChunkWidthInMeters)) / m_minChunkWidthInMeters);

	return findFirstRenderableNode (ax, az);
} 

//-----------------------------------------------------------------
/**
* Obtain the first renderable chunk which contains the specified position
*/

TerrainQuadTree::Node * TerrainQuadTree::Node::findFirstRenderableNode (const Vector& position)
{
	const int ax = static_cast <int> ((position.x >= 0 ? position.x : (position.x - m_minChunkWidthInMeters)) / m_minChunkWidthInMeters);
	const int az = static_cast <int> ((position.z >= 0 ? position.z : (position.z - m_minChunkWidthInMeters)) / m_minChunkWidthInMeters);

	return findFirstRenderableNode (ax, az);
} 

//-------------------------------------------------------------------
/**
* Obtain the first renderable chunk which contains the specified position
*/
const TerrainQuadTree::Node * TerrainQuadTree::Node::findFirstRenderableNode (const int ax, const int az) const
{
	TerrainQuadTree::ConstIterator iter (this);

	const TerrainQuadTree::Node * node = 0;

	while ((node = iter.getCurNode ()) != 0)
	{
		if (node->isSelectedForRender ())
		{
			if (node->nodeEncloses (ax, az))
				return node;
			else
				return 0;
		}

		// TODO: no need to descend into all 4 quadrants... 1 is enough

		// descend front-to-back
		IGNORE_RETURN (iter.descendOneChild (node->getQuadrant (ax, az)));

	}

	return 0;
}

//-----------------------------------------------------------------
/**
* Obtain the first renderable chunk which contains the specified position
*/
TerrainQuadTree::Node * TerrainQuadTree::Node::findFirstRenderableNode (const int ax, const int az)
{
	TerrainQuadTree::Iterator iter (this);

	TerrainQuadTree::Node * node = 0;

	while ((node = iter.getCurNode ()) != 0)
	{
		if (node->isSelectedForRender ())
		{
			if (node->nodeEncloses (ax, az))
				return node;
			else
				return 0;
		}

		// TODO: no need to descend into all 4 quadrants... 1 is enough

		// descend front-to-back
		IGNORE_RETURN (iter.descendOneChild (node->getQuadrant (ax, az)));

	}

	return 0;
}

//-----------------------------------------------------------------
/**
* hasChunk is a wrapper for findChunkNode
*
* @param aChunk the chunk to be located. If non nullptr, the parameters ax and az are ignored.
* @param ax the chunkspace coordinate of the chunk to be located. Only used if param aChunk is nullptr
* @param az the chunkspace coordinate of the chunk to be located. Only used if param aChunk is nullptr
* @param chunkSize the relative size of the chunk in chunkspace.
*/
bool TerrainQuadTree::Node::hasChunk (const ProceduralTerrainAppearance::Chunk * const chunk, const int x, const int z, const int size)
{
	return findChunkNode (chunk, x, z, size) != 0;
}

//-----------------------------------------------------------------
/**
* Simple test in chunkspace.
*/
bool TerrainQuadTree::Node::nodeEncloses (const int x, const int z) const
{
	return 
		x >= getX() && x < (getX() + getSize()) &&
		z >= getZ() && z < (getZ() + getSize());
};

//-----------------------------------------------------------------
/**
* Determine what quadrant the given point is in, in relation to the center of this node.
*/
int TerrainQuadTree::Node::getQuadrant (const int x, const int z) const
{
	const int half = getSize() / 2;
	return getQuadrantTemplate ((x - getX()) - half, (z - getZ()) - half);
}

//-----------------------------------------------------------------
/**
* Determine what quadrant the given point is in, in relation to the center of this node.
*/ 
int TerrainQuadTree::Node::getQuadrant (const Vector & position) const
{
	const Vector & center = getBoxExtent ().getSphere ().getCenter ();
	return getQuadrantTemplate (position.x - center.x, position.z - center.z);
}

//-----------------------------------------------------------------
/**
* Determine what half-quadrant the given point is in, in relation to the center of this node.
* @param x the x position in chunkspace
* @param z the z position in chunkspace
*/
int TerrainQuadTree::Node::getHalfQuadrant (const int x, const int z) const
{
	const int half = getSize() / 2;
	return getHalfQuadrantTemplate ((x - getX()) - half, (z - getZ()) - half);
	
}

//-----------------------------------------------------------------
/**
* Determine what half-quadrant the given point is in, in relation to the center of this node.
* @param v the Vector position in worldspace
*/
int TerrainQuadTree::Node::getHalfQuadrant (const Vector & v) const
{
	const Vector & center = getBoxExtent ().getSphere ().getCenter ();
	return getHalfQuadrantTemplate (v.x - center.x, v.z - center.z);
}

//-----------------------------------------------------------------

float TerrainQuadTree::Node::getDistanceSquared (const Vector & position) const
{
	if (ms_useDistance2d)
	{
		const Vector& minimum = getBoxExtent ().getMin ();
		const Vector& maximum = getBoxExtent ().getMax ();
		const Rectangle2d rectangle (minimum.x, minimum.z, maximum.x, maximum.z);
		const Vector2d position2d (position.x, position.z);

		if (rectangle.isWithin (position2d))
			return 0.f;

		Vector2d corner;

		if (position2d.x < rectangle.x0)
			corner.x = rectangle.x0;
		else
			if (position2d.x > rectangle.x1)
				corner.x = rectangle.x1;
			else
				corner.x = position2d.x;

		if (position2d.y < rectangle.y0)
			corner.y = rectangle.y0;
		else 
			if (position2d.y > rectangle.y1)
				corner.y = rectangle.y1;
			else
				corner.y = position2d.y;

		return position2d.magnitudeBetweenSquared (corner);
	}
	else
	{
		const Vector & vmin = getBoxExtent ().getMin ();
		const Vector & vmax = getBoxExtent ().getMax ();

		if (vmin.x <= position.x && vmax.x >= position.x &&
			vmin.z <= position.z && vmax.z >= position.z &&
			 (!isBoxExtentInitialized () ||
			 (vmin.y <= position.y && vmax.y >= position.y)))
			return 0.f;

		Vector corner;

		// decide which octant the vector is in, and find the distance appropriately

		if (position.z < vmin.z)
			corner.z = vmin.z;
		else 
			if (position.z > vmax.z)
				corner.z = vmax.z;
			else
				corner.z = position.z;

		// uninitialized box extent has a invalid y values, so just use the min value.
		if (position.y < vmin.y || !isBoxExtentInitialized ())
			corner.y = vmin.y;
		else 
			if (position.y > vmax.y)
				corner.y = vmax.y;
			else
				corner.y = position.y;


		if (position.x < vmin.x)
			corner.x = vmin.x;
		else
			if (position.x > vmax.x)
				corner.x = vmax.x;
			else
				corner.x = position.x;

		return position.magnitudeBetweenSquared (corner);
	}
}

//-----------------------------------------------------------------

const TerrainQuadTree::Node * TerrainQuadTree::Node::getNeighbor (const int index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, index, 4);
	return m_neighbors [index];
}

//-----------------------------------------------------------------

TerrainQuadTree::Node * TerrainQuadTree::Node::getNeighbor (const int index)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE (0, index, 4);
	return m_neighbors [index];
} //lint !e1762  //-- function could be made const (this is the non-const version)

// ======================================================================
// PUBLIC TerrainQuadTree
// ======================================================================

void TerrainQuadTree::setUseDistance2d (bool useDistance2d)
{
	ms_useDistance2d = useDistance2d;
}

//-----------------------------------------------------------------
/** 
* Construct a tree centered at 0,0 with the root node having the specified size.
*/
TerrainQuadTree::TerrainQuadTree (const int maxsize, const float minChunkWidthInMeters) : 
	m_nodes (0)
{
	//-- is there more than one of these?!? -ALS
	Node::install ();

	REPORT_LOG_PRINT (ConfigSharedTerrain::getDebugReportLogPrint (), ("TerrainQuadTree creating at position %d,%d, size=%d\n", -maxsize/2, -maxsize/2, maxsize));
	m_nodes = new Node (-maxsize/2, -maxsize/2, maxsize, minChunkWidthInMeters);
};

//-----------------------------------------------------------------

TerrainQuadTree::~TerrainQuadTree ()
{
	IGNORE_RETURN (m_nodes->removeSubNodes (true));
	delete m_nodes;
	m_nodes = 0;

	Node::remove ();
}

//-----------------------------------------------------------------

TerrainQuadTree::Iterator TerrainQuadTree::getIterator () const
{
	return Iterator (m_nodes);
}

//-----------------------------------------------------------------

TerrainQuadTree::Node * TerrainQuadTree::addChunk (ProceduralTerrainAppearance::Chunk * const chunk, const int size)
{
	return m_nodes->addChunk (chunk, size);
}

//-----------------------------------------------------------------

void TerrainQuadTree::removeChunk (ProceduralTerrainAppearance::Chunk * const chunk, bool deleteChunk)
{
	m_nodes->removeChunk (chunk, deleteChunk);
}

//-----------------------------------------------------------------

int TerrainQuadTree::getNumberOfChunks () const
{
	NOT_NULL (m_nodes);
	return m_nodes->getNumberOfChunks ();
}

//-----------------------------------------------------------------

void TerrainQuadTree::pruneTree ()
{
	NOT_NULL (m_nodes);
	m_nodes->pruneTree ();
}

//-----------------------------------------------------------------

ProceduralTerrainAppearance::Chunk * TerrainQuadTree::findChunk (const int x, const int z, const int size) const
{
	NOT_NULL (m_nodes);
	Node * const node = m_nodes->findChunkNode (0, x, z, size);
	return node ? node->getChunk () : 0;
}

//-----------------------------------------------------------------

TerrainQuadTree::Node * TerrainQuadTree::findChunkNode (const int x, const int z, const int size) const
{
	NOT_NULL (m_nodes);
	return m_nodes->findChunkNode (0, x, z, size);
}

//-----------------------------------------------------------------

bool TerrainQuadTree::hasChunk (const int x, const int z, const int size) const
{
	NOT_NULL (m_nodes);
	return m_nodes->hasChunk (0, x, z, size);
}

//-----------------------------------------------------------------

