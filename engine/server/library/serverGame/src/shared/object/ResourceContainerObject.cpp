// ======================================================================
//
// ResourceContainerObject.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ResourceContainerObject.h"

#include "UnicodeUtils.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/ResourceContainerController.h"
#include "serverGame/ResourceTypeObject.h"
#include "serverGame/ServerMessageForwarding.h"
#include "serverGame/ServerResourceClassObject.h"
#include "serverGame/ServerResourceContainerObjectTemplate.h"
#include "serverGame/ServerUniverse.h"
#include "serverGame/ServerWorld.h"
#include "sharedFoundation/ConstCharCrcLowerString.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "sharedGame/SharedObjectAttributes.h"
#include "sharedGame/SharedResourceContainerObjectTemplate.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/SlottedContainmentProperty.h"

// ======================================================================

const SharedObjectTemplate * ResourceContainerObject::m_defaultSharedTemplate = nullptr;

namespace ResourceContainerObjectNamespace
{
	const std::string OBJVAR_RESOURCE_DATA("resourceData");
};

using namespace ResourceContainerObjectNamespace;

// ======================================================================

ResourceContainerObject::ResourceContainerObject(const ServerResourceContainerObjectTemplate* newTemplate) :
TangibleObject  (newTemplate),
m_resourceType  (NetworkId::cms_invalid),
m_quantity      (0),
m_source        (NetworkId::cms_invalid),
m_maxQuantity   (newTemplate->getMaxResources()),
m_resourceName  (),
m_parentName    ()
{
	addMembersToPackages();
}
//-----------------------------------------------------------------------

ResourceContainerObject::~ResourceContainerObject()
{
}

//-----------------------------------------------------------------------

/**
 * Returns a shared template if none was given for this object.
 *
 * @return the shared template
 */
const SharedObjectTemplate * ResourceContainerObject::getDefaultSharedTemplate(void) const
{
	static const ConstCharCrcLowerString templateName("object/resource_container/base/shared_resource_container_default.iff");

	if (m_defaultSharedTemplate == nullptr)
	{
		m_defaultSharedTemplate = safe_cast<const SharedObjectTemplate *>(ObjectTemplateList::fetch(templateName));
		WARNING_STRICT_FATAL(m_defaultSharedTemplate == nullptr, ("Cannot create default shared object template %s", templateName.getString()));
		if (m_defaultSharedTemplate != nullptr)
			ExitChain::add (removeDefaultTemplate, "ResourceContainerObject::removeDefaultTemplate");
	}
	return m_defaultSharedTemplate;
}	// ResourceContainerObject::getDefaultSharedTemplate

//-----------------------------------------------------------------------

/**
 * Cleans up the default shared template.
 */
void ResourceContainerObject::removeDefaultTemplate(void)
{
	if (m_defaultSharedTemplate != nullptr)
	{
		m_defaultSharedTemplate->releaseReference();
		m_defaultSharedTemplate = nullptr;
	}
}	// ResourceContainerObject::removeDefaultTemplate

//-----------------------------------------------------------------------

/**
 * Creates a default controller for this object.
 *
 * @return the object's controller
 */
Controller* ResourceContainerObject::createDefaultController(void)
{
	Controller* _controller = new ResourceContainerController(this);

	setController(_controller);
	return _controller;
}	// ResourceContainerObject::createDefaultController

// ----------------------------------------------------------------------

bool ResourceContainerObject::addResource(ResourceTypeObject const * const resourceType, int amount, NetworkId const & sourcePlayer)
{
	if (!resourceType)
		return false;

	return addResource(resourceType->getNetworkId(), amount, sourcePlayer);
}

// ----------------------------------------------------------------------

