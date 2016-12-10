// ======================================================================
//
// InstallationSynchronizedUi.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_InstallationSynchronizedUi_H
#define	INCLUDED_InstallationSynchronizedUi_H

#include "Archive/AutoDeltaVector.h"
#include "serverGame/ServerSynchronizedUi.h"

class InstallationObject;
struct InstallationResourceData;

class InstallationSynchronizedUi : public ServerSynchronizedUi
{
public:
	explicit InstallationSynchronizedUi(InstallationObject &);
	~InstallationSynchronizedUi();
		
	typedef std::vector<InstallationResourceData> ResourceDataVector;
	typedef std::vector<NetworkId>                ResourcePoolVector;
	typedef std::vector<NetworkId>                ResourceTypeIdVector;
	typedef std::vector<std::string>              ResourceTypeNameVector;
	typedef std::vector<std::string>              ResourceTypeParentVector;
	typedef std::pair<NetworkId, float>              HopperContentElement;
	typedef std::vector<HopperContentElement>     HopperContentsVector;

	void                         getResourceData (ResourceDataVector & data);

	const ResourcePoolVector &       getResourcePools         () const;
	const ResourceTypeIdVector &     getResourceTypesId       () const;
	const ResourceTypeNameVector &   getResourceTypesName     () const;
	const ResourceTypeParentVector & getResourceTypesParent   () const;

	const NetworkId &            getSelectedResourcePool () const;
	bool                         getActive               () const;
	float                        getEfficiencyCur        () const;
	int                          getEfficiencyMax        () const;
	float                        getExtractionRate       () const;
	float                        getHopperContentsCur    () const;
	int                          getHopperContentsMax    () const;
	const HopperContentsVector & getHopperContentsVector () const;

	void                         clearResources       ();
	void                         addResource          (const NetworkId & id);
	void                         setResourceDepleted  (const NetworkId & id);

	void                         refresh              ();
	void                         resetResourcePools   ();
	
private:

	InstallationSynchronizedUi();
	InstallationSynchronizedUi(const InstallationSynchronizedUi& );
	InstallationSynchronizedUi& operator= (const InstallationSynchronizedUi &);
	
	void addResourceType (const NetworkId& pool);
	void clearResourceTypes();

	/**
	* This variable is a clever? work around for the inability to put callbacks on an AutoDeltaVector.
	* _all_ modifications to either resource vector should increment this counter.  rollover is fine.
	*/
	Archive::AutoDeltaVariable<uint8>                 m_resourcesModified;

	typedef Archive::AutoDeltaVector<NetworkId>       AutoResourcePoolVector;
	typedef Archive::AutoDeltaVector<NetworkId>       AutoResourceTypeIdVector;
	typedef Archive::AutoDeltaVector<std::string>     AutoResourceTypeNameVector;
	typedef Archive::AutoDeltaVector<std::string>     AutoResourceTypeParentVector;
	AutoResourcePoolVector                            m_resourcePools; // a vector of type ids which have non-depleted pools on this planet
	AutoResourceTypeIdVector                          m_resourceTypesId;
	AutoResourceTypeNameVector                        m_resourceTypesName;
	AutoResourceTypeParentVector                      m_resourceTypesParent;

	Archive::AutoDeltaVariable<NetworkId>             m_resourcePoolId;
	Archive::AutoDeltaVariable<bool>                  m_active;

	Archive::AutoDeltaVariable<int>                   m_maxExtractionRate;
	Archive::AutoDeltaVariable<float>                 m_currentExtractionRate;
	Archive::AutoDeltaVariable<float>                 m_installedExtractionRate;

	Archive::AutoDeltaVariable<int>                   m_hopperContentsMax;

	Archive::AutoDeltaVariable<uint8>                 m_hopperContentsModified;

	Archive::AutoDeltaVector<HopperContentElement>    m_hopperContents;
	Archive::AutoDeltaVariable<float>                 m_hopperContentsCur; // keep this after m_hopperContents, because the client triggers updating the hopper based on receiving an update to this item
	Archive::AutoDeltaVariable<int8>                  m_condition; // condition, 0-100 (percent)
};

//----------------------------------------------------------------------

inline const NetworkId & InstallationSynchronizedUi::getSelectedResourcePool () const
{
	return m_resourcePoolId.get ();
}

//----------------------------------------------------------------------

inline bool InstallationSynchronizedUi::getActive           () const
{
	return m_active.get ();
}

//----------------------------------------------------------------------

inline float InstallationSynchronizedUi::getHopperContentsCur   () const
{
	return m_hopperContentsCur.get();
}

//----------------------------------------------------------------------

inline int  InstallationSynchronizedUi::getHopperContentsMax   () const
{
	return m_hopperContentsMax.get ();
}

//----------------------------------------------------------------------

inline const InstallationSynchronizedUi::ResourcePoolVector & InstallationSynchronizedUi::getResourcePools   () const
{
	return m_resourcePools.get ();
}

//----------------------------------------------------------------------

inline const InstallationSynchronizedUi::ResourcePoolVector & InstallationSynchronizedUi::getResourceTypesId   () const
{
	return m_resourceTypesId.get ();
}

//----------------------------------------------------------------------

inline const InstallationSynchronizedUi::ResourceTypeNameVector & InstallationSynchronizedUi::getResourceTypesName  () const
{
	return m_resourceTypesName.get ();
}

//----------------------------------------------------------------------

inline const InstallationSynchronizedUi::ResourceTypeParentVector & InstallationSynchronizedUi::getResourceTypesParent   () const
{
	return m_resourceTypesParent.get ();
}

//----------------------------------------------------------------------

inline const InstallationSynchronizedUi::HopperContentsVector & InstallationSynchronizedUi::getHopperContentsVector () const
{
	return m_hopperContents.get ();
}

//----------------------------------------------------------------------


#endif
