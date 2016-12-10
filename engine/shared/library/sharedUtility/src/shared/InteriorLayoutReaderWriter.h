// ======================================================================
//
// InteriorLayoutReaderWriter.h
// asommers
//
// copyright 2002, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_InteriorLayoutReaderWriter_H
#define INCLUDED_InteriorLayoutReaderWriter_H

// ======================================================================

#include "sharedFoundation/PersistentCrcString.h"

class LessPointerComparator;
class Transform;
class Iff;

// ======================================================================

class InteriorLayoutReaderWriter
{
public:

	InteriorLayoutReaderWriter ();
	~InteriorLayoutReaderWriter ();

	bool                  load (const char* fileName);
	bool                  save (const char* fileName) const;

	CrcString const & getFileName() const;

	//-- reader interface
	int getNumberOfCellNames() const;
	CrcString const & getCellName(int index) const;
	bool hasCellName(CrcString const & cellName) const;
	int getNumberOfObjects(CrcString const & cellName) const;
	CrcString const & getObjectTemplateName(CrcString const & cellName, int index) const;
	Transform const & getTransform_o2p(CrcString const & cellName, int index) const;

	//-- preload interface
	int getNumberOfObjects() const;
	typedef std::vector<CrcString const *> CrcStringVector;
	void getObjectTemplateNames(CrcStringVector & crcStringVector);

	//-- writer interface
	void                  clear ();
	void addObject(char const * cellName, char const * objectTemplateName, Transform const & transform_o2p);

private:

	void                  load (Iff& iff);
	void                  load_0000 (Iff& iff);

	void                  save (Iff& iff) const;

private:

	InteriorLayoutReaderWriter (const InteriorLayoutReaderWriter&);
	InteriorLayoutReaderWriter& operator= (const InteriorLayoutReaderWriter&);

private:

	PersistentCrcString m_fileName;

	class Node;
	typedef std::map<CrcString const *, Node *, LessPointerComparator> NodeMap;
	NodeMap * const m_nodeMap;
	int m_numberOfObjects;
};

// ======================================================================

#endif
