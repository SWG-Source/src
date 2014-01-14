// ======================================================================
//
// NoRenderAppearanceTemplate.cpp
// Portions copyright 2004 Sony Online Entertainment
// All Rights Reserved
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/NoRenderAppearanceTemplate.h"

#include "sharedFile/Iff.h"
#include "sharedFoundation/Tag.h"
#include "sharedObject/AppearanceTemplateList.h"
#include "sharedObject/NoRenderAppearance.h"

// ======================================================================

namespace NoRenderAppearanceTemplateNamespace
{
	const Tag TAG_NRND = TAG(N,R,N,D);

	AppearanceTemplate *create(const char *newName, Iff *iff);
};
using namespace NoRenderAppearanceTemplateNamespace;

// ======================================================================

void NoRenderAppearanceTemplate::install()
{
	AppearanceTemplateList::assignBinding(TAG_NRND, NoRenderAppearanceTemplateNamespace::create);
}

// ----------------------------------------------------------------------

AppearanceTemplate *NoRenderAppearanceTemplateNamespace::create(const char *newName, Iff *iff)
{
	return new NoRenderAppearanceTemplate(newName, iff);
}

// ======================================================================


NoRenderAppearanceTemplate::NoRenderAppearanceTemplate(const char *name, Iff *iff)
: AppearanceTemplate(name)
{
	iff->enterForm(TAG_NRND);
	iff->exitForm(TAG_NRND);
}

// ----------------------------------------------------------------------

NoRenderAppearanceTemplate::~NoRenderAppearanceTemplate()
{
}

// ----------------------------------------------------------------------

Appearance * NoRenderAppearanceTemplate::createAppearance() const
{
	return new NoRenderAppearance(this);
}

// ======================================================================
