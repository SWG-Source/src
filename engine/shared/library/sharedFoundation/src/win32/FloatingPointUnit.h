// ======================================================================
//
// FloatingPointUnit.h
// jeff grills
//
// copyright 1999 Bootprint Entertainment
//
// ======================================================================

#ifndef FLOATING_POINT_UNIT_H
#define FLOATING_POINT_UNIT_H

// ======================================================================

class FloatingPointUnit
{
public:

	typedef unsigned short WORD;

	enum Precision
	{
		P_24,
		P_53,
		P_64,
#if defined(_M_X64)
		P_fixedByType, // SSE precision is selected by each float/double operation.
#endif

		P_max
	};

	enum Rounding
	{
		R_roundToNearestOrEven,
		R_chop,
		R_roundDown,
		R_roundUp,

		R_max
	};

	enum Exception
	{
		E_precision,
		E_underflow,
		E_overflow,
		E_zeroDivide,
		E_denormal,
		E_invalid,
		
		E_max
	};

private:

	static int       updateNumber;
	static WORD      status;
	static Precision precision;
	static Rounding  rounding;
	static bool      exceptionEnabled[E_max];

public:

	// Win32: raw x87 control word. x64: MXCSR control bits, excluding sticky status flags.
	// Saved words are platform-specific and must not be transferred between architectures.
	static WORD getControlWord(void);
	static void setControlWord(WORD controlWord);

public:

	static void      install(void);

	static void      update(void);

	// x87 precision control has no SSE equivalent; unavailable on x64.
#if !defined(_M_X64)
	static void      setPrecision(Precision newPrecision);
#endif
	static void      setRounding(Rounding newRounding);
	static void      setExceptionEnabled(Exception exception, bool enabled);

	static Precision getPrecision(void);
	static Rounding  getRounding(void);
	static bool      getExceptionEnabled(Exception exception);
};

// ======================================================================

inline FloatingPointUnit::Precision FloatingPointUnit::getPrecision(void)
{
	return precision;
}

// ----------------------------------------------------------------------

inline FloatingPointUnit::Rounding  FloatingPointUnit::getRounding(void)
{
	return rounding;
}

// ----------------------------------------------------------------------

inline bool FloatingPointUnit::getExceptionEnabled(Exception theException)
{
	DEBUG_FATAL(static_cast<int>(theException) < 0 || static_cast<int>(theException) >= static_cast<int>(E_max), ("exception out of range"));  //lint !e568 // non-negative quantity is never less than 0
	return exceptionEnabled[theException];
}

// ======================================================================

#endif
