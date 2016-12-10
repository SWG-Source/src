// ======================================================================
//
// Callback.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef INCLUDED_Callback_H
#define INCLUDED_Callback_H

// ======================================================================

class CallbackReceiver;
class MemoryBlockManager;

#include <vector>
#include "sharedFoundation/MemoryBlockManagerMacros.h"

// ======================================================================

class Callback
{
public:

	MEMORY_BLOCK_MANAGER_INTERFACE_WITH_INSTALL;

public:

	Callback();

	void  attachReceiver(CallbackReceiver &receiver);
	void  detachReceiver(CallbackReceiver &receiver);

	bool  hasAnyReceivers() const;

	void  performCallback();

	void  fetch();
	void  release();

private:

	~Callback();

private:

	typedef std::vector<CallbackReceiver *> CallbackReceiverVector;

	int                       m_referenceCount;
	CallbackReceiverVector    m_receivers;
	bool                      m_inPerformCallback;

private:
	// disabled
	Callback(const Callback&);
	Callback &operator =(const Callback&);
};

// ======================================================================

#endif
