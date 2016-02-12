// ======================================================================
//
// PixCounter.cpp
// Copyright 2004 Sony Online Entertainment
// All Rights Reserved
//
// ======================================================================

#include "sharedDebug/FirstSharedDebug.h"
#include "sharedDebug/PixCounter.h"

#if PRODUCTION == 0

#include "sharedFoundation/ExitChain.h"

#include <algorithm>
#include <vector>

// ======================================================================

#ifdef _WIN32
#define STDCALL __stdcall
#else
#define STDCALL
#endif

namespace PixCounterNamespace
{
	typedef std::vector<PixCounter::Counter *> Counters;
	typedef BOOL (STDCALL *SetCounterFloat)(char const * name, float * value);
	typedef BOOL (STDCALL *SetCounterInt)(char const * name, int * value);
	typedef BOOL (STDCALL *SetCounterInt64)(char const * name, __int64 * value);
	typedef BOOL (STDCALL *SetCounterString)(char const * name, char const * const * value);
	typedef void (STDCALL *PollRoutine)();
	typedef void (STDCALL *ClientInstall)(bool * active, PollRoutine pollRoutine);

	void remove();
	void STDCALL poll();

#ifdef _WIN32
	HINSTANCE ms_pixDll;
#endif
	SetCounterFloat ms_setCounterFloat;
	SetCounterInt ms_setCounterInt;
	SetCounterInt64 ms_setCounterInt64;
	SetCounterString ms_setCounterString;
	Counters ms_counters;
	bool ms_enabled;
	bool ms_connectedToPixProfiler;
}
using namespace PixCounterNamespace;

// ======================================================================

void PixCounter::install()
{
#ifdef _WIN32
	ms_pixDll = LoadLibrary("C:\\Program Files\\Microsoft DirectX 9.0 SDK (October 2004)\\Utilities\\PIX\\SoePix.PIXPlugin");
	if (ms_pixDll)
	{
		ms_setCounterFloat  = reinterpret_cast<SetCounterFloat> (GetProcAddress(ms_pixDll, "SetCounterFloat"));
		ms_setCounterInt    = reinterpret_cast<SetCounterInt>   (GetProcAddress(ms_pixDll, "SetCounterInt"));
		ms_setCounterInt64  = reinterpret_cast<SetCounterInt64> (GetProcAddress(ms_pixDll, "SetCounterInt64"));
		ms_setCounterString = reinterpret_cast<SetCounterString>(GetProcAddress(ms_pixDll, "SetCounterString"));

		NOT_NULL(ms_setCounterFloat);
		NOT_NULL(ms_setCounterInt);
		NOT_NULL(ms_setCounterInt64);
		NOT_NULL(ms_setCounterString);

		ClientInstall clientInstall = reinterpret_cast<ClientInstall>(GetProcAddress(ms_pixDll, "ClientInstall"));
		NOT_NULL(clientInstall);
		(*clientInstall)(&ms_connectedToPixProfiler, &PixCounterNamespace::poll);

		ExitChain::add(PixCounterNamespace::remove, "PixCounter");
	}
#endif
}

// ----------------------------------------------------------------------

void PixCounterNamespace::remove()
{
#ifdef _WIN32
	FreeLibrary(ms_pixDll);
	ms_pixDll = nullptr;
#endif

	ms_setCounterFloat = nullptr;
	ms_setCounterInt = nullptr;
	ms_setCounterInt64 = nullptr;
	ms_setCounterString = nullptr;

	// Make sure the counter memory gets freed at this point
	Counters().swap(ms_counters);
}

// ----------------------------------------------------------------------

void PixCounter::enable()
{
	if (!ms_enabled)
	{
		resetAllCounters();
		ms_enabled = true;
	}
}

// ----------------------------------------------------------------------

void PixCounter::disable()
{
	ms_enabled = false;
	resetAllCounters();
}

// ----------------------------------------------------------------------

bool PixCounter::isEnabled()
{
	return ms_enabled;
}

