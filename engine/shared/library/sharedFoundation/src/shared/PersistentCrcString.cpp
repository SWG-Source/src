// ======================================================================
//
// PersistentCrcString.cpp
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/PersistentCrcString.h"

#include "sharedFoundation/Crc.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/Os.h"

// ======================================================================

namespace PersistentCrcStringNamespace
{
	void remove();

	MemoryBlockManager * ms_fakeConstCharMemoryBlockManager = reinterpret_cast<MemoryBlockManager *>(0x01);
	MemoryBlockManager * ms_memoryBlockManager8;
	MemoryBlockManager * ms_memoryBlockManager16;
	MemoryBlockManager * ms_memoryBlockManager32;
	MemoryBlockManager * ms_memoryBlockManager48;
	MemoryBlockManager * ms_memoryBlockManager64;
}
using namespace PersistentCrcStringNamespace;

// ======================================================================

PersistentCrcString const PersistentCrcString::empty("", CC_true);

// ======================================================================

void PersistentCrcString::install()
{
	ms_memoryBlockManager8  = new MemoryBlockManager("PersistentCrcString8",  true,  8, 0, 0, 0);
	ms_memoryBlockManager16 = new MemoryBlockManager("PersistentCrcString16", true, 16, 0, 0, 0);
	ms_memoryBlockManager32 = new MemoryBlockManager("PersistentCrcString32", true, 32, 0, 0, 0);
	ms_memoryBlockManager48 = new MemoryBlockManager("PersistentCrcString48", true, 48, 0, 0, 0);
	ms_memoryBlockManager64 = new MemoryBlockManager("PersistentCrcString64", true, 64, 0, 0, 0);
	ExitChain::add(PersistentCrcStringNamespace::remove, "PersistentCrcStringNamespace::remove");
}

// ----------------------------------------------------------------------

void PersistentCrcStringNamespace::remove()
{
	delete ms_memoryBlockManager8;
	ms_memoryBlockManager8 = nullptr;
	delete ms_memoryBlockManager16;
	ms_memoryBlockManager16 = nullptr;
	delete ms_memoryBlockManager32;
	ms_memoryBlockManager32 = nullptr;
	delete ms_memoryBlockManager48;
	ms_memoryBlockManager48 = nullptr;
	delete ms_memoryBlockManager64;
	ms_memoryBlockManager64 = nullptr;
}

// ======================================================================

PersistentCrcString::PersistentCrcString()
: CrcString(),
	m_memoryBlockManager(nullptr),
	m_buffer(nullptr)
{
	DEBUG_FATAL(!ms_memoryBlockManager8, ("Constructing a PersistentCrcString before the class is installed"));
}

// ----------------------------------------------------------------------

PersistentCrcString::PersistentCrcString(CrcString const &rhs)
: CrcString(),
	m_memoryBlockManager(nullptr),
	m_buffer(nullptr)
{
	DEBUG_FATAL(!ms_memoryBlockManager8, ("Constructing a PersistentCrcString before the class is installed"));

	if (!rhs.isEmpty())
		PersistentCrcString::set(rhs.getString(), rhs.getCrc());
}

// ----------------------------------------------------------------------

PersistentCrcString::PersistentCrcString(PersistentCrcString const &rhs)
: CrcString(), //lint !e1738 // non copy constructor used to initialize base class // this appears to be intentional.
	m_memoryBlockManager(nullptr),
	m_buffer(nullptr)
{
	DEBUG_FATAL(!ms_memoryBlockManager8, ("Constructing a PersistentCrcString before the class is installed"));

	if (!rhs.isEmpty())
		PersistentCrcString::set(rhs.getString(), rhs.getCrc());
}

// ----------------------------------------------------------------------

PersistentCrcString::PersistentCrcString(char const * string, bool applyNormalize)
: CrcString(),
	m_memoryBlockManager(nullptr),
	m_buffer(nullptr)
{
	DEBUG_FATAL(!ms_memoryBlockManager8, ("Constructing a PersistentCrcString before the class is installed"));

	if (string)
		PersistentCrcString::set(string, applyNormalize);
}

// ----------------------------------------------------------------------

