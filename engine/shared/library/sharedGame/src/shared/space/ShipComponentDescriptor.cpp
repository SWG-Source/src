//======================================================================
//
// ShipComponentDescriptor.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/ShipComponentDescriptor.h"

#include "fileInterface/StdioFile.h"
#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedGame/ShipComponentAttachmentManager.h"
#include "sharedGame/ShipComponentDescriptorWritable.h"
#include "sharedObject/ObjectTemplate.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedUtility/DataTable.h"
#include <algorithm>
#include <map>

//======================================================================

namespace ShipComponentDescriptorNamespace
{
	typedef std::map<PersistentCrcString, ShipComponentDescriptor *> NameComponentMap;
	typedef std::map<uint32, ShipComponentDescriptor const *>        CrcComponentMap;
	typedef std::map<uint32, ShipComponentDescriptor const *>        ObjectTemplateCrcComponentMap;

	//-- the s_nameComponentMap owns the ShipComponentDescriptor objects
	NameComponentMap *            s_nameComponentMap;

	//-- the s_crcComponentMap DOES NOT own the ShipComponentDescriptor objects
	CrcComponentMap               s_crcComponentMap;
	ObjectTemplateCrcComponentMap s_objectTemplateCrcComponentMap;
	ObjectTemplateCrcComponentMap s_sharedObjectTemplateCrcComponentMap;

	bool s_installed = false;

	bool s_useWritableComponentDescriptor = false;

	static const char * const s_filename = "datatables/space/ship_components.iff";
}

using namespace ShipComponentDescriptorNamespace;

//----------------------------------------------------------------------

void ShipComponentDescriptor::install ()
{
	InstallTimer const installTimer("ShipComponentDescriptor::install ");

	if (s_installed)
		return;

	s_nameComponentMap = new NameComponentMap;
	
	s_installed = true;

	load();

	ExitChain::add(ShipComponentDescriptor::remove, "ShipComponentDescriptor::remove");
}

//----------------------------------------------------------------------

void ShipComponentDescriptor::fetchSharedObjectTemplates()
{
	for (NameComponentMap::iterator it = s_nameComponentMap->begin (); it != s_nameComponentMap->end (); ++it)
		it->second->fetchSharedObjectTemplate();
}

//----------------------------------------------------------------------

void ShipComponentDescriptor::load()
{
	Iff iff;
	
	if (!iff.open (s_filename, true))
	{
		WARNING (true, ("Data file %s not available.", s_filename));
		return;
	}
	
	DataTable dt;
	dt.load (iff);
	iff.close ();
	
	std::string const & s_colnameName              = "name";
	std::string const & s_colnameShipComponentType = "component_type";
	std::string const & s_colnameCompatibility     = "compatibility";
	std::string const & s_colnameObjectTemplate    = "object_template";
	std::string const & s_colnameSharedObjectTemplate = "shared_object_template";
	
	const int numRows = dt.getNumRows ();
	
	for (int row = 0; row < numRows; ++row)
	{
		std::string const & name             = dt.getStringValue (s_colnameName, row);
		ShipComponentType::Type const type   = ShipComponentType::getTypeFromName (dt.getStringValue (s_colnameShipComponentType, row));
		std::string const & compatibility    = dt.getStringValue (s_colnameCompatibility, row);
		std::string const & serverObjectTemplateName = dt.getStringValue (s_colnameObjectTemplate, row);
		uint32 const serverObjectTemplateCrc = Crc::normalizeAndCalculate (serverObjectTemplateName.c_str());
		std::string const & sharedTemplateName   = dt.getStringValue (s_colnameSharedObjectTemplate, row);
		uint32 const sharedObjectTemplateCrc = Crc::normalizeAndCalculate (sharedTemplateName.c_str ());

#ifdef _DEBUG
		ConstCharCrcString const & sharedCrcString = ObjectTemplateList::lookUp(sharedObjectTemplateCrc);
		DEBUG_WARNING(!sharedTemplateName.empty() && sharedCrcString.isEmpty(), ("Data error: in ship_components.tab - Component [%s] Shared template [%s] not found for row [%d]", name.c_str(), sharedTemplateName.c_str(), row));
#endif
		
		ShipComponentDescriptor * componentDescriptor = nullptr;
		
		if (s_useWritableComponentDescriptor)
			componentDescriptor = new ShipComponentDescriptorWritable (TemporaryCrcString (name.c_str (), true), type, TemporaryCrcString (compatibility.c_str (), true), serverObjectTemplateName, sharedTemplateName);
		else
			componentDescriptor = new ShipComponentDescriptor (TemporaryCrcString (name.c_str (), true), type, TemporaryCrcString (compatibility.c_str (), true), serverObjectTemplateCrc, sharedObjectTemplateCrc);
		
		if (!componentDescriptor->addShipComponentDescriptor(true, false))
		{
			WARNING(true, ("ShipComponentDescriptor error adding [%s]", name.c_str()));
		}
	} //lint !e429 //custodial componentDescriptor
}

