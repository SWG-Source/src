//========================================================================
//
// HarvesterInstallationObject.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_HarvesterInstallationObject_H
#define _INCLUDED_HarvesterInstallationObject_H

#include "serverGame/InstallationObject.h"
#include "sharedFoundation/NetworkIdArchive.h"

struct InstallationResourceData;
class ResourceClassObject;
class ResourceTypeObject;
class ResourcePoolObject;
class ServerHarvesterInstallationObjectTemplate;

/** 
  *  A HarvesterInstallationObject is used to extract resources from the environment.
  */

class HarvesterInstallationObject : public InstallationObject
{
	friend class HarvesterInstallationController;
	
public:

	HarvesterInstallationObject(const ServerHarvesterInstallationObjectTemplate* newTemplate);
	virtual ~HarvesterInstallationObject();

	virtual Controller* createDefaultController(void);
	void                addMembersToPackages();
	virtual float       alter(float time);
	virtual void        getAttributes(std::vector<std::pair<std::string, Unicode::String> > &data) const;

	//simulation functions:
	virtual void activate         (const NetworkId &actorId);
	virtual void deactivate       ();
	virtual void harvest          ();
	
	void selectResource           (const NetworkId & resourceTypeId, const NetworkId &actor);
	void discardAllHopperContents ();
	void emptyHopper              (const NetworkId & playerId, const NetworkId & resourceId, int amount, bool discard, uint8 sequenceId);

	virtual void handleCMessageTo (const MessageToPayload &message);

	//getters
	typedef std::vector<InstallationResourceData> ResourceDataVector;
	typedef std::pair<NetworkId, float>              HopperContentElement;
	typedef std::vector<HopperContentElement>     HopperContentsVector;
	
	float                   getHopperContents(HopperContentsVector * data=nullptr) const;
	void                    getResourceData(ResourceDataVector & data);
	std::vector<ResourceTypeObject const *> const & getSurveyTypes();
	int                     getMaxExtractionRate() const;
	float                   getCurrentExtractionRate() const;
	float                   getInstalledExtractionRate() const;
    int                     getMaxHopperAmount() const;
	float                   getCurrentHopperAmount() const;
	float                   getAmountOfResource(const NetworkId &resourceId) const;
	ResourceClassObject *   getMasterClass() const;
	void                    sendResourceDataToClient(CreatureObject &clientObject);
	bool                    isEmpty() const;
	NetworkId const &       getSelectedResourceTypeId() const;

	//setters
	void                    setCurrentExtractionRate(float currentExtractionRate);
	void                    setMaxExtractionRate(int maxExtractionRate);
	void                    setMaxHopperAmount(int maxHopperAmount);
	void                    addResource(const NetworkId &resourceId, float amount);
			
protected:
	ServerSynchronizedUi * createSynchronizedUi ();

private:
	void roundHopperContents();
	float getNextAlterTime(float baseAlterTime) const;
	void takeSurvey();
	ResourcePoolObject const * getSelectedResourcePool() const;

private:
  	HarvesterInstallationObject();
	HarvesterInstallationObject(const HarvesterInstallationObject& rhs);
	HarvesterInstallationObject&	operator=(const HarvesterInstallationObject& rhs);

private:
//BPM InstallationObject : BuildingObject // Begin persisted members.
	Archive::AutoDeltaVariable<float> m_installedEfficiency; ///< For active harvesters, their resource collection efficiency
	Archive::AutoDeltaVariable<NetworkId> m_resourceType; ///< The resource type this harvester is set to use
	
	Archive::AutoDeltaVariable<int> m_maxExtractionRate;
	Archive::AutoDeltaVariable<float> m_currentExtractionRate;
	Archive::AutoDeltaVariable<int> m_maxHopperAmount;
	Archive::AutoDeltaVariable<NetworkId> m_hopperResource;
	Archive::AutoDeltaVariable<float> m_hopperAmount; // amount of resource in the hopper.  0=empty, >0 = hopper containes that quantity of m_hopperResource, <0 = use object variables instead (contains multiple resources)
//EPM

	std::vector<ResourceTypeObject const *> * m_survey;
	uint32 m_surveyTime;
	
/// Members yet to be implemented.
// inputResourceTypes
};


#endif
