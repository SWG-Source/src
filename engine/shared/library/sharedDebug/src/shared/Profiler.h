// ======================================================================
//
// Profiler.h
//
// Copyright 2003 Sony Online Entertainment
// All Rights Reserved
//
// ======================================================================

#ifndef INCLUDED_Profiler_H
#define INCLUDED_Profiler_H

// ======================================================================

#include "sharedFoundation/Production.h"

// ======================================================================

class ProfilerBlock
{
public:

	ProfilerBlock(char const *name);

	void enter();
	void leave();
	void transfer(char const *newName);
	void adjustForLostBlocks();

private:

	ProfilerBlock();
	ProfilerBlock(ProfilerBlock const &);
	ProfilerBlock & operator =(ProfilerBlock const &);

private:

	char const * m_name;
};

// ----------------------------------------------------------------------

class ProfilerAutoBlock
{
public:

	ProfilerAutoBlock(char const *name);
	~ProfilerAutoBlock();

	void transfer(char const *newName);

private:

	ProfilerAutoBlock();
	ProfilerAutoBlock(ProfilerAutoBlock const &);
	ProfilerAutoBlock & operator =(ProfilerAutoBlock const &);

private:

	ProfilerBlock m_profilerBlock;
};

// ----------------------------------------------------------------------

class ProfilerAutoBlockCheck
{
public:

	ProfilerAutoBlockCheck(char const *name);
	~ProfilerAutoBlockCheck();

	void transfer(char const *newName);

private:

	ProfilerAutoBlockCheck();
	ProfilerAutoBlockCheck(ProfilerAutoBlockCheck const &);
	ProfilerAutoBlockCheck & operator =(ProfilerAutoBlockCheck const &);

private:

	ProfilerBlock m_profilerBlock;
};

//----------------------------------------------------------------------

class CuiManager;

// ----------------------------------------------------------------------

class Profiler
{
	friend class ProfilerBlock;
	friend class CuiManager;

public:

	static void install();
	static void remove();
	static void registerDebugFlags();
	
	static void        enable(bool enabled);
	static void        enableProfilerOutput(bool enabled);
	static char const *getLastFrameData();
	static void        printLastFrameData();

	static void selectionMoveUp();
	static void selectionMoveDown();
	static void selectionExpand();
	static void selectionCollapse();
	static void selectionToggleExpanded();
	static void setTemporaryExpandAll(bool temporaryExpandAll);
	static void setDisplayPercentageMinimum(int percentage);
	static void handleOperation(char const *operation);

private:

	static void DLLEXPORT enter(char const *name);
	static void DLLEXPORT leave(char const *name);
	static void DLLEXPORT transfer(char const *leaveName, char const *enterName);
	static void           adjustForLostBlocks(char const *nextName);
};

// ======================================================================

inline ProfilerBlock::ProfilerBlock(const char *name)
:
	m_name(name)
{
}

// ----------------------------------------------------------------------

inline void ProfilerBlock::enter()
{
	Profiler::enter(m_name);
}

// ----------------------------------------------------------------------

inline void ProfilerBlock::leave()
{
	Profiler::leave(m_name);
}

// ----------------------------------------------------------------------

inline void ProfilerBlock::transfer(char const *name)
{
	Profiler::transfer(m_name, name);
	m_name = name;
}

// ----------------------------------------------------------------------

inline void ProfilerBlock::adjustForLostBlocks()
{
	Profiler::adjustForLostBlocks(m_name);
}

// ======================================================================

inline ProfilerAutoBlock::ProfilerAutoBlock(char const *name)
:
	m_profilerBlock(name)
{
	m_profilerBlock.enter();
}

// ----------------------------------------------------------------------

inline ProfilerAutoBlock::~ProfilerAutoBlock()
{
	m_profilerBlock.leave();
}

// ----------------------------------------------------------------------

inline void ProfilerAutoBlock::transfer(char const *name)
{
	m_profilerBlock.transfer(name);
}

// ======================================================================

inline ProfilerAutoBlockCheck::ProfilerAutoBlockCheck(char const *name)
:
	m_profilerBlock(name)
{
	m_profilerBlock.enter();
}

// ----------------------------------------------------------------------

inline ProfilerAutoBlockCheck::~ProfilerAutoBlockCheck()
{
	m_profilerBlock.adjustForLostBlocks();
	m_profilerBlock.leave();
}

// ----------------------------------------------------------------------

inline void ProfilerAutoBlockCheck::transfer(char const *name)
{
	m_profilerBlock.transfer(name);
}

// ======================================================================

#if defined(_DEBUG)

	#define PROFILER_BLOCK_DEFINE(a, b)               ProfilerBlock a ( b )
	#define PROFILER_BLOCK_ENTER(a)                   a.enter()
	#define PROFILER_BLOCK_LEAVE(a)                   a.leave()
	#define PROFILER_BLOCK_TRANSFER(a,b)              a.transfer(b)
	#define PROFILER_BLOCK_LOST_CHECK(a)              a.adjustForLostBlocks()

	#define PABD_PASTE(a,b) a##b
	#define PABD_XPASTE(a,b) PABD_PASTE(a,b)
	#define PROFILER_AUTO_BLOCK_DEFINE(a)             ProfilerAutoBlock PABD_XPASTE(profilerAutoBlock, __LINE__) ( a )
	#define PROFILER_AUTO_BLOCK_CHECK_DEFINE(a)       ProfilerAutoBlockCheck PABD_XPASTE(profilerAutoBlockCheck, __LINE__) ( a )
	#define PROFILER_NAMED_AUTO_BLOCK_DEFINE(a,b)     ProfilerAutoBlock a ( b )
	#define PROFILER_NAMED_AUTO_BLOCK_TRANSFER(a,b)   a.transfer(b)

	#define PROFILER_GET_LAST_FRAME_DATA()            Profiler::getLastFrameData()

#else

	#define PROFILER_BLOCK_DEFINE(a, b)               
	#define PROFILER_BLOCK_CHECK_DEFINE(a, b)         
	#define PROFILER_BLOCK_ENTER(a)                   NOP
	#define PROFILER_BLOCK_LEAVE(a)                   NOP
	#define PROFILER_BLOCK_TRANSFER(a,b)              NOP
	#define PROFILER_BLOCK_LOST_CHECK(a)              NOP

	#define PROFILER_AUTO_BLOCK_DEFINE(A)             NOP
	#define PROFILER_AUTO_BLOCK_CHECK_DEFINE(a)       NOP
	#define PROFILER_NAMED_AUTO_BLOCK_DEFINE(a,b)     
	#define PROFILER_NAMED_AUTO_BLOCK_TRANSFER(a,b)   NOP

	#define PROFILER_GET_LAST_FRAME_DATA()            ""

#endif
// ======================================================================

#endif
