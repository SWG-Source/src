// ======================================================================
//
// PixCounter.h
// Copyright 2004, Sony Online Entertainment
// All Rights Reserved
//
// ======================================================================

#ifndef INCLUDED_PixCounter_H
#define INCLUDED_PixCounter_H

// ======================================================================

#include "sharedFoundation/Production.h"
#include <string>

// ======================================================================

#if PRODUCTION == 0

class PixCounter
{
public:

	/// Base class for all other counters
	class Counter
	{
	public:
		Counter();
		virtual ~Counter();

		virtual void poll() = 0;
		virtual void reset() = 0;
		virtual void enable();
		virtual void disable();

	private:
		Counter(Counter const &);
		Counter & operator = (Counter const &);
	};

	/// General integer counter
	class Integer : public Counter
	{
	public:
		Integer();

		void bindToCounter(char const * name);

		virtual void poll();
		virtual void reset();

		void operator =(int value);
		void operator +=(int value);
		void operator ++();

		int getCurrentValue() const;
		int getLastFrameValue() const;

	private:
		Integer(Integer const &);
		Integer & operator = (Integer const &);

	private:
		int m_lastFrameValue;
		int m_currentValue;
	};

	/// Integer counter whose value gets reset to 0 immediately after being polled
	class ResetInteger : public Integer
	{
	public:
		ResetInteger();
		virtual void poll();
	private:
		ResetInteger(ResetInteger const &);
		ResetInteger & operator = (ResetInteger const &);
	};

	/// General float counter
	class Float : public Counter
	{
	public:
		Float();

		void bindToCounter(char const * name);

		virtual void poll();
		virtual void reset();

		void operator =(float value);
		void operator +=(float value);

		float getCurrentValue() const;
		float getLastFrameValue() const;

	private:
		Float(Float const &);
		Float & operator = (Float const &);

	private:
		float m_lastFrameValue;
		float m_currentValue;
	};

	/// Float counter whose value gets reset to 0 immediately after being polled
	class ResetFloat : public Float
	{
	public:
		ResetFloat();
		virtual void poll();
	private:
		ResetFloat(ResetFloat const &);
		ResetFloat & operator = (ResetFloat const &);
	};

	/// General string counter
	class String : public Counter
	{
	public:
		String();

		void bindToCounter(char const * name);

		virtual void poll();
		virtual void reset();
		virtual void enable();
		virtual void disable();

		void operator =(const char * value);
		void operator +=(const char * value);

		void set(const char * format, ...);
		void append(const char * format, ...);

	private:
		String(String const &);
		String & operator = (String const &);

	protected:
		void pollAndReset();

	private:
		bool m_enabled;
		std::string m_lastFrameValue;
		char const * m_lastFrameValuePointer;
		std::string m_currentValue;
	};

	/// String counter whose value gets reset to "" immediately after being polled
	class ResetString : public String
	{
	public:
		ResetString();
		virtual void poll();
	private:
		ResetString(ResetString const &);
		ResetString & operator = (ResetString const &);
	};

public:

	static void install();

	static void update();

	static void enable();
	static void disable();
	static bool isEnabled();
	static bool connectedToPixProfiler();

	static void resetAllCounters();

	static void bindToCounter(char const * name, int * value);
	static void bindToCounter(char const * name, float * value);
	static void bindToCounter(char const * name, __int64 * value);
	static void bindToCounter(char const * name, char const * const * value);
};

#endif

// ======================================================================

#endif