bool ResourceContainerObject::addResource(const NetworkId &resourceType, int amount, const NetworkId &sourcePlayer)
{
	if (amount <= 0)
	{
		WARNING_STRICT_FATAL(amount<0,("Use removeResource to remove resources from a ResourceContainer.\n"));
		return amount == 0;
	}
	
	if (isAuthoritative())
	{
		if (m_quantity.get() + amount > m_maxQuantity.get())
			return false; // too much
		
		if (m_quantity.get()==0)
		{
			// need to set the resource attribute objvar for imported resource type
			if (resourceType.getValue() > NetworkId::cms_maxNetworkIdWithoutClusterId)
			{
				DynamicVariableList const & objvars = getObjVars();
				if (!objvars.hasItem(OBJVAR_RESOURCE_DATA) || (DynamicVariable::STRING != objvars.getType(OBJVAR_RESOURCE_DATA)))
				{
					ResourceTypeObject const * const rto = ServerUniverse::getInstance().getImportedResourceTypeById(resourceType);
					if (!rto)
						return false;

					std::string const resourceData = rto->getResourceTypeDataForExport();
					if (resourceData.empty())
						return false;

					removeObjVarItem(OBJVAR_RESOURCE_DATA);
					IGNORE_RETURN(setObjVarItem(OBJVAR_RESOURCE_DATA, resourceData));
				}
			}

			m_resourceType = resourceType; // note that m_quantity should be 0 only if this is a brand new crate
			m_source = sourcePlayer;

			updateResourceNames();
		}
		else if (resourceType != m_resourceType.get())
			return false; // can't contain two different kinds of resource
		
		m_quantity=m_quantity.get() + amount;
		if (sourcePlayer!=NetworkId::cms_invalid)
			addResourceSource(sourcePlayer, amount);
		
		setAttributeRevisionDirty();

		return true;
	}
	else
	{
		WARNING_STRICT_FATAL(true,("Cross-server container transfers are not supported.\n"));
		return false;
	}
}

// ----------------------------------------------------------------------

bool ResourceContainerObject::removeResource(const NetworkId &resourceType, int amount, std::vector<std::pair<NetworkId, int> > *sources)
{
	while (amount > 0)
	{
		int actualAmountRemoved;
		NetworkId source(NetworkId::cms_invalid);
		if (!internalRemoveResource(resourceType, amount, source, actualAmountRemoved))
			return false;
		amount -= actualAmountRemoved;
		if (sources)
		{
			sources->push_back(std::pair<NetworkId, int>(source, actualAmountRemoved));
		}
	}

	return true;
}

/**
 * Remove up to a certain amount of resource.  The resource removed will
 * always be from a single source player.  It may be necessary to call this
 * function more than once to extract the total amount desired.
 *
 * @param resourceType The type of resource desired
 * @param amount The maximum amount to remove
 * @param source (output) Which player the resource came from (can be cms_invalid)
 * @param amountRemoved (output) The actual amount removed.  Will be <= amount
 * @return True if at least 1 unit of the specified resource is removed
 */
bool ResourceContainerObject::internalRemoveResource(const NetworkId &resourceType, int amount, NetworkId &source, int &amountRemoved)
{
	if (amount <= 0)
	{
		WARNING_STRICT_FATAL(amount<0,("Trying to make a deposit with a withdrawal slip. (Called removeResource with a negative amount.)\n"));
		return amount == 0;
	}
	
	if (isAuthoritative())
	{
		if (resourceType!=m_resourceType.get())
			return false; // wrong kind of resource
		amountRemoved = std::min(m_quantity.get(), amount);

		// Identify source (if possible)
		if (m_source.get() == NetworkId::cms_invalid)
		{
			// multi-source container.  Complicated
			source = NetworkId::cms_invalid;
			DynamicVariableList::NestedList varList(getObjVars(),"source");
			DynamicVariableList::NestedList::const_iterator var(varList.begin());
			if (var!=varList.end())
			{
				int varInt;
				if (var.getValue(varInt))
				{
					amountRemoved = std::min(amountRemoved, varInt);
					int remainingAmountFromSource = varInt - amountRemoved;
					source = NetworkId(var.getName());
					if (remainingAmountFromSource > 0)
					{
						IGNORE_RETURN(setObjVarItem(var.getNameWithPath(), remainingAmountFromSource));
					}
					else
						removeObjVarItem(var.getNameWithPath());
				}
			}
		}
		else
		{
			// single-source container.  Simple
			source = m_source.get();
		}

		// Adjust the quantity remaining
		m_quantity=m_quantity.get() - amountRemoved;
		if (m_quantity.get() == 0)
			IGNORE_RETURN(permanentlyDestroy(DeleteReasons::Consumed)); // containers delete themselves when empty

		setAttributeRevisionDirty();

		return true;
	}
	else
	{
		WARNING_STRICT_FATAL(true,("Cross-server container transfers are not supported.\n"));
		return false;
	}
}

