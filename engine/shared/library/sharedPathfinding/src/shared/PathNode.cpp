// ======================================================================
//
// PathNode.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedPathfinding/FirstSharedPathfinding.h"
#include "sharedPathfinding/PathNode.h"

#include "sharedFile/Iff.h"

// ======================================================================

PathNode::PathNode()
: m_graph(nullptr),
  m_index(-1),
  m_id(-1),
  m_key(-1),
  m_type(PNT_Invalid),
  m_partId(-1),
  m_position( Vector::zero ),
  m_radius(0.0f),
  m_userId(-1)
{
	clearMarks();
}

PathNode::PathNode ( Vector const & position )
: m_graph(nullptr),
  m_index(-1),
  m_id(-1),
  m_key(-1),
  m_type(PNT_Invalid),
  m_partId(-1),
  m_position( position ),
  m_radius(0.0f),
  m_userId(-1)
{
	clearMarks();
}

PathNode::~PathNode()
{
}

void PathNode::copy ( PathNode const & P )
{
	m_id       = P.m_id;
	m_key      = P.m_key;
	m_type     = P.m_type;
	m_partId   = P.m_partId;
	m_position = P.m_position;
	m_radius   = P.m_radius;
	m_userId   = P.m_userId;
}

// ----------------------------------------------------------------------

void PathNode::read_0000 ( Iff & iff )
{
	m_index = iff.read_int32();
	m_id = iff.read_int32();
	m_key = iff.read_int32();
	m_type = static_cast<PathNodeType>(iff.read_int32());
	m_position = iff.read_floatVector();
	m_radius = iff.read_float();
}

// ----------------------------------------------------------------------

void PathNode::write ( Iff & iff ) const
{
	iff.insertChunkData( m_index );
	iff.insertChunkData( m_id );
	iff.insertChunkData( m_key );
	iff.insertChunkData( m_type );
	iff.insertChunkData( m_position );
	iff.insertChunkData( m_radius );
}

// ----------------------------------------------------------------------

bool PathNode::hasSubgraph ( void ) const
{
	if(m_type == PNT_BuildingCell) return true;
	if(m_type == PNT_BuildingCellPart) return true;
	if(m_type == PNT_CityBuilding) return true;

	return false;
}

// ----------------------------------------------------------------------

bool PathNode::isConcrete ( void ) const
{
	if(m_type == PNT_BuildingEntrance) return false;
	if(m_type == PNT_BuildingCell) return false;
	if(m_type == PNT_BuildingPortal) return false;
	if(m_type == PNT_CityBuildingEntrance) return false;
	if(m_type == PNT_CityBuilding) return false;
	if(m_type == PNT_Invalid) return false;

	return true;
}

// ======================================================================
