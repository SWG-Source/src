// GalaxyLoopTimesResponse.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_GalaxyLoopTimesResponse_H
#define	_INCLUDED_GalaxyLoopTimesResponse_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class GalaxyLoopTimesResponse : public GameNetworkMessage
{
public:
	GalaxyLoopTimesResponse(const unsigned long lastFrameMilliseconds, const unsigned long currentFrameMilliseconds);
	explicit GalaxyLoopTimesResponse(Archive::ReadIterator & source);
	~GalaxyLoopTimesResponse();

	const unsigned long  getCurrentFrameMilliseconds  () const;
	const unsigned long  getLastFrameMilliseconds     () const;

private:
	GalaxyLoopTimesResponse & operator = (const GalaxyLoopTimesResponse & rhs);
	GalaxyLoopTimesResponse(const GalaxyLoopTimesResponse & source);

	Archive::AutoVariable<unsigned long>  currentFrameMilliseconds;
	Archive::AutoVariable<unsigned long>  lastFrameMilliseconds;
};

//-----------------------------------------------------------------------

inline const unsigned long GalaxyLoopTimesResponse::getCurrentFrameMilliseconds() const
{
	return currentFrameMilliseconds.get();
}

//-----------------------------------------------------------------------

inline const unsigned long GalaxyLoopTimesResponse::getLastFrameMilliseconds() const
{
	return lastFrameMilliseconds.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_GalaxyLoopTimesResponse_H
