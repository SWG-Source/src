// ======================================================================
//
// NoRenderAppearanceTemplate.h
// Portions copyright 2004 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_NoRenderAppearanceTemplate_H
#define INCLUDED_NoRenderAppearanceTemplate_H

// ======================================================================

#include "sharedObject/AppearanceTemplate.h"

// ======================================================================

class NoRenderAppearanceTemplate : public AppearanceTemplate
{
public:

	static void install();

public:

	NoRenderAppearanceTemplate(const char *name, Iff *iff);
	virtual ~NoRenderAppearanceTemplate();

	virtual Appearance * createAppearance() const;

private:

	NoRenderAppearanceTemplate();
	NoRenderAppearanceTemplate(NoRenderAppearanceTemplate const &);
	NoRenderAppearanceTemplate & operator = (NoRenderAppearanceTemplate const &);
};

// ======================================================================

#endif
