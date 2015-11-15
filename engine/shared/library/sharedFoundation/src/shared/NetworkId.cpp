// ======================================================================
//
// NetworkId.cpp
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/NetworkId.h"

#include <string>
#include <cstdio>

// ======================================================================

const NetworkId NetworkId::cms_invalid(static_cast<NetworkIdType>(0));

// buildout network id occupies the sign bit in the 8 byte signed NetworkId (i.e. buildout network id is negative)
// 10000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000

// client side fake network id (in ClientObject.cpp) occupies the highest non-sign bit location in the 8 byte signed NetworkId
// 01000000 00000000 00000000 00000000 00000000 00000000 00000000 00000000

// the 8 bit cluster id occupies these bit locations in the 8 byte NetworkId
// 00111111 11000000 00000000 00000000 00000000 00000000 00000000 00000000

// other network id will occupy the remaining bits
// 00000000 00111111 11111111 11111111 11111111 11111111 11111111 11111111
const NetworkId::NetworkIdType NetworkId::cms_maxNetworkIdWithoutClusterId(0x3FFFFFFFFFFFFFll);
const NetworkId::NetworkIdType NetworkId::cms_networkIdWithoutClusterIdMask(0x403FFFFFFFFFFFFFll);
const NetworkId::NetworkIdType NetworkId::cms_clusterIdMask(0x3FC0000000000000ll);

// ======================================================================

NetworkId::NetworkId() :
m_value(0)
{
}

// -----------------------------------------------------------------------

NetworkId::NetworkId(NetworkIdType value) :
m_value(value)
{
}

// -----------------------------------------------------------------------

NetworkId::NetworkId(const NetworkId& source) :
m_value(source.getValue())
{
}

// -----------------------------------------------------------------------


NetworkId::~NetworkId()
{
}

// -----------------------------------------------------------------------

std::string NetworkId::getValueString() const
{
	char tmp[256];
	sprintf(tmp,INT64_FORMAT_SPECIFIER,m_value);
	return tmp;
}

// ----------------------------------------------------------------------

NetworkId::NetworkId(const std::string &value) :
	m_value(0)
{
	sscanf(value.c_str(),INT64_FORMAT_SPECIFIER,&m_value);
}

// ----------------------------------------------------------------------

NetworkId::NetworkIdType NetworkId::getValueWithClusterId(uint8 const clusterId, NetworkIdType const validForConversionMin, NetworkIdType const validForConversionMax) const
{
	if ((m_value < validForConversionMin) || (m_value > validForConversionMax) || (clusterId == 0) || (m_value == 0))
		return m_value;

	uint64 const uint64ClusterId = static_cast<uint64>(clusterId);
	return static_cast<NetworkId::NetworkIdType>((uint64ClusterId << 54) | static_cast<uint64>(m_value));
}

// ----------------------------------------------------------------------

NetworkId::NetworkIdType NetworkId::getValueWithoutClusterId() const
{
	if (m_value <= cms_maxNetworkIdWithoutClusterId)
		return m_value;

	return static_cast<NetworkId::NetworkIdType>(static_cast<uint64>(cms_networkIdWithoutClusterIdMask) & static_cast<uint64>(m_value));
}

// ----------------------------------------------------------------------

uint8 NetworkId::getValueClusterId() const
{
	if (m_value <= cms_maxNetworkIdWithoutClusterId)
		return 0;

	return static_cast<uint8>((static_cast<uint64>(cms_clusterIdMask) & static_cast<uint64>(m_value)) >> 54);
}	

// ======================================================================
