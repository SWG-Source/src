// ======================================================================
//
// FactoryObject.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_FactoryObject_H
#define INCLUDED_FactoryObject_H

#include "serverGame/ManufactureObjectInterface.h"
#include "serverGame/TangibleObject.h"
#include "Archive/AutoDeltaMap.h"


class ServerFactoryObjectTemplate;


class FactoryObject : public TangibleObject, public ManufactureObjectInterface
{
public:
	typedef std::map<StringId, float> AttribMap;
	typedef std::vector<StringId>     StringIdList;

	explicit FactoryObject(const ServerFactoryObjectTemplate* newTemplate);
	virtual ~FactoryObject();

	void                   initialize(const ManufactureSchematicObject & source);
	void                   calculateAttributes();

	bool                   isFactoryOk() const;

	static void            removeDefaultTemplate(void);
	virtual bool           canDestroy() const;
	virtual void           onContainerTransfer(ServerObject * destination, ServerObject* transferer);

	virtual Controller*    createDefaultController(void);
	void                   addMembersToPackages   ();
	virtual void           getAttributes          (std::vector<std::pair<std::string, Unicode::String> > &data) const;

	bool                   startCraftingSession(ManufactureSchematicObject & schematic);	
	bool                   endCraftingSession();
	bool                   resetCraftingSession();
	bool                   inCraftingSession() const;
	bool                   removeCraftingReferences(int count);
	bool                   addCraftingReferences(int count);

	bool                   addObject();
	bool                   removeObject(ServerObject & destination);
	int                    deleteContents(int count);
	bool                   transferObjects(FactoryObject & toCrate, int objectCount);
	FactoryObject *        makeCopy(ServerObject & destination, int count, bool destroySource = true);

	virtual void           onContainedObjectLoaded(const NetworkId &oid);

	const char *           getContainedTemplateName() const;
	const char *           getContainedSharedTemplateName() const;
	const ObjectTemplate * getContainedObjectTemplate() const;
	const TangibleObject * getContainedObject() const;

	const std::string &    getAppearanceData() const;
	const std::string &    getItemCustomAppearance() const;

	// ManufactureObjectInterface functions
	virtual uint32               getDraftSchematic() const;
	virtual bool                 getSlot(int index, Crafting::IngredientSlot & data, bool flag) const;
	virtual int                  getSlotsCount() const;
	virtual const AttribMap &    getAttributes() const;
	virtual void                 setAttribute(const ServerIntangibleObjectTemplate::SchematicAttribute & attribute);
	virtual void                 setAttribute(const StringId & name, float value);
	virtual const AttribMap &    getResourceMaxAttributes() const;
	virtual void                 setResourceMaxAttribute(const StringId & name, float value);
	virtual const StringIdList & getExperimentAttributeNames() const;
	virtual void                 setExperimentAttribute(const StringId & name, float value);
	virtual void                 getAllExperimentAttributeValues(const StringId & name, float & value, float & minValue, float & maxValue, float & resourceMaxValue) const;
	virtual void                 setExperimentAttributeLimits(const StringId & name, float minValue, float maxValue, float resourceMaxValue);
	virtual bool                 isMakingObject(void) const;
	virtual const NetworkId &    getDerivedNetworkId() const;

protected:
	virtual void        initializeFirstTimeObject();
	virtual void        endBaselines();
	virtual void        onLoadedFromDatabase();

	virtual const SharedObjectTemplate *  getDefaultSharedTemplate(void) const;

private:
  	FactoryObject();
	FactoryObject(const FactoryObject& rhs);
	FactoryObject&	operator=(const FactoryObject& rhs);

	TangibleObject *  manufactureObject();
	void              addIdToObjectList(const NetworkId & id);
	const NetworkId   removeIdFromObjectList(void);
	void              addIdToPendingList(const NetworkId & id, const ServerObject & destination);
	ServerObject *    removeIdFromPendingList(const NetworkId & id);
	int               getPendingListCount() const;
	int               getObjectListCount() const;

private:
	static const SharedObjectTemplate * m_defaultSharedTemplate;    // template to use if no shared template is given
	
	mutable bool m_badFactoryLogged;    // keep track if we've logged that this factory is bad to reduce spam

	// manf schematic data for creating new objects
	Archive::AutoDeltaMap<StringId, float> m_attributes;
	Archive::AutoDeltaVariable<CachedNetworkId> m_craftingSchematic;     // the schematic being used during a crafting session
	Archive::AutoDeltaVariable<int>        m_craftingCount;         // the number of our objects being used in the schematic
};


#endif	// INCLUDED_FactoryObject_H
