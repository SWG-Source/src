// ======================================================================
//
// AppearanceTemplateList.h
// Portions copyright 1999 Bootprint Entertainment
// Portions copyright 2000-2002 Sony Online Entertainment
// All Rights Reserved.
// 
// ======================================================================

#ifndef INCLUDED_AppearanceTemplateList_H
#define INCLUDED_AppearanceTemplateList_H

// ======================================================================

#include "sharedFoundation/Tag.h"

class Appearance;
class AppearanceTemplate;
class Iff;

// ======================================================================

class AppearanceTemplateList
{
public:

	typedef AppearanceTemplate *(*CreateFunction)(const char *name, Iff *iff);

public:

	static void                      install(bool useTimedTemplates, bool ensureDefaultAppearanceExists);

	static void                      assignBinding(Tag tag, CreateFunction createFunction);
	static void                      removeBinding(Tag tag);

	static void                      update(float elapsedTime);
	static void                      setAllowTimedTemplates (bool allowTimedTemplates);

	static const AppearanceTemplate *fetch(const char *fileName);
	static const AppearanceTemplate *fetch(const char *fileName, bool & found);
	static const AppearanceTemplate *fetch(Iff *iff);
	static const AppearanceTemplate *fetch(const AppearanceTemplate *appearanceTemplate);
	static const AppearanceTemplate *fetchNew(AppearanceTemplate *appearanceTemplate);
	static void                      release(const AppearanceTemplate *appearanceTemplate);

	static Appearance               *createAppearance(const char *fileName);

	static const char*               getDefaultAppearanceTemplateName();

	static void garbageCollect ();

private:

	// disabled
	AppearanceTemplateList();
	AppearanceTemplateList(const AppearanceTemplateList &);
	AppearanceTemplateList &operator =(const AppearanceTemplateList &);
};

// ======================================================================

#endif
