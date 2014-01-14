// ======================================================================
//
// Callback.cpp
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#include "sharedUtility/FirstSharedUtility.h"
#include "sharedUtility/Callback.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedUtility/CallbackReceiver.h"
#include <algorithm>

// ======================================================================

MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITH_INSTALL (Callback, true, 0, 0, 0);

// ======================================================================

Callback::Callback()
:	m_referenceCount(0),
	m_receivers(),
	m_inPerformCallback (false)
{
}

// ----------------------------------------------------------------------

Callback::~Callback()
{
	DEBUG_FATAL(m_referenceCount, ("Callback deleted with non-zero reference count"));
	m_receivers.clear ();
}

// ----------------------------------------------------------------------

void Callback::attachReceiver(CallbackReceiver &receiver)
{
	DEBUG_FATAL (m_inPerformCallback, ("Callback cannot attach receivers while receiving callbacks"));
	//-- pre-reserve the space to prevent the vector from agressively allocating more space
	m_receivers.reserve (m_receivers.size () + 1);
	m_receivers.push_back (&receiver);
}

// ----------------------------------------------------------------------

void Callback::detachReceiver(CallbackReceiver &receiver)
{
	DEBUG_FATAL (m_inPerformCallback, ("Callback cannot detach receivers while receiving callbacks"));
	const CallbackReceiverVector::iterator remove_begin = std::remove (m_receivers.begin (), m_receivers.end (), &receiver);

	if (remove_begin == m_receivers.end ())
	{
		DEBUG_FATAL (true, ("Callback attempted to detach unattached receeiver."));
		return;
	}

	m_receivers.erase (remove_begin, m_receivers.end ());
}

// ----------------------------------------------------------------------

bool Callback::hasAnyReceivers() const
{
	return !m_receivers.empty ();
}

// ----------------------------------------------------------------------

void Callback::performCallback()
{
	m_inPerformCallback = true;
	const CallbackReceiverVector::const_iterator end = m_receivers.end ();
	for (CallbackReceiverVector::const_iterator it = m_receivers.begin (); it != end; ++it)
	{
		CallbackReceiver * const receiver = *it;
		NOT_NULL (receiver);
		if (receiver)
			receiver->performCallback();
	}
	m_inPerformCallback = false;
}

// ----------------------------------------------------------------------

void Callback::fetch()
{
	++m_referenceCount;

	// sanity check
	DEBUG_FATAL(m_referenceCount < 1, ("invalid reference count %d after fetch()", m_referenceCount));
}

// ----------------------------------------------------------------------

void Callback::release()
{
	--m_referenceCount;

	if (m_referenceCount < 1)
	{
		DEBUG_FATAL(m_referenceCount < 0, ("bad reference count handling [%d]", m_referenceCount));
		delete this;
	}
}

// ======================================================================
