// ======================================================================
//
// runThread.h
// Acy Stapp
//
// Copyright 6/19/2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_runThread_h
#define INCLUDED_runThread_h

#include "sharedThread/ThreadHandle.h"
#include "sharedThread/Thread.h"

// ------------------------------------------------------------------------------------------
// Thread starters
// ------------------------------------------------------------------------------------------

template <class CLASS>
class MemberFunctionThreadZero: public Thread
{
public:
	typedef void (CLASS::* FUNCTION)(void);
	typedef TypedThreadHandle<MemberFunctionThreadZero<CLASS> > Handle;

	MemberFunctionThreadZero(CLASS &i_object, FUNCTION i_func): Thread(), object(i_object), func(i_func) {}
	MemberFunctionThreadZero(const std::string &i_name, CLASS &i_object, FUNCTION i_func): Thread(i_name), object(i_object), func(i_func) {}
private:
	MemberFunctionThreadZero(const MemberFunctionThreadZero &o);
	MemberFunctionThreadZero &operator =(const MemberFunctionThreadZero &o);

	void run() { (object.*func)(); }
	CLASS &object;
	FUNCTION func;
};

template <class CLASS>
typename MemberFunctionThreadZero<CLASS>::Handle runThread(CLASS &o, typename MemberFunctionThreadZero<CLASS>::FUNCTION function)
{
	return TypedThreadHandle<MemberFunctionThreadZero<CLASS> > (new MemberFunctionThreadZero<CLASS>(o, function));
}

template <class CLASS>
TypedThreadHandle<MemberFunctionThreadZero<CLASS> >  runNamedThread(const std::string &name, CLASS &o, typename MemberFunctionThreadZero<CLASS>::FUNCTION function)
{
	return TypedThreadHandle<MemberFunctionThreadZero<CLASS> > (new MemberFunctionThreadZero<CLASS>(name, o, function));
}

// ------------------------------------------------------------------------------------------

template <class CLASS, class ARG1> 
class MemberFunctionThreadOne: public Thread 
{
public:
	typedef void (CLASS::* FUNCTION)(ARG1 o);
	typedef TypedThreadHandle<MemberFunctionThreadOne<CLASS, ARG1> > Handle;

	MemberFunctionThreadOne(CLASS &i_object, FUNCTION i_func, ARG1 i_arg1): Thread(), object(i_object), func(i_func), arg1(i_arg1) {}
	MemberFunctionThreadOne(const std::string &i_name, CLASS &i_object, FUNCTION i_func, ARG1 i_arg1): Thread(i_name), object(i_object), func(i_func), arg1(i_arg1) {}
private:
	MemberFunctionThreadOne(const MemberFunctionThreadOne &o);
	MemberFunctionThreadOne &operator =(const MemberFunctionThreadOne &o);

	void run() { (object.*func)(arg1); }
	CLASS &object;
	FUNCTION func;
	ARG1 arg1;
};

template <class CLASS, class ARG1>
TypedThreadHandle<MemberFunctionThreadOne<CLASS, ARG1> >  runThread(CLASS &o, void (CLASS::* function)(ARG1 o), ARG1 arg1)
{
	return TypedThreadHandle<MemberFunctionThreadOne<CLASS, ARG1> > (new MemberFunctionThreadOne<CLASS, ARG1>(o, function, arg1));
}

template <class CLASS, class ARG1>
TypedThreadHandle<MemberFunctionThreadOne<CLASS, ARG1> >  runNamedThread(const std::string &name, CLASS &o, void (CLASS::* function)(ARG1 o), ARG1 arg1)
{
	return TypedThreadHandle<MemberFunctionThreadOne<CLASS, ARG1> > (new MemberFunctionThreadOne<CLASS, ARG1>(name, o, function, arg1));
}

// ------------------------------------------------------------------------------------------

template <class CLASS, class ARG1, class ARG2> 
class MemberFunctionThreadTwo: public Thread 
{
public:
	typedef void (CLASS::* FUNCTION)(ARG1 o, ARG2 p);
	typedef TypedThreadHandle<MemberFunctionThreadTwo<CLASS, ARG1, ARG2> > Handle;

	MemberFunctionThreadTwo(CLASS &i_object, FUNCTION i_func, ARG1 i_arg1, ARG2 i_arg2): Thread(), object(i_object), func(i_func), arg1(i_arg1), arg2(i_arg2) {}
	MemberFunctionThreadTwo(const std::string &i_name, CLASS &i_object, FUNCTION i_func, ARG1 i_arg1, ARG2 i_arg2): Thread(i_name), object(i_object), func(i_func), arg1(i_arg1), arg2(i_arg2) {}
private:
	MemberFunctionThreadTwo(const MemberFunctionThreadTwo &o);
	MemberFunctionThreadTwo &operator =(const MemberFunctionThreadTwo &o);

	void run() { (object.*func)(arg1, arg2); }
	CLASS &object;
	FUNCTION func;
	ARG1 arg1;
	ARG2 arg2;
};

template <class CLASS, class ARG1, class ARG2>
TypedThreadHandle<MemberFunctionThreadTwo<CLASS, ARG1, ARG2> >  runThread(CLASS &o, void (CLASS::* function)(ARG1 o, ARG2 p), ARG1 arg1, ARG2 arg2)
{
	return TypedThreadHandle<MemberFunctionThreadTwo<CLASS, ARG1, ARG2> > (new MemberFunctionThreadTwo<CLASS, ARG1, ARG2>(o, function, arg1, arg2));
}

