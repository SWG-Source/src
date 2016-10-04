//========================================================================
//
// ServerResourceClassObject.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ServerResourceClassObject.h"

#include "UnicodeUtils.h"
#include "serverGame/ConfigServerGame.h"
#include "serverGame/ResourceTypeObject.h"
#include "serverGame/ServerUniverse.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedRandom/Random.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include "swgServerNetworkMessages/MessageQueueResourceTypeDepleted.h"

#include <limits>
#include <map>
#include <vector>
#include <stdio.h>

// ======================================================================

namespace ServerResourceClassObjectNamespace
{
	struct ChildrenAttributeMinMax
	{
		std::string minClassName;
		int minValue;

		std::string nextMinClassName;
		int nextMinValue;

		std::string maxClassName;
		int maxValue;

		std::string nextMaxClassName;
		int nextMaxValue;
	};
}

using namespace ServerResourceClassObjectNamespace;

// ======================================================================

ServerResourceClassObject::ServerResourceClassObject() :
		ResourceClassObject(),
		m_types(),
		m_fractalData(),
		m_possiblePlanets(),
		m_minPoolSize(0),
		m_maxPoolSize(0)
{
}

//-----------------------------------------------------------------------

ServerResourceClassObject::~ServerResourceClassObject()
{
}

// ----------------------------------------------------------------------

/**
 * Create a new ResourceTypeObject based on this ResourceClass,
 * or one of its children.
 *
 */
void ServerResourceClassObject::spawnNewTypes(int howMany)
{
	DEBUG_FATAL(!ServerUniverse::getInstance().isAuthoritative(),("Programmer bug:  ServerResourceClassObject::spawnNewTypes called on non-authoritative ServerResourceClassObject."));

	if (isLeaf())
	{
		// no children, so spawn type directly from this class

		for (int i=0; i<howMany; ++i)
		{
			ResourceTypeObject * newObject = new ResourceTypeObject(*this);			
			m_types.push_back(newObject);
			ServerUniverse::getInstance().registerResourceTypeObject(*newObject, true);
		}
	}
	else
	{
		// pick children and pass the spawn message down to them
		// the odds of picking a child should be proportional to the number the child could spawn

		std::vector<SpawnChoice> choices;
		int numChoices(0);
			
		NOT_NULL (m_children);
		for (ClassList::iterator child=m_children->begin(); child!=m_children->end(); ++child)
		{
			SpawnChoice s;

			s.m_class=safe_cast<ServerResourceClassObject*>(*child);
			s.m_numCouldSpawn=s.m_class->numTypesCouldSpawn();
			s.m_numToSpawn=0;

			numChoices+=s.m_numCouldSpawn;
			if (s.m_numCouldSpawn!=0)
				choices.push_back(s);
		}

		for (int i=0; i<howMany; ++i)
		{
			if (numChoices==0)
			{
				WARNING_STRICT_FATAL(true,("Tried to spawn a ResourceType from non-leaf class %s, but no child classes were available.\n",
										   getResourceClassName().c_str()));
				return;
			}
			int selection=Random::random(numChoices-1);

			for (std::vector<SpawnChoice>::iterator j=choices.begin(); j!=choices.end(); ++j)
			{
				selection-=(*j).m_numCouldSpawn;
				if (selection < 0)
				{
					(*j).m_numToSpawn++;
					(*j).m_numCouldSpawn--;
					numChoices--;
					break;
				}
			}
			FATAL(selection>=0,("Here's the bug.\n"));
		}

		for (std::vector<SpawnChoice>::iterator k=choices.begin(); k!=choices.end(); ++k)
		{
			if ((*k).m_numToSpawn > 0)
				(*k).m_class->spawnNewTypes((*k).m_numToSpawn);
		}
	}
}

// ----------------------------------------------------------------------

/** 
 * Check whether it would be legal to spawn a new resource type from this class.
 * @return true if a new resource type could be spawned by this class (or a descendant)
 */