// ----------------------------------------------------------------------

bool PixCounter::connectedToPixProfiler()
{
	return ms_connectedToPixProfiler;
}

// ----------------------------------------------------------------------

void PixCounter::bindToCounter(char const * name, int * value)
{
	if (ms_setCounterInt && (*ms_setCounterInt)(name, value) == false)
		DEBUG_WARNING(true, ("Could not bind int pix counter %s", name));
}

// ----------------------------------------------------------------------

void PixCounter::bindToCounter(char const * name, float * value)
{
	if (ms_setCounterFloat && (*ms_setCounterFloat)(name, value) == false)
		DEBUG_WARNING(true, ("Could not bind float pix counter %s", name));
}

// ----------------------------------------------------------------------

void PixCounter::bindToCounter(char const * name, __int64 * value)
{
	if (ms_setCounterInt64 && (*ms_setCounterInt64)(name, value) == false)
		DEBUG_WARNING(true, ("Could not bind int64 pix counter %s", name));
}

// ----------------------------------------------------------------------

void PixCounter::bindToCounter(char const * name, char const * const * value)
{
	if (ms_setCounterString && (*ms_setCounterString)(name, value) == false)
		DEBUG_WARNING(true, ("Could not bind string pix counter %s", name));
}

// ----------------------------------------------------------------------

void PixCounter::resetAllCounters()
{
	Counters::iterator const iEnd = ms_counters.end();
	for (Counters::iterator i = ms_counters.begin(); i != iEnd; ++i)
		(*i)->reset();
}

// ----------------------------------------------------------------------

void PixCounter::update()
{
	if (!ms_connectedToPixProfiler)
		poll();
}

// ----------------------------------------------------------------------

void STDCALL PixCounterNamespace::poll()
{
	if (ms_enabled)
	{
		Counters::iterator const iEnd = ms_counters.end();
		for (Counters::iterator i = ms_counters.begin(); i != iEnd; ++i)
			(*i)->poll();
	}
}

// ======================================================================

PixCounter::Counter::Counter()
{
}

// ----------------------------------------------------------------------

PixCounter::Counter::~Counter()
{
	disable();
}

// ----------------------------------------------------------------------

void PixCounter::Counter::enable()
{
	ms_counters.push_back(this);
}

// ----------------------------------------------------------------------

void PixCounter::Counter::disable()
{
	Counters::iterator i = std::find(ms_counters.begin(), ms_counters.end(), this);
	if (i != ms_counters.end())
		ms_counters.erase(i);
}

// ======================================================================

PixCounter::Integer::Integer()
: Counter(),
	m_lastFrameValue(0),
	m_currentValue(0)
{
}

// ----------------------------------------------------------------------

void PixCounter::Integer::bindToCounter(char const * name)
{
	PixCounter::bindToCounter(name, &m_lastFrameValue);
	enable();
}

// ----------------------------------------------------------------------

void PixCounter::Integer::poll()
{
	m_lastFrameValue = m_currentValue;
}

// ----------------------------------------------------------------------

void PixCounter::Integer::reset()
{
	m_currentValue = 0;
}

// ----------------------------------------------------------------------

void PixCounter::Integer::operator =(int value)
{
	m_currentValue = value;
}

// ----------------------------------------------------------------------

void PixCounter::Integer::operator +=(int value)
{
	m_currentValue += value;
}

// ----------------------------------------------------------------------

void PixCounter::Integer::operator ++()
{
	++m_currentValue;
}

// ----------------------------------------------------------------------

int PixCounter::Integer::getCurrentValue() const
{
	return m_currentValue;
}

// ----------------------------------------------------------------------

int PixCounter::Integer::getLastFrameValue() const
{
	return m_lastFrameValue;
}

// ======================================================================

PixCounter::ResetInteger::ResetInteger()
: Integer()
{
}

// ----------------------------------------------------------------------

void PixCounter::ResetInteger::poll()
{
	Integer::poll();
	reset();
}

// ======================================================================

