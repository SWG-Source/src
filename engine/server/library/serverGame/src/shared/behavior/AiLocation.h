// ======================================================================
//
// AiLocation.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_AiLocation_H
#define	INCLUDED_AiLocation_H

#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Watcher.h"
#include "sharedMath/Vector.h"
#include "sharedObject/Object.h"

namespace Archive
{
	class ByteStream;
	class ReadIterator;
}

class CellProperty;

#ifdef _DEBUG
#define VALIDATE_LOCATION() validate()
#else
#define VALIDATE_LOCATION() NOP
#endif

// ======================================================================

class AiLocation
{
public:

	AiLocation ( void );
	AiLocation ( CellProperty const * cell, Vector const & point, float radius = 0.0f );
	AiLocation ( NetworkId const & cellId, Vector const & point, float radius = 0.0f );
	AiLocation ( Object const * target );
	AiLocation ( NetworkId const & targetId );
	AiLocation ( Object const * target, Vector const & offset, bool relativeOffset );
	AiLocation ( NetworkId const & targetId, Vector const & offset, bool relativeOffset );
	AiLocation ( AiLocation const & copy );
	AiLocation ( Archive::ReadIterator & source );
	
	~AiLocation();

	static const AiLocation invalid;

	AiLocation & operator = ( AiLocation const & copy );
	
	void                 initialize     ( void );
	
	void                 pack           ( Archive::ByteStream & target ) const;
	void                 unpack         ( Archive::ReadIterator & source );

	// ----------
	
	bool                 isValid        ( void ) const;
	bool                 hasChanged     ( void ) const;
	void                 setChanged     ( bool changed );
	
	Object const *       getObject      ( void ) const;
	void                 setObject      ( Object const * object );

	void                 detach         ( void );

	NetworkId const &    getObjectId    ( void ) const;
	
	CellProperty const * getCell        ( void ) const;
	NetworkId const &    getCellId      ( void ) const;
	
	Vector               getPosition_p  ( void ) const;
	Vector               getPosition_w  ( void ) const;

	Vector               getPosition_p  ( CellProperty const * relativeCell ) const;

	void                 setPosition_p  ( Vector const & position_p );
	void                 setPosition_w  ( Vector const & position_w );

	Vector               getOffsetPosition_p ( void ) const;
	Vector               getOffsetPosition_w ( void ) const;
	Vector const &       getOffset_p() const;
	void                 setOffset_p(Vector const & offset, bool relative);

	float                getRadius      ( void ) const;
	void                 setRadius      ( float newRadius );

	Vector               getOffset      ( void ) const;
	void                 setOffset      ( Vector const & newOffset );

	bool                 getRelativeOffset ( void ) const;

	void                 update         ( void );
	void                 clear          ( void );

	bool                 isInWorldCell  ( void ) const;

	bool                 isCuttable     ( void ) const;
	void                 setCuttable    ( bool cuttable );

	int                  getDebugId     ( void ) const;
	void                 getDebugInfo   ( std::string & outString ) const;

	// ----------

protected:

	bool                 validate       ( void ) const;

	typedef ConstWatcher<Object> ObjectWatcher;

	bool                 m_valid;
	bool                 m_attached;

	ObjectWatcher        m_object;
	NetworkId            m_objectId;
	
	ObjectWatcher        m_cellObject;
	Vector               m_position_p;
	Vector               m_position_w;
	float                m_radius;
	Vector               m_offset_p;
	bool                 m_relativeOffset;

	bool                 m_hasChanged;
	
	bool                 m_cuttable;

	int                  m_debugId;
};

// ----------------------------------------------------------------------

inline bool AiLocation::isValid ( void ) const
{
	return m_valid;
}

inline void AiLocation::setChanged( bool changed )
{
	m_hasChanged = changed;
}

inline Object const * AiLocation::getObject ( void ) const
{
	return m_object;
}

inline NetworkId const & AiLocation::getObjectId ( void ) const
{
	return m_objectId;
}

inline void AiLocation::setRadius ( float newRadius )
{
	m_radius = newRadius;
}

inline Vector AiLocation::getOffset ( void ) const
{
	return m_offset_p;
}

inline void AiLocation::setOffset ( Vector const & newOffset )
{
	m_offset_p = newOffset;
}

inline bool AiLocation::getRelativeOffset ( void ) const
{
	return m_relativeOffset;
}

inline bool AiLocation::isCuttable ( void ) const
{
	return m_cuttable;
}

inline void AiLocation::setCuttable ( bool cuttable )
{
	m_cuttable = cuttable;
}

inline int AiLocation::getDebugId ( void ) const
{
	return m_debugId;
}


// ======================================================================


#endif

