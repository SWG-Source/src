//========================================================================
//
// ResourceContainerObject.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_ResourceContainerObject_H
#define _INCLUDED_ResourceContainerObject_H

#include "serverGame/TangibleObject.h"

class ResourceTypeObject;
class ServerResourceContainerObjectTemplate;

// ======================================================================

class ResourceContainerObject : public TangibleObject
{
  public:
	
	ResourceContainerObject(const ServerResourceContainerObjectTemplate* newTemplate);
	virtual               ~ResourceContainerObject();

	static void            removeDefaultTemplate(void);
	virtual Controller*    createDefaultController(void);

	// Container transfers
	bool                   addResource     (const NetworkId &resourceType, int amount, const NetworkId &sourcePlayer);
	bool                   addResource     (ResourceTypeObject const * const resourceType, int amount, NetworkId const & sourcePlayer);
	bool                   removeResource  (const NetworkId &resourceType, int amount, std::vector<std::pair<NetworkId, int> > *sources);
	bool                   removeResourceWithoutExperience(const NetworkId &resourceType, int amount); //TODO: rewrite crafting to handle experience, remove this function
	bool                   transferTo      (ResourceContainerObject &destination, int amount);
	bool                   splitContainer  (int amount, const CachedNetworkId &destContainer, int arrangementId, const Vector &newLocation, ServerObject *actor);
	bool                   debugRecycle    ();

	ResourceTypeObject const * const getResourceType () const;
	NetworkId              getResourceTypeId () const;
	int                    getQuantity     () const;
	int                    getMaxQuantity  () const;

	std::string            getResourceCtsData() const;
	void                   setResourceCtsData(int quantity, const std::string & resourceData);

	std::string            debugPrint      () const;
	virtual void           getAttributes   (std::vector<std::pair<std::string, Unicode::String> > &data) const;
	virtual void           endBaselines    ();

	virtual std::string    getItemLog() const;
	
  protected:

	virtual const SharedObjectTemplate *  getDefaultSharedTemplate(void) const;

  private:
	void                   addResourceSource(const NetworkId &source, int amount);
	bool                   transferHelper   (ResourceContainerObject &destination, int amount, const NetworkId &resourceSourcePlayer);
	bool                   internalRemoveResource (const NetworkId &resourceType, int amount, NetworkId &source, int &amountRemoved);
		
  private:
	ResourceContainerObject();
	ResourceContainerObject(const ResourceContainerObject& rhs);
	ResourceContainerObject&	operator=(const ResourceContainerObject& rhs);

  private:
	void                    addMembersToPackages();
	void                    updateResourceNames ();

	static const SharedObjectTemplate * m_defaultSharedTemplate;	// template to use if no shared template is given

	// persisted
	Archive::AutoDeltaVariable<NetworkId>        m_resourceType;

	Archive::AutoDeltaVariable<int>              m_quantity;
	Archive::AutoDeltaVariable<NetworkId>        m_source;

	//non-persisted
	Archive::AutoDeltaVariable<int>              m_maxQuantity;
	Archive::AutoDeltaVariable<Unicode::String>  m_resourceName;
	Archive::AutoDeltaVariable<std::string>      m_parentName;
	Archive::AutoDeltaVariable<StringId>         m_resourceNameId;
};

// ----------------------------------------------------------------------

inline int ResourceContainerObject::getQuantity() const
{
	return m_quantity.get();
}
	
// ----------------------------------------------------------------------

inline int ResourceContainerObject::getMaxQuantity() const
{
	return m_maxQuantity.get();
}

// ======================================================================

#endif	// _INCLUDED_ResourceContainerObject_H