PixCounter::Float::Float()
: Counter(),
	m_lastFrameValue(0.0f),
	m_currentValue(0.0f)
{
}

// ----------------------------------------------------------------------

void PixCounter::Float::bindToCounter(char const * name)
{
	PixCounter::bindToCounter(name, &m_lastFrameValue);
	enable();
}

// ----------------------------------------------------------------------

void PixCounter::Float::poll()
{
	m_lastFrameValue = m_currentValue;
}

// ----------------------------------------------------------------------

void PixCounter::Float::reset()
{
	m_currentValue = 0.0f;
}

// ----------------------------------------------------------------------

void PixCounter::Float::operator =(float value)
{
	m_currentValue = value;
}

// ----------------------------------------------------------------------

void PixCounter::Float::operator +=(float value)
{
	m_currentValue += value;
}

// ----------------------------------------------------------------------

float PixCounter::Float::getCurrentValue() const
{
	return m_currentValue;
}

// ----------------------------------------------------------------------

float PixCounter::Float::getLastFrameValue() const
{
	return m_lastFrameValue;
}

// ======================================================================

PixCounter::ResetFloat::ResetFloat()
: Float()
{
}

// ----------------------------------------------------------------------

void PixCounter::ResetFloat::poll()
{
	Float::poll();
	reset();
}

// ======================================================================

PixCounter::String::String()
: Counter(),
	m_enabled(false),
	m_lastFrameValue(),
	m_lastFrameValuePointer(nullptr),
	m_currentValue()
{
}

// ----------------------------------------------------------------------

void PixCounter::String::bindToCounter(char const * name)
{
	PixCounter::bindToCounter(name, &m_lastFrameValuePointer);
	enable();
}

// ----------------------------------------------------------------------

void PixCounter::String::poll()
{
	m_lastFrameValue = m_currentValue;
	m_lastFrameValuePointer = &(m_lastFrameValue.c_str()[0]);
}

// ----------------------------------------------------------------------

void PixCounter::String::pollAndReset()
{
	m_lastFrameValue.swap(m_currentValue);
	m_currentValue.clear();
	m_lastFrameValuePointer = &(m_lastFrameValue.c_str()[0]);
}

// ----------------------------------------------------------------------

void PixCounter::String::reset()
{
	m_currentValue.clear();
}

// ----------------------------------------------------------------------

void PixCounter::String::enable()
{
	Counter::enable();
	m_enabled = true;
}

// ----------------------------------------------------------------------

void PixCounter::String::disable()
{
	Counter::disable();
	m_enabled = false;
	m_lastFrameValue.clear();
	m_currentValue.clear();
}

// ----------------------------------------------------------------------

void PixCounter::String::operator =(const char * value)
{
	if (ms_enabled && m_enabled)
		m_currentValue = value;
}

// ----------------------------------------------------------------------

void PixCounter::String::operator +=(const char * value)
{
	if (ms_enabled && m_enabled)
		m_currentValue += value;
}

// ----------------------------------------------------------------------

void PixCounter::String::set(const char * format, ...)
{
	if (ms_enabled && m_enabled)
	{
		va_list va;
		va_start(va, format);

			char buffer[512];
			vsnprintf(buffer, sizeof(buffer), format, va);
			buffer[sizeof(buffer)-1] = '\0';
			operator =(buffer);

		va_end(va);
	}
}

// ----------------------------------------------------------------------

void PixCounter::String::append(const char * format, ...)
{
	if (ms_enabled && m_enabled)
	{
		va_list va;
		va_start(va, format);

			char buffer[512];
			vsnprintf(buffer, sizeof(buffer), format, va);
			buffer[sizeof(buffer)-1] = '\0';
			operator +=(buffer);

		va_end(va);
	}
}

// ======================================================================

PixCounter::ResetString::ResetString()
: String()
{
}

// ----------------------------------------------------------------------

void PixCounter::ResetString::poll()
{
	String::pollAndReset();
}

// ======================================================================

#endif
