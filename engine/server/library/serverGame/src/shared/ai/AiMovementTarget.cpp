// ======================================================================
//
// AiMovementTarget.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/AiMovementTarget.h"

#include "serverGame/AiMovementArchive.h"
#include "serverGame/AiCreatureController.h"
#include "serverGame/CreatureObject.h"


// ======================================================================

AiMovementTarget::AiMovementTarget( AICreatureController * controller )
:	AiMovementPathFollow(controller),
	m_target()
{
}

// ----------

AiMovementTarget::AiMovementTarget( AICreatureController * controller, ServerObject const * target )
:	AiMovementPathFollow(controller),
	m_target(target)
{
}

// ----------

AiMovementTarget::AiMovementTarget( AICreatureController * controller, CellProperty const * cell, Vector const & target, float radius )
: AiMovementPathFollow(controller),
  m_target(cell, target, radius)
{
}

// ----------

AiMovementTarget::AiMovementTarget( AICreatureController * controller, Archive::ReadIterator & source )
:	AiMovementPathFollow(controller, source),
	m_target(source)
{
}

// ----------

AiMovementTarget::~AiMovementTarget()
{
}

// ----------------------------------------------------------------------

void AiMovementTarget::pack( Archive::ByteStream & target ) const
{
	AiMovementPathFollow::pack(target);
	m_target.pack(target);
}

// ----------------------------------------------------------------------

/**
 * Decide whether we can hibernate or not. If we are targetting a player or
 * a creature who isn't hibernating, don't hibernate.
 */
bool AiMovementTarget::getHibernateOk( void ) const
{
static const int MAX_RECURSION = 50;
static int recursionCount = 0;
static const AiMovementTarget * preventRecurse = nullptr;

	// prevent recursing too many times
	if (++recursionCount == MAX_RECURSION)
	{
		WARNING_STRICT_FATAL(true, ("AiMovementTarget::getHibernateOk has recursed "
			"too many times!"));
		--recursionCount;
		return true;
	}

	// there might be a chance that calling getHibernate() on the creature we
	// are folloing will eventually lead back to us, so use the preventRecurse
	// variable to stop that from happening
	if (preventRecurse == this)
	{
		WARNING_STRICT_FATAL(true, ("AiMovementTarget::getHibernateOk has recursed "
			"back on itself!"));
		--recursionCount;
		return true;
	}
	if (preventRecurse == nullptr)
		preventRecurse = this;
	
	bool hibernate = true;
	if (m_target.isValid() && m_target.getObjectId() != NetworkId::cms_invalid)
	{
		const Object * o = m_target.getObject();
		if (o != nullptr && o->asServerObject() != nullptr)
		{
			// if the target is a player, don't hibernate
			if (o->asServerObject()->isPlayerControlled())
				hibernate = false;
			// hibernate if who we're following is hibernating
			else if (o->asServerObject()->asCreatureObject() != nullptr)
				hibernate = (safe_cast<const CreatureController *>(o->getController()))->getHibernate();
		}
		else
			hibernate = AiMovementPathFollow::getHibernateOk();
	}
	else
		hibernate = AiMovementPathFollow::getHibernateOk();

	// clean up recusion checkers
	if (preventRecurse == this)
		preventRecurse = nullptr;
	--recursionCount;
	return hibernate;
}

// ----------------------------------------------------------------------

AiLocation const & AiMovementTarget::getTarget() const
{
	return m_target;
}

// ======================================================================

