// ======================================================================
//
// runThread.cpp
// Acy Stapp
//
// Copyright 6/19/2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedThread/FirstSharedThread.h"
#include "sharedThread/RunThread.h"

FuncPtrThreadZero::Handle runThread(void (* func)())
{
  return TypedThreadHandle<FuncPtrThreadZero> (new FuncPtrThreadZero(func));
}

FuncPtrThreadZero::Handle runNamedThread(const std::string &name, void (* func)())
{
	return TypedThreadHandle<FuncPtrThreadZero> (new FuncPtrThreadZero(name, func));
}
