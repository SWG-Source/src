// ======================================================================
//
// TerrainQuadTree.h
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TerrainQuadTree_H
#define INCLUDED_TerrainQuadTree_H

// ======================================================================

#include "sharedCollision/BoxExtent.h"
#include "sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedTerrain/ProceduralTerrainAppearance.h"

// ======================================================================
/**
* TerrainQuadTree is a wrapper class for a quadtree of TerrainQuadTree::Nodes
* 
* @author jwatson
* @date 2001-01-12
* @see nothing
*/

class TerrainQuadTree 
{
public:

	/**
	* TerrainQuadTree::Node is a node in the Terrain Quad Tree. The interface is pretty fat. (not phat)
	*/

	class Node
	{
		MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL;

	public:

		static void install ();
		static void remove ();

	public:
		
		/**
		* neighbors
		*/
		enum
		{
			NORTH	= 0,
			WEST	= 1,
			SOUTH	= 2,
			EAST	= 3,
			NUM_NEIGHBORS = 4
		};

		/**
		* neighbor flags
		*/
		enum
		{
			 NORTH_FLAG	= (1<<0)
			,WEST_FLAG	= (1<<1)
			,SOUTH_FLAG	= (1<<2)
			,EAST_FLAG	= (1<<3)
		};

		static const int REVERSE_DIRECTIONS[4];

		/**
		* quadrants
		*/
		enum
		{
			NORTHEAST = 0,
			NORTHWEST = 1,
			SOUTHWEST = 2,
			SOUTHEAST = 3

		};

		static const int OPPOSITE_QUADRANTS[4];

		/**
		* half-quadrants
		*/
		enum
		{
			ENE = 0,
			NNE = 1,
			NNW = 2,
			WNW = 3,
			WSW = 4,
			SSW = 5,
			SSE = 6,
			ESE = 7
		};
	
		static const int OPPOSITE_HALFQUADRANTS[8];

		/**
		* MAX_SIZE should be the square root of the maximum numeric capacity of 32 bit unsigned int
		*/
		enum 
		{ 
			MAX_SIZE = 1 << 14
		};

	public:

		Node (int newX, int newZ, int newSize, float theMinChunkWidthInMeters);
		~Node (); 

		void         destroyNode (Node * node);
		void         removeNode (Node * node);

		void         addNode (Node * node, int quadrant);
		Node *       addChunk (ProceduralTerrainAppearance::Chunk * newChunk, const int chunkSize);

		int          removeSubNodes (bool deleteChunks);
		int          removeSubNodeChunks (bool deleteChunks, bool recurse=true);

		Node *       findChunkNode (const ProceduralTerrainAppearance::Chunk * aChunk, int ax, int az, int chunkSize);

		const BoxExtent & getBoxExtent () const;
		bool         isBoxExtentInitialized () const;

		bool         hasChunk (const ProceduralTerrainAppearance::Chunk * aChunk, int ax, int az, int chunkSize);

		bool         nodeEncloses (int ax, int az) const;

		void         pruneTree ();
		void         removeChunk (ProceduralTerrainAppearance::Chunk * aChunk, bool deleteChunk);

		const ProceduralTerrainAppearance::Chunk * getChunk () const;
		ProceduralTerrainAppearance::Chunk * getChunk ();

		int32        getX () const;
		int32        getZ () const;
		int32        getSize () const;

		int          getNumberOfChunks () const;

		unsigned     getChildIndex () const;
		Node *       getParent ();
		const Node * getParent () const;
		Node *       getSubNode (const int which) const;

		int          getQuadrant (const int ax, const int az) const;
		int          getQuadrant (const Vector & v) const;
		int          getHalfQuadrant (const int ax, const int az) const;
		int          getHalfQuadrant (const Vector & v) const;

		float        getDistanceSquared (const Vector & v) const;

		Node *       findNode (int ax, int az, int asize);

		const Node * findFirstRenderableNode (const Vector& position) const;
		Node *       findFirstRenderableNode (const Vector& position);

		const Node * findFirstRenderableNode (int ax, int az) const;
		Node *       findFirstRenderableNode (int ax, int az);

		Node *       findFirstRenderableLowerNeighbor (const Node * otherNode, int dir);
		Node *       findFirstRenderablePeerNeighbor (const Node * otherNode, int dir);

		bool         isSelectedForRender () const;
		void         setSelectedForRender (bool b);

		//-- larger neighbor stuff

		bool         getHasLargerNeighbor (int dir) const;
		void         setHasLargerNeighbor (int dir, bool b);
		void         clearHasLargerNeighbors ();
		bool         getHasAnyLargerNeighbors () const;
		unsigned     getHasLargerNeighbors () const;

		bool         isOutsideBuildRange () const;
		void         setOutsideBuildRange (bool b);