// ----------------------------------------------------------------------

std::string ResourceContainerObject::getResourceCtsData() const
{
	// resource came from another galaxy, so all the data is stored in the objvar
	DynamicVariableList const & objvars = getObjVars();
	if (objvars.hasItem(OBJVAR_RESOURCE_DATA) && (DynamicVariable::STRING == objvars.getType(OBJVAR_RESOURCE_DATA)))
	{
		std::string resourceData;
		IGNORE_RETURN(objvars.getItem(OBJVAR_RESOURCE_DATA, resourceData));
		return resourceData;
	}
	// resource came from this cluster, so construct the resource data
	else
	{
		ResourceTypeObject const * const typeObj = getResourceType();
		if (typeObj)
			return typeObj->getResourceTypeDataForExport();
		
		return std::string();
	}
}

// ----------------------------------------------------------------------

void ResourceContainerObject::setResourceCtsData(int quantity, const std::string & resourceData)
{
	if (!isAuthoritative())
		return;
	
	if (m_resourceType.get().isValid())
		return;

	if (m_quantity.get() > 0)
		return;

	if (quantity <= 0)
		return;

	if (resourceData.empty())
		return;

	removeObjVarItem(OBJVAR_RESOURCE_DATA);
	NetworkId const importedResourceType = ResourceTypeObject::addImportedResourceType(resourceData);
	if (importedResourceType.isValid())
	{
		if (importedResourceType.getValue() > NetworkId::cms_maxNetworkIdWithoutClusterId)
		{
			// need to tell all proxy game servers, if any, to create the imported
			// resource type, if it hasn't already been created on that game server
			ProxyList const &proxyList = getExposedProxyList();
			if (!proxyList.empty())
			{
				ServerMessageForwarding::begin(std::vector<uint32>(proxyList.begin(), proxyList.end()));

				GenericValueTypeMessage<std::pair<NetworkId, std::string> > const addImportedResourceTypeMsg("AddImportedResourceType", std::make_pair(importedResourceType, resourceData));
				ServerMessageForwarding::send(addImportedResourceTypeMsg);

				ServerMessageForwarding::end();
			}
		}

		IGNORE_RETURN(!addResource(importedResourceType, quantity, NetworkId::cms_invalid));
	}

	if (!getObjVars().hasItem(OBJVAR_RESOURCE_DATA))
	{
		if (!m_resourceType.get().isValid() || (m_resourceType.get().getValue() > NetworkId::cms_maxNetworkIdWithoutClusterId))
			IGNORE_RETURN(setObjVarItem(OBJVAR_RESOURCE_DATA, resourceData));
	}
}

// ----------------------------------------------------------------------

std::string ResourceContainerObject::debugPrint() const
{
	char buffer[20];
	IGNORE_RETURN(snprintf(buffer, 20, "%i",m_quantity.get()));
	ResourceTypeObject const * const r=ServerUniverse::getInstance().getResourceTypeById(m_resourceType.get());
	if (r)
		return (std::string("Resource Container with ")+buffer+" units of "+r->getResourceName());
	else
		return ("Invalid resource container");
}

// ----------------------------------------------------------------------

