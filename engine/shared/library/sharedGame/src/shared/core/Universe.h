// ======================================================================
//
// Universe.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_Universe_H
#define INCLUDED_Universe_H

class ResourceClassObject;

// ======================================================================

/**
 * Singleton that manages Universe objects and functions that are global
 * to the entire game universe.  Client and Server should both derive
 * specific versions of this for their own purposes.
 */
class Universe
{
  public:
	static Universe &             getInstance                 ();
	
  public:
	ResourceClassObject *         getResourceTreeRoot         () const; 
	ResourceClassObject *         getResourceClassByName      (const std::string &name) const;
	ResourceClassObject *         getResourceClassByNameCrc   (uint32 nameCrc) const;
	void                          registerResourceClassObject (ResourceClassObject &newClass);
	virtual void                  resourceClassTreeLoaded     (std::map<int, ResourceClassObject*> &resourceClasses) = 0;
	virtual ResourceClassObject * makeResourceClassObject     () = 0;

	bool                          derivesFromResource         (const std::string & base, const std::string & leaf) const;

  protected:
	//
	// Resource Class Objects
	//
	typedef std::map<std::string, ResourceClassObject*>    ResourceClassNameMap;
	typedef std::map<uint32, ResourceClassObject*>         ResourceClassNameCrcMap;
	ResourceClassNameMap *                                    m_resourceClassNameMap;
	ResourceClassNameCrcMap *                                 m_resourceClassNameCrcMap;
	ResourceClassObject *                                     m_resourceTreeRoot;

  protected:
	static void                   installDerived              (Universe *derivedInstance);
	static void                   remove                      ();
	
	Universe();
	virtual ~Universe();

	static Universe *ms_theInstance;
	static bool      ms_installed;
};

// ======================================================================

/**
* Get the root node of the resource hierarchy
*/

inline ResourceClassObject *Universe::getResourceTreeRoot() const
{
	return m_resourceTreeRoot;
}

// ----------------------------------------------------------------------

inline Universe &Universe::getInstance()
{
	DEBUG_FATAL(!ms_theInstance,("Called Universe::getInstance() before Universe::install()\n"));
	return *ms_theInstance;
}

// ======================================================================

#endif