		const Node * getNeighbor (int index) const;
		Node *       getNeighbor (int index);

	private:

		Node ();
		Node (const Node &);
		Node & operator= (const Node &);

		int		     internalRemoveSubNodes (bool deleteChunks);
		int		     internalRemoveSubNodeChunks (bool deleteChunks, bool recurse) const;

		/**
		* find the quadrant given the x and z coordinates relative
		* to the center of this node
		*/
		template<class T> int getQuadrantTemplate (const T ax, const T az) const
		{
			// left quadrants
			if (ax < 0)
			{
				// bottom left quadrant
				if (az < 0)
					return SOUTHWEST;
				// top left quadrant
				else
					return NORTHWEST;
			}
			// bottom right quadrant
			else if (az < 0)
				return SOUTHEAST;
			// top right quadrant
			else
				return NORTHEAST;
		}

		/**
		* find the half-quadrant given the x and z coordinates relative
		* to the center of this node
		*/	
		template<class T> int getHalfQuadrantTemplate (const T ax, const T az) const
		{
			// left quadrants
			if (ax < 0)
			{
				// bottom left quadrant
				if (az < 0)
				{
					return (ax < az) ? WSW : SSW;
				}
				// top left quadrant
				else
				{
					return (-ax > az) ? NNW : WNW;
				}
			}
			// bottom right quadrant
			else if (az < 0)
			{
				return (ax < -az) ? SSE : ESE;
			}
			// top right quadrant
			else
			{
				return (ax > az) ? ENE : NNE;
			}
		}

	private:

		/**
		* The chunkspace coordinates of the node.
		*/
		const short   m_x;
		const short   m_z;

		/**
		* The chunkspace size (width & height) of this node. Size is always a power of 2.
		* The maximum number of chunks contained under a node is size*size.
		*/
		const short m_size;

		unsigned short m_childIndex             : 2; // The index of this child node as a subnode of its parent.
		unsigned short m_worldExtentInitialized : 1;
		unsigned short hasLargerNeighborFlags   : 4;
		unsigned short m_selectedForRender      : 1;
		unsigned short m_outsideBuildRange      : 1;

		/**
		* This is the width of the smallest chunk on the map. REDUNDANT, store elsewhere?
		*/
		const float m_minChunkWidthInMeters;

		/**
		* The chunk which lives at this node. Currently all chunks are assumed
		* to live on size 1 leaf nodes.
		*/
		ProceduralTerrainAppearance::Chunk * m_chunk;

		/**
		* Subnodes are arranged spatially the same as mathematical quadrants.
		*/

		Node *      m_subNodes[4];
		Node *      m_parent;
		Node *      m_neighbors[4];

		/**
		* Current number of size 1 chunks contained beneath this node.
		*/
		int         m_numberOfChunks;

		/**
		* The worldspace extents of this node. This extent contains all subnodes.
		*/
		BoxExtent   m_worldExtent;
	};

	/**
	* Constants for Iterator tree traversal
	*/

	class IteratorConstants
	{
	public:
		static const int ORDER_PRE[4];

		// traversal starting with the specified corner, front to back
		static const int ORDER_ENE[4];
		static const int ORDER_NNE[4];
		static const int ORDER_NNW[4];
		static const int ORDER_WNW[4];
		static const int ORDER_WSW[4];
		static const int ORDER_SSW[4];
		static const int ORDER_SSE[4];
		static const int ORDER_ESE[4];

		static const int * const HALF_QUADRANT_ORDERS[8];
	};

	/**
	* TerrainQuadTree::Iterator is a preorder iterator
	* To use an iterator, loop while the current node is non-nullptr. If the
	* current node's subtree should be processed further, call descend ()
	* on the iterator, otherwise call advance () to go to the next node.
	* Either advance () or descend () should be called every loop iteration,
	* otherwise an infinite loop will result.
	*
	*
	* This template floatly depends on having a pointer type as its template parameter.
	* T must be assignable from zero and have a default constructor. A zero sentinel
	* is used to improve performance.
	*/

	template <class T>
	class IteratorTemplate
	{
	public:

		explicit IteratorTemplate (T node);

		void reset (T node);

		T advance ();

		bool isEmpty () const;

		T getCurNode ();

		T descend (const int * theOrder = IteratorConstants::ORDER_PRE);

		T descendOneChild (int index);

		void push (T node);


	private:

		IteratorTemplate ();

		enum
		{
			maxDepth = 64 // 16 deep x 4 nodes
		};

		//* The iterator is a stack of nodes
		T s[maxDepth];
		int depth;
	};

	/**
	* The ordinary, non-const Iterator
	*/
	typedef IteratorTemplate<Node *> Iterator;

	/**
	* The const Iterator
	*/

