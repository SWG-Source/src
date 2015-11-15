// ======================================================================
//
// PortalPropertyTemplateList.cpp
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/PortalPropertyTemplateList.h"

#include "sharedFile/TreeFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedObject/PortalPropertyTemplate.h"

#include <map>
#include <vector>

// ======================================================================

namespace PortalPropertyTemplateListNamespace
{
	typedef std::map<CrcString const *, PortalPropertyTemplate *, LessPointerComparator> TemplateList;

	void remove();

	static bool           ms_installed;
	static TemplateList   ms_templateList;
}

using namespace PortalPropertyTemplateListNamespace;

// ======================================================================

void PortalPropertyTemplateList::install()
{
	DEBUG_FATAL(ms_installed, ("double install"));
	ms_installed = true;
	ExitChain::add(remove, "PortalPropertyTemplateListImp::remove");
}

// ----------------------------------------------------------------------

void PortalPropertyTemplateListNamespace::remove()
{
	DEBUG_FATAL(!ms_templateList.empty(), ("%d PortalPropertyTemplates still allocated", static_cast<int>(ms_templateList.size())));
}

// ----------------------------------------------------------------------

const PortalPropertyTemplate *PortalPropertyTemplateList::fetch(const CrcString &name)
{
	PortalPropertyTemplate *result;

	TemplateList::iterator i = ms_templateList.find(&name);
	if (i == ms_templateList.end())
	{
		result = new PortalPropertyTemplate(name);
		const bool inserted = ms_templateList.insert(TemplateList::value_type(&result->getCrcString(), result)).second;
		UNREF(inserted);
		DEBUG_FATAL(!inserted, ("item was already present"));
	}
	else
		result = i->second;

	result->fetch();
	return result;
}

// ----------------------------------------------------------------------

void PortalPropertyTemplateList::erase(const PortalPropertyTemplate &portalPropertyTemplate)
{
	TemplateList::iterator i = ms_templateList.find(&portalPropertyTemplate.getCrcString());
	DEBUG_FATAL(i == ms_templateList.end(), ("item not found"));
	ms_templateList.erase(i);
}

// ======================================================================