PersistentCrcString::PersistentCrcString(char const * string, uint32 crc)
: CrcString(),
	m_memoryBlockManager(nullptr),
	m_buffer(nullptr)
{
	DEBUG_FATAL(!ms_memoryBlockManager8, ("Constructing a PersistentCrcString before the class is installed"));

	NOT_NULL(string);
	PersistentCrcString::set(string, crc);
}

// ----------------------------------------------------------------------

PersistentCrcString::PersistentCrcString(char const * string, ConstChar)
: CrcString(),
	m_memoryBlockManager(ms_fakeConstCharMemoryBlockManager),
	m_buffer(const_cast<char *>(string))
{
#ifdef _DEBUG
	DEBUG_FATAL(strlen(string)+1 >= Os::MAX_PATH_LENGTH, ("source string too long %d/%d",strlen(string)+1, Os::MAX_PATH_LENGTH));
	char buffer[Os::MAX_PATH_LENGTH];
	normalize(buffer, string);
	DEBUG_FATAL(strcmp(string, buffer) != 0, ("source string was not normalized"));
#endif

	calculateCrc();
}

// ----------------------------------------------------------------------

PersistentCrcString::~PersistentCrcString()
{
	if (!ms_memoryBlockManager8 && m_memoryBlockManager && m_memoryBlockManager != ms_fakeConstCharMemoryBlockManager)
	{
		DEBUG_FATAL(true, ("Destructing a PersistentCrcString after the class was removed"));
	}

	internalFree();
}

// ----------------------------------------------------------------------

char const * PersistentCrcString::getString() const
{
	return m_buffer ? m_buffer : "";
}

// ----------------------------------------------------------------------

void PersistentCrcString::internalFree()
{
	if (m_buffer)
	{
		if (m_memoryBlockManager)
		{
			if (m_memoryBlockManager == ms_fakeConstCharMemoryBlockManager)
			{
				m_memoryBlockManager = nullptr;
			}
			else
			{
				m_memoryBlockManager->free(m_buffer);
				m_memoryBlockManager = nullptr;
			}
		}
		else
			delete [] m_buffer;

		m_buffer = nullptr;
	}
}

// ----------------------------------------------------------------------

void PersistentCrcString::clear()
{
	internalFree();
	m_crc = Crc::crcNull;
}

// ----------------------------------------------------------------------

void PersistentCrcString::internalSet(char const * string, bool applyNormalize)
{
	internalFree();

	const int stringLength = istrlen(string) + 1;
	DEBUG_FATAL(stringLength > Os::MAX_PATH_LENGTH, ("string too long for a filename"));

	m_memoryBlockManager = nullptr;
	if (stringLength <= 8)
		m_memoryBlockManager = ms_memoryBlockManager8;
	else
		if (stringLength <= 16)
			m_memoryBlockManager = ms_memoryBlockManager16;
		else
			if (stringLength <= 32)
				m_memoryBlockManager = ms_memoryBlockManager32;
			else
				if (stringLength <= 48)
					m_memoryBlockManager = ms_memoryBlockManager48;
				else
					if (stringLength <= 64)
						m_memoryBlockManager = ms_memoryBlockManager64;

	if (m_memoryBlockManager)
		m_buffer = reinterpret_cast<char *>(m_memoryBlockManager->allocate());
	else
		m_buffer = new char[static_cast<size_t>(stringLength)];

	if (applyNormalize)
		normalize(m_buffer, string);
	else
		strcpy(m_buffer, string);
}

// ----------------------------------------------------------------------

void PersistentCrcString::set(char const * string, bool applyNormalize)
{
	NOT_NULL(string);
	internalSet(string, applyNormalize);
	calculateCrc();
}

// ----------------------------------------------------------------------

void PersistentCrcString::set(char const * string, uint32 crc)
{
	NOT_NULL(string);
	internalSet(string, false);
	m_crc = crc;
}

//----------------------------------------------------------------------

PersistentCrcString const & PersistentCrcString::operator=(PersistentCrcString const & rhs)
{
	if (this != &rhs)
		CrcString::set(rhs);

	return *this;
} //lint !e1539 //m_buffer not assigned

// ======================================================================
