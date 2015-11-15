// ==================================================================
//
// ServerWeatherMessage.h
// Copyright 2002 Sony Online Entertainment
//
// ==================================================================

#ifndef	INCLUDED_ServerWeatherMessage_H
#define	INCLUDED_ServerWeatherMessage_H

// ==================================================================

#include "Archive/AutoDeltaByteStream.h"
#include "sharedMath/Vector.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

// ==================================================================

class ServerWeatherMessage : public GameNetworkMessage
{
public: 

	ServerWeatherMessage (int index, const Vector& windVelocity_w);
	explicit ServerWeatherMessage (Archive::ReadIterator &source);
	virtual ~ServerWeatherMessage();

	int           getIndex () const;
	const Vector& getWindVelocity_w () const;

public:

	static const char* const cms_name;

private:

	Archive::AutoVariable<int>    m_index;
	Archive::AutoVariable<Vector> m_windVelocity_w;
};

// ==================================================================

#endif 
