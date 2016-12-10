//========================================================================
//
// DraftSchematicObject.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef INCLUDED_DraftSchematicObject_H
#define INCLUDED_DraftSchematicObject_H

#include "serverGame/IntangibleObject.h"
#include "serverGame/ServerDraftSchematicObjectTemplate.h"
#include "sharedGame/CraftingDataArchive.h"
#include "sharedGame/SharedDraftSchematicObjectTemplate.h"

class ManufactureSchematicObject;


/**
  * A DraftSchematicObject is used to create designs for new objects.
  */
class DraftSchematicObject : public IntangibleObject
{
public:
	// factory methods
	static const DraftSchematicObject * getSchematic(const std::string & name);
	static const DraftSchematicObject * getSchematic(uint32 crc);
	static void install();
	static void remove();

public:

	virtual ~DraftSchematicObject();

	static void                                                    removeDefaultTemplate(void);
	static void                                                    requestResourceWeights(ServerObject & requester, uint32 draftSchematicCrc);

	const std::pair<uint32, uint32> &                              getCombinedCrc() const;

	ServerIntangibleObjectTemplate::CraftingType                   getCategory(void) const;
	const ServerObjectTemplate *                                   getCraftedObjectTemplate() const;
	const ServerFactoryObjectTemplate *                            getCrateObjectTemplate() const;
	bool                                                           getSlot(ServerDraftSchematicObjectTemplate::IngredientSlot & slot, const StringId & name) const;
	bool                                                           getSlot(ServerDraftSchematicObjectTemplate::IngredientSlot & slot, int index) const;
	int                                                            getSlotsCount(void) const;
	int                                                            getAttribIndex(const StringId & name) const;
	const SharedDraftSchematicObjectTemplate::SchematicAttribute & getAttrib(int index) const;
	const SharedDraftSchematicObjectTemplate::SchematicAttribute & getAttrib(const StringId & name) const;
	const SharedDraftSchematicObjectTemplate::SchematicAttribute & getAttribMin(int index) const;
	const SharedDraftSchematicObjectTemplate::SchematicAttribute & getAttribMax(int index) const;
	bool                                                           getAttribMinMax(const StringId & name, int & minValue, int & maxValue) const;
	bool                                                           getAttribMinMax(int index, int & minValue, int & maxValue) const;
	int                                                            getAttribsCount(void) const;
	const StringId                                                 getExperimentalAttribute(int index) const;
	size_t                                                         getExperimentalAttributesCount(void) const;
	const std::string &                                            getSkillCommands(int index) const;
	int                                                            getSkillCommandsCount(void) const;
	int                                                            getItemsPerContainer(void) const;
	float                                                          getManufactureTime(void) const;
	float                                                          getPrototypeTime(void) const;
	bool                                                           mustDestroyIngredients(void) const;
	virtual void                                                   getAttributes(std::vector<std::pair<std::string, Unicode::String> > &data) const;

	const StringId                getExperimentalAttributeFromAttribute(int index) const;
	const StringId                getExperimentalAttributeFromAttribute(const StringId & attribName) const;
	const std::vector<StringId> & getFromAttributeFromExperimentalAttribute(const StringId & experimentAttribName) const;

	ManufactureSchematicObject * createManufactureSchematic(const CachedNetworkId & creator) const;

protected:

	virtual const SharedObjectTemplate *  getDefaultSharedTemplate(void) const;

// if in debug mode, make constructor public for datalint, else make it private
#ifdef _DEBUG
public:
	explicit DraftSchematicObject(const ServerDraftSchematicObjectTemplate* newTemplate);
#else
private:
	explicit DraftSchematicObject(const ServerDraftSchematicObjectTemplate* newTemplate);
#endif

private:
	DraftSchematicObject();
	DraftSchematicObject(const DraftSchematicObject& rhs);
	DraftSchematicObject&	operator=(const DraftSchematicObject& rhs);

private:
	typedef std::map<StringId, int>                    NameToIndexMap;
	typedef std::map<StringId, std::vector<StringId> > ExpAttribsMap;
	
	NameToIndexMap m_slots;                   // for quick slot lookup by name
	NameToIndexMap m_attributes;              // for quick attribute lookup by name
	ExpAttribsMap  m_experimentalAttributes;  // maps experimental attribs to attribs

	const std::pair<uint32, uint32> m_combinedCrc;  // server and shared template crcs

private:

	static const SharedObjectTemplate * m_defaultSharedTemplate;	// template to use if no shared template is given

	// schematic factory fields
	typedef std::map<uint32, const DraftSchematicObject *> SchematicMap;

	// since draft schematics don't have valid NetworkIds, we keep track of them
	// by their template name
	static SchematicMap *m_schematics;
};


//========================================================================

inline ServerIntangibleObjectTemplate::CraftingType DraftSchematicObject::getCategory() const
{
	return (safe_cast<const ServerDraftSchematicObjectTemplate *>(getObjectTemplate()))->getCategory();
}

//------------------------------------------------------------------------------

