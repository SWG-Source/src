/**
***  Copyright  (C) 1999-2001 Intel Corporation. All rights reserved.
***
*** The information and source code contained herein is the exclusive
*** property of Intel Corporation and may not be disclosed, examined
*** or reproduced in whole or in part without explicit written authorization
*** from the company.
**/

#if !defined(_VT_TIGGERS_)
#define VT_TRIGGERS_API _declspec(dllimport)
#else
#define VT_TRIGGERS_API _declspec(dllexport)
#endif

#define DEFAULTTIMEINTERVAL 1000

#ifdef __cplusplus
extern "C" {
VT_TRIGGERS_API DWORD VT_RegisterTrigger(LPTSTR pszTriggerName, DWORD dwIncrements, LPTSTR pszUnits, HANDLE* pHandle);
VT_TRIGGERS_API DWORD VT_UnregisterTrigger(HANDLE hTriggerID);
VT_TRIGGERS_API DWORD VT_SetTrigger(HANDLE hTriggerID);
}
#endif
