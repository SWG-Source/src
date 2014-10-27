// ======================================================================
//
// CallStackCollector.cpp
// asommers
//
// Copyright 2004, Sony Online Entertainment
// All Rights Reserved
//
// ======================================================================

#include "sharedDebug/FirstSharedDebug.h"
#include "sharedDebug/CallStackCollector.h"

#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/DebugHelp.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedFoundation/PersistentCrcString.h"

#include <algorithm>
#include <map>
#include <vector>

// ======================================================================

#define DESIRED_CALLSTACK_DEPTH 24

#define CALLSTACK_DEPTH DESIRED_CALLSTACK_DEPTH + 2

// ======================================================================

namespace CallStackCollectorNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void remove();
	void clear();
	void debugReport();

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class Node
	{
	public:

		explicit Node(char const * name);
		~Node();

		CrcString const & getName() const;
		void addCallStack(uint32 * callStack);

		void debugReport() const;

	private:

		struct CallStackEntry
		{
		public:

			static bool compare(CallStackEntry const * a, CallStackEntry const * b);

		public:

			uint32 * m_callStack;
			int m_calls;
		};

	private:

		Node(Node const &);
		Node & operator=(Node const &);

	private:

		PersistentCrcString const m_name;
		typedef std::map<uint32, CallStackEntry> CallStackEntryMap;
		CallStackEntryMap m_callStackEntryMap;
	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef std::map<CrcString const *, Node *, LessPointerComparator> NodeMap;
	NodeMap ms_nodeMap;

	bool ms_debugReport;
	bool ms_enabled;
	bool ms_clear;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}

using namespace CallStackCollectorNamespace;

// ======================================================================

bool CallStackCollectorNamespace::Node::CallStackEntry::compare(CallStackEntry const * const a, CallStackEntry const * const b)
{
	return b->m_calls < a->m_calls;
}

// ======================================================================

CallStackCollectorNamespace::Node::Node(char const * const name) :
	m_name(name, false),
	m_callStackEntryMap()
{
}

// ----------------------------------------------------------------------

CallStackCollectorNamespace::Node::~Node()
{
	for (CallStackEntryMap::iterator iter = m_callStackEntryMap.begin(); iter != m_callStackEntryMap.end(); ++iter)
		delete [] iter->second.m_callStack;
}

// ----------------------------------------------------------------------

CrcString const & CallStackCollectorNamespace::Node::getName() const
{
	return m_name;
}

// ----------------------------------------------------------------------

void CallStackCollectorNamespace::Node::addCallStack(uint32 * const callStack)
{
	//-- Compute crc of memory
	uint32 const crc = Crc::calculate(callStack, (sizeof(uint32) * CALLSTACK_DEPTH));

	//-- Find callstack in list
	CallStackEntryMap::iterator iter = m_callStackEntryMap.find(crc);
	if (iter != m_callStackEntryMap.end())
	{
		//-- Increment count of existing callstack
		++iter->second.m_calls;
	}
	else
	{
		//-- Create new callstack
		uint32 * const newCallStack = new uint32[CALLSTACK_DEPTH];
		memcpy(newCallStack, callStack, sizeof(*newCallStack));

		CallStackEntry callStackEntry;
		callStackEntry.m_callStack = newCallStack;
		callStackEntry.m_calls = 1;
		m_callStackEntryMap.insert(std::make_pair(crc, callStackEntry));
	}
}

// ----------------------------------------------------------------------

void CallStackCollectorNamespace::Node::debugReport() const
{
	//-- Find top 3 entries per node
	typedef std::vector<CallStackEntry const *> CallStackEntryList;
	CallStackEntryList callStackEntryList;

	for (CallStackEntryMap::const_iterator iter = m_callStackEntryMap.begin(); iter != m_callStackEntryMap.end(); ++iter)
		callStackEntryList.push_back(&iter->second);

	std::stable_sort(callStackEntryList.begin(), callStackEntryList.end(), CallStackEntry::compare);

	//-- Print out entries
	size_t const c_maximumNumberOfPrintedEntries = 3;
	for (size_t i = 0; i < std::min(c_maximumNumberOfPrintedEntries, callStackEntryList.size()); ++i)
	{
		CallStackEntry const * const callStackEntry = callStackEntryList[i];

		char libName[256];
		char fileName[256];
		int line = 0;

		REPORT_LOG(true, ("CallStackCollector(%s:%i) callstack called %d times\n", m_name.getString(), i, callStackEntry->m_calls));

		for (size_t j = 2; j < CALLSTACK_DEPTH; ++j)
		{
			if (DebugHelp::lookupAddress(callStackEntry->m_callStack[j], libName, fileName, sizeof(fileName), line))
				REPORT_LOG(true, ("  %s(%d) : caller %d\n", fileName, line, j - 1));
			else
				REPORT_LOG(true, ("  unknown(0x%08X) : caller %d\n", static_cast<int>(callStackEntry->m_callStack[j]), j - 1));
		}
	}
}

// ======================================================================

void CallStackCollector::install()
{
	DebugFlags::registerFlag(ms_enabled, "SharedDebug/CallStackCollector", "enabled");
	DebugFlags::registerFlag(ms_debugReport, "SharedDebug/CallStackCollector", "debugReport", debugReport);
	DebugFlags::registerFlag(ms_clear, "SharedDebug/CallStackCollector", "clear");
	ExitChain::add(CallStackCollectorNamespace::remove, "CallStackCollectorNamespace::remove");
}

// ----------------------------------------------------------------------

void CallStackCollector::sample(char const * const name)
{
	if (ms_clear)
		clear();

	if (!ms_enabled)
		return;

	//-- Get the desired node
	Node * node = 0;
	ConstCharCrcString const crcName(name);
	NodeMap::iterator iter = ms_nodeMap.find((const CrcString*)&crcName);
	if (iter != ms_nodeMap.end())
		node = iter->second;
	else
	{
		node = new Node(name);
		ms_nodeMap.insert(std::make_pair(&node->getName(), node));
	}

	//-- Sample the callstack
	uint32 callStack[CALLSTACK_DEPTH];
	DebugHelp::getCallStack(&callStack[0], CALLSTACK_DEPTH);

	//-- Add to the node
	node->addCallStack(&callStack[0]);
}

// ----------------------------------------------------------------------

void CallStackCollectorNamespace::remove()
{
	DebugFlags::unregisterFlag(ms_debugReport);

	clear();
}

// ----------------------------------------------------------------------

void CallStackCollectorNamespace::clear()
{
	for (NodeMap::iterator iter = ms_nodeMap.begin(); iter != ms_nodeMap.end(); ++iter)
		delete iter->second;

	ms_nodeMap.clear();

	ms_clear = false;
}

// ----------------------------------------------------------------------

void CallStackCollectorNamespace::debugReport()
{
	if (ms_debugReport)
	{
		for (NodeMap::iterator iter = ms_nodeMap.begin(); iter != ms_nodeMap.end(); ++iter)
			iter->second->debugReport();

		ms_debugReport = false;
	}
}

// ======================================================================
