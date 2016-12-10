//========================================================================
//
// ManufactureInstallationObject.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_ManufactureInstallationObject_H
#define _INCLUDED_ManufactureInstallationObject_H

#include "serverGame/InstallationObject.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedFoundation/StationId.h"

class FactoryObject;
class ManufactureSchematicObject;
class ServerManufactureInstallationObjectTemplate;


/** 
  * A ManufactureInstallationObject is used to create new objects from a 
  * manufacture schematic. 
  */
class ManufactureInstallationObject : public InstallationObject
{
	friend class InstallationController;
	
public:

	ManufactureInstallationObject(const ServerManufactureInstallationObjectTemplate* newTemplate);
	virtual ~ManufactureInstallationObject();


	virtual Controller* createDefaultController();
	void                addMembersToPackages();
	virtual void        getAttributes(AttributeVector &data) const;
	virtual void        onContainerChildLostItem(ServerObject * destination, ServerObject& item, ServerObject* source, ServerObject* transferer);
	virtual void        onContainerChildGainItem(ServerObject& item, ServerObject* source, ServerObject* transferer);

	virtual float       alter(float time);
	
	uint32              getValidSchematicTypes() const;
	int                 getNumNewItems() const;
	const std::string & getOwnerName() const;
	StationId           getOwnerStationId() const;
	virtual void        setOwnerId(const NetworkId &id);
	
	ServerObject * getInputHopper() const;
	ServerObject * getOutputHopper() const;

	bool                         addSchematic(ManufactureSchematicObject & schematic, ServerObject * transferer);
	virtual bool                 onContainerAboutToLoseItem(ServerObject * destination, ServerObject& item, ServerObject* transferer);
	ManufactureSchematicObject * getSchematic() const;

	float getTimePerObject() const;

	//simulation functions:
	virtual void activate         (const NetworkId &actorId);
	virtual void deactivate       ();
	virtual void harvest          ();

	bool         createObject     (bool testOnly = false);

protected:
	virtual void        initializeFirstTimeObject();
	virtual void        endBaselines();
	
	void setTickCount(float count);

private:
  	ManufactureInstallationObject();
	ManufactureInstallationObject(const ManufactureInstallationObject& rhs);
	ManufactureInstallationObject&	operator=(const ManufactureInstallationObject& rhs);

private:

	class TaskManufactureObject;
	friend class TaskManufactureObject;

	typedef std::vector<std::pair<CachedNetworkId, int> > IngredientVector;

	void              restoreIngredients(IngredientVector const &ingredients);
	void              destroyIngredients(IngredientVector const &ingredients);
	bool              transferCraftedIngredientToSchematic(ServerObject & inputHopper, ManufactureSchematicObject & schematic, const NetworkId & craftedId, int ingredientCount, IngredientVector &ingredients);
	const NetworkId & transferTemplateIngredientToSchematic(ServerObject & inputHopper, ManufactureSchematicObject & schematic, const ObjectTemplate & componentTemplate, IngredientVector &ingredients);
	const NetworkId & transferResourceTypeToSchematic(ServerObject & inputHopper, ManufactureSchematicObject & schematic, const NetworkId & resourceTypeId, int resourceCount);

	FactoryObject * getCurrentCrate(const ManufactureSchematicObject & contents);
	FactoryObject * makeNewCrate(const ManufactureSchematicObject & contents);

private:

	CachedNetworkId                                  m_currentCrate; // crate we are stuffing objects into
};


inline void ManufactureInstallationObject::setTickCount(float count)
{
	InstallationObject::setTickCount(count);
}


#endif	// _INCLUDED_ManufactureInstallationObject_H
