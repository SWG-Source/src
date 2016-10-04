// ======================================================================
//
// CachedNetworkId.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================


#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/CachedNetworkId.h"

#include "sharedObject/NetworkIdManager.h"

#include <string>

const CachedNetworkId CachedNetworkId::cms_cachedInvalid (NetworkId::cms_invalid);
const NetworkId CachedNetworkId::cms_invalid(static_cast<NetworkId::NetworkIdType>(0));

//----------------------------------------------------------------------

void CachedNetworkId::checkValidity() const
{
	if (!m_object)
		return;

	DEBUG_FATAL(*this == NetworkId::cms_invalid, ("Invalid 0 network id"));
	DEBUG_FATAL(*this != m_object->getNetworkId(), ("Cached validity check failed"));
}

//----------------------------------------------------------------------

CachedNetworkId::CachedNetworkId() : 
m_id(cms_invalid),
m_object(nullptr)
{
}

// ----------------------------------------------------------

CachedNetworkId::CachedNetworkId(const NetworkId& id) : 
m_id(id),
m_object(nullptr)
{
}

// ----------------------------------------------------------

CachedNetworkId::CachedNetworkId(NetworkId::NetworkIdType value) : 
m_id(value),
m_object(nullptr)
{
		
}

// ----------------------------------------------------------

CachedNetworkId::CachedNetworkId(const Object& object) : 
m_id(object.getNetworkId()),
m_object(const_cast<Object*>(&object))
{
	
}

// ----------------------------------------------------------

CachedNetworkId::CachedNetworkId(const std::string &value) :
m_id(value),
m_object(nullptr)
{
		
}

// ----------------------------------------------------------

CachedNetworkId::CachedNetworkId(const CachedNetworkId& rhs) : 
m_id(rhs.m_id),
m_object(rhs.m_object)
{
	
}

// ----------------------------------------------------------

CachedNetworkId& CachedNetworkId::operator= (const CachedNetworkId& rhs)
{
	if (this == &rhs)
		return *this;
	
	m_id = rhs.m_id;

	m_object = rhs.m_object.getPointer();
	return *this;
}

// ----------------------------------------------------------

CachedNetworkId& CachedNetworkId::operator= (const NetworkId& rhs)
{
	m_id = rhs;
	m_object = nullptr;
	return *this;
}
// ----------------------------------------------------------

CachedNetworkId& CachedNetworkId::operator= (const Object& object)
{
	m_id = object.getNetworkId();
	m_object = const_cast<Object*>(&object);
	return *this;
}

// ----------------------------------------------------------

CachedNetworkId::operator NetworkId const & ( void ) const
{
	return m_id;
}

bool CachedNetworkId::operator == ( NetworkId const & id ) const
{
	return m_id == id;
}

bool CachedNetworkId::operator != ( NetworkId const & id ) const
{
	return m_id != id;
}

NetworkId::NetworkIdType CachedNetworkId::getValue() const
{
	return m_id.getValue();
}

std::string CachedNetworkId::getValueString() const
{
	return m_id.getValueString();
}

bool CachedNetworkId::operator < (const NetworkId& rhs) const
{
	return m_id < rhs;
}

size_t CachedNetworkId::getHashValue() const
{
	return m_id.getHashValue();
}

bool CachedNetworkId::isValid() const
{
	return m_id.isValid();
}

bool CachedNetworkId::operator !() const
{
	return m_id.operator !();
}



// ----------------------------------------------------------

CachedNetworkId::~CachedNetworkId() 
{
}

// ----------------------------------------------------------

Object* CachedNetworkId::getObject() const
{
	if (isValid()) // don't do a hash lookup if m_value == 0
	{
		if (m_object)
			return m_object;
		m_object = NetworkIdManager::getObjectById(m_id);

		return m_object;
	}

	return nullptr;
}

// ----------------------------------------------------------
