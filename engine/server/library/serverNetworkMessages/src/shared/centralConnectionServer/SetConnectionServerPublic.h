// SetConnectionServerPublic.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_SetConnectionServerPublic_H
#define	_INCLUDED_SetConnectionServerPublic_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class SetConnectionServerPublic : public GameNetworkMessage
{
public:
	explicit SetConnectionServerPublic(const bool isPublic);
	explicit SetConnectionServerPublic(Archive::ReadIterator & source);
	~SetConnectionServerPublic();

	const bool  getIsPublic  () const;

private:
	SetConnectionServerPublic & operator = (const SetConnectionServerPublic & rhs);
	SetConnectionServerPublic(const SetConnectionServerPublic & source);

	Archive::AutoVariable<bool>  m_isPublic;
};

//-----------------------------------------------------------------------

inline const bool SetConnectionServerPublic::getIsPublic() const
{
	return m_isPublic.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_SetConnectionServerPublic_H
