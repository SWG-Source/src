// ======================================================================
//
// ManufactureSchematicSynchronizedUi.h
// Copyright 2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef	INCLUDED_ManufactureSchematicSynchronizedUi_H
#define	INCLUDED_ManufactureSchematicSynchronizedUi_H

#include "Archive/AutoDeltaVector.h"
#include "serverGame/ServerSynchronizedUi.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "StringId.h"

class ManufactureSchematicObject;

namespace Crafting
{
	class SimpleIngredient;
};

//----------------------------------------------------------------------


class ManufactureSchematicSynchronizedUi : public ServerSynchronizedUi
{
public:
	explicit ManufactureSchematicSynchronizedUi(ManufactureSchematicObject & owner);
	virtual ~ManufactureSchematicSynchronizedUi();

	void        setReady();

	void        createNewSlot(const StringId & name, float complexity);
	void        setSlotType(const StringId & name, int type);
	void        setSlotOption(const StringId & name, int option);
	void        setSlotIndex(const StringId & name, int index);
	void        setSlotComplexity(const StringId & name, float complexity);
	void        addSlotIngredient(const StringId & name, const NetworkId & ingredient, int count);
	void        setSlotIngredientCount(const StringId & name, const NetworkId & ingredientId, int amount);

	void        clearSlot(const StringId & name, float complexity);
	void        removeSlot(const StringId & name);

	const std::vector<StringId> & getAttributeNames(void) const;
	float                         getAttribute(const StringId & name) const;
	void                          setAttribute(const StringId & name, float value);
	float                         getMinAttribute(const StringId & name) const;
	void                          setMinAttribute(const StringId & name, float value);
	float                         getMaxAttribute(const StringId & name) const;
	void                          setMaxAttribute(const StringId & name, float value);
	float                         getResourceMaxAttribute(const StringId & name) const;
	void                          setResourceMaxAttribute(const StringId & name, float value);
	void                          getAllAttributeValues(const StringId & name, float & value, float & minValue, float & maxValue, float & resourceMaxValue) const;
	void                          setAttributeLimits(const StringId & name, float minValue, float maxValue, float resourceMaxValue);

	void                          setExperimentMod(float mod);

	const std::vector<std::string> & getAppearances(void) const;
	void                             addAppearance(const std::string & appearance);

	void                setCustomization(const std::string & name, int value, int minValue, int maxValue);
	void                setCustomization(const std::string & name, int value);
	const std::string & getCustomizationName(int index);
	int                 getMinCustomization(const std::string & name);
	int                 getMaxCustomization(const std::string & name);
	int                 getCurrentCustomization(const std::string & name);

private:

	ManufactureSchematicSynchronizedUi();
	ManufactureSchematicSynchronizedUi(const ManufactureSchematicSynchronizedUi& );
	ManufactureSchematicSynchronizedUi& operator= (const ManufactureSchematicSynchronizedUi &);

	void        signalIngredientsChanged ();
	void        signalCustomizationChanged ();
	void        addAttribute(const StringId & name);

private:
	typedef std::vector<NetworkId> NetworkIdList;
	typedef std::vector<int>       IntList;

	// The following fields are parallel arrays for ingredients
	Archive::AutoDeltaVector<StringId>          m_slotName;            // slot name @todo: can we make this a string?
	Archive::AutoDeltaVector<int>               m_slotType;            // enum Crafting::IngredientType
	Archive::AutoDeltaVector<NetworkIdList>     m_slotIngredient;      // actual ingredients used per slot
	Archive::AutoDeltaVector<IntList>           m_slotIngredientCount; // number of ingredients used per slot
	Archive::AutoDeltaVector<float>             m_slotComplexity;      // cuurent slot complexity
	Archive::AutoDeltaVector<int>               m_slotDraftOption;     // the draft schematic slot option
	Archive::AutoDeltaVector<int>               m_slotDraftIndex;      // the draft schematic slot index
	Archive::AutoDeltaVariable<uint8>           m_slotIngredientsChanged;  // used to signal changes

	// The following fields are parallel arrays for experimentation
	Archive::AutoDeltaVector<StringId>      m_attributeName;        // attributes that can be experimented with
	Archive::AutoDeltaVector<float>         m_attributeValue;       // current value of the attributes
	Archive::AutoDeltaVector<float>         m_minAttribute;         // min value of the attribute
	Archive::AutoDeltaVector<float>         m_maxAttribute;         // max value of the attribute
	Archive::AutoDeltaVector<float>         m_resourceMaxAttribute; // max value of the attribute due to resource selection

	// The following fields are parallel arrays for customizations
	Archive::AutoDeltaVector<std::string>   m_customName;       // customization property name
	Archive::AutoDeltaVector<int>           m_customIndex;      // palette/decal current index
	Archive::AutoDeltaVector<int>           m_customMinIndex;   // palette/decal minimum index
	Archive::AutoDeltaVector<int>           m_customMaxIndex;   // palette/decal maximum index
	Archive::AutoDeltaVariable<uint8>       m_customChanged;    // used to signal changes

	Archive::AutoDeltaVariable<float>     m_experimentMod;      // appoximent mod that will be applied to experimentation tries
	Archive::AutoDeltaVector<std::string> m_appearance;         // list of appearances the player may choose from

	Archive::AutoDeltaVariable<bool>        m_ready;            // flag that the data is ready
};

//----------------------------------------------------------------------


inline void ManufactureSchematicSynchronizedUi::setReady(void)
{
	m_ready = true;
}

inline const std::vector<StringId> & ManufactureSchematicSynchronizedUi::getAttributeNames(void) const
{
	return m_attributeName.get();
}

inline void ManufactureSchematicSynchronizedUi::setExperimentMod(float mod)
{
	m_experimentMod = mod;
}

inline const std::vector<std::string> & ManufactureSchematicSynchronizedUi::getAppearances(void) const
{
	return m_appearance.get();
}

inline void ManufactureSchematicSynchronizedUi::addAppearance(const std::string & appearance)
{
	m_appearance.push_back(appearance);
}

//----------------------------------------------------------------------


#endif
