// ======================================================================
//
// CallbackReceiver.h
// copyright 2001 Sony Online Entertainment
// 
// ======================================================================

#ifndef INCLUDED_CallbackReceiver_H
#define INCLUDED_CallbackReceiver_H

// ======================================================================

class CallbackReceiver
{
public:

	virtual void performCallback() = 0;

protected:

	CallbackReceiver() {}
	virtual ~CallbackReceiver() {}

};

// ======================================================================

#endif
