// ======================================================================
//
// AiLocation.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiLocation.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ServerObject.h"
#include "serverGame/ServerWorld.h"
#include "sharedCollision/CollisionUtils.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedObject/CellProperty.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/World.h"
#include "sharedTerrain/TerrainObject.h"


static int gs_debugCounter = -6000000;

const AiLocation AiLocation::invalid;

namespace AiLocationArchive
{
	// define some values for data combinations so we don't have to send
	// all the location data over the network
	enum AiLocationPackage
	{
		ALP_None,         // uninitialized location
		ALP_Cell,         // cell id, local position, radius
		ALP_World,        // world position, radius
		ALP_Object,       // object id
		ALP_OffsetObject, // object id, offset, relative flag
	};
}

#define AI_WARNING(A) WARNING(ConfigServerGame::getReportAiWarnings(),(A);

// ----------------------------------------------------------------------

AiLocation::AiLocation ( void )
:	m_valid(false),
	m_attached(false),
	m_object(nullptr),
	m_objectId(),
	m_cellObject(nullptr),
	m_position_p(Vector::zero),
	m_position_w(Vector::zero),
	m_radius(0.0f),
	m_offset_p(Vector::zero),
	m_relativeOffset(false),
	m_hasChanged(true),
	m_cuttable(true),
	m_debugId(gs_debugCounter++)
{
}

AiLocation::AiLocation ( CellProperty const * cell, Vector const & position, float radius )
:	m_valid(true),
	m_attached(false),
	m_object(nullptr),
	m_objectId(),
	m_cellObject(cell ? &cell->getOwner() : nullptr),
	m_position_p(position),
	m_position_w(CollisionUtils::transformToWorld(cell,position)),
	m_radius(radius),
	m_offset_p(Vector::zero),
	m_relativeOffset(false),
	m_hasChanged(true),
	m_cuttable(true),
	m_debugId(gs_debugCounter++)
{
	VALIDATE_LOCATION();
}

AiLocation::AiLocation ( NetworkId const & cellId, Vector const & position, float radius )
:	m_valid(true),
	m_attached(false),
	m_object(nullptr),
	m_objectId(),
	m_cellObject(nullptr),
	m_position_p(position),
	m_position_w(position),
	m_radius(radius),
	m_offset_p(Vector::zero),
	m_relativeOffset(false),
	m_hasChanged(true),
	m_cuttable(true),
	m_debugId(gs_debugCounter++)
{
	if(cellId.isValid())
	{
		m_cellObject = NetworkIdManager::getObjectById(cellId);
	}
	else
	{
		m_cellObject = &CellProperty::getWorldCellProperty()->getOwner();
	}

	if(m_cellObject)
	{
		m_position_w = CollisionUtils::transformToWorld(m_cellObject->getCellProperty(),m_position_p);
	}

	VALIDATE_LOCATION();
}

AiLocation::AiLocation ( Object const * object )
:	m_valid(true),
	m_attached(true),
	m_object(nullptr),
	m_objectId(),
	m_cellObject(nullptr),
	m_position_p(Vector::zero),
	m_position_w(Vector::zero),
	m_radius(0.0f),
	m_offset_p(Vector::zero),
	m_relativeOffset(false),
	m_hasChanged(true),
	m_cuttable(true),
	m_debugId(gs_debugCounter++)
{
	setObject(object);
	VALIDATE_LOCATION();
}

AiLocation::AiLocation ( NetworkId const & objectId )
:	m_valid(true),
	m_attached(true),
	m_object(nullptr),
	m_objectId(),
	m_cellObject(nullptr),
	m_position_p(Vector::zero),
	m_position_w(Vector::zero),
	m_radius(0.0f),
	m_offset_p(Vector::zero),
	m_relativeOffset(false),
	m_hasChanged(true),
	m_cuttable(true),
	m_debugId(gs_debugCounter++)
{
	setObject(NetworkIdManager::getObjectById(objectId));
	VALIDATE_LOCATION();
}

AiLocation::AiLocation ( Object const * object, Vector const & offset, bool relativeOffset )
:	m_valid(true),
	m_attached(true),
	m_object(nullptr),
	m_objectId(),
	m_cellObject(nullptr),
	m_position_p(Vector::zero),
	m_position_w(Vector::zero),
	m_radius(0.0f),
	m_offset_p(offset),
	m_relativeOffset(relativeOffset),
	m_hasChanged(true),
	m_cuttable(true),
	m_debugId(gs_debugCounter++)
{
	setObject(object);
	VALIDATE_LOCATION();
}

