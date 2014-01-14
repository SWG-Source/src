// ======================================================================
//
// FloorTri.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_FloorTri_H
#define INCLUDED_FloorTri_H

// ======================================================================

#include "sharedMath/Vector.h"
#include "sharedCollision/CollisionEnums.h"

class Iff;

// ======================================================================
// Dumb Data

class IndexedTri
{
public:

    IndexedTri();

    // ----------

    int     getCornerIndex  ( int whichCorner ) const;
    void    setCornerIndex  ( int whichCorner, int newIndex );

    void    read_0000       ( Iff & iff );
    void    write           ( Iff & iff ) const;

    // ----------

protected:

    int     m_corners  [3];     //! Vertex indices
};

// ----------------------------------------------------------------------

inline int      IndexedTri::getCornerIndex      ( int whichCorner ) const
{
    return m_corners[whichCorner % 3];
}

inline void     IndexedTri::setCornerIndex      ( int whichCorner, int newIndex )
{
    m_corners[whichCorner % 3] = newIndex;
}

// ======================================================================

class ConnectedTri : public IndexedTri
{
public:

    ConnectedTri();

    int     getIndex            ( void ) const;
    void    setIndex            ( int newIndex );

    bool    hasNeighbor         ( int whichEdge ) const;
    int     getNeighborIndex    ( int whichEdge ) const;
    void    setNeighborIndex    ( int whichEdge, int newIndex );

    void    read_0000           ( Iff & iff );
    void    write               ( Iff & iff ) const;

    // ----------

protected:

    int         m_index;            //! This triangle's index
    int         m_neighbors[3];     //! Neighbor triangle indices
};

// ----------

inline int      ConnectedTri::getIndex          ( void ) const
{
    return m_index;
}

inline void     ConnectedTri::setIndex          ( int newIndex )
{
    m_index = newIndex;
}

inline bool     ConnectedTri::hasNeighbor           ( int whichEdge ) const
{
    return getNeighborIndex(whichEdge) != -1;
}

inline int      ConnectedTri::getNeighborIndex      ( int whichEdge ) const
{
    return m_neighbors[whichEdge % 3];
}

inline void     ConnectedTri::setNeighborIndex      ( int whichEdge, int newIndex )
{
    m_neighbors[whichEdge % 3] = newIndex;
}

// ----------------------------------------------------------------------

class FloorTri : public ConnectedTri
{
public:

    FloorTri();
    ~FloorTri();

    // ----------

    Vector const &  getNormal      ( void ) const;
    void            setNormal      ( Vector const & newNormal );

    bool            isFallthrough  ( void ) const;
    void            setFallthrough ( bool bFallthrough );

    bool            isCrossable    ( int whichEdge ) const;
    void            setCrossable   ( int whichEdge, bool bCrossable );

	FloorEdgeType   getEdgeType    ( int whichEdge ) const;
	void            setEdgeType    ( int whichEdge, FloorEdgeType newType );

    int             getPartTag     ( void ) const;
    void            setPartTag     ( int newTag );

    int             getPortalId    ( int whichEdge ) const;
    void            setPortalId    ( int whichEdge, int newId );

    void            read_0000      ( Iff & iff );
    void            read_0001      ( Iff & iff );
    void            read_0002      ( Iff & iff );
    void            write          ( Iff & iff ) const;

	void            setHeightFunc  ( Vector const & func );
	float           getHeightAt    ( float x, float z ) const;

    // ----------

    int             getMark        ( void ) const;
    void            setMark        ( int newMark ) const;

    int             getEdgeMark    ( int whichEdge ) const;
    void            setEdgeMark    ( int whichEdge, int newMark ) const;

    // ----------

	static int getOnDiskSize_0001 ( void );
	static int getOnDiskSize_0002 ( void );

protected:

    Vector        m_normal;           //! This triangle's normal
    FloorEdgeType m_edgeTypes[3];     
    bool          m_fallthrough;      //! If this is true, you'll fall onto the terrain under the floor instead of standing on the floor
    int           m_partTag;          //! Each disjoint part of a floor has a different part tag

    mutable int   m_mark;             //! Mark value, used in various algorithms to turn them from O(N^2) to O(N).
    mutable int   m_edgeMarks[3];     //! Mark value for each edge of this floor tri.

    int           m_portalIds[3];     //! Adjacent portal IDs for the edges of the tri. 
                                      //! This probably is wasteful since most of the time they'll be -1, but...

	Vector        m_heightFunc;       //! y(x,z) = m_heightFunc.x * x + m_heightFunc.y + m_heightFunc.z * z
};

// ----------

inline Vector const &   FloorTri::getNormal ( void ) const
{
    return m_normal;
}

inline void     FloorTri::setNormal         ( Vector const & newNormal )
{
    m_normal = newNormal;
}

// ----------

inline int      FloorTri::getMark               ( void ) const
{
    return m_mark;
}

inline void     FloorTri::setMark               ( int newMark ) const
{
    m_mark = newMark;
}

// ----------

inline int      FloorTri::getEdgeMark           ( int whichEdge ) const
{
    return m_edgeMarks[whichEdge];
}

inline void     FloorTri::setEdgeMark           ( int whichEdge, int newMark ) const
{
    m_edgeMarks[whichEdge] = newMark;
}
// ----------

inline bool     FloorTri::isFallthrough         ( void ) const
{
    return m_fallthrough;
}

inline void     FloorTri::setFallthrough        ( bool bFallthrough )
{
    m_fallthrough = bFallthrough;
}

// ----------

inline bool     FloorTri::isCrossable           ( int whichEdge ) const
{
	if(m_portalIds[whichEdge % 3] == -1)
	{
	    return m_edgeTypes[whichEdge % 3] == FET_Crossable;
	}
	else
	{
		return true;
	}
}

inline void     FloorTri::setCrossable          ( int whichEdge, bool bCrossable )
{
    m_edgeTypes[whichEdge % 3] = bCrossable ? FET_Crossable : FET_Uncrossable;
}

// ----------

inline int      FloorTri::getPartTag            ( void ) const
{
    return m_partTag;
}

inline void     FloorTri::setPartTag            ( int newTag ) 
{
    m_partTag = newTag;
}

// ----------

inline FloorEdgeType FloorTri::getEdgeType           ( int whichEdge ) const
{
	if(m_portalIds[whichEdge % 3] == -1)
	{
		return m_edgeTypes[whichEdge % 3];
	}
	else
	{
		return FET_Crossable;
	}
}

inline void FloorTri::setEdgeType ( int whichEdge, FloorEdgeType newType )
{
	m_edgeTypes[whichEdge % 3] = newType;
}

// ----------

inline int      FloorTri::getPortalId           ( int whichEdge ) const
{
    return m_portalIds[whichEdge];
}

inline void     FloorTri::setPortalId           ( int whichEdge, int newId )
{
    m_portalIds[whichEdge] = newId;
}

// ======================================================================

#endif

