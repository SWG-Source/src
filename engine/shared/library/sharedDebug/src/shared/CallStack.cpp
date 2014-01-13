// ======================================================================
//
// CallStack.cpp
// asommers
//
// Copyright 2004, Sony Online Entertainment
// All Rights Reserved
//
// ======================================================================

#include "sharedDebug/FirstSharedDebug.h"
#include "sharedDebug/CallStack.h"

#include "sharedDebug/DebugHelp.h"

#include <algorithm>

// ======================================================================

#define DESIRED_CALLSTACK_DEPTH 24

#define CALLSTACK_DEPTH std::min(static_cast<size_t>(CallStack::S_callStack), static_cast<size_t>(DESIRED_CALLSTACK_DEPTH + 2))

// ======================================================================

CallStack::CallStack()
{
	Zero(m_callStack);
}

// ----------------------------------------------------------------------

CallStack::~CallStack()
{
}

// ----------------------------------------------------------------------

bool CallStack::operator<(const CallStack &o) const
{
	return memcmp(m_callStack, o.m_callStack, sizeof(m_callStack))<0;
}

// ----------------------------------------------------------------------

void CallStack::sample()
{
	DebugHelp::getCallStack(m_callStack, CALLSTACK_DEPTH);
}

// ----------------------------------------------------------------------

void CallStack::debugPrint() const
{
	char libName[256];
	char fileName[256];
	int line = 0;

	for (size_t i = 2; i < CALLSTACK_DEPTH; ++i)
	{
		if (DebugHelp::lookupAddress(m_callStack[i], libName, fileName, sizeof(fileName), line))
			DEBUG_REPORT_PRINT(true, (" (0x%08X) %s(%d) : caller %d\n", static_cast<int>(m_callStack[i]), fileName, line, i - 1));
		else
			DEBUG_REPORT_PRINT(true, ("  unknown(0x%08X) : caller %d\n", static_cast<int>(m_callStack[i]), i - 1));
	}
}

// ----------------------------------------------------------------------

void CallStack::debugLog() const
{
	char libName[256];
	char fileName[256];
	int line = 0;

	for (size_t i = 2; i < CALLSTACK_DEPTH && m_callStack[i]!=0; ++i)
	{
		if (DebugHelp::lookupAddress(m_callStack[i], libName, fileName, sizeof(fileName), line))
			DEBUG_REPORT_LOG(true, ("  (0x%08X) %s(%d) : caller %d\n", static_cast<int>(m_callStack[i]), fileName, line, i - 1));
		else
			DEBUG_REPORT_LOG(true, ("  unknown(0x%08X) : caller %d\n", static_cast<int>(m_callStack[i]), i - 1));
	}
}

// ======================================================================