/**
 * Transfer resources from one container to another.
 * @return true if all the resource was tranferred, false if no resources
 * or less than the requested amount was transferred.
 * @todo Inter-server authority issues
 */
bool ResourceContainerObject::transferTo(ResourceContainerObject &destination, int amount)
{
	if (isAuthoritative())
	{
		if ((m_resourceType.get() != destination.m_resourceType.get() && (destination.m_resourceType.get() != NetworkId::cms_invalid))
			|| amount > m_quantity.get())
			return false;

		NetworkId sourcePlayer(NetworkId::cms_invalid);
		int amountRemoved = 0;
		while (amount > 0)
		{
			if (!internalRemoveResource(m_resourceType.get(), amount, sourcePlayer, amountRemoved))
				return false;
			if (!destination.addResource(m_resourceType.get(), amountRemoved, sourcePlayer))
			{
				// failed to add resource to destination, put it back into the source
				IGNORE_RETURN(addResource(m_resourceType.get(), amountRemoved, sourcePlayer));
				return false;
			}

			amount -= amountRemoved;
		}
		return true;		
	}
	else
	{
		WARNING_STRICT_FATAL(true,("Cross-server container transfers are not supported.\n"));
		return false;
	}
}

// ----------------------------------------------------------------------

/** 
 * Split this container into two containers
 * @param amount The amount to put in the new ResourceContainer
 * @param destContainer The container into which the new ResourceContainer should be placed, or cms_Invalid if no container
 * @param arrangementId -1 If destContainer is not slotted, otherwise the ID of the arrangement to use
 * @param newLocation The coordinates at which to place the new ResourceContainer.  (Ignored if it is going into a non-positional container.)
 * @param actor The player making the split, or nullptr
 */

bool ResourceContainerObject::splitContainer(int amount, const CachedNetworkId &destContainer, int arrangementId, const Vector &newLocation, ServerObject *actor)
{
	UNREF(actor);

	bool result = false;
	
	ResourceContainerObject *newCrate = nullptr;
	
	if (destContainer == CachedNetworkId::cms_cachedInvalid)
	{
		// create in world
		Transform tr;
		tr.setPosition_p(newLocation);
		newCrate = dynamic_cast<ResourceContainerObject*>(ServerWorld::createNewObject(getObjectTemplateName(), tr, 0, isPersisted()));
	}
	else if (arrangementId < 0)
	{
		// create in volume container
		ServerObject * destObject = safe_cast<ServerObject *>(destContainer.getObject());
		if (destObject == nullptr)
			return false;
		newCrate = dynamic_cast<ResourceContainerObject*>(ServerWorld::createNewObject(getObjectTemplateName(), *destObject, isPersisted()));
	}
	else
	{
		// create in slotted container
		ServerObject * destObject = safe_cast<ServerObject *>(destContainer.getObject());
		if (destObject == nullptr)
			return false;
		const SlottedContainmentProperty* containmentProperty = ContainerInterface::getSlottedContainmentProperty(*destObject);
		if (containmentProperty == nullptr)
			return false;
		const SlottedContainmentProperty::SlotArrangement & slots = containmentProperty->getSlotArrangement(arrangementId);
		if (slots.empty())
			return false;
		newCrate = dynamic_cast<ResourceContainerObject*>(ServerWorld::createNewObject(getObjectTemplateName(), *destObject, slots.front(), isPersisted()));
	}
	if (newCrate)
	{
		result = transferTo(*newCrate, amount);

		if (!result)
			IGNORE_RETURN(newCrate->permanentlyDestroy(DeleteReasons::BadContainerTransfer));
	}
	
	return result;
}

//----------------------------------------------------------------------