int ServerResourceClassObject::numTypesCouldSpawn() const
{
	int result=m_maxTypes - getNumTypes();
	return (result>0)?result:0;
}

// ----------------------------------------------------------------------

/**
 * Build a string out of the ResourceClass tree, for debugging.
 */
void ServerResourceClassObject::debugOutput(std::string &output, int numSpaces) const
{
	char buffer[200];

	for (int i=0; i<numSpaces; ++i)
	{
		output+="  ";
	}
	
	output+=m_resourceClassName;
	output+=" (";
	output+=getFriendlyName().getCanonicalRepresentation();
	output+=") ";
	sprintf(buffer,"(%i, %i - %i)",getNumTypes(),m_minTypes,m_maxTypes);
	output+=buffer;
	output+="\n";
	
	if (isLeaf())
	{
		for (TypeList::const_iterator i=m_types.begin(); i!=m_types.end(); ++i)
		{
			(*i)->debugOutput(output,numSpaces+1);
		}
	}
	else
	{
		NOT_NULL (m_children);
		for (ClassList::const_iterator i=m_children->begin(); i!=m_children->end(); ++i)
		{
			(*i)->debugOutput(output,numSpaces+1);
		}
	}
}

// ----------------------------------------------------------------------

void ServerResourceClassObject::addType(ResourceTypeObject & newType)
{
	NOT_NULL (m_children);
	WARNING_STRICT_FATAL(!m_children->empty(),("Resource Class %s has both child types and child classes.\n",getResourceClassName().c_str()));
	
	//The list will be pretty small, so iterating through it actually isn't that bad.  Almost certanly faster than making it into a map to avoid this iteration.
	//We have to check, because we add things right away when we spawn them, and setupUniverse() would therefore create a duplicate.
	for (TypeList::const_iterator i=m_types.begin(); i!=m_types.end(); ++i)
		if (*i==&newType) return;
	
	m_types.push_back(&newType);
} //lint !e1764 // newType can't be made const because push_back takes a non-const pointer

// ----------------------------------------------------------------------

/**
 * Computes the number of ResourceTypes derived from this object or its
 * children.
 * @todo Might be a lot more efficient to cache this data, so that we
 * don't have to recursively add it up each time.
 */
int ServerResourceClassObject::getNumTypes() const
{
	int count=0;
	
	if (m_types.empty())
	{
		NOT_NULL (m_children);
		for (ClassList::const_iterator j=m_children->begin(); j!=m_children->end(); ++j)
			count += safe_cast<ServerResourceClassObject*>((*j))->getNumTypes();
	}
	else
	{
		for (TypeList::const_iterator i=m_types.begin(); i!=m_types.end(); ++i)
			if (!(*i)->isDepleted())
				++count;
	}
	
	return count;
}

// ----------------------------------------------------------------------

/**
 * Walks the resource tree and checks that the minTypes and maxTypes
 * parameters all make sense.  Adjusts them where they do not.
 * For example, parent.maxTypes should be >= sum(children.minTypes), etc.
 */
