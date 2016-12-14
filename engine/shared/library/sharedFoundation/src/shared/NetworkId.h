// ======================================================================
//
// NetworkId.h
// Portions copyright 1999 Bootprint Entertainment
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_NetworkId_H
#define INCLUDED_NetworkId_H

namespace DB
{
	class BindableNetworkId;
}

// ======================================================================

class NetworkId
{
public:

	typedef int64 NetworkIdType;

	static const NetworkId cms_invalid;
	static const NetworkIdType cms_maxNetworkIdWithoutClusterId;
	static const NetworkIdType cms_networkIdWithoutClusterIdMask;
	static const NetworkIdType cms_clusterIdMask;
	struct Hash;
	friend struct NetworkId::Hash;

public:

	NetworkId();
	explicit NetworkId(NetworkIdType value);
	explicit NetworkId(const std::string &value);
	NetworkId(const NetworkId&);

	~NetworkId();

	NetworkIdType       getValueWithClusterId(uint8 clusterId, NetworkIdType validForConversionMin, NetworkIdType validForConversionMax) const;
	NetworkIdType       getValueWithoutClusterId() const;
	uint8               getValueClusterId() const;
	NetworkIdType       getValue() const;
	std::string         getValueString() const;

	NetworkId& operator= (const NetworkId&);
	bool       operator< (const NetworkId& rhs) const;
	bool       operator> (const NetworkId& rhs) const;
	bool       operator== (const NetworkId& rhs) const;
	bool       operator!= (const NetworkId& rhs) const;
	size_t     getHashValue() const;

	bool isValid() const;
	bool operator !() const;

	struct Hash
	{
		size_t operator()(const NetworkId & source) const
		{
			return source.getHashValue(); //TODO:  make better hash function
		};
	};

private:
	NetworkIdType       m_value;

	friend class DB::BindableNetworkId;

  private:
	explicit NetworkId(int value); // catch people who incorrectly try to construct one of these from an int
	// also makes constructing one of these from any other incorrect numeric type ambiguous
};

// -----------------------------------------------------------------------

inline NetworkId::NetworkIdType NetworkId::getValue() const
{
	return m_value;
}

// -----------------------------------------------------------------------

inline NetworkId& NetworkId::operator= (const NetworkId& rhs)
{
	m_value = rhs.m_value;
	return *this;
}

// -----------------------------------------------------------------------

inline bool NetworkId::operator< (const NetworkId& rhs) const
{
	return m_value < rhs.m_value;
}

inline bool NetworkId::operator> (const NetworkId& rhs) const
{
        return m_value > rhs.m_value;
}

// -----------------------------------------------------------------------

inline bool NetworkId::operator== (const NetworkId& rhs) const
{
	return m_value == rhs.m_value;
}

// -----------------------------------------------------------------------

inline bool NetworkId::operator!= (const NetworkId& rhs) const
{
	return m_value != rhs.m_value;
}

// -----------------------------------------------------------------------

inline bool NetworkId::isValid () const
{
	return m_value != 0;
}

// -----------------------------------------------------------------------

inline bool NetworkId::operator! () const
{
	return m_value == 0;
}

//------------------------------------------------------------------------

inline size_t NetworkId::getHashValue () const
{
	static uint32 const cs_randomValues[] = 
	{
		0x904618ec,
		0x95c2f264,
		0xe55c30fe,
		0x2c92d2d1,
		0x8376930b,
		0x17662d70,
		0x2a785b31,
		0x1e7c7281,
		0x60bc4645,
		0x9a0cdf8f,
		0xa9bc133d,
		0x0e9a17ec,
		0xcd768a0d,
		0xe03e104f,
		0xecfab1bd,
		0x764a3a6f
	};

	uint32 a = static_cast< uint32 >( m_value >> 32 );
	uint32 b = static_cast< uint32 >( m_value & 0xffffffff );
	return ( ( b * 0x10001 ) ^ a ) ^ cs_randomValues[ b & 0xf ] ; // multiply lower 32 by a large prime then XOR a then XOR some random value from table
}

// -----------------------------------------------------------------------

// STL standard hash function
namespace std
{

template <>
struct hash<NetworkId>  // stl standard hash
{
  size_t operator()(const NetworkId &x) const { return x.getHashValue(); }
};

}

#endif
