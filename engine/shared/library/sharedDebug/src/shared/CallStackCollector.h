// ======================================================================
//
// CallStackCollector.h
// asommers
//
// Copyright 2004, Sony Online Entertainment
// All Rights Reserved
//
// ======================================================================

#ifndef INCLUDED_CallStackCollector_H
#define INCLUDED_CallStackCollector_H

// ======================================================================

class CallStackCollector
{
public:

	static void install();

	static void sample(char const * name);
};

// ======================================================================

#endif
