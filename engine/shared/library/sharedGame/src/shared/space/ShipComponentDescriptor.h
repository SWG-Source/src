//======================================================================
//
// ShipComponentDescriptor.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipComponentDescriptor_H
#define INCLUDED_ShipComponentDescriptor_H

//======================================================================

#include "sharedGame/ShipComponentType.h"
#include "sharedFoundation/PersistentCrcString.h"

class ObjectTemplate;

//----------------------------------------------------------------------

class ShipComponentDescriptor
{
public:
	
	typedef std::vector<std::string> StringVector;
	typedef std::map<PersistentCrcString, ShipComponentDescriptor *> NameComponentMap;

	static void install ();
	static void fetchSharedObjectTemplates();
	static void remove  ();
	static void load();
	static bool save(std::string const & filename);

	static char const * getFilename();

	static void setUseWritableComponentDescriptor(bool onlyUseThisForTools);

	static ShipComponentDescriptor const * findShipComponentDescriptorByCrc            (uint32 componentCrc);
	static ShipComponentDescriptor const * findShipComponentDescriptorByName           (CrcString const & componentName);
	static ShipComponentDescriptor const * findShipComponentDescriptorByObjectTemplate (uint32 objectTemplateCrc);
	static ShipComponentDescriptor const * findShipComponentDescriptorBySharedObjectTemplate (uint32 sharedObjectTemplateCrc);

	CrcString const &        getName                    () const;
	uint32                   getCrc                     () const;
	ShipComponentType::Type  getComponentType           () const;
	CrcString const &        getCompatibility           () const;
	uint32                   getObjectTemplateCrc       () const;
	uint32                   getSharedObjectTemplateCrc () const;
	ObjectTemplate const * getSharedObjectTemplate() const;

	static StringVector getComponentDescriptorNames();
	static NameComponentMap const & getNameComponentMap();

protected:
	ShipComponentDescriptor (CrcString const & name, ShipComponentType::Type type, CrcString const & compatibility, std::string const & objectTemplateName, std::string const & sharedTemplateName);
	virtual ~ShipComponentDescriptor ();

	virtual bool setName(std::string const & name);
	virtual void removeShipComponentDescriptor();
	virtual bool addShipComponentDescriptor(bool checkValidity, bool strict);

	virtual std::string const & getObjectTemplateName() const;
	virtual std::string const & getSharedTemplateName() const;
	virtual void setCompatibility(std::string const & compat);
	virtual void setComponentType(ShipComponentType::Type type);

	bool setObjectTemplateCrcs(uint32 crc, uint32 sharedCrc);

private:
	ShipComponentDescriptor (CrcString const & name, ShipComponentType::Type type, CrcString const & compatibility, uint32 objectTemplateCrc, uint32 sharedObjectTemplateCrc);
	ShipComponentDescriptor ();

	ShipComponentDescriptor (const ShipComponentDescriptor & rhs);
	ShipComponentDescriptor & operator= (const ShipComponentDescriptor & rhs);

	void fetchSharedObjectTemplate();

private:
	
	PersistentCrcString       m_name;
	ShipComponentType::Type   m_componentType;
	PersistentCrcString       m_compatibility;
	uint32                    m_objectTemplateCrc;
	uint32                    m_sharedObjectTemplateCrc;
	ObjectTemplate const * m_sharedObjectTemplate;
};

//----------------------------------------------------------------------

inline CrcString const & ShipComponentDescriptor::getName () const
{
	return m_name;
}

//----------------------------------------------------------------------

inline ShipComponentType::Type  ShipComponentDescriptor::getComponentType () const
{
	return m_componentType;
}

//----------------------------------------------------------------------

inline CrcString const &  ShipComponentDescriptor::getCompatibility () const
{
	return m_compatibility;
}

//----------------------------------------------------------------------

inline uint32 ShipComponentDescriptor::getObjectTemplateCrc () const
{
	return m_objectTemplateCrc;
}

//----------------------------------------------------------------------

inline uint32 ShipComponentDescriptor::getSharedObjectTemplateCrc () const
{
	return m_sharedObjectTemplateCrc;
}

//----------------------------------------------------------------------

inline ObjectTemplate const * ShipComponentDescriptor::getSharedObjectTemplate() const
{
	return m_sharedObjectTemplate;
}

//----------------------------------------------------------------------

inline uint32 ShipComponentDescriptor::getCrc  () const
{
	return m_name.getCrc ();
}

//======================================================================

#endif
