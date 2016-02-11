//========================================================================
//
// ResourceClassObject.cpp
//
// copyright 2002 Sony Online Entertainment
//
//========================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/ResourceClassObject.h"

#include "UnicodeUtils.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedGame/Universe.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include <cstdio>
#include <vector>


// ======================================================================

std::string *ResourceClassObject::ms_attNameHeader = 0;
std::string *ResourceClassObject::ms_attMaxHeader = 0;
std::string *ResourceClassObject::ms_attMinHeader = 0;

namespace ResourceClassObjectNamespace
{
	std::string ms_resourceNameStringFile = "resource/resource_names";
	std::set<std::string> ms_classesExcludedFromCommodities;
}

using namespace ResourceClassObjectNamespace;

// ======================================================================

ResourceClassObject::ResourceClassObject() : 
		m_resourceClassName(),
		m_parentClass(nullptr),
		m_friendlyName(nullptr),
		m_minTypes(0),
		m_maxTypes(0),
		m_minPools(0),
		m_maxPools(0),
		m_crateTemplate(),
		m_nameTable(),
		m_recycled(false),
		m_permanent(false),
		m_recycledVersion(nullptr),
		m_resourceAttributeRanges(new ResourceAttributeRangesType()),
		m_children(new ClassList)
{
}

//-----------------------------------------------------------------------

ResourceClassObject::~ResourceClassObject()
{
	delete m_children;
	m_children = nullptr;

	delete m_resourceAttributeRanges;
	m_resourceAttributeRanges = nullptr;

	m_parentClass = nullptr;
	m_recycledVersion = nullptr;
	
	if (m_friendlyName != nullptr)
	{
		delete m_friendlyName;
		m_friendlyName = nullptr;
	}
}

// ----------------------------------------------------------------------

void ResourceClassObject::install()
{
	ExitChain::add(&remove, "ResourceClassObject::remove");
	
	ms_attNameHeader = new std::string[11];
	ms_attNameHeader[0]="Attribute 1";
	ms_attNameHeader[1]="Attribute 2";
	ms_attNameHeader[2]="Attribute 3";
	ms_attNameHeader[3]="Attribute 4";
	ms_attNameHeader[4]="Attribute 5";
	ms_attNameHeader[5]="Attribute 6";
	ms_attNameHeader[6]="Attribute 7";
	ms_attNameHeader[7]="Attribute 8";
	ms_attNameHeader[8]="Attribute 9";
	ms_attNameHeader[9]="Attribute 10";
	ms_attNameHeader[10]="Attribute 11";
	
	ms_attMaxHeader = new std::string[11];
	ms_attMaxHeader[0]="Att 1 max";
	ms_attMaxHeader[1]="Att 2 max";
	ms_attMaxHeader[2]="Att 3 max";
	ms_attMaxHeader[3]="Att 4 max";
	ms_attMaxHeader[4]="Att 5 max";
	ms_attMaxHeader[5]="Att 6 max";
	ms_attMaxHeader[6]="Att 7 max";
	ms_attMaxHeader[7]="Att 8 max";
	ms_attMaxHeader[8]="Att 9 max";
	ms_attMaxHeader[9]="Att 10 max";
	ms_attMaxHeader[10]="Att 11 max";
	
	ms_attMinHeader = new std::string[11];
	ms_attMinHeader[0]="Att 1 min";
	ms_attMinHeader[1]="Att 2 min";
	ms_attMinHeader[2]="Att 3 min";
	ms_attMinHeader[3]="Att 4 min";
	ms_attMinHeader[4]="Att 5 min";
	ms_attMinHeader[5]="Att 6 min";
	ms_attMinHeader[6]="Att 7 min";
	ms_attMinHeader[7]="Att 8 min";
	ms_attMinHeader[8]="Att 9 min";
	ms_attMinHeader[9]="Att 10 min";
	ms_attMinHeader[10]="Att 11 min";
}

// ----------------------------------------------------------------------

void ResourceClassObject::remove()
{
	delete[] ms_attMaxHeader;
	delete[] ms_attMinHeader;
	delete[] ms_attNameHeader;
	ms_attMaxHeader = 0;
	ms_attMinHeader = 0;
	ms_attNameHeader = 0;
}

