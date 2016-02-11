// ======================================================================
// 
// Formation.cpp
// Copyright Sony Online Entertainment, Inc.
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/Formation.h"

#include "serverGame/ConfigServerGame.h"
#include "serverGame/Squad.h"
#include "sharedCollision/CollisionProperty.h"
#include "sharedLog/Log.h"

// ======================================================================
//
// Formation
//
// ======================================================================

// ----------------------------------------------------------------------
Vector const Formation::getPosition_w(Transform const & transform_l2w, Vector const & position_l)
{
	Transform transform(transform_l2w);

	Vector const & localFrameI_p = transform_l2w.getLocalFrameI_p();
	Vector const & localFrameK_p = transform_l2w.getLocalFrameK_p();
	float const iPhi = localFrameI_p.phi();
	float const kPhi = localFrameK_p.phi();

	transform.roll_l(iPhi);
	transform.pitch_l(-kPhi);

	return transform.rotateTranslate_l2p(position_l);
}

#ifdef _DEBUG
// ----------------------------------------------------------------------
char const * const Formation::getShapeString(Formation::Shape const shape)
{
	switch (shape)
	{
		case Formation::S_claw: { return "claw"; }
		case Formation::S_wall: { return "wall"; }
		case Formation::S_sphere: { return "sphere"; }
		case Formation::S_delta: { return "delta"; }
		case Formation::S_broad: { return "broad"; }
		case Formation::S_x: { return "x"; }
		case Formation::S_count: { break; }
		case Formation::S_invalid: { break; }
		default: { break; }
	}

	return "invalid";
}
#endif // _DEBUG

// ----------------------------------------------------------------------
Formation::Formation()
 : m_shape(S_broad)
 , m_spacing(1.0f)
 , m_transform()
 , m_dirty(true)
{
}

// ----------------------------------------------------------------------
void Formation::markDirty()
{
	m_dirty = true;
}

// ----------------------------------------------------------------------
bool Formation::isDirty() const
{
	return m_dirty;
}

// ----------------------------------------------------------------------
void Formation::setShape(Shape const shape)
{
#ifdef _DEBUG
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("Formation::setShape() shape(%s)", getShapeString(shape)));
#endif

	m_dirty = true;
	m_shape = shape;
}

// ----------------------------------------------------------------------
Formation::Shape Formation::getShape() const
{
	return m_shape;
}

// ----------------------------------------------------------------------
void Formation::setSpacing(float const spacing)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("Formation::setSpacing() spacing(%.2f)", spacing));

	m_spacing = spacing;
}

// ----------------------------------------------------------------------
float Formation::getSpacing() const
{
	return m_spacing;
}

// ----------------------------------------------------------------------
void Formation::setTransform(Transform const & transform)
{
	LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("Formation::setTransform()"));

	m_transform = transform;
}

// ----------------------------------------------------------------------
Transform const & Formation::getTransform() const
{
	return m_transform;
}

// ----------------------------------------------------------------------
void Formation::build(Squad & squad)
{
	DEBUG_FATAL(squad.isEmpty(), ("Building a formation on an empty squad."));

	Object const * const leaderObject = squad.getLeader().getObject();
	CollisionProperty const * const leaderCollisionProperty = (leaderObject != nullptr) ? leaderObject->getCollisionProperty() : nullptr;
	float const leaderRadius = (leaderCollisionProperty != nullptr) ? (leaderCollisionProperty->getBoundingSphere_l().getRadius() * 2.0f) : 1.0f;

	int slotIndex = 0;
	Squad::UnitMap const & unitSet = squad.getUnitMap();
	Squad::UnitMap::const_iterator iterUnitMap = unitSet.begin();

	for (; iterUnitMap != unitSet.end(); ++iterUnitMap)
	{
		Vector const position_l(getPosition_l(squad, slotIndex));
		LOGC((ConfigServerGame::isSpaceAiLoggingEnabled() && (leaderRadius <= 1.0f)), "space_debug_ai", ("Formation::buildFormation() Unable to get the radius of the squad leader."));
		
		// Set the unit's local-space formation position

		NetworkId const & unit = iterUnitMap->first;

		squad.setUnitFormationPosition_l(unit, position_l * leaderRadius);

		++slotIndex;
	}

	m_dirty = false;
}

// ----------------------------------------------------------------------
Vector Formation::getPosition_l(Squad const & squad, int const slotIndex) const
{
	Vector result;

	switch (m_shape)
	{
		case Formation::S_x:
		case Formation::S_claw:
			{
				float const rowSign = ((slotIndex % 2) == 0) ? 1.0f : -1.0f;
				float const row = ceilf(static_cast<float>(slotIndex - 1) / 4.0f) * rowSign;
				float const columnSign = ((static_cast<int>(floorf(static_cast<float>(slotIndex) / 2.0f)) % 2) == 0) ? 1.0f : -1.0f;
				float const column = floorf(static_cast<float>(slotIndex) / 2.0f) * columnSign;
				
				float const xPercent = acosf(PI_OVER_4) / 2.0f;
				float const yPercent = asinf(PI_OVER_4) / 2.0f;
				if (m_shape == Formation::S_x)
				{
					result = Vector(column * xPercent, row * yPercent, 0.0f);
				}
				else
				{
					float const depth = floorf(static_cast<float>(slotIndex - 1) / 4.0f);
					result = Vector(column * xPercent, row * yPercent, depth);
				}
			}
			break;
		case Formation::S_wall:
			{
				int const unitsPerRow = static_cast<int>(ceilf(sqrtf(static_cast<float>(squad.getUnitCount()))));
				float const rowSign = ((static_cast<int>(floorf(static_cast<float>(slotIndex - 1) / static_cast<float>(unitsPerRow))) % 2) == 0) ? 1.0f : -1.0f;
				float const row = floorf(static_cast<float>(slotIndex + (unitsPerRow - 1)) / (static_cast<float>(unitsPerRow) * 2.0f)) * rowSign;
				float const columnSign = ((slotIndex % 2) == 0) ? 1.0f : -1.0f;
				float const column = ceilf(static_cast<float>((slotIndex - 1) % unitsPerRow) / 2.0f) * columnSign;
				result = Vector(column, row, 0.0f);
			}
			break;
		case Formation::S_sphere:
		case Formation::S_delta:
		case Formation::S_broad:
			{
				int const unitsPerRow = 21;
				float const sign = ((slotIndex % 2) == 0) ? 1.0f : -1.0f;
				float const column = ceilf(static_cast<float>((slotIndex) % unitsPerRow) / 2.0f) * sign;
				float const depth = -floorf(static_cast<float>(slotIndex) / static_cast<float>(unitsPerRow));
				result = Vector(column, 0.0f, depth);
			}
			break;
		case Formation::S_invalid:
		case Formation::S_count:
		default:
			{
				FATAL(true, ("Invalid formation(%d) specified", static_cast<int>(m_shape)));
			}
			break;
	}

	//LOGC(ConfigServerGame::isSpaceAiLoggingEnabled(), "space_debug_ai", ("getFormationPosition_l() squadId(%d) formation(%d) slotIndex(%d) direction_l(%.2f, %.2f, %.2f)", m_squadId, static_cast<int>(m_shape), slotIndex, result.x, result.y, result.z));

	return result * m_spacing;
}

// ======================================================================
