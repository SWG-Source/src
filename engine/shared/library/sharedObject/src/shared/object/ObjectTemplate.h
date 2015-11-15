// ======================================================================
//
// ObjectTemplate.h
// copyright 1999 Bootprint Entertainment
// copyright 2001 Sony Online Entertainment
//
// Base class for ObjectTemplates
//
// ======================================================================

#ifndef INCLUDED_ObjectTemplate_H
#define INCLUDED_ObjectTemplate_H

// ======================================================================

#include "sharedFoundation/DataResource.h"
#include "sharedFoundation/Tag.h"

class Iff;
class Object;
class ServerObjectTemplate;
class SharedObjectTemplate;

// ======================================================================

class ObjectTemplate : public DataResource
{
public:

	static void        install();

	static Object     *createObject(const char *fileName);
	static const bool  isLoaded(const std::string & fileName);

public:

	explicit ObjectTemplate(const std::string &filename);
	virtual ~ObjectTemplate() = 0;

	virtual ServerObjectTemplate *       asServerObjectTemplate();
	virtual ServerObjectTemplate const * asServerObjectTemplate() const;
	virtual SharedObjectTemplate *       asSharedObjectTemplate();
	virtual SharedObjectTemplate const * asSharedObjectTemplate() const;

	void            loadFromIff(Iff &iff);
	void            saveToIff(Iff &iff);

	const ObjectTemplate * getBaseTemplate(void) const;

	virtual Object *createObject() const;
	virtual Tag     getId() const = 0;
	virtual void    addReference() const;
	virtual void    releaseReference() const;

	bool               derivesFrom (const std::string & potentialAncestorName) const;

#ifdef _DEBUG
public:
	// special code used by datalint
	virtual void testValues(void) const;
#endif

protected:

	virtual void release() const;

	virtual void preLoad();
	virtual void load(Iff &iff) = 0;
	virtual void postLoad();

protected:

	// template to use as a base for this template
	const ObjectTemplate *m_baseData;

private:

	ObjectTemplate(const ObjectTemplate &);
	ObjectTemplate &operator =(const ObjectTemplate &);
};


// ======================================================================

inline const ObjectTemplate * ObjectTemplate::getBaseTemplate(void) const
{
	return m_baseData;
}


// ======================================================================


#endif	
