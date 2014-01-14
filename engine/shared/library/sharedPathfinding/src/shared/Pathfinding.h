// ======================================================================
//
// Pathfinding.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_Pathfinding_H
#define INCLUDED_Pathfinding_H

class CellProperty;
class BaseClass;
class Iff;
class DebugShapeRenderer;

// ======================================================================

class Pathfinding
{
public:

	static void        install       ( void );
	static void        remove        ( void );

	static void        setupCell     ( CellProperty const * newCell );

	static BaseClass * graphFactory  ( Iff & iff );
	static void        graphWriter   ( BaseClass const * baseGraph, Iff & iff );
	static void        graphRenderer ( BaseClass const * baseGraph, DebugShapeRenderer * renderer );
};

// ======================================================================

#endif

