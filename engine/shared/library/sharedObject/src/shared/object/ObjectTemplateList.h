// ======================================================================
//
// ObjectTemplateList.h
// copyright 1999 Bootprint Entertainment
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ObjectTemplateList_H
#define INCLUDED_ObjectTemplateList_H

// ======================================================================

#include "sharedFoundation/Tag.h"

class ConstCharCrcString;
class CrcString;
class CrcStringTable;
class Iff;
class Object;
class ObjectTemplate;

// ======================================================================

class ObjectTemplateList
{
	friend class ObjectTemplate;

public:

	typedef ObjectTemplate *(*CreateDataResourceFunc)(const std::string &filename);

public:

	static void install(bool loadObjectTemplateCrcStringTable);
	static void remove();

	static void logLoadedObjectTemplates();

	// functions to create a new DataResource
	static const ObjectTemplate  *fetch(Iff &source);
	static const ObjectTemplate  *fetch(const CrcString &filename);
	static const ObjectTemplate  *fetch(uint32 crc);
	static const ObjectTemplate  *fetch(const std::string &filename);
	static const ObjectTemplate  *fetch(const char *filename);
	static Object                *createObject (const CrcString &fileName);
	static bool                   isLoaded(const std::string &fileName);

	// function to reload a data resource with new data
	static ObjectTemplate        *reload(Iff &source);

	static void                   registerTemplate(Tag id, CreateDataResourceFunc createFunc);
	static CreateDataResourceFunc assignBinding(Tag id, CreateDataResourceFunc createFunc);
	static CreateDataResourceFunc removeBinding(Tag tag);

	// CrcStringTable functions
	static void                      loadCrcStringTable(const char *fileName);
	static ConstCharCrcString const  lookUp(const char * string);
	static ConstCharCrcString const  lookUp(uint32 crc);

	static void garbageCollect ();

	static void getAllTemplateNamesFromCrcStringTable(std::vector<const char *> & objectTemplateNames);

private:

	static void release(const ObjectTemplate &dataResource);

private:

	ObjectTemplateList();
	~ObjectTemplateList();
	ObjectTemplateList(const ObjectTemplateList &);
	ObjectTemplateList &operator =(const ObjectTemplateList &);
};

// ======================================================================

#endif	
