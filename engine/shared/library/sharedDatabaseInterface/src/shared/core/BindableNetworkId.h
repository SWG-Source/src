// ======================================================================
//
// BindableNetworkId.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_BindableNetworkId_H
#define INCLUDED_BindableNetworkId_H

// ======================================================================

#include "sharedDatabaseInterface/DbBindableInt64.h"
#include "sharedFoundation/NetworkId.h"

// ======================================================================

namespace DB
{

	/**
	 * A Bindable type for an object Id.
	 * This class is intended to provide a layer between NetworkId and
	 * the database type, so that if the implementation of NetworkId is
	 * changed, only this class and the database schema need to be changed.
	 *
	 * Also, this class allows the code for persisting NetworkId's to follow
	 * the same form as the code persisting other types.
	 */
	class BindableNetworkId:public BindableInt64
	{
	  public:
		BindableNetworkId (const NetworkId &rhs);
		BindableNetworkId();
		
		BindableNetworkId &operator=(const NetworkId &rhs);

		void getValue(NetworkId &buffer) const;
		NetworkId getValue() const;

		void setValue(const NetworkId &buffer);
	};
}

// ======================================================================

inline DB::BindableNetworkId &DB::BindableNetworkId::operator=(const NetworkId &rhs)
{
	BindableInt64::setValue(rhs.getValue());
	return *this;
}

// ----------------------------------------------------------------------

inline void DB::BindableNetworkId::getValue(NetworkId &buffer) const
{
	if (isNull())
		buffer.m_value = NetworkId::cms_invalid.getValue();
	else
		buffer.m_value = BindableInt64::getValue();
}

// ----------------------------------------------------------------------

inline NetworkId DB::BindableNetworkId::getValue() const
{
	if (isNull())
		return NetworkId::cms_invalid;
	else
		return NetworkId(BindableInt64::getValue());
}

// ======================================================================

#endif
