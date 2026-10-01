// ======================================================================
//
// FloatingPointUnit.cpp
// copyright 1999 Bootprint Entertainment
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/FloatingPointUnit.h"

#include "sharedFoundation/ConfigSharedFoundation.h"

#if defined(_M_X64)
#include <xmmintrin.h>
#endif

// ======================================================================

int                          FloatingPointUnit::updateNumber;
ushort                       FloatingPointUnit::status;
FloatingPointUnit::Precision FloatingPointUnit::precision;
FloatingPointUnit::Rounding  FloatingPointUnit::rounding;
bool                         FloatingPointUnit::exceptionEnabled[E_max];

// ======================================================================

#if defined(_M_X64)
// MXCSR has rounding and exception masks, but no adjustable significand precision.
const WORD CONTROL_MASK         = 0xffc0;
const WORD ROUND_MASK           = _MM_ROUND_MASK;
const WORD ROUND_NEAREST        = _MM_ROUND_NEAREST;
const WORD ROUND_CHOP           = _MM_ROUND_TOWARD_ZERO;
const WORD ROUND_DOWN           = _MM_ROUND_DOWN;
const WORD ROUND_UP             = _MM_ROUND_UP;
const WORD EXCEPTION_PRECISION  = _MM_MASK_INEXACT;
const WORD EXCEPTION_UNDERFLOW  = _MM_MASK_UNDERFLOW;
const WORD EXCEPTION_OVERFLOW   = _MM_MASK_OVERFLOW;
const WORD EXCEPTION_ZERO_DIVIDE = _MM_MASK_DIV_ZERO;
const WORD EXCEPTION_DENORMAL   = _MM_MASK_DENORM;
const WORD EXCEPTION_INVALID    = _MM_MASK_INVALID;
const WORD EXCEPTION_ALL        = _MM_MASK_MASK;
#else
const WORD PRECISION_MASK        = BINARY4(0000,0011,0000,0000);
const WORD PRECISION_24          = BINARY4(0000,0000,0000,0000);
const WORD PRECISION_53          = BINARY4(0000,0010,0000,0000);
const WORD PRECISION_64          = BINARY4(0000,0011,0000,0000);

const WORD ROUND_MASK            = BINARY4(0000,1100,0000,0000);
const WORD ROUND_NEAREST         = BINARY4(0000,0000,0000,0000);
const WORD ROUND_CHOP            = BINARY4(0000,1100,0000,0000);
const WORD ROUND_DOWN            = BINARY4(0000,0100,0000,0000);
const WORD ROUND_UP              = BINARY4(0000,1000,0000,0000);

const WORD EXCEPTION_PRECISION   = BINARY4(0000,0000,0010,0000);
const WORD EXCEPTION_UNDERFLOW   = BINARY4(0000,0000,0001,0000);
const WORD EXCEPTION_OVERFLOW    = BINARY4(0000,0000,0000,1000);
const WORD EXCEPTION_ZERO_DIVIDE = BINARY4(0000,0000,0000,0100);
const WORD EXCEPTION_DENORMAL    = BINARY4(0000,0000,0000,0010);
const WORD EXCEPTION_INVALID     = BINARY4(0000,0000,0000,0001);
const WORD EXCEPTION_ALL         = BINARY4(0000,0000,0011,1111);

#endif

// ======================================================================

void FloatingPointUnit::install(void)
{
#if defined(_M_X64)
	precision = P_fixedByType;
#else
	precision = P_24;
#endif
	rounding  = R_roundToNearestOrEven;
	memset(exceptionEnabled, 0, sizeof(exceptionEnabled));

	// preserve all other bits
	status  = getControlWord();
#if defined(_M_X64)
	status &= ~(ROUND_MASK | EXCEPTION_ALL);
	status |= ROUND_NEAREST | EXCEPTION_ALL;
#else
	status &= ~(PRECISION_MASK | ROUND_MASK | EXCEPTION_ALL);

	// set to single precision, rounding, and all exceptions masked
	status |= PRECISION_24 | ROUND_NEAREST | EXCEPTION_ALL;
#endif

	// check the config platform flags to see if we should enable some exceptions
	if (ConfigSharedFoundation::getFpuExceptionPrecision())
	{
		exceptionEnabled[E_precision] = true;
		status &= ~EXCEPTION_PRECISION;
	}

	if (ConfigSharedFoundation::getFpuExceptionUnderflow())
	{
		exceptionEnabled[E_underflow] = true;
		status &= ~EXCEPTION_UNDERFLOW;
	}

	if (ConfigSharedFoundation::getFpuExceptionOverflow())
	{
		exceptionEnabled[E_overflow] = true;
		status &= ~EXCEPTION_OVERFLOW;
	}

	if (ConfigSharedFoundation::getFpuExceptionZeroDivide())
	{
		exceptionEnabled[E_zeroDivide] = true;
		status &= ~EXCEPTION_ZERO_DIVIDE;
	}

	if (ConfigSharedFoundation::getFpuExceptionDenormal())
	{
		exceptionEnabled[E_denormal] = true;
		status &= ~EXCEPTION_DENORMAL;
	}

	if (ConfigSharedFoundation::getFpuExceptionInvalid())
	{
		exceptionEnabled[E_invalid] = true;
		status &= ~EXCEPTION_INVALID;
	}

	setControlWord(status);
}

