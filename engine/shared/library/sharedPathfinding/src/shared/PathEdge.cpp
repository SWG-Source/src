// ======================================================================
//
// PathEdge.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedPathfinding/FirstSharedPathfinding.h"
#include "sharedPathfinding/PathEdge.h"

#include "sharedFile/Iff.h"

// ======================================================================

PathEdge::PathEdge()
: m_indexA(-1),
  m_indexB(-1),
  m_laneWidthRight(0.0f),
  m_laneWidthLeft(0.0f)
{
}

PathEdge::PathEdge ( int indexA, int indexB )
: m_indexA(indexA),
  m_indexB(indexB),
  m_laneWidthRight(0.0f),
  m_laneWidthLeft(0.0f)
{
}

PathEdge::~PathEdge()
{
}

// ----------------------------------------------------------------------

void PathEdge::read_0000 ( Iff & iff )
{
	m_indexA = iff.read_int32();
	m_indexB = iff.read_int32();
	m_laneWidthRight = iff.read_float();
	m_laneWidthLeft = iff.read_float();
}

// ----------------------------------------------------------------------

void PathEdge::write ( Iff & iff ) const
{
	iff.insertChunkData( m_indexA );
	iff.insertChunkData( m_indexB );
	iff.insertChunkData( m_laneWidthRight );
	iff.insertChunkData( m_laneWidthLeft );
}

// ======================================================================
