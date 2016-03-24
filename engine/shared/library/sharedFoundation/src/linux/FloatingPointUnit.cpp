// ======================================================================
//
// FloatingPointUnit.cpp
// jeff grills
//
// copyright 1999 Bootprint Entertainment
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/FloatingPointUnit.h"

#include "sharedFoundation/ConfigSharedFoundation.h"

// ======================================================================

int                          FloatingPointUnit::updateNumber;
WORD                         FloatingPointUnit::status;
FloatingPointUnit::Precision FloatingPointUnit::precision;
FloatingPointUnit::Rounding  FloatingPointUnit::rounding;
bool                         FloatingPointUnit::exceptionEnabled[E_max];

// ======================================================================

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

// ======================================================================

void FloatingPointUnit::install(void)
{
	precision = P_24;
	rounding  = R_roundToNearestOrEven;
	memset(exceptionEnabled, 0, sizeof(exceptionEnabled));

	// preserve all other bits
	status  = getControlWord();
	status &= ~(PRECISION_MASK | ROUND_MASK | EXCEPTION_ALL);

	// set to single precision, rounding, and all exceptions masked
	status |= PRECISION_24 | ROUND_NEAREST | EXCEPTION_ALL;

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
		DEBUG_REPORT_LOG_PRINT(true, ("FPU: update=%d, in mode=%04x, should be in mode=%04x", updateNumber, static_cast<int>(currentStatus), static_cast<int>(status)));
		setControlWord(status);
	}

	++updateNumber;
}

// ----------------------------------------------------------------------

WORD FloatingPointUnit::getControlWord(void)
{
	return status;
}

// ----------------------------------------------------------------------

void FloatingPointUnit::setControlWord(WORD controlWord)
{
	UNREF(controlWord);
}

// ----------------------------------------------------------------------

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