// ----------------------------------------------------------------------

/**
 * Build a string out of the ResourceClass tree, for debugging.
 */
void ResourceClassObject::debugOutput(std::string &output, int numSpaces) const
{
	for (int i=0; i<numSpaces; ++i)
	{
		output+="  ";
	}
	
	output+=m_resourceClassName;
	output+=" (";
	output+=getFriendlyName().getCanonicalRepresentation();
	output+=") ";
	output+="\n";
	
	for (ClassList::iterator j=m_children->begin(); j!=m_children->end(); ++j)
	{
		(*j)->debugOutput(output,numSpaces+1);
	}
}

// ----------------------------------------------------------------------

void ResourceClassObject::setupUniverse()
{
	Universe::getInstance().registerResourceClassObject(*this);
	ResourceClassObject *parent = getParent();
	if (parent)
		parent->addChildClass(*this);
}

// ----------------------------------------------------------------------

void ResourceClassObject::addChildClass(ResourceClassObject &child)
{
	m_children->push_back(&child);
} //lint !e1764 // child needs to be non-const because the vector holds non-const pointers

// ----------------------------------------------------------------------

/** 
 * Return true if this class is derived from (or is equal to) the class
 * specfied.
 */

bool ResourceClassObject::isDerivedFrom(const ResourceClassObject &masterClass) const
{
	if (this == &masterClass)
		return true;
	else
	{
		const ResourceClassObject *parentClass = getParent();
		if (parentClass)
			return parentClass->isDerivedFrom(masterClass);
		else
			return false;
	}
}

// ----------------------------------------------------------------------

/**
 * Return the template of the kind of crate that can hold this resource.
 */
void ResourceClassObject::getCrateTemplate(std::string &buffer) const
{
	if (m_crateTemplate.length() > 0)
		buffer=m_crateTemplate;
	else
	{
		const ResourceClassObject *parentClass = getParent();
		if (parentClass)
			parentClass->getCrateTemplate(buffer);
		else
			buffer=getDefaultResourceCrate();
	}
}

// ----------------------------------------------------------------------

/** 
 * Get the name of the table to use when generating names for ResourceTypes
 * derived from this class.
 */

void ResourceClassObject::getNameTable(std::string &buffer) const
{
	if (m_nameTable.length() > 0)
		buffer=m_nameTable;
	else
	{
		ResourceClassObject *parentClass = getParent();
		if (parentClass)
			parentClass->getNameTable(buffer);
		else
			buffer=getDefaultResourceNameTable();
	}	
}

// ----------------------------------------------------------------------

/** 
 * Get the name of the table to use when generating names for ResourceTypes
 * derived from this class.
 */
std::string ResourceClassObject::getNameTable() const
{
	std::string buffer;
	getNameTable(buffer);
	return buffer;
}

// ----------------------------------------------------------------------

ResourceClassObject::ResourceAttributeRangesType const & ResourceClassObject::getResourceAttributeRanges() const
{
static const ResourceAttributeRangesType emptyRanges;

	if (m_resourceAttributeRanges != nullptr)
		return *m_resourceAttributeRanges;
	return emptyRanges;
}

// ----------------------------------------------------------------------

void ResourceClassObject::setAttributesFromDataTable(DataTable &table, int row)
{
	for (int attNumber = 0; attNumber < 11; ++attNumber)
	{
		const std::string &attName = table.getStringValue (ms_attNameHeader[attNumber],row);
		if (attName.size() > 0)
		{
			IGNORE_RETURN(m_resourceAttributeRanges->insert(
							  std::make_pair(attName,
											 std::make_pair(table.getIntValue(ms_attMinHeader[attNumber],row),
															table.getIntValue(ms_attMaxHeader[attNumber],row)))));
		}
	} 
}

// ----------------------------------------------------------------------

/** 
 * Parse the resource spreadsheet and build ResourceClassObjects from it.
 * The spreadsheet is formatted to be easy for a human to edit, so it
 * is a little awkward to parse in C++.
 */

