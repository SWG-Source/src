// ======================================================================
//
// PortalPropertyTemplateList.h
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_PortalPropertyTemplateList_H
#define INCLUDED_PortalPropertyTemplateList_H

// ======================================================================

class CrcString;
class PortalPropertyTemplate;

// ======================================================================

class PortalPropertyTemplateList
{
	friend class PortalPropertyTemplate;

public:

	static void install();

	static const PortalPropertyTemplate *fetch(const CrcString &crcLowerString);

private:

	static void erase(const PortalPropertyTemplate &portalPropertyTemplate);
};

// ======================================================================

#endif
