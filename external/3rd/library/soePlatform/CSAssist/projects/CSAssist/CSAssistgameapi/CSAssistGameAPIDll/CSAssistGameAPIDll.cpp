// CSAssistGameAPIDll.cpp : Defines the entry point for the DLL application.
//
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <cstdio>

//#include "stdafx.h"
#include "CSAssistGameAPIDll.h"

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}


// This is an example of an exported variable
//CSASSISTGAMEAPIDLL_API int nCSAssistGameAPIDll=0;

// This is an example of an exported function.
//CSASSISTGAMEAPIDLL_API int fnCSAssistGameAPIDll(void)
//{
//	return 42;
//}

// This is the constructor of a class that has been exported.
// see CSAssistGameAPIDll.h for the class definition
//CCSAssistGameAPIDll::CCSAssistGameAPIDll()
//{ 
//	return; 
//}