// ----------------------------------------------------------------------

void FloatingPointUnit::update(void)
{
	WORD currentStatus = getControlWord();

	if (currentStatus != status)
	{
//		DEBUG_REPORT_LOG_PRINT(true, ("FPU: update=%d, in mode=%04x, should be in mode=%04x\n", updateNumber, static_cast<int>(currentStatus), static_cast<int>(status)));
		setControlWord(status);
	}

	++updateNumber;
}

// ----------------------------------------------------------------------

WORD FloatingPointUnit::getControlWord(void)
{
#if defined(_M_X64)
	return static_cast<WORD>(_mm_getcsr() & CONTROL_MASK);
#else
	WORD controlWord = 0;

	__asm fnstcw controlWord;
	return controlWord;
#endif
}

// ----------------------------------------------------------------------

void FloatingPointUnit::setControlWord(WORD controlWord)
{
#if defined(_M_X64)
	// Updating control must not clear accumulated exception flags or reserved bits.
	unsigned int const current = _mm_getcsr();
	_mm_setcsr((current & ~static_cast<unsigned int>(CONTROL_MASK)) | (controlWord & CONTROL_MASK));
#else
	UNREF(controlWord);
	__asm fldcw controlWord;
#endif
}

// ----------------------------------------------------------------------

// Keep the legacy Win32 setter, including its old-state behavior, unchanged.
#if !defined(_M_X64)
void FloatingPointUnit::setPrecision(Precision newPrecision)
{
	WORD bits = 0;

	switch (precision)
	{
		case P_24:
			bits = PRECISION_24;
			break;

		case P_53:
			bits = PRECISION_53;
			break;

		case P_64:
			bits = PRECISION_64;
			break;

		case P_max:
		default:
			DEBUG_FATAL(true, ("bad case"));
	}

	// record the current state
	precision = newPrecision;

	// set the proper bit pattern
	status &= ~PRECISION_MASK;
	status |= bits;

	// slam it into the FPU
	setControlWord(status);
}

// ----------------------------------------------------------------------

#endif

void FloatingPointUnit::setRounding(Rounding newRounding)
{
	WORD bits = 0;

	switch (newRounding)
	{
		case R_roundToNearestOrEven:
			bits = ROUND_NEAREST;
			break;

		case R_chop:
			bits = ROUND_CHOP;
			break;

		case R_roundDown:
			bits = ROUND_DOWN;
			break;

		case R_roundUp:
			bits = ROUND_UP;
			break;

		case R_max:
		default:
			DEBUG_FATAL(true, ("bad case"));
	}

	// record the current state
	rounding = newRounding;

	// set the proper bit pattern
	status &= ~ROUND_MASK;
	status |= bits;

	// slam it into the FPU
	setControlWord(status);
}

// ----------------------------------------------------------------------

void FloatingPointUnit::setExceptionEnabled(Exception exception, bool enabled)
{
	WORD bits = 0;

	switch (exception)
	{
		case E_precision:
			bits = EXCEPTION_PRECISION;
			break;

		case E_underflow:
			bits = EXCEPTION_UNDERFLOW;
			break;

		case E_overflow:
			bits = EXCEPTION_OVERFLOW;
			break;

		case E_zeroDivide:
			bits = EXCEPTION_ZERO_DIVIDE;
			break;

		case E_denormal:
			bits = EXCEPTION_DENORMAL;
			break;

		case E_invalid:
			bits = EXCEPTION_INVALID;
			break;

		case E_max:
		default:
			DEBUG_FATAL(true, ("bad case"));
	}

	// record the current state
	exceptionEnabled[exception] = enabled;

	// twiddle the bit appropriately.  these bits masks, so set the bit to disable the exception, clear the bit to enable it.
	if (enabled)
		status &= ~bits;
	else
		status |= bits;

	// slam it into the FPU
	setControlWord(status);
}

// ======================================================================
