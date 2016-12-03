// ======================================================================
//
// SetupSharedXml.cpp
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedXml/FirstSharedXml.h"
#include "sharedXml/SetupSharedXml.h"

#include "libxml/parser.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedXml/XmlTreeDocumentList.h"

// ======================================================================

namespace SetupSharedXmlNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void remove();

	void *xmlAllocate(size_t byteCount);
	void  xmlFree2(void *memory);
	char *xmlDuplicateString(char const *source);
	void *xmlReAllocate(void *memory, size_t byteCount);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool  s_installed;
}

using namespace SetupSharedXmlNamespace;

// ======================================================================
// namespace SetupSharedXmlNamespace
// ======================================================================

void SetupSharedXmlNamespace::remove()
{
	FATAL(!s_installed, ("SetupSharedXml not installed."));
	s_installed = false;

	xmlCleanupParser();
}

// ----------------------------------------------------------------------

void *SetupSharedXmlNamespace::xmlAllocate(size_t byteCount)
{
	return new byte[byteCount];
}

// ----------------------------------------------------------------------

void SetupSharedXmlNamespace::xmlFree2(void *memory)
{
	delete [] reinterpret_cast<byte*>(memory);
}

// ----------------------------------------------------------------------

char *SetupSharedXmlNamespace::xmlDuplicateString(char const *source)
{
	return DuplicateString(source);
}

// ----------------------------------------------------------------------

void *SetupSharedXmlNamespace::xmlReAllocate(void *memory, size_t byteCount)
{
	return realloc(memory, byteCount);
}

// ======================================================================
// class SetupSharedXml: PUBLIC STATIC
// ======================================================================

void SetupSharedXml::install()
{
	InstallTimer const installTimer("SetupSharedXml::install");

	FATAL(s_installed, ("SetupSharedXml already installed."));

	//-- Tell libxml to use our memory management functions via these stubs.
	int const result = xmlMemSetup(SetupSharedXmlNamespace::xmlFree2, SetupSharedXmlNamespace::xmlAllocate, SetupSharedXmlNamespace::xmlReAllocate, SetupSharedXmlNamespace::xmlDuplicateString);
	FATAL(result != 0, ("sharedXml failed to install memory handlers with error code [%d].", result));

	XmlTreeDocumentList::install();

	s_installed = true;
	ExitChain::add(SetupSharedXmlNamespace::remove, "SetupSharedXml");
}

// ======================================================================
