// ======================================================================
//
// Fatal.h
// copyright 1998 Bootprint Entertainment
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_Fatal_H
#define INCLUDED_Fatal_H

// ======================================================================

void           FatalInstall();

void           FatalSetVersionString(const char *string);
void           FatalSetThrowExceptions(bool throwExceptions);
DLLEXPORT void Fatal(const char *format, ...);

DLLEXPORT void DebugFatal(const char *format, ...);

int            GetNumberOfWarnings(void);
DLLEXPORT void Warning(const char *format, ...);
DLLEXPORT void WarningStackDepth(int stackDepth, const char *format, ...);
DLLEXPORT void ConsoleWarning(const char *format, ...);

void           SetWarningStrictFatal(bool fatal);
bool           GetWarningStrictFatal();
DLLEXPORT void WarningStrictFatal(const char *format, ...);

typedef void (*WarningCallback) (char const * const);
void SetWarningCallback(WarningCallback);

// ======================================================================

#ifdef _DEBUG
	#define LINEINFO(a) fprintf(stderr, "\n%s in %s() file %s:%d \n", a, __FUNCTION__, __FILE__ , __LINE__)
#else
	#define LINEINFO(a) fprintf(stderr, "%s: ", a)
#endif

#define FATAL(a, b) ((a) ? LINEINFO("FATAL"), Fatal b : NOP)
#ifdef _DEBUG
	#define DEBUG_FATAL(a, b)   ((a) ? LINEINFO("DEBUG FATAL"), DebugFatal b : NOP)
#else
	#define DEBUG_FATAL(a, b)   NOP
#endif

#define WARNING(a, b) ((a) ? LINEINFO("WARNING"), Warning b : NOP)
#define WARNING_STACK_DEPTH(a, b) ((a) ? LINEINFO("WARNING"), WarningStackDepth b : NOP)
#ifdef _DEBUG
	#define DEBUG_WARNING(a, b)   WARNING(a, b)
#else
	#define DEBUG_WARNING(a, b)   NOP
#endif

#define CONSOLE_WARNING(a, b) ((a) ? LINEINFO("WARNING"), ConsoleWarning b : NOP)
#ifdef _DEBUG
	#define DEBUG_CONSOLE_WARNING(a, b)   CONSOLE_WARNING(a, b)
#else
	#define DEBUG_CONSOLE_WARNING(a, b)   NOP
#endif

#ifdef _DEBUG
	#define WARNING_DEBUG_FATAL(a, b)   DEBUG_FATAL(a, b)
#else
	#define WARNING_DEBUG_FATAL(a, b)   WARNING(a, b)
#endif

#define WARNING_STRICT_FATAL(a, b) ((a) ? LINEINFO("WARNING STRICT FATAL"), WarningStrictFatal b : NOP)

#ifdef _DEBUG

	template <class T>
	inline T *NonNull(T *pointer, const char *name)
	{
		WARNING(!pointer, ("%s pointer is nullptr", name));
		return pointer;
	}

	#define NON_NULL(a) NonNull(a, #a)
	#define NOT_NULL(a) FATAL(!a, ("%s pointer is nullptr", #a))

	// FATAL if the specified pointer is not nullptr (i.e. assert that the pointer is nullptr, the opposite of NOT_NULL).
	#define IS_NULL(a)  FATAL(a, ("%s pointer is not nullptr, unexpected.", #a))

#else

	#define NON_NULL(a) (a)
	#define NOT_NULL(a) UNREF(a)

	#define IS_NULL(a)  UNREF(a)

#endif

// ======================================================================
/**
 * Fatal if the specified var does not satisfy (low <= var < high).
 *
 * The parameter types must be integral and must be convertable to
 * an int.
 */

template <class T>
inline void ValidateRangeInclusiveExclusive(const T &low, const T &var, const T &high, const char *varName)
{
	FATAL(low > high, ("range check low [%d] > high [%d]", static_cast<int>(low), static_cast<int>(high)));
	FATAL( (var < low) || (var >= high), ("%s [%d] out of valid range [%d..%d)", varName, static_cast<int>(var), static_cast<int>(low), static_cast<int>(high)));
}

#ifdef _DEBUG
#define VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(low, var, high)    ValidateRangeInclusiveExclusive(low, var, high, #var)
#else
#define VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(low, var, high)    NOP
#endif

// ======================================================================
/**
 * Fatal if the specified var does not satisfy (low <= var <= high).
 *
 * The parameter types must be integral and must be convertable to
 * an int.
 */

template <class T>
inline void ValidateRangeInclusiveInclusive(const T &low, const T &var, const T &high, const char *varName)
{
	FATAL(low > high, ("range check low [%d] > high [%d]", static_cast<int>(low), static_cast<int>(high)));
	FATAL( (var < low) || (var > high), ("%s [%d] out of valid range [%d..%d]", varName, static_cast<int>(var), static_cast<int>(low), static_cast<int>(high)));
}

#ifdef _DEBUG
#define VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(low, var, high)  ValidateRangeInclusiveInclusive(low, var, high, #var)
#else
#define VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(low, var, high)  NOP
#endif

// ======================================================================

class FatalException
{
public:

	enum Zero
	{
		ZeroSourceString
	};

private:

	char *message;

public:

	FatalException(char *newMessage, Zero zero);
	FatalException(const FatalException &fatalException);
	~FatalException(void);

	FatalException &operator =(const FatalException &fatalException);

	const char *getMessage(void) const;
};

struct _EXCEPTION_POINTERS;

// ======================================================================

#endif
