// ======================================================================
//
// CachedNetworkId.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_CachedNetworkId_H
#define INCLUDED_CachedNetworkId_H

// ======================================================================

#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/Watcher.h"

#include "sharedObject/Object.h"

class CachedNetworkId;

// ======================================================================

class CachedNetworkId
{
public:
	
	static const CachedNetworkId cms_cachedInvalid;
	static const NetworkId cms_invalid;

public:

	                         CachedNetworkId ();
	explicit                 CachedNetworkId (const NetworkId& id);
	explicit                 CachedNetworkId (NetworkId::NetworkIdType value);
	explicit                 CachedNetworkId (const Object &object);
	explicit                 CachedNetworkId (const std::string &value);
	                         CachedNetworkId (const CachedNetworkId&);

	                        ~CachedNetworkId();

	CachedNetworkId&         operator=       (const CachedNetworkId&);
	CachedNetworkId&         operator=       (const NetworkId&);
	CachedNetworkId&         operator=       (const Object& object);

	operator NetworkId const & ( void ) const;

	bool                     operator == ( NetworkId const & id ) const;
	bool                     operator != ( NetworkId const & id ) const;
	bool                     operator < (const NetworkId& rhs) const;
	bool                     operator !() const;

	NetworkId::NetworkIdType getValue() const;
	std::string              getValueString() const;
	size_t                   getHashValue() const;

	size_t operator()(const CachedNetworkId & x) const;

	Object*                  getObject() const;

	bool                     isValid() const;
	void                     checkValidity() const;

private:

	NetworkId                m_id;

	mutable Watcher<Object>  m_object;
};

//----------------------------------------------------------------------

inline size_t CachedNetworkId::operator()(const CachedNetworkId & x) const
{
	return x.getHashValue();
}

#endif

