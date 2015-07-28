// ======================================================================
//
// MxCifQuadTree.h
//
// Copyright 2002, Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_MxCifQuadTree_H
#define INCLUDED_MxCifQuadTree_H


#include <vector>


//==============================================================================

/*******************************************************************************

	An mx-cif quadtree is a quadtree that stores an object at the minimum quad 
	that completely contains the object. They are further sub-divided into a
	one-dimentional variation of the mx-cif quadtree (mx-cif bintree) based on
	if the object crosses the x or y axis of the quad (objects that cross both are
	placed in the y-axis bintree). 

	There is an optimization of the mx-cif quadtree that could be implemented (at
	the expense of more memory) where the object is sub-divided and put into
	a quad one lower than its normal level. This is very similar to what is done
	in bsp trees.

	See http://www.cs.umd.edu/users/brabec/quadtree/rectangles/cifquad.html, or
	"Design and Analysis of Spatial Data Structures" by H. Samet (which 
	unfortunately is out of print).

*******************************************************************************/

//==============================================================================

class MxCifQuadTreeBounds;

//==============================================================================

class MxCifQuadTree
{
public:
	 MxCifQuadTree(float minX, float minY, float maxX, float maxY, int maxDepth);
	~MxCifQuadTree();

	bool addObject(const MxCifQuadTreeBounds & object);
	bool removeObject(const MxCifQuadTreeBounds & object);
	void getObjectsAt(float x, float y, std::vector<const MxCifQuadTreeBounds *> & objects) const;
	void getAllObjects(std::vector<const MxCifQuadTreeBounds *> & objects) const;

private:

	/**
	 * A one-dimentional variation of an MxCifQuadTree
	 */
	class MxCifBinTree
	{
	public:
				 MxCifBinTree(float min, float max, int maxDepth);
		virtual ~MxCifBinTree();

		bool         addObject(const MxCifQuadTreeBounds & object);
		bool         removeObject(const MxCifQuadTreeBounds & object);
		virtual void getObjectsAt(float x, float y, std::vector<const MxCifQuadTreeBounds *> & objects) const = 0;
		void         getAllObjects(std::vector<const MxCifQuadTreeBounds *> & objects) const;

	protected:

		bool                   split(void);
		
		virtual MxCifBinTree * createChild(float min, float max, int maxDepth) const = 0;
		virtual bool           isObjectInRange(const MxCifQuadTreeBounds & object) const = 0;

	protected:
		float m_min;            // min value of our range
		float m_max;            // max value of our range
		float m_center;         // center range value computed from the bounds
		int   m_maxDepth;       // max depth we can recurse from our level

		MxCifBinTree * m_left;  // left child
		MxCifBinTree * m_right; // right child

		// @todo: use an unique_ptr here?
		std::vector<const MxCifQuadTreeBounds *> m_objects;   // objects that are contained in our bounds
	};

	/**
	 * An MxCifBinTree optimized for searches in the X direction.
	 */
	class MxCifXBinTree : public MxCifBinTree
	{
	public:
				 MxCifXBinTree(float min, float max, int maxDepth);
		virtual ~MxCifXBinTree();

		virtual void getObjectsAt(float x, float y, std::vector<const MxCifQuadTreeBounds *> & objects) const;

	protected:

		virtual MxCifBinTree * createChild(float min, float max, int maxDepth) const
		{
			return new MxCifXBinTree(min, max, maxDepth);
		}

		virtual bool isObjectInRange(const MxCifQuadTreeBounds & object) const;
	};

	/**
	 * An MxCifBinTree optimized for searches in the Y direction.
	 */
	class MxCifYBinTree : public MxCifBinTree
	{
	public:
				 MxCifYBinTree(float min, float max, int maxDepth);
		virtual ~MxCifYBinTree();

		virtual void getObjectsAt(float x, float y, std::vector<const MxCifQuadTreeBounds *> & objects) const;

	protected:

		virtual MxCifBinTree * createChild(float min, float max, int maxDepth) const
		{
			return new MxCifXBinTree(min, max, maxDepth);
		}
		virtual bool isObjectInRange(const MxCifQuadTreeBounds & object) const;
	};

private:

	bool split(void);

private:
	float m_minX, m_minY;       // lower-left bound
	float m_maxX, m_maxY;       // upper-right bound
	float m_centerX, m_centerY; // center computed from the bounds
	int m_maxDepth;             // max depth we can recurse from our level
	
	MxCifQuadTree * m_urTree;   // upper-right child
	MxCifQuadTree * m_ulTree;   // upper-left child
	MxCifQuadTree * m_llTree;   // lower-left child
	MxCifQuadTree * m_lrTree;   // lower-right child

	MxCifXBinTree m_xAxisTree;  // objects that have a minimum intersection of the x-axis
	MxCifYBinTree m_yAxisTree;  // objects that have a minimum intersection of the -axis
};

//==============================================================================


inline MxCifQuadTree::MxCifXBinTree::MxCifXBinTree(float min, float max, int maxDepth) :
	MxCifQuadTree::MxCifBinTree(min, max, maxDepth)
{
}

inline MxCifQuadTree::MxCifYBinTree::MxCifYBinTree(float min, float max, int maxDepth) :
	MxCifQuadTree::MxCifBinTree(min, max, maxDepth)
{
}

#endif // INCLUDED_MxCifQuadTree_H