inline const ServerObjectTemplate * DraftSchematicObject::getCraftedObjectTemplate() const
{
	return (safe_cast<const ServerDraftSchematicObjectTemplate*>(getObjectTemplate()))->getCraftedObjectTemplate();
}

//------------------------------------------------------------------------------

inline const ServerFactoryObjectTemplate * DraftSchematicObject::getCrateObjectTemplate() const
{
	return (safe_cast<const ServerDraftSchematicObjectTemplate*>(getObjectTemplate()))->getCrateObjectTemplate();
}

//------------------------------------------------------------------------------

inline const std::pair<uint32, uint32> & DraftSchematicObject::getCombinedCrc() const
{
	return m_combinedCrc;
}

//------------------------------------------------------------------------------

inline int DraftSchematicObject::getSlotsCount(void) const
{
	return (safe_cast<const ServerDraftSchematicObjectTemplate*>(getObjectTemplate()))->getSlotsCount();
}

//------------------------------------------------------------------------------

inline int DraftSchematicObject::getAttribIndex(const StringId & name) const
{
	NameToIndexMap::const_iterator iter = m_attributes.find(name);
	if (iter == m_attributes.end())
		return -1;
	return (*iter).second;
}

//------------------------------------------------------------------------------

inline const SharedDraftSchematicObjectTemplate::SchematicAttribute & 
	DraftSchematicObject::getAttrib(int index) const
{
static SharedDraftSchematicObjectTemplate::SchematicAttribute slot;

	(safe_cast<const SharedDraftSchematicObjectTemplate*>(getSharedTemplate()))->getAttributes(slot, index);
	return slot;
}

//------------------------------------------------------------------------------

inline const SharedDraftSchematicObjectTemplate::SchematicAttribute & 
	DraftSchematicObject::getAttrib(const StringId & name) const
{
static SharedDraftSchematicObjectTemplate::SchematicAttribute slot;

	NameToIndexMap::const_iterator iter = m_attributes.find(name);
	if (iter != m_attributes.end())
		return getAttrib((*iter).second);

	return slot;
}

//------------------------------------------------------------------------------

inline const SharedDraftSchematicObjectTemplate::SchematicAttribute & 
	DraftSchematicObject::getAttribMin(int index) const
{
static SharedDraftSchematicObjectTemplate::SchematicAttribute slot;

	(safe_cast<const SharedDraftSchematicObjectTemplate*>(getSharedTemplate()))->getAttributesMin(slot, index);
	return slot;
}

//------------------------------------------------------------------------------

inline const SharedDraftSchematicObjectTemplate::SchematicAttribute & 
	DraftSchematicObject::getAttribMax(int index) const
{
static SharedDraftSchematicObjectTemplate::SchematicAttribute slot;

	(safe_cast<const SharedDraftSchematicObjectTemplate*>(getSharedTemplate()))->getAttributesMax(slot, index);
	return slot;
}

//------------------------------------------------------------------------------

inline int DraftSchematicObject::getAttribsCount(void) const
{
	return (safe_cast<const SharedDraftSchematicObjectTemplate*>(getSharedTemplate()))->getAttributesCount();
}

//------------------------------------------------------------------------------

inline const StringId DraftSchematicObject::getExperimentalAttribute(int index) const
{
	ExpAttribsMap::const_iterator iter = m_experimentalAttributes.begin();
	for (int i = 0; i < index; ++i, ++iter);
	return (*iter).first;
}

//------------------------------------------------------------------------------

inline size_t DraftSchematicObject::getExperimentalAttributesCount(void) const
{
	return m_experimentalAttributes.size();
}

//------------------------------------------------------------------------------

inline const std::string & DraftSchematicObject::getSkillCommands(int index) const
{
	return (safe_cast<const ServerDraftSchematicObjectTemplate*>(getObjectTemplate()))->getSkillCommands(index);
}

//------------------------------------------------------------------------------

inline int DraftSchematicObject::getSkillCommandsCount() const
{
	return (safe_cast<const ServerDraftSchematicObjectTemplate*>(getObjectTemplate()))->getSkillCommandsCount();
}

//------------------------------------------------------------------------------

inline int DraftSchematicObject::getItemsPerContainer(void) const
{
	return (safe_cast<const ServerDraftSchematicObjectTemplate*>(getObjectTemplate()))->getItemsPerContainer();
}

//------------------------------------------------------------------------------

inline float DraftSchematicObject::getManufactureTime(void) const
{
	return (safe_cast<const ServerDraftSchematicObjectTemplate*>(getObjectTemplate()))->getManufactureTime();
}

//------------------------------------------------------------------------------

inline float DraftSchematicObject::getPrototypeTime(void) const
{
	return (safe_cast<const ServerDraftSchematicObjectTemplate*>(getObjectTemplate()))->getPrototypeTime();
}

//------------------------------------------------------------------------------

inline bool DraftSchematicObject::mustDestroyIngredients() const
{
	return (safe_cast<const ServerDraftSchematicObjectTemplate*>(getObjectTemplate()))->getDestroyIngredients();
}


//========================================================================

#endif	// INCLUDED_DraftSchematicObject_H