AiLocation::AiLocation ( NetworkId const & objectId, Vector const & offset, bool relativeOffset )
:	m_valid(true),
	m_attached(true),
	m_object(nullptr),
	m_objectId(),
	m_cellObject(nullptr),
	m_position_p(Vector::zero),
	m_position_w(Vector::zero),
	m_radius(0.0f),
	m_offset_p(offset),
	m_relativeOffset(relativeOffset),
	m_hasChanged(true),
	m_cuttable(true),
	m_debugId(gs_debugCounter++)
{
	setObject(NetworkIdManager::getObjectById(objectId));
	VALIDATE_LOCATION();
}

// ----------

AiLocation::AiLocation ( AiLocation const & loc )
:	m_valid(loc.m_valid),
	m_attached(loc.m_attached),
	m_object(loc.m_object),
	m_objectId(loc.m_objectId),
	m_cellObject(loc.m_cellObject),
	m_position_p(loc.m_position_p),
	m_position_w(loc.m_position_w),
	m_radius(loc.m_radius),
	m_offset_p(loc.m_offset_p),
	m_relativeOffset(loc.m_relativeOffset),
	m_hasChanged(true),
	m_cuttable(loc.m_cuttable),
	m_debugId(gs_debugCounter++)
{
}

// ----------

AiLocation::AiLocation ( Archive::ReadIterator & source )
:	m_valid(),
	m_attached(),
	m_object(),
	m_objectId(),
	m_cellObject(),
	m_position_p(),
	m_position_w(),
	m_radius(),
	m_offset_p(),
	m_relativeOffset(),
	m_hasChanged(),
	m_cuttable(),
	m_debugId()
{
	unpack(source);
}

// ----------

AiLocation::~AiLocation()
{
}

// ----------

AiLocation & AiLocation::operator = ( AiLocation const & loc )
{
	m_valid = loc.m_valid;
	m_attached = loc.m_attached;
	m_object = loc.m_object;
	m_objectId = loc.m_objectId;
	m_cellObject = loc.m_cellObject;
	m_position_p = loc.m_position_p;
	m_position_w = loc.m_position_w;
	m_radius = loc.m_radius;
	m_offset_p = loc.m_offset_p;
	m_relativeOffset = loc.m_relativeOffset;
	m_hasChanged = true;
	m_cuttable = loc.m_cuttable;

	VALIDATE_LOCATION();

	return *this;
}

// ----------

void AiLocation::pack(Archive::ByteStream & target) const
{
	Archive::put(target, m_valid);
	Archive::put(target, m_attached);
	Archive::put(target, m_relativeOffset);
	Archive::put(target, m_hasChanged);
	Archive::put(target, m_cuttable);
	Archive::put(target, m_debugId);
	Archive::put(target, m_radius);

	if (getObjectId() != NetworkId::cms_invalid)
	{
		if (getOffset() == Vector::zero)
		{
			Archive::put(target, static_cast<uint8>(AiLocationArchive::ALP_Object));
			Archive::put(target, getObjectId());
		}
		else
		{
			Archive::put(target, static_cast<uint8>(AiLocationArchive::ALP_OffsetObject));
			Archive::put(target, getObjectId());
			Archive::put(target, getOffset());
		}
	}
	else if (getCellId() != NetworkId::cms_invalid)
	{
		Archive::put(target, static_cast<uint8>(AiLocationArchive::ALP_Cell));
		Archive::put(target, getCellId());
		Archive::put(target, getPosition_p());
	}
	else
	{
		Archive::put(target, static_cast<uint8>(AiLocationArchive::ALP_World));
		Archive::put(target, getPosition_w());
	}
}

// ----------