void ServerResourceClassObject::checkRanges()
{
	int childMinTypes(0);
	int childMaxTypes(0);
		
	if (!isLeaf())
	{
		NOT_NULL (m_children);

		bool hasZeroMinTypesChild = false;
		for (ClassList::const_iterator i=m_children->begin(); i!=m_children->end(); ++i)
		{
			safe_cast<ServerResourceClassObject*>((*i))->checkRanges();
			childMinTypes+=(*i)->getMinTypes();
			childMaxTypes+=(*i)->getMaxTypes();

			if ((*i)->getMinTypes() <= 0)
				hasZeroMinTypesChild = true;
		}

		if (childMaxTypes < m_minTypes)
		{
			m_minTypes=childMaxTypes;
			DEBUG_REPORT_LOG(true,("RESOURCE TREE ERROR:  minTypes for ResourceClass %s(%s) reduced to %i because children can only spawn that many.\n",
								   getResourceClassName().c_str(),getResourceClassName().c_str(),childMaxTypes));
		}
		if (childMinTypes > m_maxTypes)
		{
			m_maxTypes=childMinTypes;
			DEBUG_REPORT_LOG(true,("RESOURCE TREE ERROR:  maxTypes for ResourceClass %s(%s) increased to %i because children require at least that many.\n",
								   getResourceClassName().c_str(),getResourceClassName().c_str(),childMinTypes));
		}

		if (childMinTypes > m_minTypes)
		{
			m_minTypes=childMinTypes;
			DEBUG_REPORT_LOG(true,("RESOURCE TREE ERROR:  minTypes for ResourceClass %s(%s) increased to %i because children require at least that many.\n",
								   getResourceClassName().c_str(),getResourceClassName().c_str(),childMinTypes));
		}

		if (childMaxTypes < m_maxTypes)
		{
			m_maxTypes=childMaxTypes;
			DEBUG_REPORT_LOG(true,("RESOURCE TREE ERROR:  maxTypes for ResourceClass %s(%s) reduced to %i because children can only spawn that many.\n",
								   getResourceClassName().c_str(),getResourceClassName().c_str(),childMaxTypes));
		}

		// check for possible starvation
		if (hasZeroMinTypesChild && (m_minTypes == childMinTypes))
		{
			DEBUG_REPORT_LOG(true,("RESOURCE TREE ERROR:  minTypes value of %i for ResourceClass %s(%s) is the same as minTypes for children, which will cause no random spawn of the children (i.e. starvation).\n",
								   m_minTypes,getResourceClassName().c_str(),getResourceClassName().c_str()));
		}

		if (childMaxTypes < childMinTypes)
		{
			DEBUG_REPORT_LOG(true,("RESOURCE TREE UNCORRECTIBLE ERROR:  Children of ResourceClass %s (%s) require minimum %i types but allow maximum %i types\n",
								   getResourceClassName().c_str(),getResourceClassName().c_str(),childMinTypes,childMaxTypes));
			return;
		}
						
	}
	else
		if (ConfigServerGame::getSpawnAllResources())
			m_minTypes = std::max(1,m_minTypes); // in this mode, all leaf classes are set to spawn at least one type
		
	if (m_maxTypes < m_minTypes)
	{
		m_maxTypes=m_minTypes;
		DEBUG_REPORT_LOG(true,("RESOURCE TREE ERROR:  maxTypes for ResourceClass %s(%s) increased to %i because minTypes was greater than maxTypes.\n",
							   getResourceClassName().c_str(),getResourceClassName().c_str(),m_maxTypes));
	}
}

// ----------------------------------------------------------------------

