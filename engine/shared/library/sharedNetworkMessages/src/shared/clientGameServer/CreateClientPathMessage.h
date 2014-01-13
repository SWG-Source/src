// ==================================================================
//
// CreateClientPathMessage.h
// Copyright 2003, Sony Online Entertainment
//
// ==================================================================

#ifndef	INCLUDED_CreateClientPathMessage_H
#define	INCLUDED_CreateClientPathMessage_H

// ==================================================================

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedMath/Vector.h"

// ==================================================================

class CreateClientPathMessage : public GameNetworkMessage
{
public: 

	explicit CreateClientPathMessage (std::vector<Vector> const & pointList);
	explicit CreateClientPathMessage (Archive::ReadIterator & source);
	virtual ~CreateClientPathMessage ();

	std::vector<Vector> const & getPointList () const;

private:

	CreateClientPathMessage ();

public:

	static char const * const cms_name;

private:

	Archive::AutoArray<Vector> m_pointList;
};

// ==================================================================

#endif 