void ResourceClassObject::loadTreeFromIff()
{
	DataTable * resourceDataTable = DataTableManager::getTable("datatables/resource/resource_tree.iff", true);
	if (resourceDataTable)
	{
		// These strings are easier to hard-code than to snprintf
		std::string classHeader[8];
		classHeader[0]= "CLASS 1"; 
		classHeader[1]= "CLASS 2";
		classHeader[2]= "CLASS 3";
		classHeader[3]= "CLASS 4";
		classHeader[4]= "CLASS 5";
		classHeader[5]= "CLASS 6";
		classHeader[6]= "CLASS 7";
		classHeader[7]= "CLASS 8";
				
		std::map<int, ResourceClassObject*> resourceClasses;
		int numRows = resourceDataTable->getNumRows();
		ResourceClassObject * parents[8];
		for (int i=0; i<8; ++i)
			parents[i]=nullptr;
		
		static const std::string colName_resourceClassName = "ENUM";
		static const std::string colName_maxTypes          = "Maximum # types";
		static const std::string colName_minTypes          = "Minimum # types";
		static const std::string colName_maxPools          = "Maximum # Pools";
		static const std::string colName_minPools          = "Minimum # Pools";
		static const std::string colName_crateTemplate     = "Resource Container Type";
		static const std::string colName_nameTable         = "Random Name Class";
		static const std::string colName_index             = "INDEX";
		static const std::string colName_recycled          = "Recycled";
		static const std::string colName_permanent         = "Permanent";

		for (int row=0; row<numRows; ++row)
		{
			if (resourceDataTable->getIntValue(colName_index,row)<=0)
			{
				DEBUG_WARNING(true,("Designer bug:  Resource data table has a suspicious row %i (the value in the index column is missing or wrong)",row));
			}
			else
			{
				ResourceClassObject *newClass = Universe::getInstance().makeResourceClassObject();
				
				// in the spreadsheet, the name is indented to indicate what derives from what
				for (int wheresTheName=0; wheresTheName < 8; ++wheresTheName)
				{
					const std::string &possibleName = resourceDataTable->getStringValue(classHeader[wheresTheName],row);
					if (possibleName.size()!=0)
					{
						if (wheresTheName==0)
							newClass->m_parentClass = nullptr;
						else
						{
							newClass->m_parentClass = parents[wheresTheName-1];
							DEBUG_FATAL(!newClass->m_parentClass,("Can't determine parent class for ResourceClass %s.  Probably a problem in the resource data table\n",
																  possibleName.c_str()));
						}
						parents[wheresTheName]=newClass;
						break;
					}
				}
				
				newClass->m_resourceClassName = resourceDataTable->getStringValue (colName_resourceClassName, row);
				newClass->m_friendlyName = new StringId(ms_resourceNameStringFile, newClass->m_resourceClassName);
				newClass->m_maxTypes          = resourceDataTable->getIntValue    (colName_maxTypes,          row);
				newClass->m_minTypes          = resourceDataTable->getIntValue    (colName_minTypes,          row);
				newClass->m_maxPools          = resourceDataTable->getIntValue    (colName_maxPools,          row);
				newClass->m_minPools          = resourceDataTable->getIntValue    (colName_minPools,          row);
				newClass->m_crateTemplate     = resourceDataTable->getStringValue (colName_crateTemplate,     row);
				newClass->m_nameTable         = resourceDataTable->getStringValue (colName_nameTable,         row);
				newClass->m_recycled          = (resourceDataTable->getIntValue   (colName_recycled,          row) != 0);
				newClass->m_permanent         = (resourceDataTable->getIntValue   (colName_permanent,         row) != 0);

				newClass->setupUniverse();

				newClass->setAttributesFromDataTable(*resourceDataTable, row);

				if (newClass->m_recycled)
				{
					newClass->m_parentClass->m_recycledVersion = newClass;
				}
				
				resourceClasses[resourceDataTable->getIntValue(colName_index,row)]=newClass; // used to match up with fractal parameters
			}
		}

		Universe::getInstance().resourceClassTreeLoaded(resourceClasses);	   
	} // if (resourceDataTable)
	else
		DEBUG_WARNING(true,("Unable to open resource data table.\n"));

	DataTableManager::close("datatables/resource/resource_tree.iff");

	// read in the list of resource classes that are excluded from the commodities system
	DataTable * resourceClassExclusionDataTable = DataTableManager::getTable("datatables/commodity/resource_class_exclusion.iff", true);
	if (resourceClassExclusionDataTable)
	{
		int const classColumn = resourceClassExclusionDataTable->findColumnNumber("class");
		if (classColumn > 0)
		{
			unsigned int const numRows = static_cast<unsigned int>(resourceClassExclusionDataTable->getNumRows());
			if (numRows == 0)
			{
				DEBUG_WARNING(true, ("Resource commodities exclusion data table datatables/commodity/resource_class_exclusion.iff is empty.\n"));
			}
			else
			{
				std::string className;
				for (unsigned int i = 0; i < numRows; ++i)
				{
					className = resourceClassExclusionDataTable->getStringValue(classColumn, i);
					if (Universe::getInstance().getResourceClassByName(className))
						IGNORE_RETURN(ms_classesExcludedFromCommodities.insert(className));
					else
						DEBUG_WARNING(true, ("Resource commodities exclusion data table datatables/commodity/resource_class_exclusion.iff contains invalid resource class name (%s).\n", className.c_str()));
				}
			}
		}
	}
	else
	{
		DEBUG_WARNING(true,("Unable to open resource commodities exclusion data table datatables/commodity/resource_class_exclusion.iff.\n"));
	}

	DataTableManager::close("datatables/commodity/resource_class_exclusion.iff");
}

