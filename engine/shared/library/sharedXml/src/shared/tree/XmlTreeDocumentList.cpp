// ======================================================================
//
// XmlTreeDocumentList.cpp
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedXml/FirstSharedXml.h"
#include "sharedXml/XmlTreeDocumentList.h"

#include "libxml/parser.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/CrcString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedXml/XmlTreeDocument.h"

#include <map>

// ======================================================================

namespace XmlTreeDocumentListNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef std::map<CrcString const*, XmlTreeDocument*, LessPointerComparator>  NamedDocumentMap;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void  remove();

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool              s_installed;
	NamedDocumentMap  s_documents;

#ifdef _DEBUG
	bool              s_logXmlDomTreeSize;
#endif
}

using namespace XmlTreeDocumentListNamespace;

// ======================================================================
// namespace XmlTreeDocumentListNamespace
// ======================================================================

void XmlTreeDocumentListNamespace::remove()
{
	FATAL(!s_installed, ("XmlTreeDocumentList not installed."));
	s_installed = false;

	//-- Clean up and report on any XmlTreeDocumentList resources still open.
	//   Keep these leaked so we can debug allocation call stacks when we do have them leaking.
	if (!s_documents.empty())
	{
		DEBUG_WARNING(true, ("XmlTreeDocumentList: leaked [%d] xml tree documents, see below for details.", static_cast<int>(s_documents.size())));

		NamedDocumentMap::iterator const endIt = s_documents.end();
		for (NamedDocumentMap::iterator it = s_documents.begin(); it != endIt; ++it)
		{
			DEBUG_REPORT_LOG(true, ("-- xml tree document name [%s], reference count [%d].\n", it->first ? it->first->getString() : "<nullptr name>", it->second ? it->second->getReferenceCount() : -1));
		}
	}
}

// ======================================================================
// class XmlTreeDocumentList: PUBLIC STATIC
// ======================================================================

void XmlTreeDocumentList::install()
{
	FATAL(s_installed, ("XmlTreeDocumentList already installed."));

#ifdef _DEBUG
	DebugFlags::registerFlag(s_logXmlDomTreeSize, "SharedXml", "logXmlDomTreeSize");
#endif

	s_installed = true;
	ExitChain::add(XmlTreeDocumentListNamespace::remove, "XmlTreeDocumentList");
}

// ----------------------------------------------------------------------

XmlTreeDocument const *XmlTreeDocumentList::fetch(CrcString const &filename)
{
	FATAL(!s_installed, ("XmlTreeDocumentList not installed."));

	//-- Check if we already have an entry.
	NamedDocumentMap::iterator const lowerBound = s_documents.lower_bound(&filename);
	bool const haveEntry = (lowerBound != s_documents.end()) && !s_documents.key_comp()(&filename, lowerBound->first);

	//-- Handle existing entry.
	if (haveEntry)
	{
		FATAL(!lowerBound->second, ("XmlTreeDocumentList::fetch() found an entry for filename [%s] but the mapped document was nullptr.", filename.getString()));

		// Increment reference count and return.
		lowerBound->second->fetch();
		return lowerBound->second;
	}

	//-- Get file size and contents.
	char const *const cPathName = filename.getString();

	AbstractFile *const file = TreeFile::open(cPathName, AbstractFile::PriorityData, false);
	FATAL(!file, ("XmlTreeDocumentList::fetch(): TreeFile::open() failed for filename [%s].", filename.getString()));

	int const   fileSize     = file->length();
	byte *const fileContents = file->readEntireFileAndClose();

	//-- Close the file.
	file->close();
	delete file;

	//-- Create an XML DOM tree out of it.
	xmlDocPtr const xmlDocument = xmlParseMemory(reinterpret_cast<char const *>(fileContents), fileSize);
	FATAL(!xmlDocument, ("xmlParseMemory() returned nullptr when parsing contents of file [%s].", cPathName));

	// Release initial file contents buffer.
	delete [] fileContents;

	//-- Create the XmlTreeDocument, bump up reference count and add it to the map.
	XmlTreeDocument *const newDocument = new XmlTreeDocument(filename, xmlDocument);
	newDocument->fetch();

	std::pair<NamedDocumentMap::iterator, bool> const insertResult = s_documents.insert(NamedDocumentMap::value_type(&newDocument->getName(), newDocument));
	FATAL(!insertResult.second, ("XmlTreeDocumentList::fetch() failed to insert document filename [%s] into the map.", filename.getString()));

	return newDocument;
}

// ======================================================================
// class XmlTreeDocumentList: PRIVATE STATIC
// ======================================================================

void XmlTreeDocumentList::stopTracking(XmlTreeDocument const *document)
{
	FATAL(!s_installed, ("XmlTreeDocumentList not installed."));
	FATAL(!document, ("XmlTreeDocumentList::stopTracking(): nullptr document passed in."));

	//-- Find the map entry for the xml tree document.
	NamedDocumentMap::iterator const findIt = s_documents.find(&(document->getName()));
	FATAL(findIt == s_documents.end(), ("XmlTreeDocumentList::stopTracking(): xml tree document [%s] not tracked.", document->getName().getString()));

	//-- Remove the entry.
	s_documents.erase(findIt);
}

// ======================================================================
