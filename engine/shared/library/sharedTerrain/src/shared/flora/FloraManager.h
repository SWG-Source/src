//===================================================================
//
// FloraManager.h
// asommers
//
// copyright 2001, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_FloraManager_H
#define INCLUDED_FloraManager_H

//===================================================================

class Object;

//===================================================================

class FloraManager
{
public:

	static const Object* getReferenceObject ();
	static void          setReferenceObject (const Object* referenceObject);

public:

	FloraManager ();
	virtual ~FloraManager ()=0;
};

//===================================================================

#endif
