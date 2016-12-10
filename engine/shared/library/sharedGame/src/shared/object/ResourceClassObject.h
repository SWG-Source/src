//========================================================================
//
// ResourceClassObject.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef INCLUDED_ResourceClassObject_H
#define INCLUDED_ResourceClassObject_H

class DataTable;
class StringId;

// ======================================================================

/**
* Represents a resource class, i.e. a non-leaf node in the resource
* tree.  The resource class hierarchy is static at runtime, and is read
* in from a datatable.
*/

class ResourceClassObject
{
public:
	typedef std::map<std::string, std::pair<int, int> > ResourceAttributeRangesType;

public:
	ResourceClassObject();
	virtual ~ResourceClassObject();
	
	static void install();
	static void remove();
	static bool isClassExcludedFromCommodities(std::string const & className);
	static std::set<std::string> const & getClassesExcludedFromCommodities();

public:
	// getter functions
	virtual void            debugOutput          (std::string &output, int numSpaces=0) const;
	bool                    isRoot               () const;
	bool                    isLeaf               () const;
	const std::string &     getResourceClassName () const;
	const StringId &        getFriendlyName      () const;
	ResourceClassObject *   getParent            () const;
	ResourceClassObject const * getRecycledVersion() const;
	void                    getNameTable         (std::string &buffer) const;
	std::string             getNameTable         () const;
	bool                    isDerivedFrom        (const ResourceClassObject &masterClass) const;
	void                    getCrateTemplate     (std::string &buffer) const;
	int                     getMinTypes          () const;
	int                     getMaxTypes          () const;
	void                    getChildren          (std::vector<const ResourceClassObject *> & children, bool recurse) const;
	void                    getLeafChildren      (std::vector<const ResourceClassObject *> & children) const;
	int                     getMinPools          () const;
	int                     getMaxPools          () const;
	bool                    isRecycled           () const;
	bool                    isPermanent() const;
	
	ResourceAttributeRangesType const & getResourceAttributeRanges() const;
	
public:
	// Universe system functions (rebuild data structures on load)
	virtual void  setupUniverse  ();
	void          addChildClass  (ResourceClassObject &child);
	
public:
	static void   loadTreeFromIff();
	
protected:
	std::string            m_resourceClassName; ///< The (abbreviated) name of the resource class.
	ResourceClassObject *  m_parentClass;
	StringId *             m_friendlyName;      ///< The string id of the printable name for this resource class
	int                    m_minTypes;          ///< The minimum number of non-depleted types that should be derived from this class or its children.  Used to decide when to spawn a new type
	int                    m_maxTypes;          ///< The maximum number of non-depleted types that should be derived from this class or its children.  Used to decide when a new type must be spawned elsewhere in the tree
	int                    m_minPools;
	int                    m_maxPools;
	std::string            m_crateTemplate;     ///< The kind of crate to use for this class
	std::string            m_nameTable;         ///< The name table to use when generating names for ResourceTypes derived from class
	bool                   m_recycled;          ///< The resource is designated as "recycled" (influences how it is spawned and used)
	bool                   m_permanent;         ///< resource never expires
	ResourceClassObject *  m_recycledVersion;   ///< Pointer to the class representing the recycled version of this resource
	ResourceAttributeRangesType * m_resourceAttributeRanges;
	
protected:
	typedef std::vector<ResourceClassObject*> ClassList;
	ClassList * m_children;
	
private:
	virtual const std::string & getDefaultResourceCrate     () const;
	virtual const std::string & getDefaultResourceNameTable () const;
	void                        setAttributesFromDataTable  (DataTable &table, int row);
	
private:
	ResourceClassObject(const ResourceClassObject& rhs);
	ResourceClassObject& operator=(const ResourceClassObject& rhs);

private:
	static std::string *ms_attNameHeader;
	static std::string *ms_attMaxHeader;
	static std::string *ms_attMinHeader;
};

// ----------------------------------------------------------------------

inline const std::string & ResourceClassObject::getResourceClassName () const
{
	return m_resourceClassName;
}

// ----------------------------------------------------------------------

inline const StringId & ResourceClassObject::getFriendlyName () const
{
	return *NON_NULL(m_friendlyName);
}

// ----------------------------------------------------------------------

inline bool ResourceClassObject::isRoot() const
{
	return (m_parentClass == nullptr);
}

// ----------------------------------------------------------------------

inline ResourceClassObject *ResourceClassObject::getParent() const
{
	return m_parentClass;
}

// ----------------------------------------------------------------------

inline int ResourceClassObject::getMinTypes() const
{
	return m_minTypes;
}

// ----------------------------------------------------------------------

inline int ResourceClassObject::getMaxTypes() const
{
	return m_maxTypes;
}

// ======================================================================

#endif