//----------------------------------------------------------------------

bool ShipComponentDescriptor::save(std::string const & filename)
{
	//-- fill this string with the text contents of the tab file
	std::string tabStr;
	
	//-- TAB HEADER COLNAMES
	tabStr += 
		"name\t"
		"object_template\t"
		"shared_object_template\t"
		"component_type\t"
		"compatibility\n";
	
	//-- TAB HEADER COLTYPES
	tabStr += 
		"s\t"
		"s\t"
		"s\t"
		"s\t"
		"s\n";

	//-- TAB DATA ROWS
	{
		StringVector const & componentNames = ShipComponentDescriptor::getComponentDescriptorNames();
		
		for (StringVector::const_iterator it = componentNames.begin(); it != componentNames.end(); ++it)
		{
			std::string const & componentName = *it;
			ShipComponentDescriptor const * const shipComponentDescriptor = NON_NULL(findShipComponentDescriptorByName(ConstCharCrcString(componentName.c_str())));

			tabStr += componentName + "\t";			
			tabStr += shipComponentDescriptor->getObjectTemplateName() + "\t";
			tabStr += shipComponentDescriptor->getSharedTemplateName() + "\t";
			tabStr += ShipComponentType::getNameFromType(static_cast<ShipComponentType::Type>(shipComponentDescriptor->getComponentType())) + "\t";
			tabStr += std::string(shipComponentDescriptor->getCompatibility().getString());
			tabStr.push_back('\n');
		}
	}
	
	bool retval = false;

	StdioFileFactory sff;
	AbstractFile * const af = sff.createFile(filename.c_str(), "wb");
	if (nullptr != af && af->isOpen())
	{
		int const bytesWritten = af->write(static_cast<int>(tabStr.size()), tabStr.c_str());
		retval = (bytesWritten == static_cast<int>(tabStr.size()));
	}
	else
	{
		WARNING(true, ("ShipComponentDescriptor failed to write file [%s]", filename.c_str()));
	}

	delete af;
	return retval;
}

//----------------------------------------------------------------------

void ShipComponentDescriptor::remove ()
{
	for (NameComponentMap::iterator it = s_nameComponentMap->begin (); it != s_nameComponentMap->end (); ++it)
		delete (*it).second;

	s_nameComponentMap->clear ();
	delete s_nameComponentMap;
	s_nameComponentMap = 0;

	s_crcComponentMap.clear  ();
	s_objectTemplateCrcComponentMap.clear ();
	s_sharedObjectTemplateCrcComponentMap.clear ();

	s_installed = false;
}

//----------------------------------------------------------------------

void ShipComponentDescriptor::setUseWritableComponentDescriptor(bool onlyUseThisForTools)
{
	DEBUG_FATAL (s_installed, ("already installed, setUseWritableComponentDescriptor must be called prior to installation"));

	s_useWritableComponentDescriptor = onlyUseThisForTools;
}

