// ======================================================================
//
// XmlTreeDocumentList.h
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_XmlTreeDocumentList_H
#define INCLUDED_XmlTreeDocumentList_H

// ======================================================================

class CrcString;
class XmlTreeDocument;

// ======================================================================

class XmlTreeDocumentList
{
friend class XmlTreeDocument;

public:

	static void install();

	static XmlTreeDocument const *fetch(CrcString const &filename);

private:

	static void stopTracking(XmlTreeDocument const *document);

};

// ======================================================================

#endif
