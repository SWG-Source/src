// ======================================================================
//
// ObjectTemplate.h - This is a scaled-down version of ObjectTemplate from 
// sharedObject library in order to avoid nasty dependencies.
//
// jeff grills
//
// copyright 1999 Bootprint Entertainment
//
// ======================================================================

#ifndef _INCLUDED_ObjectTemplate_H
#define _INCLUDED_ObjectTemplate_H

// ======================================================================

class ObjectTemplate;

#include "sharedFile/Iff.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/DataResource.h"
#include "sharedFoundation/DataResourceList.h"



typedef DataResourceList<ObjectTemplate> ObjectTemplateList;


// ======================================================================

// Base class for ObjectTemplates

class ObjectTemplate : public DataResource
{
public:

	explicit ObjectTemplate(const std::string & filename);
	virtual ~ObjectTemplate(void) = 0;

	void loadFromIff(Iff &file);
	void saveToIff(Iff &file);

	virtual Tag               getId(void) const = 0;
	virtual void              addReference(void) const;
	virtual void              releaseReference(void) const;

protected:

	virtual void release(void) const;

	virtual void preLoad(void);
	virtual void load(Iff &file) = 0;
	virtual void postLoad(void);

	virtual void preSave(void);
	virtual void save(Iff &file) = 0;
	virtual void postSave(void);

//	static void registerTemplate(Tag id, 
//		ObjectTemplateList::CreateDataResourceFunc createFunc);

protected:

	const ObjectTemplate *m_baseData;		// template to use as a base for this template

private:

	ObjectTemplate(const ObjectTemplate &);
	ObjectTemplate &operator =(const ObjectTemplate &);
};

// ======================================================================

inline void ObjectTemplate::release(void) const
{
	ObjectTemplateList::release(*this);
}	// ObjectTemplate::release

inline void ObjectTemplate::loadFromIff(Iff &file)
{
	preLoad();
	load(file);
	postLoad();
}	// ObjectTemplate::loadFromIff

/**
 * Function called before the template is loaded.
 */
inline void ObjectTemplate::preLoad(void)
{
}	// ObjectTemplate::preLoad

/**
 * Function called after the template is loaded.
 */
inline void ObjectTemplate::postLoad(void)
{
}	// ObjectTemplate::postLoad

inline void ObjectTemplate::saveToIff(Iff &file)
{
	preSave();
	save(file);
	postSave();
}	// ObjectTemplate::saveToIff

/**
 * Function called before the template is saved.
 */
inline void ObjectTemplate::preSave(void)
{
}	// ObjectTemplate::preSave

/**
 * Saves the template data to an iff file.
 *
 * @param file		file to save to
 */
inline void ObjectTemplate::save(Iff &file)
{
	UNREF(file);
}	// ObjectTemplate::save

/**
 * Function called after the template is saved.
 */
inline void ObjectTemplate::postSave(void)
{
}	// ObjectTemplate::postSave


#endif	// _INCLUDED_ObjectTemplate_H