// ----------------------------------------------------------------------

const std::string &ResourceClassObject::getDefaultResourceCrate() const
{
	static std::string nothing;
	return nothing;
}

// ----------------------------------------------------------------------

const std::string &ResourceClassObject::getDefaultResourceNameTable() const
{
	static std::string nothing;
	return nothing;
}

// ----------------------------------------------------------------------
	
bool ResourceClassObject::isLeaf() const
{
	return m_children->empty();
}

//----------------------------------------------------------------------

void ResourceClassObject::getChildren(std::vector<const ResourceClassObject *> & children, bool recurse) const
{
	if (m_children == nullptr)
		return;

	for (ClassList::const_iterator i = m_children->begin(); i != m_children->end(); ++i)
	{
		if (*i != nullptr)
		{
			children.push_back(*i);
			if (recurse)
				(*i)->getChildren(children, recurse);
		}
	}
}

//----------------------------------------------------------------------

void ResourceClassObject::getLeafChildren(std::vector<const ResourceClassObject *> & children) const
{
	if (m_children == nullptr)
		return;

	for (ClassList::const_iterator i = m_children->begin(); i != m_children->end(); ++i)
	{
		if (*i != nullptr)
		{
			if ((*i)->isLeaf())
				children.push_back(*i);
			else
				(*i)->getLeafChildren(children);
		}
	}
}

// ----------------------------------------------------------------------

int ResourceClassObject::getMinPools() const
{
	return m_minPools;
}

// ----------------------------------------------------------------------

int ResourceClassObject::getMaxPools() const
{
	return m_maxPools;
}

// ----------------------------------------------------------------------

ResourceClassObject const * ResourceClassObject::getRecycledVersion() const
{
	if (m_recycledVersion)
		return m_recycledVersion;
	else if (m_parentClass)
		return m_parentClass->getRecycledVersion();
	else return nullptr;
}

// ----------------------------------------------------------------------

bool ResourceClassObject::isRecycled() const
{
	return m_recycled;
}

//----------------------------------------------------------------------

bool ResourceClassObject::isPermanent() const
{
	return m_permanent;
}

//----------------------------------------------------------------------

bool ResourceClassObject::isClassExcludedFromCommodities(std::string const & className)
{
	return (ms_classesExcludedFromCommodities.count(className) > 0);
}

//----------------------------------------------------------------------

std::set<std::string> const & ResourceClassObject::getClassesExcludedFromCommodities()
{
	return ms_classesExcludedFromCommodities;
}

// ======================================================================
