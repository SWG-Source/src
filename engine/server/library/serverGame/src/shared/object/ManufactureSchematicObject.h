//========================================================================
//
// ManufactureSchematicObject.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef INCLUDED_ManufactureSchematicObject_H
#define INCLUDED_ManufactureSchematicObject_H

#include "Archive/AutoDeltaByteStream.h"
#include "Archive/AutoDeltaMap.h"
#include "serverGame/IntangibleObject.h"
#include "serverGame/ManufactureObjectInterface.h"
#include "serverGame/ServerIntangibleObjectTemplate.h"


class CreatureObject;
class DraftSchematicObject;
class FactoryObject;
class ManufactureSchematicSynchronizedUi;
class ResourceContainerObject;
class ServerManufactureSchematicObjectTemplate;
class TangibleObject;

namespace Crafting
{
	struct CustomInfo;
	struct IngredientSlot;
	class  SimpleIngredient;
	class ComponentIngredient;
}

//========================================================================

/**
  * A ManufactureSchematicObject is used to create new objects.
  */
class ManufactureSchematicObject : public IntangibleObject, public ManufactureObjectInterface
{
public:
	typedef std::map<StringId, float> AttribMap;
	typedef std::vector<StringId>     StringIdList;

	explicit ManufactureSchematicObject(const ServerManufactureSchematicObjectTemplate* newTemplate);
	virtual ~ManufactureSchematicObject();

	virtual ManufactureSchematicObject *       asManufactureSchematicObject();
	virtual ManufactureSchematicObject const * asManufactureSchematicObject() const;

	static void                     removeDefaultTemplate ();

	virtual void                    kill                  ();

	void                            init                  (const DraftSchematicObject & schematic, const CachedNetworkId & creator);
	void                            reset                 ();
	void                            addMembersToPackages  ();
	virtual int                     getVolume             () const;
	virtual void                    getAttributes         (AttributeVector &data) const;

	ServerIntangibleObjectTemplate::CraftingType getCategory() const;

	virtual uint32                  getDraftSchematic() const;
	const NetworkId &               getCreatorId() const;
	NetworkId                       getOriginalId() const;

	bool                            mustDestroyIngredients() const;

	bool                            getSlot(const StringId & name, Crafting::IngredientSlot & data);
	virtual bool                    getSlot(int index, Crafting::IngredientSlot & data, bool flag) const;
	virtual int                     getSlotsCount() const;
	void                            setSlotType(const StringId & name, int type);
	void                            setSlotOption(const StringId & name, int option);
	void                            setSlotIndex(const StringId & name, int index);
	void                            modifySlotComplexity(const StringId & name, float complexityDelta);
	void                            addSlotResource(const StringId & name, const NetworkId & resourceId, int count, const NetworkId & sourceId);
	void                            addSlotComponent(const StringId & name, const TangibleObject & component, ServerIntangibleObjectTemplate::IngredientType ingredientType);
	TangibleObject *                getComponent(const Crafting::ComponentIngredient & info) const;
	void                            setSlotSources(int slotIndex, const std::vector<std::pair<NetworkId, int> > & sources);
	void                            clearSlot(const StringId & name);
	void                            clearSlot(int index);
	void                            clearSlotSources();
	void                            clearSlotSources(int index);
	void                            removeSlot(const StringId & name);

	void                            addCraftingFactory(const FactoryObject & factory);
	void                            removeCraftingFactory(const FactoryObject & factory);
	bool                            addIngredient(ServerObject & component);
	bool                            addIngredient(const NetworkId & resourceId, int count, const NetworkId & sourceId);
	bool                            removeIngredient(ServerObject & component, ServerObject & destination);
	bool                            removeIngredient(const NetworkId & resourceId, int count, ResourceContainerObject & destination);
	void                            destroyAllIngredients();
	bool                            hasIngredient (const NetworkId & componentId) const;

	virtual const AttribMap &       getAttributes() const;
	virtual void                    setAttribute(const ServerIntangibleObjectTemplate::SchematicAttribute & attribute);
	virtual void                    setAttribute(const StringId & name, float value);
	virtual const AttribMap &       getResourceMaxAttributes() const;
	virtual void                    setResourceMaxAttribute(const StringId & name, float value);

	void                            setExperimentMod(float mod);

	virtual const StringIdList &    getExperimentAttributeNames() const;
	float                           getExperimentAttribute(const StringId & name) const;
	virtual void                    setExperimentAttribute(const StringId & name, float value);
	float                           getMinExperimentAttribute(const StringId & name) const;
	void                            setMinExperimentAttribute(const StringId & name, float value);
	float                           getMaxExperimentAttribute(const StringId & name) const;
	void                            setMaxExperimentAttribute(const StringId & name, float value);
	float                           getResourceMaxExperimentAttribute(const StringId & name) const;
	void                            setResourceMaxExperimentAttribute(const StringId & name, float value);
	virtual void                    getAllExperimentAttributeValues(const StringId & name, float & value, float & minValue, float & maxValue, float & resourceMaxValue) const;
	virtual void                    setExperimentAttributeLimits(const StringId & name, float minValue, float maxValue, float resourceMaxValue);