void AiLocation::unpack( Archive::ReadIterator & source )
{
	Archive::get(source, m_valid);
	Archive::get(source, m_attached);
	Archive::get(source, m_relativeOffset);
	Archive::get(source, m_hasChanged);
	Archive::get(source, m_cuttable);
	Archive::get(source, m_debugId);
	Archive::get(source, m_radius);

	uint8 package;
	Archive::get(source, package);
	switch (static_cast<AiLocationArchive::AiLocationPackage>(package))
	{
		case AiLocationArchive::ALP_None:
			// do nothing
			break;
		case AiLocationArchive::ALP_Cell:
			{
				NetworkId cellId;
				Archive::get(source, cellId);
				Archive::get(source, m_position_p);
				m_position_w = m_position_p;
				if (cellId.isValid())
				{
					m_cellObject = NetworkIdManager::getObjectById(cellId);
				}
				else
				{
					m_cellObject = &CellProperty::getWorldCellProperty()->getOwner();
				}

				if (m_cellObject)
				{
					m_position_w = CollisionUtils::transformToWorld(m_cellObject->getCellProperty(),m_position_p);
				}
			}
			break;
		case AiLocationArchive::ALP_World:
			{
				Archive::get(source, m_position_w);
				m_position_p = m_position_w;
			}
			break;
		case AiLocationArchive::ALP_Object:
			{
				NetworkId objectId;
				Archive::get(source, objectId);
				setObject(NetworkIdManager::getObjectById(objectId));
			}
			break;
		case AiLocationArchive::ALP_OffsetObject:
			{
				NetworkId objectId;
				Archive::get(source, objectId);
				Archive::get(source, m_offset_p);
				setObject(NetworkIdManager::getObjectById(objectId));
			}
			break;
		default:
			WARNING(true, ("AiLocation::AiLocation(archive) received unknown data "
				"type %d", static_cast<int>(package)));
			break;
	}
	
	VALIDATE_LOCATION();	
}

// ----------

void AiLocation::setObject ( Object const * object )
{
	if(m_attached)
	{
		if (object == nullptr)
		{
			clear();
			return;
		}

		m_object = object;
		m_objectId = m_object->getNetworkId();
		m_position_p = m_object->getPosition_p();
		m_position_w = m_object->getPosition_w();
		m_cellObject = &(m_object->getParentCell()->getOwner());

		VALIDATE_LOCATION();
	}
}

// ----------------------------------------------------------------------

void AiLocation::detach ( void )
{
	m_object = nullptr;
	m_objectId = NetworkId::cms_invalid;
	m_attached = false;
}

// ----------------------------------------------------------------------

CellProperty const * AiLocation::getCell ( void ) const
{
	return m_cellObject ? m_cellObject->getCellProperty() : nullptr;
}

// ----------

NetworkId const & AiLocation::getCellId ( void ) const
{
	CellProperty const * cell = getCell();

	if(cell && (cell != CellProperty::getWorldCellProperty()))
	{
		return cell->getOwner().getNetworkId();
	}
	else
	{
		return NetworkId::cms_invalid;
	}
}

// ----------------------------------------------------------------------

bool AiLocation::hasChanged ( void ) const
{
	return m_hasChanged;
}

// ----------------------------------------------------------------------

Vector AiLocation::getPosition_p ( void ) const
{
	if(m_cellObject.getPointer() == nullptr)
	{
		WARNING(ConfigServerGame::getReportAiWarnings(),("AiLocation::getPosition_p - Locations's parent cell has disappeared\n"));
		
		AiLocation * self = const_cast<AiLocation*>(this);

		self->m_position_p = self->m_position_w;

		self->m_cellObject = &(CellProperty::getWorldCellProperty()->getOwner());

		VALIDATE_LOCATION();
	}

	return m_position_p;
}

// ----------

Vector AiLocation::getPosition_w ( void ) const
{
	return m_position_w;
}

// ----------

Vector AiLocation::getPosition_p ( CellProperty const * relativeCell ) const
{
	Vector pos_p = getPosition_p();

	return CollisionUtils::transformToCell(getCell(),pos_p,relativeCell);
}

// ----------

void AiLocation::setPosition_p ( Vector const & position_p )
{
	NAN_CHECK(position_p);

	m_position_p = position_p;
	m_position_w = CollisionUtils::transformToWorld(getCell(),position_p);
}

// ----------

void AiLocation::setPosition_w ( Vector const & position_w )
{
	NAN_CHECK(position_w);

	m_position_p = CollisionUtils::transformFromWorld(position_w,getCell());
	m_position_w = position_w;
}

// ----------------------------------------------------------------------

Vector AiLocation::getOffsetPosition_p ( void ) const
{
	Vector pos_p = getPosition_p();

	Vector offset = m_relativeOffset ? m_object->rotate_o2p(m_offset_p) : m_offset_p;

	return pos_p + offset;
}

// ----------