void ResourceContainerObject::updateResourceNames()
{
	if (!isAuthoritative())
		return;

	m_resourceName.set (Unicode::emptyString);
	m_parentName.set   (std::string ());
	m_resourceNameId.set (StringId::cms_invalid);

	setAttributeRevisionDirty();

	if (!m_resourceType.get ().isValid ())
	{
		setObjectName (Unicode::emptyString);
		return;
	}

	//-- setup the parent inheritance chain.
	//-- the client needs this information

	ResourceTypeObject const * const type = ServerUniverse::getInstance().getResourceTypeById(m_resourceType.get());

	if (type)
	{
		m_resourceName = Unicode::narrowToWide(type->getResourceName ());
		if ((m_resourceName.get())[0] == '@')
		{
			m_resourceNameId = StringId(Unicode::wideToNarrow(m_resourceName.get()));
		}
		
		ResourceClassObject const & parentClass = type->getParentClass();

		m_parentName = parentClass.getResourceClassName ();

		ResourceClassObject const * const parentClass2 = parentClass.getParent ();

		if (parentClass2)
		{
			setObjectName(Unicode::emptyString);
			setObjectNameStringId (parentClass2->getFriendlyName());
		}
		else
		{
			setObjectName(Unicode::emptyString);
			setObjectNameStringId (parentClass.getFriendlyName());
		}

	}
	else
		DEBUG_WARNING (true, ("Unable to find specified resource type %s", m_resourceType.get ().getValueString ().c_str ()));
}

// ----------------------------------------------------------------------

std::string ResourceContainerObject::getItemLog() const
{
	char tmpBuffer[1024];
	IGNORE_RETURN(snprintf(tmpBuffer, sizeof(tmpBuffer) - 1, ": %d units of %s", getQuantity(), Unicode::wideToNarrow(m_resourceName.get()).c_str()));
	tmpBuffer[sizeof(tmpBuffer)-1] = 0;

	return TangibleObject::getItemLog() + tmpBuffer;
}

// ----------------------------------------------------------------------

void ResourceContainerObject::getAttributes(std::vector<std::pair<std::string, Unicode::String> > &data) const
{
	TangibleObject::getAttributes(data);

	char valueBuffer[32];
	IGNORE_RETURN(snprintf (valueBuffer, sizeof (valueBuffer), "%d/%d", getQuantity (), getMaxQuantity ()));
	data.push_back (std::make_pair (SharedObjectAttributes::resource_contents, Unicode::narrowToWide (valueBuffer)));

	ResourceTypeObject const * const typeObj = getResourceType();
	if (typeObj)
	{
		data.push_back (std::make_pair (SharedObjectAttributes::resource_name, Unicode::narrowToWide(typeObj->getResourceName ())));

		ResourceClassObject const & parent = typeObj->getParentClass ();
		data.push_back (std::make_pair (SharedObjectAttributes::resource_class, Unicode::narrowToWide ("@" + parent.getFriendlyName ().getCanonicalRepresentation())));

		typeObj->getResourceAttributes(data);
	}
}

// ----------------------------------------------------------------------