template <class CLASS, class ARG1, class ARG2>
TypedThreadHandle<MemberFunctionThreadTwo<CLASS, ARG1, ARG2> >  runNamedThread(const std::string &name, CLASS &o, void (CLASS::* function)(ARG1 o, ARG2 p), ARG1 arg1, ARG2 arg2)
{
	return TypedThreadHandle<MemberFunctionThreadTwo<CLASS, ARG1, ARG2> > (new MemberFunctionThreadTwo<CLASS, ARG1, ARG2>(name, o, function, arg1, arg2));
}			

// -----------------------------------------------------------------------------

class FuncPtrThreadZero: public Thread
{
public:
	typedef void (* FUNCTION)();
	typedef TypedThreadHandle<FuncPtrThreadZero> Handle;

	explicit FuncPtrThreadZero(void (* i_func)()): Thread(), function(i_func) {}
	FuncPtrThreadZero(const std::string &i_name, void (* i_func)()): Thread(i_name), function(i_func) {}
private:
	FuncPtrThreadZero();
	FuncPtrThreadZero(const FuncPtrThreadZero &o);
	FuncPtrThreadZero &operator =(const FuncPtrThreadZero &o);
	void run() { function(); }
	FUNCTION function;
};

TypedThreadHandle<FuncPtrThreadZero> runThread(FuncPtrThreadZero::FUNCTION func);
TypedThreadHandle<FuncPtrThreadZero> runNamedThread(const std::string &name, FuncPtrThreadZero::FUNCTION func);

// -----------------------------------------------------------------------------

template <class ARG1>
class FuncPtrThreadOne: public Thread
{
public:
	typedef void (* FUNCTION)(ARG1 arg1);
	typedef TypedThreadHandle<FuncPtrThreadOne<ARG1> > Handle;

	FuncPtrThreadOne(FUNCTION i_func, ARG1 i_arg1): Thread(), function(i_func), arg1(i_arg1) {}
	FuncPtrThreadOne(const std::string &i_name, FUNCTION i_func, ARG1 i_arg1): Thread(i_name), function(i_func), arg1(i_arg1) {}
private:
	FuncPtrThreadOne(const FuncPtrThreadOne &o);
	FuncPtrThreadOne &operator =(const FuncPtrThreadOne &o);
	void run() { function(arg1); }
	FUNCTION function;
	ARG1 arg1;
};

template <class ARG1>
TypedThreadHandle<FuncPtrThreadOne<ARG1> >  runThread(typename FuncPtrThreadOne<ARG1>::FUNCTION func, ARG1 arg1)
{
	return TypedThreadHandle<FuncPtrThreadOne<ARG1> > (new FuncPtrThreadOne<ARG1>(func, arg1));
}

template <class ARG1>
TypedThreadHandle<FuncPtrThreadOne<ARG1> >  runNamedThread(const std::string &name, typename FuncPtrThreadOne<ARG1>::FUNCTION func, ARG1 arg1)
{
	return TypedThreadHandle<FuncPtrThreadOne<ARG1> > (new FuncPtrThreadOne<ARG1>(name, func, arg1));
}

// -----------------------------------------------------------------------------

template <class ARG1, class ARG2>
class FuncPtrThreadTwo: public Thread
{
public:
	typedef void (* FUNCTION)(ARG1, ARG2);
	typedef TypedThreadHandle<FuncPtrThreadTwo<ARG1,ARG2> > Handle;

	FuncPtrThreadTwo(FUNCTION i_func, ARG1 i_arg1, ARG2 i_arg2): Thread(), function(i_func), arg1(i_arg1), arg2(i_arg2) {}
	FuncPtrThreadTwo(const std::string &i_name, FUNCTION i_func, ARG1 i_arg1, ARG2 i_arg2): Thread(i_name), function(i_func), arg1(i_arg1), arg2(i_arg2) {}
private:
	FuncPtrThreadTwo(const FuncPtrThreadTwo &o);
	FuncPtrThreadTwo &operator =(const FuncPtrThreadTwo &o);
	void run() { function(arg1, arg2); }
	FUNCTION function;
	ARG1 arg1;
	ARG2 arg2;
};

template <class ARG1, class ARG2>
TypedThreadHandle<FuncPtrThreadTwo<ARG1,ARG2> >  runThread(typename FuncPtrThreadTwo<ARG1,ARG2>::FUNCTION func, ARG1 arg1, ARG2 arg2)
{
	return TypedThreadHandle<FuncPtrThreadTwo<ARG1,ARG2> > (new FuncPtrThreadTwo<ARG1,ARG2>(func, arg1, arg2));
}

template <class ARG1, class ARG2>
TypedThreadHandle<FuncPtrThreadTwo<ARG1,ARG2> >  runNamedThread(const std::string &name, typename FuncPtrThreadTwo<ARG1,ARG2>::FUNCTION func, ARG1 arg1, ARG2 arg2)
{
	return TypedThreadHandle<FuncPtrThreadTwo<ARG1,ARG2> > (new FuncPtrThreadTwo<ARG1,ARG2>(name, func, arg1, arg2));
}

#endif