	typedef IteratorTemplate<const Node *> ConstIterator;

public:

	static void setUseDistance2d (bool useDistance2d);

public:

	explicit TerrainQuadTree (int maxsize, float theMinChunkWidthInMeters);
	~TerrainQuadTree ();

	Node *       getTopNode ();
	const Node * getTopNode () const;

	Iterator     getIterator () const;
	Node *       addChunk (ProceduralTerrainAppearance::Chunk * chunk, const int chunkSize);
	void         removeChunk (ProceduralTerrainAppearance::Chunk * chunk, bool deleteChunk);
	int          getNumberOfChunks () const;
	void         pruneTree (); 

	ProceduralTerrainAppearance::Chunk * findChunk (int ax, int az, int chunkSize) const;
	Node *       findChunkNode (int ax, int az, int chunkSize) const;
	bool         hasChunk (int ax, int az, int chunkSize) const;

private:

	Node * m_nodes;

private:

	TerrainQuadTree ();
	TerrainQuadTree (const TerrainQuadTree&);
	TerrainQuadTree& operator= (const TerrainQuadTree&);
};

//-----------------------------------------------------------------
// TerrainQuadTree::Node
//-----------------------------------------------------------------
/**
* Get the BoxExtent which contains this node. This is the enclosing BoxExtent
* of all chunks which are children of this node and its subnodes.
* @return the BoxExtent
*/
inline const BoxExtent & TerrainQuadTree::Node::getBoxExtent () const
{
	return m_worldExtent;
}

//-----------------------------------------------------------------
/**
* Have any float-world chunks ever been added to this Node or its' subnodes?
* If not, then the BoxExtent is not yet valid.
*/

inline bool TerrainQuadTree::Node::isBoxExtentInitialized () const 
{
	return m_worldExtentInitialized;
}

//-----------------------------------------------------------------
/**
* Get the chunk located at this node. Currently chunks are only found on size 1 leaf nodes.
*/ 
inline const ProceduralTerrainAppearance::Chunk * TerrainQuadTree::Node::getChunk () const
{ 
	return m_chunk; 
} 

//-----------------------------------------------------------------

inline ProceduralTerrainAppearance::Chunk * TerrainQuadTree::Node::getChunk ()
{ 
	return m_chunk; 
} 

//-----------------------------------------------------------------
/**
* Get the X position of this node in chunkspace.
*/
inline int32 TerrainQuadTree::Node::getX () const
{ 
	return m_x;
}

//-----------------------------------------------------------------
/**
* Get the z position of this node in chunkspace.
*/
inline int32 TerrainQuadTree::Node::getZ () const 
{ 
	return m_z;
}

//-----------------------------------------------------------------
/**
* Get the size of this node in chunkspace.
*/
inline int32 TerrainQuadTree::Node::getSize () const 
{ 
	return m_size;
}

//-----------------------------------------------------------------
/**
* Get the number of chunks contained within this entire subtree.
*/
inline int TerrainQuadTree::Node::getNumberOfChunks () const 
{ 
	return m_numberOfChunks;
}

//-----------------------------------------------------------------
/**
* Get the childIndex, or the quadrant of this node as related to it's parent node.
*/
inline unsigned TerrainQuadTree::Node::getChildIndex () const 
{
	return m_childIndex;
}

//-----------------------------------------------------------------
/**
* Get the parent node.
*/
inline TerrainQuadTree::Node * TerrainQuadTree::Node::getParent ()
{
	return m_parent;
}

//-----------------------------------------------------------------
/**
* Get the parent node
*/
inline const TerrainQuadTree::Node * TerrainQuadTree::Node::getParent () const
{
	return m_parent;
}

//-----------------------------------------------------------------
/**
* Get the child node in the specified quadrant of this node.
*/
inline TerrainQuadTree::Node * TerrainQuadTree::Node::getSubNode (const int childIndex) const
{
	return m_subNodes [childIndex];
}

//-----------------------------------------------------------------
/**
* Is this node selected for rendering? If so, then it is rendered, and collision detection occurs with it.
*/
inline bool TerrainQuadTree::Node::isSelectedForRender () const
{ 
	return m_selectedForRender;
}

//-----------------------------------------------------------------

inline void TerrainQuadTree::Node::setSelectedForRender (const bool selectedForRender)
{
	m_selectedForRender = selectedForRender;
}

//-----------------------------------------------------------------
/**
* Has this node been flagged as having a larger neighbor in the specified direction?
*/
inline bool TerrainQuadTree::Node::getHasLargerNeighbor (const int direction) const
{
	return 0!=(hasLargerNeighborFlags & (1<<direction));
}