void ResourceContainerObject::endBaselines()
{
	TangibleObject::endBaselines();

	if (isAuthoritative())
	{
		// if resource is an imported resource, register the imported resource type on
		// this game server, if it hasn't already been registered on this game server;
		// this must be done on each game server as it receives the resource container
		DynamicVariableList const & objvars = getObjVars();
		if (objvars.hasItem(OBJVAR_RESOURCE_DATA) && (DynamicVariable::STRING == objvars.getType(OBJVAR_RESOURCE_DATA)))
		{
			bool const alreadyRegistered = (m_resourceType.get().isValid() && (m_resourceType.get().getValue() > NetworkId::cms_maxNetworkIdWithoutClusterId) && ServerUniverse::getInstance().getImportedResourceTypeById(m_resourceType.get()));
			if (!alreadyRegistered)
			{
				std::string resourceData;
				IGNORE_RETURN(objvars.getItem(OBJVAR_RESOURCE_DATA, resourceData));
				NetworkId const importedResourceType = ResourceTypeObject::addImportedResourceType(resourceData);
				if (importedResourceType.isValid())
				{
					if (importedResourceType != m_resourceType.get())
						m_resourceType = importedResourceType;

					if (importedResourceType.getValue() <= NetworkId::cms_maxNetworkIdWithoutClusterId)
						removeObjVarItem(OBJVAR_RESOURCE_DATA);
				}
			}
		}

		updateResourceNames();
	}
	else
	{
		// if resource is an imported resource, register the imported resource type on
		// this game server, if it hasn't already been registered on this game server;
		// this must be done on each game server as it receives the resource container
		if (m_resourceType.get().isValid() && (m_resourceType.get().getValue() > NetworkId::cms_maxNetworkIdWithoutClusterId) && !ServerUniverse::getInstance().getImportedResourceTypeById(m_resourceType.get()))
		{
			DynamicVariableList const & objvars = getObjVars();
			if (objvars.hasItem(OBJVAR_RESOURCE_DATA) && (DynamicVariable::STRING == objvars.getType(OBJVAR_RESOURCE_DATA)))
			{
				std::string resourceData;
				IGNORE_RETURN(objvars.getItem(OBJVAR_RESOURCE_DATA, resourceData));
				IGNORE_RETURN(ResourceTypeObject::addImportedResourceType(resourceData));
			}
		}
	}
}

// ----------------------------------------------------------------------

void ResourceContainerObject::addResourceSource(const NetworkId &source, int amount)
{
	if (source == m_source.get())
		return;  // single-source container adding more resource from the same source -- no change needed

	if (m_source.get() != NetworkId::cms_invalid)
	{
		// converting a single-source container into a multi-source container.  All resources except the ones from the new source get credited to the first source
		const std::string & varname = std::string("source.") + m_source.get().getValueString();
		IGNORE_RETURN(setObjVarItem(varname, m_quantity.get() - amount));
		m_source = NetworkId::cms_invalid;
	}
	
	const std::string & varname = std::string("source.") + source.getValueString ();
	int intvar = 0;
	if (getObjVars().getItem(varname,intvar))
	{
		IGNORE_RETURN(setObjVarItem(varname, intvar+amount));
	}
	else
	{
		removeObjVarItem(varname);

		IGNORE_RETURN(setObjVarItem(varname, amount));
	}
}

// ----------------------------------------------------------------------

bool ResourceContainerObject::removeResourceWithoutExperience(const NetworkId &resourceType, int amount)
{
	while (amount > 0)
	{
		int amountRemoved = 0;
		NetworkId source;
		if (!internalRemoveResource(resourceType, amount, source, amountRemoved))
			return false;
		amount -= amountRemoved;
	}
	return true;
}

// ----------------------------------------------------------------------

ResourceTypeObject const * const ResourceContainerObject::getResourceType() const
{
	return ServerUniverse::getInstance().getResourceTypeById(m_resourceType.get());
}

// ----------------------------------------------------------------------

NetworkId ResourceContainerObject::getResourceTypeId() const
{
	return m_resourceType.get();
}

// ----------------------------------------------------------------------

/**
 * For debugging only, convert this container to the recycled equivalent.
 * This is not intended for uses other than debugging, because the container
 * might not be the right kind of container to hold the recycled resource
 */
bool ResourceContainerObject::debugRecycle()
{
	if (!isAuthoritative())
		return false;

	ResourceTypeObject const * const resourceType=getResourceType();
	if (!resourceType)
		return false;
	if (resourceType->isRecycled())
		return true; // already recycled
	ResourceTypeObject const * const recycledType=resourceType->getRecycledVersion();
	if (!recycledType)
		return false; // no recycled version of this resource
	DEBUG_FATAL(recycledType == resourceType,("Programmer bug:  resourceType->getRecycledVersion on %s returned the same type.",resourceType->getResourceName().c_str()));

	m_resourceType = recycledType->getNetworkId();
	updateResourceNames();
	return true;
}

// ======================================================================
