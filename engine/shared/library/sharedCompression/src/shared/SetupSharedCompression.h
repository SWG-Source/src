// ======================================================================
//
// SetupSharedCompression.h
// Copyright 2003, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_SetupSharedCompression_H
#define INCLUDED_SetupSharedCompression_H

// ======================================================================

class SetupSharedCompression
{
public:

	struct Data
	{
		int numberOfThreadsAccessingZlib;

		Data();
	};

public:

	static void install();
	static void install(Data const &data);

private:
	SetupSharedCompression();
	SetupSharedCompression(const SetupSharedCompression &);
	SetupSharedCompression &operator =(const SetupSharedCompression &);
};


// ======================================================================

#endif