	const std::vector<std::string> & getAppearances() const;
	void                             addAppearance(const std::string & appearance);
	const std::string                getCustomAppearance() const;
	void                             setCustomAppearance(const std::string & appearance);
	void                             setCustomAppearance(int index);
	const std::string &              getAppearanceData() const;

	void                            setCustomization(const std::string & name, int value, int minValue, int maxValue);
	bool                            setCustomization(int index, int value, ServerObject & prototype);
	bool                            setCustomization(const std::string & name, int value, ServerObject & prototype);

	int                             getItemsPerContainer         () const;
	void                            setItemsPerContainer         (int count);
	float                           getManufactureTime           () const;
	void                            setManufactureTime           (float time);
	void                            computeComplexity            ();

	void                            signalClientChange           ();

	void                            requestSlots                 (ServerObject & player) const;

	virtual bool                    isMakingObject(void) const;
	ServerObject *                  manufactureObject(const NetworkId & creatorId, ServerObject & container, const SlotId & containerSlotId, bool prototype);
	ServerObject *                  manufactureObject(const Vector & position);
	bool                            setObjectComponents(TangibleObject * object, bool checkResources);

	virtual const NetworkId &       getDerivedNetworkId() const;

	void                               getAttribBonuses(std::vector<std::pair<int, int> > & attribBonuses) const;
	int                                getAttribBonus(int attribute) const;
	void                               setAttribBonus(int attribute, int bonus);

	virtual void                    getByteStreamFromAutoVariable(const std::string & name, Archive::ByteStream & target) const;
	virtual void                    setAutoVariableFromByteStream(const std::string & name, const Archive::ByteStream & source);

public:
	// <name, <isComponent, countPerItem> >
	typedef std::pair<Unicode::String, std::pair<bool, int> >   IngredientInfo;
	typedef std::vector<IngredientInfo>                      IngredientInfoVector;
	void                            getIngredientInfo (IngredientInfoVector & iiv) const;

protected:
	ServerSynchronizedUi * createSynchronizedUi ();

	virtual void                    initializeFirstTimeObject();
	virtual void                    endBaselines();

	virtual const SharedObjectTemplate *  getDefaultSharedTemplate() const;

private:
	ManufactureSchematicObject();
	ManufactureSchematicObject(const ManufactureSchematicObject& rhs);
	ManufactureSchematicObject&	operator=(const ManufactureSchematicObject& rhs);

	void                            recalculateData();

private:
	static const SharedObjectTemplate * m_defaultSharedTemplate;	// template to use if no shared template is given

	Archive::AutoDeltaVariable<uint32>      m_draftSchematicSharedTemplate;
	Archive::AutoDeltaVariable<uint32>      m_draftSchematic;         // draft schematic this schematic is based on
	Archive::AutoDeltaVariable<NetworkId>   m_creatorId;              // id of crafter
	Archive::AutoDeltaVariable<Unicode::String> m_creatorName;            // name of crafter
	Archive::AutoDeltaVariable<int>         m_itemsPerContainer;      // number of items per container (FactoryObject) when manufacturing
	// @todo: move m_attributes to a non-persisted package after old manf schematics are gone
	Archive::AutoDeltaMap<StringId, float>  m_attributes;             // object attributes this schematic affects
	Archive::AutoDeltaVariable<float>       m_manufactureTime;        // time (in secs) to manufacture one object

	// not persisted
	Archive::AutoDeltaVariable<bool>             m_isCrafting;
	Archive::AutoDeltaVariable<std::string>      m_customAppearance;        // this is the custom appearance template
	Archive::AutoDeltaVariable<std::string>      m_appearanceData;          // this is the custom colors data
	Archive::AutoDeltaMap<StringId, float>       m_resourceMaxAttributes;
	Archive::AutoDeltaVariable<uint8>            m_schematicChangedSignal;
	Archive::AutoDeltaVector<CachedNetworkId>    m_factories;               // FactoryObjects that have references being used as components

	// internal flag - no need to sync
	bool m_isMakingObject;
};

//========================================================================

inline const NetworkId & ManufactureSchematicObject::getCreatorId() const
{
	return m_creatorId.get();
}

//------------------------------------------------------------------------------

inline int ManufactureSchematicObject::getItemsPerContainer() const
{
	return m_itemsPerContainer.get();
}

//------------------------------------------------------------------------------

inline void ManufactureSchematicObject::setItemsPerContainer(int count)
{
	m_itemsPerContainer = count;
}

//------------------------------------------------------------------------------

inline float ManufactureSchematicObject::getManufactureTime() const
{
	return m_manufactureTime.get();
}

//------------------------------------------------------------------------------

inline void ManufactureSchematicObject::setManufactureTime(float time)
{
	m_manufactureTime = time;
}

//------------------------------------------------------------------------------

inline void ManufactureSchematicObject::signalClientChange()
{
	m_schematicChangedSignal = static_cast<uint8>(m_schematicChangedSignal.get() + 1);
}

//------------------------------------------------------------------------------

inline const std::string & ManufactureSchematicObject::getAppearanceData() const
{
	return m_appearanceData.get();
}


//========================================================================


#endif	// INCLUDED_ManufactureSchematicObject_H