//----------------------------------------------------------------------

ShipComponentDescriptor::ShipComponentDescriptor () :
m_name                    (),
m_componentType           (ShipComponentType::SCT_num_types),
m_compatibility           (),
m_objectTemplateCrc       (0),
m_sharedObjectTemplateCrc (0),
m_sharedObjectTemplate(0)
{
}

//----------------------------------------------------------------------

ShipComponentDescriptor::ShipComponentDescriptor (CrcString const & name, ShipComponentType::Type type, CrcString const & compatibility, uint32 objectTemplateCrc, uint32 sharedObjectTemplateCrc) :
m_name                    (name),
m_componentType           (type),
m_compatibility           (compatibility),
m_objectTemplateCrc       (objectTemplateCrc),
m_sharedObjectTemplateCrc (sharedObjectTemplateCrc),
m_sharedObjectTemplate(0)
{
}

//----------------------------------------------------------------------

ShipComponentDescriptor::ShipComponentDescriptor(CrcString const & name, ShipComponentType::Type type, CrcString const & compatibility, std::string const & objectTemplateName, std::string const & sharedTemplateName) :
m_name                    (name),
m_componentType           (type),
m_compatibility           (compatibility),
m_objectTemplateCrc       (Crc::normalizeAndCalculate(objectTemplateName.c_str())),
m_sharedObjectTemplateCrc (Crc::normalizeAndCalculate(sharedTemplateName.c_str())),
m_sharedObjectTemplate(0)
{
}

//----------------------------------------------------------------------

ShipComponentDescriptor::~ShipComponentDescriptor ()
{
	if (m_sharedObjectTemplate)
	{
		m_sharedObjectTemplate->releaseReference();
		m_sharedObjectTemplate = 0;
	}
}

//----------------------------------------------------------------------

void ShipComponentDescriptor::fetchSharedObjectTemplate()
{
	if (m_sharedObjectTemplate)
	{
		m_sharedObjectTemplate->releaseReference();
		m_sharedObjectTemplate = 0;
	}

	if (m_sharedObjectTemplateCrc != 0)
		m_sharedObjectTemplate = ObjectTemplateList::fetch(m_sharedObjectTemplateCrc);
}

//----------------------------------------------------------------------

ShipComponentDescriptor const * ShipComponentDescriptor::findShipComponentDescriptorByCrc        (uint32 componentCrc)
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	const CrcComponentMap::const_iterator it = s_crcComponentMap.find (componentCrc);
	if (it != s_crcComponentMap.end ())
		return (*it).second;

	return nullptr;
}

//----------------------------------------------------------------------

ShipComponentDescriptor const * ShipComponentDescriptor::findShipComponentDescriptorByName        (CrcString const & componentName)
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	const NameComponentMap::const_iterator it = s_nameComponentMap->find (PersistentCrcString(componentName));
	if (it != s_nameComponentMap->end ())
		return (*it).second;

	return nullptr;
}

//----------------------------------------------------------------------

ShipComponentDescriptor const * ShipComponentDescriptor::findShipComponentDescriptorByObjectTemplate (uint32 objectTemplateCrc)
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	const ObjectTemplateCrcComponentMap::const_iterator it = s_objectTemplateCrcComponentMap.find (objectTemplateCrc);
	if (it != s_objectTemplateCrcComponentMap.end ())
		return (*it).second;

	return nullptr;
}

//----------------------------------------------------------------------

ShipComponentDescriptor const * ShipComponentDescriptor::findShipComponentDescriptorBySharedObjectTemplate (uint32 sharedObjectTemplateCrc)
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	const ObjectTemplateCrcComponentMap::const_iterator it = s_sharedObjectTemplateCrcComponentMap.find (sharedObjectTemplateCrc);
	if (it != s_sharedObjectTemplateCrcComponentMap.end ())
		return (*it).second;

	return nullptr;
}

