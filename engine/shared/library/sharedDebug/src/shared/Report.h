// ======================================================================
//
// Report.cpp
// copyright 1999 Bootprint Entertainment
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_Report_H
#define INCLUDED_Report_H

// ======================================================================

class Report
{
public:

	enum
	{
		RF_print   = BINARY2(0000,0001),
		RF_log     = BINARY2(0000,0010),
		RF_warning = BINARY2(0000,0100),
		RF_fatal   = BINARY2(0000,1000),
		RF_console = BINARY2(0001,0000),
		RF_dialog  = BINARY2(0010,0000)
	};

public:

	typedef void (*Callback)(char const *string);

public:

	static void           install();

	static void           bindLogCallback(Callback callback);
	static void           bindWarningCallback(Callback callback);
	static void           bindFatalCallback(Callback callback);

	static DLLEXPORT void setFlags(int flags);
	static void           puts(const char *string);
	static void           vprintf(const char *format, va_list va);
	static DLLEXPORT void printf(const char *format, ...);
};

// ======================================================================

#define REPORT(expr, flags, printf2)             ((expr) ? Report::setFlags(flags), Report::printf printf2 : NOP)
#define REPORT_LOG(expr, printf)                 REPORT(expr, Report::RF_log, printf)
#define REPORT_PRINT(expr, printf)               REPORT(expr, Report::RF_print, printf)
#define REPORT_LOG_PRINT(expr, printf)           REPORT(expr, Report::RF_log | Report::RF_print, printf)

#ifdef _DEBUG

	#define DEBUG_REPORT(expr, flags, printf)      REPORT(expr, flags, printf)
	#define DEBUG_REPORT_LOG(expr, printf)         REPORT_LOG(expr, printf)
	#define DEBUG_REPORT_PRINT(expr, printf)       REPORT_PRINT(expr, printf)
	#define DEBUG_REPORT_LOG_PRINT(expr,printf)    REPORT_LOG_PRINT(expr, printf)
 
#else

	#define DEBUG_REPORT(expr, flags, printf2)     NOP
	#define DEBUG_REPORT_LOG(expr, printf)         NOP
	#define DEBUG_REPORT_PRINT(expr, printf)       NOP
	#define DEBUG_REPORT_LOG_PRINT(expr, printf)   NOP

#endif

// ======================================================================

#endif
