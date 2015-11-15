// ======================================================================
//
// Footprint.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedCollision/FirstSharedCollision.h"
#include "sharedCollision/FloorTri.h"

#include "sharedFile/Iff.h"

IndexedTri::IndexedTri()
{
	m_corners[0] = -1;
	m_corners[1] = -1;
	m_corners[2] = -1;
}

// ----------

void	IndexedTri::read_0000	( Iff & iff )
{
	setCornerIndex( 0, iff.read_int32() );
	setCornerIndex( 1, iff.read_int32() );
	setCornerIndex( 2, iff.read_int32() );
}

void	IndexedTri::write		( Iff & iff ) const
{
	iff.insertChunkData( getCornerIndex(0) );
	iff.insertChunkData( getCornerIndex(1) );
	iff.insertChunkData( getCornerIndex(2) );
}

// ----------------------------------------------------------------------

ConnectedTri::ConnectedTri()
: IndexedTri(),
  m_index(-1)
{
	m_neighbors[0] = -1;
	m_neighbors[1] = -1;
	m_neighbors[2] = -1;
}

void	ConnectedTri::read_0000	( Iff & iff )
{
	IndexedTri::read_0000(iff);

	setIndex( iff.read_int32() );

	setNeighborIndex( 0, iff.read_int32() );
	setNeighborIndex( 1, iff.read_int32() );
	setNeighborIndex( 2, iff.read_int32() );
}

void	ConnectedTri::write	( Iff & iff ) const
{
	IndexedTri::write(iff);

	iff.insertChunkData( getIndex() );

	iff.insertChunkData( getNeighborIndex(0) );
	iff.insertChunkData( getNeighborIndex(1) );
	iff.insertChunkData( getNeighborIndex(2) );
}

// ----------------------------------------------------------------------

FloorTri::FloorTri() 
: ConnectedTri(),
  m_normal( Vector::zero ),
  m_fallthrough( false ),
  m_partTag( -1 ),
  m_mark( 0 )
{
	m_edgeTypes[0] = FET_Uncrossable;
	m_edgeTypes[1] = FET_Uncrossable;
	m_edgeTypes[2] = FET_Uncrossable;

	m_edgeMarks[0] = 0;
	m_edgeMarks[1] = 0;
	m_edgeMarks[2] = 0;

    m_portalIds[0] = -1;
    m_portalIds[1] = -1;
    m_portalIds[2] = -1;
}

FloorTri::~FloorTri()
{
}

// ----------------------------------------------------------------------

void	FloorTri::read_0000		( Iff & iff )
{
	ConnectedTri::read_0000(iff);

	setNormal( iff.read_floatVector() );

	setCrossable( 0, iff.read_bool8() );
	setCrossable( 1, iff.read_bool8() );
	setCrossable( 2, iff.read_bool8() );

	setFallthrough( iff.read_bool8() );
}

// ----------

void    FloorTri::read_0001     ( Iff & iff )
{
	ConnectedTri::read_0000(iff);

	setNormal( iff.read_floatVector() );

	setCrossable( 0, iff.read_bool8() );
	setCrossable( 1, iff.read_bool8() );
	setCrossable( 2, iff.read_bool8() );

	setFallthrough( iff.read_bool8() );

    setPartTag( iff.read_int32() );

    setPortalId( 0, iff.read_int32() );
    setPortalId( 1, iff.read_int32() );
    setPortalId( 2, iff.read_int32() );
}

// ----------

void    FloorTri::read_0002     ( Iff & iff )
{
	ConnectedTri::read_0000(iff);

	setNormal( iff.read_floatVector() );

	setEdgeType( 0, static_cast<FloorEdgeType>(iff.read_uint8()) );
	setEdgeType( 1, static_cast<FloorEdgeType>(iff.read_uint8()) );
	setEdgeType( 2, static_cast<FloorEdgeType>(iff.read_uint8()) );

	setFallthrough( iff.read_bool8() );

    setPartTag( iff.read_int32() );

    setPortalId( 0, iff.read_int32() );
    setPortalId( 1, iff.read_int32() );
    setPortalId( 2, iff.read_int32() );
}

// ----------------------------------------------------------------------

void	FloorTri::write			( Iff & iff ) const
{
	ConnectedTri::write(iff);

	iff.insertChunkFloatVector( getNormal() );
	
	iff.insertChunkData( static_cast<uint8>(getEdgeType(0)) );
	iff.insertChunkData( static_cast<uint8>(getEdgeType(1)) );
	iff.insertChunkData( static_cast<uint8>(getEdgeType(2)) );

	iff.insertChunkData( isFallthrough() );

    iff.insertChunkData( getPartTag() );

    iff.insertChunkData( getPortalId(0) );
    iff.insertChunkData( getPortalId(1) );
    iff.insertChunkData( getPortalId(2) );
}

// ----------------------------------------------------------------------

void FloorTri::setHeightFunc ( Vector const & heightFunc )
{
	m_heightFunc = heightFunc;
}

float FloorTri::getHeightAt ( float x, float z ) const
{
	return (x * m_heightFunc.x) + m_heightFunc.y + (z * m_heightFunc.z);
}

// ----------------------------------------------------------------------
// Old versions of FloorMesh didn't explicitly write the number of
// floor tris in the mesh. We can determine how many tris are in the
// IFF chunk if we know how much data was written for each floor tri.

int FloorTri::getOnDiskSize_0001 ( void )
{
	// Version 0001 has 12 bytes for ConnectedTri, 16 bytes for IndexedTri,
	// and 32 bytes for FloorTri == 60 bytes total.

	return 60;
}

// ----------------------------------------------------------------------
