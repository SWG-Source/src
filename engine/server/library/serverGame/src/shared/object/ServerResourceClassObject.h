//========================================================================
//
// ServerResourceClassObject.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef INCLUDED_ServerResourceClassObject_H
#define INCLUDED_ServerResourceClassObject_H

// ======================================================================

#include "serverUtility/ResourceFractalData.h"
#include "sharedGame/ResourceClassObject.h"

class DynamicVariableList;
class NetworkId;
class ResourceTypeObject;

// ======================================================================

/**
 * Represents a resource class, i.e. a non-leaf node in the resource
 * tree.  The resource class hierarchy is static at runtime, and is read
 * in from a datatable.
 */

class ServerResourceClassObject : public ResourceClassObject
{
  public:
	typedef std::vector<std::string> PossiblePlanetsType;

  public:
	ServerResourceClassObject();
	virtual ~ServerResourceClassObject();

  public:
	// getter functions
	virtual void  debugOutput        (std::string &output, int numSpaces=0) const;
	int           numTypesCouldSpawn () const;
	int           getNumTypes        () const;
	void          getResourceTypes   (std::vector<ResourceTypeObject const *> & types) const;
	void          getAllDerivedResourceTypes(std::vector<ResourceTypeObject const *> & types) const;
	bool          hasDerivedResourceType() const;
	ResourceTypeObject const * getAResourceType() const;
	ResourceFractalData const & getFractalData() const;
	PossiblePlanetsType const & getPossiblePlanets() const;
	int           getMinPoolSize() const;
	int           getMaxPoolSize() const;

	// Universe system functions (rebuild data structures on load)
	void          addType            (ResourceTypeObject & newType);

	// Setup functions, insure the system is in a consistent state
	void          checkRanges        ();
	void          checkAttributeLimits();
	void          spawnAsNeeded      ();
	static void   loadDistributionFromIff (std::map<int, ResourceClassObject*> &resourceClasses);

  private:
	// simulation helper functions
	void          spawnNewTypes       (int howMany);
		
  private:
	ServerResourceClassObject(const ServerResourceClassObject& rhs);
	ServerResourceClassObject& operator=(const ServerResourceClassObject& rhs);

  private:
	typedef std::vector<ResourceTypeObject*>  TypeList;
	
  private:
	TypeList  m_types;
	ResourceFractalData m_fractalData;
	PossiblePlanetsType m_possiblePlanets;
	int m_minPoolSize;
	int m_maxPoolSize;

  private:
	struct SpawnChoice
	{
		ServerResourceClassObject *m_class;
		int m_numCouldSpawn;
		int m_numToSpawn;
	};

};

// ======================================================================

#endif