/**
* Walks the resource tree and checks the resource attribute ranges
* for any possible inconsistency
*/
void ServerResourceClassObject::checkAttributeLimits()
{
	const ServerResourceClassObject::ResourceAttributeRangesType & myClassAttribs = getResourceAttributeRanges();

	if (!isLeaf())
	{
		typedef std::map<std::string, ChildrenAttributeMinMax> ChildrenClassAttributeRanges;
		ChildrenClassAttributeRanges childrenClassAttributeRanges;

		for (ClassList::const_iterator i=m_children->begin(); i!=m_children->end(); ++i)
		{
			safe_cast<ServerResourceClassObject*>((*i))->checkAttributeLimits();

			const ServerResourceClassObject::ResourceAttributeRangesType & classAttribs = safe_cast<ServerResourceClassObject*>((*i))->getResourceAttributeRanges();
			for (ServerResourceClassObject::ResourceAttributeRangesType::const_iterator iClassAttribs = classAttribs.begin(); iClassAttribs != classAttribs.end(); ++iClassAttribs)
			{
				ChildrenClassAttributeRanges::iterator found = childrenClassAttributeRanges.find(iClassAttribs->first);
				if (found != childrenClassAttributeRanges.end())
				{
					// min
					if (iClassAttribs->second.first < found->second.nextMinValue)
					{
						found->second.nextMinValue = iClassAttribs->second.first;
						found->second.nextMinClassName = safe_cast<ServerResourceClassObject*>((*i))->getResourceClassName();

						if (found->second.nextMinValue < found->second.minValue)
						{
							std::swap(found->second.minValue, found->second.nextMinValue);
							std::swap(found->second.minClassName, found->second.nextMinClassName);
						}
					}

					// max
					if (iClassAttribs->second.second > found->second.nextMaxValue)
					{
						found->second.nextMaxValue = iClassAttribs->second.second;
						found->second.nextMaxClassName = safe_cast<ServerResourceClassObject*>((*i))->getResourceClassName();

						if (found->second.nextMaxValue > found->second.maxValue)
						{
							std::swap(found->second.maxValue, found->second.nextMaxValue);
							std::swap(found->second.maxClassName, found->second.nextMaxClassName);
						}
					}
				}
				else
				{
					ChildrenAttributeMinMax camm;
					camm.minValue = iClassAttribs->second.first;
					camm.minClassName = safe_cast<ServerResourceClassObject*>((*i))->getResourceClassName();
					camm.nextMinValue = std::numeric_limits<int>::max();
					camm.nextMinClassName.clear();
					camm.maxValue = iClassAttribs->second.second;
					camm.maxClassName = camm.minClassName;
					camm.nextMaxValue = std::numeric_limits<int>::min();
					camm.nextMaxClassName.clear();

					childrenClassAttributeRanges[iClassAttribs->first] = camm;
				}
			}
		}

		ServerResourceClassObject::ResourceAttributeRangesType::const_iterator iterClassAttribs;
		ServerResourceClassObject::ResourceAttributeRangesType::const_iterator findClassAttribs;
		ChildrenClassAttributeRanges::const_iterator iterChildrenClassAttribs;
		ChildrenClassAttributeRanges::const_iterator findChildrenClassAttribs;
		for (iterClassAttribs = myClassAttribs.begin(); iterClassAttribs != myClassAttribs.end(); ++iterClassAttribs)
		{
			findChildrenClassAttribs = childrenClassAttributeRanges.find(iterClassAttribs->first);
			if (findChildrenClassAttribs != childrenClassAttributeRanges.end())
			{
				if (iterClassAttribs->second.first != findChildrenClassAttribs->second.minValue)
				{
					DEBUG_REPORT_LOG(true,("RESOURCE TREE ERROR (attribute ranges): class (%s), attribute (%s), class min (%d), children min (%d, %s) (%d, %s)\n",
						getResourceClassName().c_str(), iterClassAttribs->first.c_str(), iterClassAttribs->second.first, findChildrenClassAttribs->second.minValue, findChildrenClassAttribs->second.minClassName.c_str(), findChildrenClassAttribs->second.nextMinValue, findChildrenClassAttribs->second.nextMinClassName.c_str()));
				}

				if (iterClassAttribs->second.second != findChildrenClassAttribs->second.maxValue)
				{
					DEBUG_REPORT_LOG(true,("RESOURCE TREE ERROR (attribute ranges): class (%s), attribute (%s), class max (%d), children max (%d, %s) (%d, %s)\n",
						getResourceClassName().c_str(), iterClassAttribs->first.c_str(), iterClassAttribs->second.second, findChildrenClassAttribs->second.maxValue, findChildrenClassAttribs->second.maxClassName.c_str(), findChildrenClassAttribs->second.nextMaxValue, findChildrenClassAttribs->second.nextMaxClassName.c_str()));
				}
			}
		}

		for (iterClassAttribs = myClassAttribs.begin(); iterClassAttribs != myClassAttribs.end(); ++iterClassAttribs)
		{
			findChildrenClassAttribs = childrenClassAttributeRanges.find(iterClassAttribs->first);
			if (findChildrenClassAttribs == childrenClassAttributeRanges.end())
			{
				DEBUG_REPORT_LOG(true,("RESOURCE TREE ERROR (attribute ranges): class (%s), attribute (%s), class min (%d), class max (%d), MISSING CHILDREN MIN/MAX\n",
					getResourceClassName().c_str(), iterClassAttribs->first.c_str(), iterClassAttribs->second.first, iterClassAttribs->second.second));
			}
		}

		for (iterChildrenClassAttribs = childrenClassAttributeRanges.begin(); iterChildrenClassAttribs != childrenClassAttributeRanges.end(); ++iterChildrenClassAttribs)
		{
			findClassAttribs = myClassAttribs.find(iterChildrenClassAttribs->first);
			if (findClassAttribs == myClassAttribs.end())
			{
				DEBUG_REPORT_LOG(true,("RESOURCE TREE ERROR (attribute ranges): class (%s), attribute (%s), MISSING MY MIN/MAX, children min (%d, %s) (%d, %s), children max (%d, %s) (%d, %s)\n",
					getResourceClassName().c_str(), iterChildrenClassAttribs->first.c_str(), iterChildrenClassAttribs->second.minValue, iterChildrenClassAttribs->second.minClassName.c_str(), iterChildrenClassAttribs->second.nextMinValue, iterChildrenClassAttribs->second.nextMinClassName.c_str(), iterChildrenClassAttribs->second.maxValue, iterChildrenClassAttribs->second.maxClassName.c_str(), iterChildrenClassAttribs->second.nextMaxValue, iterChildrenClassAttribs->second.nextMaxClassName.c_str()));
			}
		}
	}

	for (ServerResourceClassObject::ResourceAttributeRangesType::const_iterator iterClassAttribs = myClassAttribs.begin(); iterClassAttribs != myClassAttribs.end(); ++iterClassAttribs)
	{
		if (iterClassAttribs->second.first < 1)
		{
			DEBUG_REPORT_LOG(true,("RESOURCE TREE ERROR (attribute ranges): class (%s), attribute (%s), min (%d) is < 1\n",
				getResourceClassName().c_str(), iterClassAttribs->first.c_str(), iterClassAttribs->second.first));
		}

		if (iterClassAttribs->second.second > 1000)
		{
			DEBUG_REPORT_LOG(true,("RESOURCE TREE ERROR (attribute ranges): class (%s), attribute (%s), max (%d) is > 1000\n",
				getResourceClassName().c_str(), iterClassAttribs->first.c_str(), iterClassAttribs->second.second));
		}

		if (iterClassAttribs->second.first > iterClassAttribs->second.second)
		{
			DEBUG_REPORT_LOG(true,("RESOURCE TREE ERROR (attribute ranges): class (%s) (%s), attribute (%s), min (%d) is > max (%d)\n",
				getResourceClassName().c_str(), iterClassAttribs->first.c_str(), (isLeaf() ? "leaf" : "non-leaf"), iterClassAttribs->second.first, iterClassAttribs->second.second));
		}
	}
}