//----------------------------------------------------------------------

ShipComponentDescriptor::StringVector ShipComponentDescriptor::getComponentDescriptorNames()
{
	StringVector sv;
	sv.reserve(s_nameComponentMap->size());

	{
		for (NameComponentMap::const_iterator it = s_nameComponentMap->begin(); it != s_nameComponentMap->end(); ++it)
		{
			std::string const componentName((*it).first.getString());
			sv.push_back(componentName);
		}
	}

	std::sort(sv.begin(), sv.end());
	return sv;
}

//----------------------------------------------------------------------

bool ShipComponentDescriptor::setName(std::string const & name)
{
	ShipComponentDescriptor const * const dupeNameShipComponentDescriptor = findShipComponentDescriptorByName(ConstCharCrcString(name.c_str()));
	if (nullptr != dupeNameShipComponentDescriptor)
	{
		WARNING(true, ("ShipComponentDescriptor attempt to set name [%s] already exists", name.c_str()));
		return false;
	}

	ShipComponentDescriptor const * const shipComponentDescriptorInList = findShipComponentDescriptorByCrc(getCrc());

	if (this == shipComponentDescriptorInList)
	{
		uint32 const newCrc = Crc::normalizeAndCalculate(name.c_str());
		ShipComponentAttachmentManager::udpateComponentCrc(getCrc(), newCrc);
		removeShipComponentDescriptor();
		m_name.set(name.c_str(), true);

		if (!addShipComponentDescriptor(true, true))
			return false;
	}
	else
	{
		m_name.set(name.c_str(), true);
	}

	return true;
}

//----------------------------------------------------------------------

bool ShipComponentDescriptor::setObjectTemplateCrcs(uint32 crc, uint32 sharedCrc)
{
	ShipComponentDescriptor const * const dupeTemplateShipComponentDescriptor = findShipComponentDescriptorByObjectTemplate(crc);
	if (nullptr != dupeTemplateShipComponentDescriptor && this != dupeTemplateShipComponentDescriptor)
	{
		WARNING(true, ("ShipComponentDescriptor attempt to set ot crc [%s] already exists", dupeTemplateShipComponentDescriptor->getName().getString()));
		return false;
	}

	ShipComponentDescriptor const * const dupeSharedTemplateShipComponentDescriptor = findShipComponentDescriptorBySharedObjectTemplate(crc);
	if (nullptr != dupeSharedTemplateShipComponentDescriptor && this != dupeSharedTemplateShipComponentDescriptor)
	{
		WARNING(true, ("ShipComponentDescriptor attempt to set shared ot crc [%s] already exists", dupeSharedTemplateShipComponentDescriptor->getName().getString()));
		return false;
	}

	ShipComponentDescriptor const * const shipComponentDescriptorInList = findShipComponentDescriptorByCrc(getCrc());

	if (this == shipComponentDescriptorInList)
	{
		removeShipComponentDescriptor();
		m_objectTemplateCrc = crc;
		m_sharedObjectTemplateCrc = sharedCrc;

		if (!addShipComponentDescriptor(true, false))
			return false;
	}
	else
	{
		m_objectTemplateCrc = crc;
		m_sharedObjectTemplateCrc = sharedCrc;
	}

	fetchSharedObjectTemplate();

	return true;
}

//----------------------------------------------------------------------

void ShipComponentDescriptor::removeShipComponentDescriptor()
{
	IGNORE_RETURN(s_nameComponentMap->erase(m_name));
	IGNORE_RETURN(s_crcComponentMap.erase(getCrc()));
	IGNORE_RETURN(s_objectTemplateCrcComponentMap.erase(getObjectTemplateCrc()));
	IGNORE_RETURN(s_sharedObjectTemplateCrcComponentMap.erase(getSharedObjectTemplateCrc()));
}

//----------------------------------------------------------------------