Vector AiLocation::getOffsetPosition_w ( void ) const
{
	CellProperty const * cell = getCell();

	if(cell)
	{
		return cell->getOwner().getTransform_o2w().rotateTranslate_l2p(getOffsetPosition_p());
	}
	else
	{
		return getOffsetPosition_p();
	}
}

// ----------------------------------------------------------------------

void AiLocation::setOffset_p(Vector const & offset, bool relative)
{
	m_offset_p = offset;
	m_relativeOffset = relative;
	m_hasChanged = true;
}

// ----------------------------------------------------------------------

float AiLocation::getRadius ( void ) const
{
	return m_radius;
}

// ----------------------------------------------------------------------

void AiLocation::update ( void )
{
	if(!m_attached) return;

	if(!m_object)
	{
		// object this location refers to has disappeared

		int id = static_cast<int>(m_objectId.getValue());

		WARNING(ConfigServerGame::getReportAiWarnings(),("AiLocation::update - The object this location was attached (%d) to has disappeared\n",id));
		m_attached = false;
		m_valid = false;
		return;
	}

	if(!m_cellObject)
	{
		WARNING(ConfigServerGame::getReportAiWarnings(),("AiLocation::getPosition_p - Location at (%f,%f,%f) - parent cell has disappeared\n",m_position_w.x,m_position_w.y,m_position_w.z));

		m_position_p = m_position_w;
		m_cellObject = &(CellProperty::getWorldCellProperty()->getOwner());
	}

	Vector const offset = m_relativeOffset ? m_object->rotate_o2p(m_offset_p) : m_offset_p;
	Vector const newPosition_p = m_object->getPosition_p() + offset;
	float const positionDeltaSquared = newPosition_p.magnitudeBetweenSquared(m_position_p);
	CellProperty const * const objectCell = m_object->getParentCell();

	if ((positionDeltaSquared > 0.00001f) || (getCell() != objectCell))
	{
		m_position_p = newPosition_p;
		m_hasChanged = true;
		m_cellObject = &(m_object->getParentCell()->getOwner());
		m_position_w = CollisionUtils::transformToWorld(getCell(), m_position_p);
	}

	VALIDATE_LOCATION();
}

// ----------------------------------------------------------------------

void AiLocation::clear ( void )
{
	// Note that this explicitly does _not_ clear m_objectId - if an
	// AiLocation is detached from its object because the object was 
	// deleted, the behaviors will still need to know the now-invalid 
	// NetworkId so they can send it to the scripts.

	m_valid = false;
	m_attached = false;
	m_object = nullptr;
	m_cellObject = nullptr;
	m_position_p = Vector::zero;
	m_radius = 0.0f;
	m_offset_p = Vector::zero;
	m_relativeOffset = false;
	m_hasChanged = true;
}

// ----------------------------------------------------------------------

bool AiLocation::isInWorldCell ( void ) const
{
	return isValid() && ( (getCell() == nullptr) || (getCell() == CellProperty::getWorldCellProperty()) );

}

// ----------------------------------------------------------------------

bool AiLocation::validate ( void ) const
{
	NAN_CHECK(m_position_p);
	NAN_CHECK(m_radius);
	NAN_CHECK(m_offset_p);

	TerrainObject * terrain = TerrainObject::getInstance();

	if(terrain == nullptr) return true;

	float w = terrain->getMapWidthInMeters() / 2.0f;

	bool valid = true;
	
	valid &= (m_position_p.x <=  w);
	valid &= (m_position_p.x >= -w);

	valid &= (m_position_p.z <=  w);
	valid &= (m_position_p.z >= -w);

	float dist2 = getPosition_w().magnitudeSquared();

	valid &= (dist2 >= 9.0f);

	if(!valid)
	{
		DEBUG_WARNING(ConfigServerGame::getReportAiWarnings(),("AiLocation::validate - Location (%f,%f,%f) is OOW or too near the origin - terrain size %fx%f\n",m_position_p.x,m_position_p.y,m_position_p.z,w*2.0f,w*2.0f));
	}

	return valid;
}

// ----------------------------------------------------------------------

void AiLocation::getDebugInfo( std::string & outString ) const
{
	Vector const & w = getPosition_w();
	outString += FormattedString<1024>().sprintf("world location: %.2f, %.2f, %.2f\n", w.x, w.y, w.z);
}

// ----------------------------------------------------------------------

Vector const & AiLocation::getOffset_p() const
{
	return m_offset_p;
}
