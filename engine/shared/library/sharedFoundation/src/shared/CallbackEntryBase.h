// CallbackEntryBase.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_CallbackEntryBase_H
#define	_INCLUDED_CallbackEntryBase_H

//-----------------------------------------------------------------------

class Scheduler;

//-----------------------------------------------------------------------
/** @brief Internally used class that identifies callbacks.
	
	@see CallbackEntry
	@see StaticCallbackEntry

	@author Justin Randall
*/
class CallbackEntryBase
{
protected:
	CallbackEntryBase(const unsigned long when);
	virtual ~CallbackEntryBase() = 0;

public:
	virtual void         expired() = 0;
	const unsigned long  getExpireCount() const;
	
	/** @brief Compare functor supplied for use with a priority queue.
	*/
	struct Compare
	{
		bool operator ()(const CallbackEntryBase * lhs, const CallbackEntryBase * rhs) const
		{
			return lhs->expireCount > rhs->expireCount;
		}
	};

protected:
	friend struct Compare;

	unsigned long  expireCount; 
};

//-----------------------------------------------------------------------
/** @return the expiration count ("time", "frame", whatever) of the callback.
*/
inline const unsigned long CallbackEntryBase::getExpireCount() const
{
	return expireCount;
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_CallbackEntryBase_H
