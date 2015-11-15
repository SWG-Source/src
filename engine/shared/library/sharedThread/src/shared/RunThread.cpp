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

#if 0
// Testing stuff

class testclass
{
public:
	void zeroArg() {}
	void oneArg(testclass *) {}
	void twoArg(testclass *, int) {}
};

testclass m;

typedef MemberFunctionThreadZero<testclass>::Handle AsyncTestMemberFunctionZero;
AsyncTestMemberFunctionZero temp = runThread(m, testclass::zeroArg);

typedef MemberFunctionThreadOne<testclass, testclass *>::Handle AsyncTestMemberFunctionOne;
AsyncTestMemberFunctionOne temp2 = runThread(m, testclass::oneArg, &m);

typedef MemberFunctionThreadTwo<testclass, testclass *, int>::Handle AsyncTestMemberFunctionTwo;
AsyncTestMemberFunctionTwo temp3 = runThread(m, testclass::twoArg, &m, 4);
#endif
