// ======================================================================
//
// XmlTreeDocument.h
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_XmlTreeDocument_H
#define INCLUDED_XmlTreeDocument_H

// ======================================================================

class XmlTreeNode;

struct _xmlDoc;
typedef struct _xmlDoc xmlDoc;

#include "sharedFoundation/PersistentCrcString.h"
#include <string>

// ======================================================================

class XmlTreeDocument
{
friend class XmlTreeDocumentList;

public:

	XmlTreeNode  getRootTreeNode() const;

	void  fetch() const;
	void  release() const;
	int   getReferenceCount() const;

	CrcString const &getName() const;

	static XmlTreeDocument *createDocument(const char * rootNodeName);
	static XmlTreeDocument *buildDocumentFromText(const std::string &inputText);

	void setRootNode(XmlTreeNode * node);

	void getAsText(std::string &documentText);

private:

	XmlTreeDocument(CrcString const &name, xmlDoc *xmlDocument);
	XmlTreeDocument(xmlDoc *xmlDocument);
	~XmlTreeDocument();

	// Disabled.
	XmlTreeDocument();
	XmlTreeDocument(XmlTreeDocument const &);
	XmlTreeDocument &operator =(XmlTreeDocument const&);

private:

	PersistentCrcString  m_name;
	xmlDoc              *m_xmlDocument;
	mutable int          m_referenceCount;

	bool				 m_track;

};

// ======================================================================

#endif