bool ShipComponentDescriptor::addShipComponentDescriptor(bool checkValidity, bool strict)
{
	if (checkValidity)
	{
		{
			ShipComponentDescriptor const * const shipComponentDescriptor = findShipComponentDescriptorByName(m_name);
			if (nullptr != shipComponentDescriptor)
			{
				WARNING(true, ("ShipComponentDescriptor::addShipComponentDescriptor name [%s] is already in map", m_name.getString()));
				return false;
			}
		}
		
		{
			ShipComponentDescriptor const * const shipComponentDescriptor = findShipComponentDescriptorByCrc(getCrc());
			if (nullptr != shipComponentDescriptor)
			{
				WARNING(true, ("ShipComponentDescriptor::addShipComponentDescriptor [%s] crc [0x%x] is already in map via [%s]", m_name.getString(), static_cast<int>(getCrc()), shipComponentDescriptor->getName().getString()));
				return false;
			}
		}
		
		{
			uint32 const objectTemplateCrc = getObjectTemplateCrc();
			if (0 != objectTemplateCrc)
			{
				ShipComponentDescriptor const * const shipComponentDescriptor = findShipComponentDescriptorByObjectTemplate(objectTemplateCrc);
				if (nullptr != shipComponentDescriptor)
				{
					WARNING(true, ("ShipComponentDescriptor::addShipComponentDescriptor [%s] server template [%s] [0x%x] is already in map via [%s]", m_name.getString(), getObjectTemplateName().c_str(), static_cast<int>(getObjectTemplateCrc()), shipComponentDescriptor->getName().getString()));
					if (strict)
						return false;
				}
			}
		}
		
		{
			uint32 const sharedObjectTemplateCrc = getSharedObjectTemplateCrc();
			if (0 != sharedObjectTemplateCrc)
			{
				ShipComponentDescriptor const * const shipComponentDescriptor = findShipComponentDescriptorBySharedObjectTemplate(sharedObjectTemplateCrc);
				if (nullptr != shipComponentDescriptor)
				{
					WARNING(true, ("ShipComponentDescriptor::addShipComponentDescriptor [%s] shared template [%s] [0x%x] is already in map via [%s]", m_name.getString(), getSharedTemplateName().c_str(), static_cast<int>(getSharedObjectTemplateCrc()), shipComponentDescriptor->getName().getString()));
					if (strict)
						return false;
				}
			}
		}
	}
	
	IGNORE_RETURN (s_nameComponentMap->insert             (std::make_pair (m_name, this)));
	IGNORE_RETURN (s_crcComponentMap.insert               (std::make_pair (getCrc(), this)));
	IGNORE_RETURN (s_objectTemplateCrcComponentMap.insert (std::make_pair (getObjectTemplateCrc(), this)));
	IGNORE_RETURN (s_sharedObjectTemplateCrcComponentMap.insert (std::make_pair (getSharedObjectTemplateCrc(), this)));
	
	return true;
}

//----------------------------------------------------------------------

std::string const & ShipComponentDescriptor::getObjectTemplateName() const
{
	static const std::string empty;
	return empty;
}

//----------------------------------------------------------------------

std::string const & ShipComponentDescriptor::getSharedTemplateName() const
{
	static const std::string empty;
	return empty;
}

//----------------------------------------------------------------------

ShipComponentDescriptor::NameComponentMap const & ShipComponentDescriptor::getNameComponentMap()
{
	return *NON_NULL(s_nameComponentMap);
}

//----------------------------------------------------------------------

void ShipComponentDescriptor::setComponentType(ShipComponentType::Type type)
{
	m_componentType = type;
}

//----------------------------------------------------------------------

void ShipComponentDescriptor::setCompatibility(std::string const & compat)
{
	m_compatibility.set(compat.c_str(), true);
}

//----------------------------------------------------------------------

char const * ShipComponentDescriptor::getFilename()
{
	return s_filename;
}

//======================================================================