// ----------------------------------------------------------------------

/**
 * As part of the import process, spawn enough types to bring everything
 * up to the required minimums.
 */
void ServerResourceClassObject::spawnAsNeeded()
{
	if (!isLeaf())
	{
		NOT_NULL (m_children);
		for (ClassList::iterator child=m_children->begin(); child!=m_children->end(); ++child)
		{
			safe_cast<ServerResourceClassObject*>(*child)->spawnAsNeeded();
		}
	}

	if (getNumTypes() < m_minTypes)
		spawnNewTypes(m_minTypes-getNumTypes());
}

// ----------------------------------------------------------------------

void ServerResourceClassObject::loadDistributionFromIff(std::map<int, ResourceClassObject*> &resourceClasses)
{
	DataTable * distributionDataTable = DataTableManager::getTable("datatables/resource/resource_distribution.iff", true);
	if (distributionDataTable)
	{
		int numRows = distributionDataTable->getNumRows();
		for (int row=0; row<numRows; ++row)
		{
			int index = distributionDataTable->getIntValue("Resource Index#",row);
			ServerResourceClassObject *resClass = safe_cast<ServerResourceClassObject*>(resourceClasses[index]);
			if (resClass)
			{
				std::string planetList (distributionDataTable->getStringValue("Planet",row));
				planetList += ","; // Adding ',' at end makes parsing easier
				size_t pos = 0;
				size_t nextComma = 0;
				for (nextComma = planetList.find(',',0); nextComma != std::string::npos; nextComma = planetList.find(',',pos))
				{
					resClass->m_possiblePlanets.push_back(std::string(planetList, pos, nextComma-pos));
					pos = nextComma+1;
				}

				resClass->m_minPoolSize = distributionDataTable->getIntValue("Pool Size Min",row);
				resClass->m_maxPoolSize = distributionDataTable->getIntValue("Pool Size Max",row);
			
				resClass->m_fractalData.m_scaleX = distributionDataTable->getFloatValue("Fractal X Scale",row);
				resClass->m_fractalData.m_scaleY = distributionDataTable->getFloatValue("Fractal Y Scale",row);
				resClass->m_fractalData.m_bias = distributionDataTable->getFloatValue("Fractal Bias",row);
				resClass->m_fractalData.m_gain = distributionDataTable->getFloatValue("Fractal Gain",row);
				resClass->m_fractalData.m_combinationRule = distributionDataTable->getIntValue("Fractal Combo Rule",row);
				resClass->m_fractalData.m_frequency = distributionDataTable->getFloatValue("Fractal Frequency",row);
				resClass->m_fractalData.m_amplitude = distributionDataTable->getFloatValue("Fractal Amplitude",row);
				resClass->m_fractalData.m_octaves = distributionDataTable->getIntValue("Fractal Octaves",row);				
			}
		}
		DataTableManager::close("datatables/resource/resource_distribution.iff");
		distributionDataTable = 0;
	}
}

