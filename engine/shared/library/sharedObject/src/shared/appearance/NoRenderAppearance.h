// ======================================================================
//
// NoRenderAppearance.h
// Portions copyright 1999 Bootprint Entertainment
// Portions copyright 2001, 2002 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_NoRenderAppearance_H
#define INCLUDED_NoRenderAppearance_H

// ======================================================================

class NoRenderAppearanceTemplate;

#include "sharedObject/Appearance.h"

// ======================================================================

class NoRenderAppearance : public Appearance
{
	friend class NoRenderAppearanceTemplate;
	
private:

	NoRenderAppearance(NoRenderAppearanceTemplate const * appearanceTemplate);

private:
	// disabled
	NoRenderAppearance();
	NoRenderAppearance(NoRenderAppearance const &);
	NoRenderAppearance & operator = (NoRenderAppearance const &);
};

// ======================================================================

#endif
