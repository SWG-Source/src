
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the CSASSISTGAMEAPIDLL_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// CSASSISTGAMEAPIDLL_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
/*
#ifdef USE_CSADLL
#ifdef CSASSISTGAMEAPIDLL_EXPORTS
#define CSASSISTGAMEAPIDLL_API __declspec(dllexport)
#else
#define CSASSISTGAMEAPIDLL_API __declspec(dllimport)
#endif
#else
#define CSASSISTGAMEAPIDLL_API
#endif
*/
#define CSASSISTGAMEAPIDLL_API __declspec(dllexport)

// This class is exported from the CSAssistGameAPIDll.dll
//class CSASSISTGAMEAPIDLL_API CCSAssistGameAPIDll {
//public:
//	CCSAssistGameAPIDll(void);
	// TODO: add your methods here.
//};

//extern CSASSISTGAMEAPIDLL_API int nCSAssistGameAPIDll;

//CSASSISTGAMEAPIDLL_API int fnCSAssistGameAPIDll(void);