// ----------------------------------------------------------------------

void ServerResourceClassObject::getResourceTypes(std::vector<ResourceTypeObject const *> & types) const
{
	for (TypeList::const_iterator i = m_types.begin(); i != m_types.end(); ++i)
	{
		types.push_back(*i);
	}
}

// ----------------------------------------------------------------------

void ServerResourceClassObject::getAllDerivedResourceTypes(std::vector<ResourceTypeObject const *> & types) const
{
	if (isLeaf())
	{
		getResourceTypes(types);
	}
	else
	{
		for (ClassList::iterator child=m_children->begin(); child!=m_children->end(); ++child)
			(safe_cast<ServerResourceClassObject const *>(*child))->getAllDerivedResourceTypes(types);
	}
}

// ----------------------------------------------------------------------

ResourceFractalData const & ServerResourceClassObject::getFractalData() const
{
	return m_fractalData;
}

// ----------------------------------------------------------------------

int ServerResourceClassObject::getMinPoolSize() const
{
	return m_minPoolSize;
}

// ----------------------------------------------------------------------

int ServerResourceClassObject::getMaxPoolSize() const
{
	return m_maxPoolSize;
}

// ----------------------------------------------------------------------

ServerResourceClassObject::PossiblePlanetsType const & ServerResourceClassObject::getPossiblePlanets() const
{
	return m_possiblePlanets;
}

// ----------------------------------------------------------------------

/**
 * Return a resource type derived from this class
 */
ResourceTypeObject const * ServerResourceClassObject::getAResourceType() const
{
	if (!m_types.empty())
		return m_types.front();
	else
		return nullptr;
}

// ----------------------------------------------------------------------

bool ServerResourceClassObject::hasDerivedResourceType() const
{
	if (getMinTypes() > 0)
		return true;

	if (!m_types.empty())
		return true;

	for (ClassList::iterator child=m_children->begin(); child!=m_children->end(); ++child)
		if (safe_cast<ServerResourceClassObject const *>(*child)->hasDerivedResourceType())
			return true;

	return false;
}

// ======================================================================
