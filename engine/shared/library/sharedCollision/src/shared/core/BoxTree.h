// ======================================================================
//
// BoxTree.h
// copyright (c) 2001 Sony Online Entertainment
//
// This is a very simple, limited bounding box hierarchy used for speeding
// up collision queries
//
// ======================================================================

#ifndef INCLUDED_BoxTree_H
#define INCLUDED_BoxTree_H

#include "sharedMath/AxialBox.h"
#include "sharedFoundation/Tag.h"

class BoxTreeNode;
class DebugShapeRenderer;

class Line3d;
class Ray3d;
class Segment3d;
class VectorArgb;
class BoxTreeNode;
class Iff;

typedef std::vector<BoxTreeNode> BoxTreeNodeVec;
typedef std::vector<AxialBox> BoxVec;
typedef std::vector<int> IdVec;

// ----------------------------------------------------------------------

class BoxTree
{
public:

	static void install();

public:

	BoxTree();
	~BoxTree();
	
	// ----------

	BoxTreeNode const * getRoot ( void ) const;

	// ----------

	void    build           ( BoxVec const & boxes );
	
	void    drawDebugShapes ( DebugShapeRenderer * renderer ) const;
	
	int     getNodeCount    ( void ) const;
	float   calcWeight      ( void ) const;
	float   calcBalance     ( void ) const;
	
	bool    testOverlap     ( AxialBox const & box, IdVec & outIds ) const;
	bool    testOverlap     ( Line3d const & line, IdVec & outIds ) const;
	bool    testOverlap     ( Ray3d const & ray, IdVec & outIds ) const;
	bool    testOverlap     ( Segment3d const & segment, IdVec & outIds ) const;
	
	bool    findClosest     ( Vector const & V, float maxDistance, float & outDistance, int & outId ) const;

	void    clearTestCounter( void );
	int     getTestCounter  ( void ) const;
	
	void    write           ( Iff & iff );
	void    read            ( Iff & iff );

protected:

	void    clear           ( void );

	void    read_0000       ( Iff & iff );

	void    pack            ( void );

	bool    isFlat          ( void ) const;

	// ----------

	BoxTreeNodeVec *    m_flatNodes;
	
	BoxTreeNode *       m_root;
	
	int                 m_testCounter;

private:

	BoxTree( BoxTree const & copy );
	BoxTree & operator = ( BoxTree const & copy );
};

// ----------------------------------------------------------------------

inline void BoxTree::clearTestCounter ( void )
{
	m_testCounter = 0;
}

inline int BoxTree::getTestCounter ( void ) const
{
	return m_testCounter;
}

// ----------

inline bool BoxTree::isFlat ( void ) const
{
	return m_flatNodes != nullptr;
}

// ======================================================================

#endif // #ifndef INCLUDED_BoxTree_H