//-----------------------------------------------------------------
/**
* Flag this node's larger neighbor status in the specified direction
*/
inline void TerrainQuadTree::Node::setHasLargerNeighbor (const int direction, const bool hasLargerNeighbor)
{
	if (hasLargerNeighbor)
	{
		hasLargerNeighborFlags |= (1<<direction);
	}
	else
	{
		hasLargerNeighborFlags &= ~(1<<direction);
	}
}

//-----------------------------------------------------------------

inline void TerrainQuadTree::Node::clearHasLargerNeighbors () 
{
	hasLargerNeighborFlags=0;
}

//-----------------------------------------------------------------
/**
* Quick check to see if any neighbors are larger.
*/
inline bool TerrainQuadTree::Node::getHasAnyLargerNeighbors () const
{
	return hasLargerNeighborFlags!=0;
}

//-----------------------------------------------------------------
/**
* Get 4-element bool array of larger neighbor flags.
*/
inline unsigned TerrainQuadTree::Node::getHasLargerNeighbors() const
{
	return hasLargerNeighborFlags;
}

//-----------------------------------------------------------------

inline bool TerrainQuadTree::Node::isOutsideBuildRange () const
{
	return m_outsideBuildRange;
}

//-----------------------------------------------------------------

inline void TerrainQuadTree::Node::setOutsideBuildRange (const bool outsideBuildRange)
{
	m_outsideBuildRange = outsideBuildRange;
}

//-----------------------------------------------------------------
// TerrainQuadTree::IteratorTemplate
//-----------------------------------------------------------------
/**
* Construct an Iterator starting with the specified node. Initialize
* the sentinel and setup the stack with the selected node.
* @param node The top of the tree to traverse.
*/

template <class T>
inline TerrainQuadTree::IteratorTemplate<T>::IteratorTemplate (T node)
{
	NOT_NULL (node);

	s[0] = 0;
	s[1] = node;
	depth = 1;
}
//-----------------------------------------------------------------
/**
* Reset the iterator. The sentinel should be already correct.
*/

template <class T>
inline void TerrainQuadTree::IteratorTemplate<T>::reset (T node)
{
	NOT_NULL (node);
	DEBUG_FATAL (s[0] != 0, ("Corrupt sentinel")); // verify the sentinel
	s[1] = node;
	depth = 1;
}

//-----------------------------------------------------------------

template <class T>
inline void TerrainQuadTree::IteratorTemplate<T>::push (T node)
{
	NOT_NULL (node);
	DEBUG_FATAL (depth >= maxDepth, ("TerrainQuadTree::IteratorTemplate<T>: max depth exceeded"));

	s[++depth] = node;
}

//-----------------------------------------------------------------
/**
* advance pops the current node off the stack and returns a pointer to
* the next one, if there is one.
*/
template <class T>
inline T TerrainQuadTree::IteratorTemplate<T>::advance ()
{
	DEBUG_FATAL (depth <= 0, ("TerrainQuadTree::IteratorTemplate<T>: advancing past root"));

	return s[--depth];
}
//-----------------------------------------------------------------

template <class T>
inline bool TerrainQuadTree::IteratorTemplate<T>::isEmpty () const
{
	return depth==0;
}

//-----------------------------------------------------------------
/**
* get the top node of the stack
*/
template <class T>
inline T TerrainQuadTree::IteratorTemplate<T>::getCurNode ()
{ 
	return s[depth];
}

//-----------------------------------------------------------------
/**
* descend pushes the children of the current node onto the stack,
* in the reverse of the order specified. The result is that
* multiple calls to advance () will return the nodes in the order
* specified. The current node is removed from the stack, and the
* resulting top of the stack is returned.
*/
template <class T>
inline T TerrainQuadTree::IteratorTemplate<T>::descend (const int * theOrder)
{
	DEBUG_FATAL (isEmpty (), ("descent into empty iterator"));

	T node = s[depth--];

	// go through the order backwards since our iterator uses a stack rather than a queue
	for (int i = 3; i >= 0; --i)
	{
		const int index = theOrder [i];

		if (node->getSubNode (index))
			push (node->getSubNode (index));
	}

	return s[depth];
}

//-----------------------------------------------------------------

template <class T>
inline T TerrainQuadTree::IteratorTemplate<T>::descendOneChild (const int index)
{
	DEBUG_FATAL (isEmpty (), ("descent into empty iterator"));

	T node = s[depth--];

	if (node->getSubNode (index))
		push (node->getSubNode (index));

	return getCurNode ();
}

//-----------------------------------------------------------------
// TerrainQuadTree
//-----------------------------------------------------------------

inline TerrainQuadTree::Node * TerrainQuadTree::getTopNode () 
{ 
	return m_nodes; 
}

//-----------------------------------------------------------------

inline const TerrainQuadTree::Node * TerrainQuadTree::getTopNode () const 
{ 
	return m_nodes; 
}

//-----------------------------------------------------------------

#endif

