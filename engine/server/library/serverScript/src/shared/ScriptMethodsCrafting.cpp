//========================================================================
//
// ScriptMethodsCrafting.cpp - implements script methods dealing with crafting
// and manufacturing.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "UnicodeUtils.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/DraftSchematicObject.h"
#include "serverGame/FactoryObject.h"
#include "serverGame/ManufactureInstallationObject.h"
#include "serverGame/ManufactureObjectInterface.h"
#include "serverGame/ManufactureSchematicObject.h"
#include "serverGame/ObserveTracker.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/ResourceContainerObject.h"
#include "serverGame/ServerWorld.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedGame/AssetCustomizationManager.h"
#include "sharedLog/Log.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/MemoryBlockManagedObject.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplateList.h"
#include "sharedObject/PaletteColorCustomizationVariable.h"
#include "sharedObject/RangedIntCustomizationVariable.h"
#include "sharedObject/VolumeContainer.h"

#include <vector>

using namespace JNIWrappersNamespace;


// ======================================================================

namespace ScriptMethodsCraftingNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct VariableInfo
	{
		VariableInfo(std::string const &newName, int newMinValueInclusive, int newSetValue, int newMaxValueExclusive);

		std::string        name;
		int                minValueInclusive;
		int                value;
		int                maxValueExclusive;
	};

	typedef std::vector<VariableInfo>  VariableInfoVector;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	
	bool install();
	void collectRangedIntVariableCallback(const std::string &fullVariablePathName, const CustomizationVariable *customizationVariable, void *context);

	jboolean     JNICALL startCraftingSession(JNIEnv *env, jobject self, jlong player, jlong station);
	jboolean     JNICALL endCraftingSession(JNIEnv *env, jobject self, jlong crafter, jlong tool, jlong prototype);
	jboolean     JNICALL setCraftingLevelAndStation(JNIEnv *env, jobject self, jlong player, jint craftingLevel, jlong station);
	jint         JNICALL getCraftingLevel(JNIEnv *env, jobject self, jlong player);
	jlong        JNICALL getCraftingStation(JNIEnv *env, jobject self, jlong player);
	jboolean     JNICALL sendUseableDraftSchematics(JNIEnv *env, jobject self, jlong player, jintArray schematics);
	jboolean     JNICALL setSchematicAttribute(JNIEnv *env, jobject self, jlong manufacturingSchematic, jobject attribute, jboolean experiment);
	jboolean     JNICALL setSchematicAttributes(JNIEnv *env, jobject self, jlong manufacturingSchematic, jobjectArray attributes, jboolean experiment);
	jobject      JNICALL getSchematicAttribute(JNIEnv *env, jobject self, jlong manufacturingSchematic, jobject name, jboolean experiment);
	jobjectArray JNICALL getSchematicAttributes(JNIEnv *env, jobject self, jlong manufacturingSchematic, jobjectArray names, jboolean experiment);
	jobjectArray JNICALL getAllSchematicAttributes(JNIEnv *env, jobject self, jlong manufacturingSchematic, jboolean experiment);
	jobject      JNICALL getSchematicForExperimentalAttributes(JNIEnv *env, jobject self, jlong manufacturingSchematic, jobjectArray attributeNames);
	jboolean     JNICALL setSchematicExperimentMod(JNIEnv *env, jobject self, jlong manufacturingSchematic, jfloat experimentMod);
	jboolean     JNICALL setSchematicAppearances(JNIEnv *env, jobject self, jlong manufacturingSchematic, jobjectArray appearances);
	jboolean     JNICALL setSchematicCustomizations(JNIEnv *env, jobject self, jlong manufacturingSchematic, jobjectArray customizations);
	jint         JNICALL getSchematicItemCount(JNIEnv *env, jobject self, jlong manufacturingSchematic);
	jboolean     JNICALL setSchematicItemCount(JNIEnv *env, jobject self, jlong manufacturingSchematic, jint count);
	jint         JNICALL getSchematicItemsPerContainer(JNIEnv *env, jobject self, jlong manufacturingSchematic);
	jboolean     JNICALL setSchematicItemsPerContainer(JNIEnv *env, jobject self, jlong manufacturingSchematic, jint count);
	jfloat       JNICALL getSchematicManufactureTime(JNIEnv *env, jobject self, jlong manufacturingSchematic);
	jboolean     JNICALL setSchematicManufactureTime(JNIEnv *env, jobject self, jlong manufacturingSchematic, jfloat time);
	jboolean     JNICALL setCreatorXp(JNIEnv *env, jobject self, jlong object, jint xp);
	void         JNICALL getIngredientsForManufactureStation(JNIEnv *env, jobject self, jlong station, jobjectArray ingredients);
	jlong        JNICALL getManufactureStationInputHopper(JNIEnv *env, jobject self, jlong station);
	jlong        JNICALL getManufactureStationOutputHopper(JNIEnv *env, jobject self, jlong station);
	jstring      JNICALL getManufactureStationSchematic(JNIEnv *env, jobject self, jlong station);
	void         JNICALL getValidManufactureSchematicsForStation(JNIEnv *env, jobject self, jlong player, jlong station, jobjectArray schematics);
	jboolean     JNICALL hasValidManufactureSchematicsForStation(JNIEnv *env, jobject self, jlong player, jlong station);
	jboolean     JNICALL transferManufactureSchematicToPlayer(JNIEnv *env, jobject self, jlong station, jlong player);
	jboolean     JNICALL transferManufactureSchematicToStation(JNIEnv *env, jobject self, jlong schematic, jlong station);
	void         JNICALL getRepairableObjectsForTool(JNIEnv *env, jobject self, jlong player, jlong tool, jobjectArray objects);
	jintArray    JNICALL getAttributeBonuses(JNIEnv *env, jobject self, jlong target);
	jint         JNICALL getAttributeBonus(JNIEnv *env, jobject self, jlong target, jint attribute);
	jboolean     JNICALL setAttributeBonus(JNIEnv *env, jobject self, jlong target, jint attribute, jint bonus);
	jboolean     JNICALL setAttributeBonuses(JNIEnv *env, jobject self, jlong target, jintArray bonuses);
	jobject      JNICALL getSkillModBonuses(JNIEnv *env, jobject self, jlong target);
	jint         JNICALL getSkillModBonus(JNIEnv *env, jobject self, jlong target, jstring skillMod);
	jboolean     JNICALL setSkillModBonus(JNIEnv *env, jobject self, jlong target, jstring skillMod, jint bonus);
	jboolean     JNICALL setSkillModBonuses(JNIEnv *env, jobject self, jlong target, jobjectArray skillMod, jintArray bonus);
	jboolean     JNICALL setCategorizedSkillModBonus(JNIEnv *env, jobject self, jlong target, jstring category, jstring skillMod, jint bonus);
	void         JNICALL removeCategorizedSkillModBonuses(JNIEnv *env, jobject self, jlong target, jstring category);
	jint         JNICALL getSkillModSockets(JNIEnv *env, jobject self, jlong target);
	jboolean     JNICALL setSkillModSockets(JNIEnv *env, jobject self, jlong target, jint sockets);
	jlong        JNICALL makeCraftedItem(JNIEnv *env, jobject self, jstring draftSchematic, jfloat qualityPercent, jlong container);
	jobject      JNICALL getSchematicData(JNIEnv *env, jobject self, jlong manufacturingSchematic);
	jobject      JNICALL getDraftSchematicData(JNIEnv *env, jobject self, jstring draftSchematic);
	jobject      JNICALL getDraftSchematicDataCrc(JNIEnv *env, jobject self, jint draftSchematicCrc);
	void         JNICALL recomputeCrateAttributes(JNIEnv *env, jobject self, jlong crate);
}

using namespace ScriptMethodsCraftingNamespace;


//========================================================================
// install
//========================================================================

bool ScriptMethodsCraftingNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsCraftingNamespace::c)}
	JF("_startCraftingSession", "(JJ)Z", startCraftingSession),
	JF("_endCraftingSession", "(JJJ)Z", endCraftingSession),
	JF("_setCraftingLevelAndStation", "(JIJ)Z", setCraftingLevelAndStation),
	JF("_getCraftingLevel", "(J)I", getCraftingLevel),
	JF("_getCraftingStation", "(J)J", getCraftingStation),
	JF("_sendUseableDraftSchematics", "(J[I)Z", sendUseableDraftSchematics),
	JF("__setSchematicAttribute", "(JLscript/draft_schematic$attribute;Z)Z", setSchematicAttribute),
	JF("__setSchematicAttributes", "(J[Lscript/draft_schematic$attribute;Z)Z", setSchematicAttributes),
	JF("__getSchematicAttribute", "(JLscript/string_id;Z)Lscript/draft_schematic$attribute;", getSchematicAttribute),
	JF("__getSchematicAttributes", "(J[Lscript/string_id;Z)[Lscript/draft_schematic$attribute;", getSchematicAttributes),
	JF("__getAllSchematicAttributes", "(JZ)[Lscript/draft_schematic$attribute;", getAllSchematicAttributes),
	JF("_getSchematicForExperimentalAttributes", "(J[Lscript/string_id;)Lscript/draft_schematic;", getSchematicForExperimentalAttributes),
	JF("_setSchematicExperimentMod", "(JF)Z", setSchematicExperimentMod),
	JF("_setSchematicAppearances", "(J[Ljava/lang/String;)Z", setSchematicAppearances),
	JF("_setSchematicCustomizations", "(J[Lscript/draft_schematic$custom;)Z", setSchematicCustomizations),
	JF("_getSchematicItemCount", "(J)I", getSchematicItemCount),
	JF("_setSchematicItemCount", "(JI)Z", setSchematicItemCount),
	JF("_getSchematicItemsPerContainer", "(J)I", getSchematicItemsPerContainer),
	JF("_setSchematicItemsPerContainer", "(JI)Z", setSchematicItemsPerContainer),
	JF("_getSchematicManufactureTime", "(J)F", getSchematicManufactureTime),
	JF("_setSchematicManufactureTime", "(JF)Z", setSchematicManufactureTime),
	JF("_setCreatorXp", "(JI)Z", setCreatorXp),
	JF("_getIngredientsForManufactureStation", "(J[[Ljava/lang/String;)V", getIngredientsForManufactureStation),
	JF("_getManufactureStationInputHopper", "(J)J", getManufactureStationInputHopper),
	JF("_getManufactureStationOutputHopper", "(J)J", getManufactureStationOutputHopper),
	JF("_getManufactureStationSchematic", "(J)Ljava/lang/String;", getManufactureStationSchematic),
	JF("_getValidManufactureSchematicsForStation", "(JJ[[Ljava/lang/String;)V", getValidManufactureSchematicsForStation),
	JF("_hasValidManufactureSchematicsForStation", "(JJ)Z", hasValidManufactureSchematicsForStation),
	JF("_transferManufactureSchematicToPlayer", "(JJ)Z", transferManufactureSchematicToPlayer),
	JF("_transferManufactureSchematicToStation", "(JJ)Z", transferManufactureSchematicToStation),
	JF("_getRepairableObjectsForTool", "(JJ[[Ljava/lang/String;)V", getRepairableObjectsForTool),
	JF("_getAttributeBonuses", "(J)[I", getAttributeBonuses),
	JF("_getAttributeBonus", "(JI)I", getAttributeBonus),
	JF("_setAttributeBonus", "(JII)Z", setAttributeBonus),
	JF("_setAttributeBonuses", "(J[I)Z", setAttributeBonuses),
	JF("_getSkillModBonuses", "(J)Lscript/dictionary;", getSkillModBonuses),
	JF("_getSkillModBonus", "(JLjava/lang/String;)I", getSkillModBonus),
	JF("_setSkillModBonus", "(JLjava/lang/String;I)Z", setSkillModBonus),
	JF("_setSkillModBonuses", "(J[Ljava/lang/String;[I)Z", setSkillModBonuses),
	JF("_setCategorizedSkillModBonus", "(JLjava/lang/String;Ljava/lang/String;I)Z", setCategorizedSkillModBonus),
	JF("_removeCategorizedSkillModBonuses", "(JLjava/lang/String;)V", removeCategorizedSkillModBonuses),
	JF("_getSkillModSockets", "(J)I", getSkillModSockets),
	JF("_setSkillModSockets", "(JI)Z", setSkillModSockets),
	JF("_makeCraftedItem", "(Ljava/lang/String;FJ)J", makeCraftedItem),
	JF("_getSchematicData", "(J)Lscript/draft_schematic;", getSchematicData),
	JF("getSchematicData", "(Ljava/lang/String;)Lscript/draft_schematic;", getDraftSchematicData),
	JF("getSchematicData", "(I)Lscript/draft_schematic;", getDraftSchematicDataCrc),
	JF("_recomputeCrateAttributes", "(J)V", recomputeCrateAttributes),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


// ======================================================================
// namespace ScriptMethodsCraftingNamespace
// ======================================================================

ScriptMethodsCraftingNamespace::VariableInfo::VariableInfo(std::string const &newName, int newMinValueInclusive, int newSetValue, int newMaxValueExclusive):
	name(newName),
	minValueInclusive(newMinValueInclusive),
	value(newSetValue),
	maxValueExclusive(newMaxValueExclusive)
{
}

// ----------------------------------------------------------------------

void ScriptMethodsCraftingNamespace::collectRangedIntVariableCallback(const std::string &fullVariablePathName, const CustomizationVariable *customizationVariable, void *context)
{
	//-- Check if variable is a palette-color style variable.  If not, ignore for now.
	//   NOTE: replace PaletteColorCustomizationVariable with RangedIntCustomizatoinVariable
	//   if you want any ranged int (including texture choice ranges and palette colors) to be
	//   passed to crafting.
	PaletteColorCustomizationVariable const *const paletteColorVariable = dynamic_cast<PaletteColorCustomizationVariable const *>(customizationVariable);
	if (!paletteColorVariable)
	{
		// Nothing to do.
		return;
	}

	//-- Check if variable is a ranged int style variable.
	RangedIntCustomizationVariable const *const rangedIntVariable = paletteColorVariable;

	//-- Get variable info list from context.
	NOT_NULL(context);
	VariableInfoVector *const variableInfoVector = static_cast<VariableInfoVector*>(context);

	//-- Add variable info to vector.
	int minValueInclusive = 0;
	int maxValueExclusive = 0;

	rangedIntVariable->getRange(minValueInclusive, maxValueExclusive);
	variableInfoVector->push_back(VariableInfo(fullVariablePathName, minValueInclusive, rangedIntVariable->getValue(), maxValueExclusive));
}

//========================================================================
// class JavaLibrary crafting utility functions
//========================================================================

/**
 * Creates a Java draft_schematic.attribute instance from an object attribute.
 *
 * @param manfSchematic		manufacture schematic the attribute belongs to
 * @param draftSchematic	draft schematic the attribute belongs to
 * @param attribIndex		index of the attribute
 *
 * @return the attribute instance, or nullptr on error
 */
LocalRefPtr JavaLibrary::createObjectAttribute(const ManufactureObjectInterface & manfSchematic,
	const DraftSchematicObject & draftSchematic, int attribIndex)
{
	if (attribIndex < 0 || attribIndex >= draftSchematic.getAttribsCount())
		return LocalRef::cms_nullPtr;

	const SharedDraftSchematicObjectTemplate::SchematicAttribute &
		minAttrib = draftSchematic.getAttribMin(attribIndex);
	const SharedDraftSchematicObjectTemplate::SchematicAttribute &
		maxAttrib = draftSchematic.getAttribMax(attribIndex);

	if (minAttrib.name.getTable().empty() || minAttrib.name.getText().empty())
		return LocalRef::cms_nullPtr;

	// create the attribute object
	LocalRefPtr attribute = allocObject(JavaLibrary::getClsDraftSchematicAttrib());
	if (attribute == LocalRef::cms_nullPtr)
		return LocalRef::cms_nullPtr;

	// set the name
	LocalRefPtr name;
	if (!ScriptConversion::convert(minAttrib.name, name))
		return LocalRef::cms_nullPtr;

	setObjectField(*attribute, JavaLibrary::getFidDraftSchematicAttribName(), *name);

	// get the resourceMax and current values
	float minValue = static_cast<float>(minAttrib.value);
	float maxValue = static_cast<float>(maxAttrib.value);
	float resourceMaxValue = maxValue;
	float currentValue = minValue;

	std::map<StringId, float>::const_iterator result = manfSchematic.getAttributes().find(
		minAttrib.name);
	if (result != manfSchematic.getAttributes().end())
		currentValue = (*result).second;

	result = manfSchematic.getResourceMaxAttributes().find(
		minAttrib.name);
	if (result != manfSchematic.getResourceMaxAttributes().end())
		resourceMaxValue = (*result).second;

	// set the other fields
	setFloatField(*attribute, JavaLibrary::getFidDraftSchematicAttribMinValue(), minValue);
	setFloatField(*attribute, JavaLibrary::getFidDraftSchematicAttribMaxValue(), maxValue);
	setFloatField(*attribute, JavaLibrary::getFidDraftSchematicAttribResourceMaxValue(), resourceMaxValue);
	setFloatField(*attribute, JavaLibrary::getFidDraftSchematicAttribCurrentValue(), currentValue);

	return attribute;
}	// JavaLibrary::createObjectAttribute

/**
 * Creates a Java draft_schematic.attribute instance from an experimental attribute.
 *
 * @param manfSchematic		manufacture schematic the attribute belongs to
 * @param attribName		name of the attribute
 *
 * @return the attribute instance, or nullptr on error
 */
LocalRefPtr JavaLibrary::createExperimentAttribute(const ManufactureObjectInterface & manfSchematic,
	const StringId & attribName)
{
	if (attribName.getTable().empty() || attribName.getText().empty())
		return LocalRef::cms_nullPtr;

	// create the attribute object
	LocalRefPtr attribute = allocObject(JavaLibrary::getClsDraftSchematicAttrib());
	if (attribute == LocalRef::cms_nullPtr)
		return LocalRef::cms_nullPtr;

	// set the name
	LocalRefPtr name;
	if (!ScriptConversion::convert(attribName, name))
		return LocalRef::cms_nullPtr;

	setObjectField(*attribute, JavaLibrary::getFidDraftSchematicAttribName(), *name);

	// set the other fields
	float value, minValue, maxValue, resourceValue;
	manfSchematic.getAllExperimentAttributeValues(attribName, value, minValue, maxValue,
		resourceValue);
	setFloatField(*attribute, JavaLibrary::getFidDraftSchematicAttribCurrentValue(), value);
	setFloatField(*attribute, JavaLibrary::getFidDraftSchematicAttribMinValue(), minValue);
	setFloatField(*attribute, JavaLibrary::getFidDraftSchematicAttribMaxValue(), maxValue);
	setFloatField(*attribute, JavaLibrary::getFidDraftSchematicAttribResourceMaxValue(), resourceValue);

	return attribute;
}	// JavaLibrary::createExperimentAttribute


//========================================================================
// class JavaLibrary JNI conversion methods
//========================================================================

/**
 * Converts a C++ ManufactureSchematicObject to a Java draft_schematic.
 *
 * @param source		the ManufactureSchematicObject to convert
 *
 * @return a new draft_schematic object
 */
LocalRefPtr JavaLibrary::convert(const ManufactureObjectInterface & source)
{
int i;

	const DraftSchematicObject * draft = DraftSchematicObject::getSchematic(
		source.getDraftSchematic());
	if (draft == nullptr)
		return LocalRef::cms_nullPtr;

	LocalRefPtr target = allocObject(ms_clsDraftSchematic);
	if (target == LocalRef::cms_nullPtr)
		return LocalRef::cms_nullPtr;

	// set the ingredients
	int numSlots = source.getSlotsCount();
	LocalObjectArrayRefPtr slots = createNewObjectArray(numSlots, ms_clsDraftSchematicSlot);
	if (slots == LocalObjectArrayRef::cms_nullPtr)
		return LocalRef::cms_nullPtr;

	Crafting::IngredientSlot sourceSlot;
	ServerDraftSchematicObjectTemplate::IngredientSlot draftSlot;
	for (i = 0; i < numSlots; ++i)
	{
		if (!source.getSlot(i, sourceSlot, true))
		{
			WARNING(true, ("JavaLibrary::convert manf data: unable to get slot %d "
				"from manf schematic %s", i, source.getDerivedNetworkId().getValueString().c_str()));
			break;
		}

		// get the amount of ingredient required from the draft schematic
		if (!draft->getSlot(draftSlot, sourceSlot.name))
		{
			WARNING(true, ("JavaLibrary::convert manf data: unable to get slot %s "
				"from draft schematic %s", sourceSlot.name.getCanonicalRepresentation().c_str(),
				draft->getObjectTemplateName()));
			break;
		}
		int amountRequired = draftSlot.options[sourceSlot.draftSlotOption].
			ingredients[0].count;

		// create the ingredient object
		LocalRefPtr slot = convert(source, sourceSlot, amountRequired,
			draftSlot.appearance, draftSlot.options[0].ingredients[0].ingredient);
		if (slot == LocalRef::cms_nullPtr)
		{
			WARNING(true, ("Unable to convert schematic slot %s to Java data",
				sourceSlot.name.getCanonicalRepresentation().c_str()));
			break;
		}

		setObjectArrayElement(*slots, i, *slot);
	}
	if (i != numSlots)
	{
		return LocalRef::cms_nullPtr;
	}
	setObjectField(*target, ms_fidDraftSchematicSlots, *slots);

	// if we are converting the schematic in order to manufacture an object,
	// just convert the object attributes and ingredient sources and exit
	if (source.isMakingObject())
	{
		int numObjectAttribs = draft->getAttribsCount();
		LocalObjectArrayRefPtr attributes = createNewObjectArray(numObjectAttribs, JavaLibrary::getClsDraftSchematicAttrib());
		if (attributes == LocalObjectArrayRef::cms_nullPtr)
		{
			return LocalRef::cms_nullPtr;
		}
		for (i = 0; i < numObjectAttribs; ++i)
		{
			// create the attribute object
			LocalRefPtr attribute = JavaLibrary::createObjectAttribute(source, *draft, i);
			if (attribute == LocalRef::cms_nullPtr)
				break;
			setObjectArrayElement(*attributes, i, *attribute);
		}
		if (i != numObjectAttribs)
		{
			return LocalRef::cms_nullPtr;
		}
		setObjectField(*target, ms_fidDraftSchematicAttribs, *attributes);
		return target;
	}

	// set the crafting category
	setIntField(*target, ms_fidDraftSchematicCategory, draft->getCategory());
	// set the base complexity
	setFloatField(*target, ms_fidDraftSchematicComplexity, draft->getComplexity());

	//
	// set the attributes, the experimental attributes, and the mapping between them
	//

	LocalRefPtr attribMap = createNewObject(ms_clsHashtable, ms_midHashtable);
	if (attribMap == LocalRef::cms_nullPtr)
		return LocalRef::cms_nullPtr;

	// create a local frame reference big enough so we can keep around the
	// references to the object attributes and experiment attributes
	const std::vector<StringId> & experimentNames = source.getExperimentAttributeNames();
	int numExperimentAttribs = experimentNames.size();
	int numObjectAttribs = draft->getAttribsCount();

	// set the experimental attributes
	std::vector<LocalRefPtr> experimentAttribRefs;
	LocalObjectArrayRefPtr attributes = createNewObjectArray(numExperimentAttribs, JavaLibrary::getClsDraftSchematicAttrib());
	if (attributes == LocalObjectArrayRef::cms_nullPtr)
	{
		return LocalRef::cms_nullPtr;
	}
	for (i = 0; i < numExperimentAttribs; ++i)
	{
		// create the attribute object
		LocalRefPtr attribute = JavaLibrary::createExperimentAttribute(source, experimentNames[i]);
		if (attribute == LocalRef::cms_nullPtr)
			break;

		setObjectArrayElement(*attributes, i, *attribute);
		experimentAttribRefs.push_back(attribute);
	}
	if (i != numExperimentAttribs)
	{
		return LocalRef::cms_nullPtr;
	}
	setObjectField(*target, ms_fidDraftSchematicExperimentalAttribs, *attributes);

	// set the object attributes
	attributes = createNewObjectArray(numObjectAttribs, JavaLibrary::getClsDraftSchematicAttrib());
	if (attributes == LocalObjectArrayRef::cms_nullPtr)
	{
		return LocalRef::cms_nullPtr;
	}
	for (i = 0; i < numObjectAttribs; ++i)
	{
		// create the attribute object
		LocalRefPtr attribute = JavaLibrary::createObjectAttribute(source, *draft, i);
		if (attribute == LocalRef::cms_nullPtr)
			break;
		setObjectArrayElement(*attributes, i, *attribute);

		// find the experimental attribute associated with the object attibute
		// and add it to the attribute map
		StringId expAttrib(draft->getExperimentalAttributeFromAttribute(i));
		for (int j = 0; j < numExperimentAttribs; ++j)
		{
			if (expAttrib == experimentNames[j])
			{
				callObjectMethod(*attribMap, ms_midMapPut,
					attribute->getValue(), experimentAttribRefs[j]->getValue());
				break;
			}
		}
	}
	if (i != numObjectAttribs)
	{
		return LocalRef::cms_nullPtr;
	}
	setObjectField(*target, JavaLibrary::getFidDraftSchematicAttribs(), *attributes);
	setObjectField(*target, JavaLibrary::getFidDraftSchematicAttribMap(), *attribMap);

	// restore our frame reference
	experimentAttribRefs.clear();

	// set the customization info from the shared object template
	const ServerObjectTemplate * objectTemplate = draft->getCraftedObjectTemplate();
	if (objectTemplate == nullptr)
	{
		return LocalRef::cms_nullPtr;
	}
	const SharedObjectTemplate * sharedTemplate = safe_cast<const SharedObjectTemplate *>(
		ObjectTemplateList::fetch(objectTemplate->getSharedTemplate()));
	const SharedTangibleObjectTemplate * sharedTangibleTemplate = dynamic_cast<
		const SharedTangibleObjectTemplate *>(sharedTemplate);
	if (sharedTangibleTemplate != nullptr)
	{
		// New method using the AssetCustomizationManager mechanism.

		//-- Retrieve customization declarations for the appearance asset associated with
		//   the crafted object's shared object template.

		// Create a dummy object.
		Object *dummyObject = new MemoryBlockManagedObject();

		// Create a CustomizationData instance.
		CustomizationData *customizationData = new CustomizationData(*dummyObject);
		customizationData->fetch();

		// Retrieve CustomizationData for crafted object's appearance template name.
		bool const skipSharedOwnerVariables = true;
		AssetCustomizationManager::addCustomizationVariablesForAsset(TemporaryCrcString(sharedTangibleTemplate->getAppearanceFilename().c_str(), true), *customizationData, skipSharedOwnerVariables);

		// Retrieve all ranged int customization variables from CusotmizationData.
		VariableInfoVector  variableInfoList;
		bool const includeRemoveVariables = false;
		customizationData->iterateOverConstVariables(collectRangedIntVariableCallback, &variableInfoList, includeRemoveVariables);

		int const count = static_cast<int>(variableInfoList.size());
#ifdef _DEBUG
		LOG("crafting-customizations", ("customization variable count: %d for (%s)", count, sharedTangibleTemplate->getAppearanceFilename().c_str()));
#endif

		//-- Create the Java array to hold the ranged int customization entries.
		LocalObjectArrayRefPtr customizations = createNewObjectArray(count, ms_clsDraftSchematicCustom);
		if (customizations == LocalObjectArrayRef::cms_nullPtr)
		{
			customizationData->release();
			delete dummyObject;
			return LocalRef::cms_nullPtr;
		}

		//-- Fill in the Java array with customization entries.
		int i;

		for (i = 0; i < count; ++i)
		{
			VariableInfo const &info = variableInfoList[static_cast<VariableInfoVector::size_type>(i)];

			// create the customization object
			LocalRefPtr customization = allocObject(ms_clsDraftSchematicCustom);
			if (customization == LocalRef::cms_nullPtr)
				break;

			// set the name
			JavaString name(info.name.c_str());
			setObjectField(*customization, JavaLibrary::getFidDraftSchematicCustomName(), name);

			// set the values
			setIntField(*customization, JavaLibrary::getFidDraftSchematicCustomValue(), info.value);
			setIntField(*customization, JavaLibrary::getFidDraftSchematicCustomMinValue(), info.minValueInclusive);
			setIntField(*customization, JavaLibrary::getFidDraftSchematicCustomMaxValue(), info.maxValueExclusive - 1);
//			setBooleanField(*customization, ms_fidDraftSchematicCustomLocked,
//				JNI_FALSE);

			setObjectArrayElement(*customizations, i, *customization);
		}
		if (i != count)
		{
			customizationData->release();
			delete dummyObject;
			return LocalRef::cms_nullPtr;
		}
		setObjectField(*target, ms_fidDraftSchematicCustomizations, *customizations);

		//-- Cleanup.
		customizationData->release();
		delete dummyObject;
	}
	else
		setObjectField(*target, ms_fidDraftSchematicCustomizations, LocalRef(0));

	return target;
}	// JavaLibrary::convert(const ManufactureObjectInterface &)

/**
 * Converts a C++ Crafting::IngredientSlot to a Java draft_schematic$slot.
 *
 * @param schematic				the manf schematic the slot belongs to
 * @param source				the IngredientSlot to convert
 * @param amountRequired		amount of ingredients required to fill the slot
 * @param appearance			appearance data associated with the slot
 * @param requiredIngredient	name of the ingredient needed for the slot
 *
 * @return a new draft_schematic object
 */
LocalRefPtr JavaLibrary::convert(const ManufactureObjectInterface & schematic,
	const Crafting::IngredientSlot & source, int amountRequired,
	const std::string & appearance, const std::string & requiredIngredient)
{
	int i;

	// create the slot object
	LocalRefPtr slot = allocObject(ms_clsDraftSchematicSlot);
	if (slot == LocalRef::cms_nullPtr)
		return LocalRef::cms_nullPtr;

	// set the slot name
	LocalRefPtr slotName;
	if (!ScriptConversion::convert(source.name, slotName))
	{
		return LocalRef::cms_nullPtr;
	}
	setObjectField(*slot, ms_fidDraftSchematicSlotName, *slotName);

	// set the slot appearance data
	JavaString appearanceName(appearance.c_str());
	setObjectField(*slot, ms_fidDraftSchematicSlotAppearance, appearanceName);

	JavaString ingredientName(requiredIngredient);
	if (ingredientName.getValue() == 0)
		return LocalRef::cms_nullPtr;

	// set the slot data
	setIntField(*slot, ms_fidDraftSchematicSlotOption, source.draftSlotOption);
	setIntField(*slot, ms_fidDraftSchematicSlotIngredientType, source.ingredientType);
	setFloatField(*slot, ms_fidDraftSchematicSlotComplexity, source.complexity);
	setIntField(*slot, ms_fidDraftSchematicSlotAmountRequired, amountRequired);
	setObjectField(*slot, ms_fidDraftSchematicSlotIngredientName, ingredientName);

	// create the ingredients array
	int count = source.ingredients.size();
	LocalObjectArrayRefPtr ingredients = createNewObjectArray(count, ms_clsDraftSchematicSimpleIngredient);
	if (ingredients == LocalObjectArrayRef::cms_nullPtr)
	{
		return LocalRef::cms_nullPtr;
	}
	for (i = 0; i < count; ++i)
	{
		// create the ingredient object
		LocalRefPtr ingredient = allocObject(ms_clsDraftSchematicSimpleIngredient);
		if (ingredient == LocalRef::cms_nullPtr)
		{
			WARNING(true, ("Unable to allocate Java memory for simple ingredient data"));
			break;
		}

		// set the ingredient source and xp type
		LocalRefPtr sourceId = getObjId(source.ingredients[i]->source);
		setObjectField(*ingredient, ms_fidDraftSchematicSimpleIngredientSource,
			*sourceId);
		setIntField(*ingredient, ms_fidDraftSchematicSimpleIngredientXpType,
			source.ingredients[i]->xpType);


		// set the ingredient id
		if (source.ingredientType == Crafting::IT_resourceType ||
			source.ingredientType == Crafting::IT_resourceClass)
		{
			LocalRefPtr ingredientId = getObjId(source.ingredients[i]->ingredient);
			if (ingredientId == LocalRef::cms_nullPtr)
			{
				WARNING(true, ("Unable to convert ingredient resource network id "
					"%s to a Java obj_id", source.ingredients[i]->ingredient.getValueString().c_str()));
				break;
			}
			setObjectField(*ingredient, ms_fidDraftSchematicSimpleIngredientIngredient,
				*ingredientId);
		}
		else
		{
			const Crafting::ComponentIngredient * componentData = safe_cast<
				const Crafting::ComponentIngredient *>(source.ingredients[i].get());
//			TangibleObject * component = schematic.getComponent(*componentData);
//			NOT_NULL(component);
//			LocalRef ingredientId(getObjId(component->getNetworkId()));
			LocalRefPtr componentId = getObjId(componentData->ingredient);
			if (componentId == LocalRef::cms_nullPtr)
			{
				WARNING(true, ("Unable to convert ingredient component network id "
					"%s to a Java obj_id", componentData->ingredient.getValueString().c_str()));
				break;
			}
			setObjectField(*ingredient, ms_fidDraftSchematicSimpleIngredientIngredient,
				*componentId);
			setIntField(*ingredient, ms_fidDraftSchematicSimpleIngredientXpAmount,
				componentData->sourceXp);
		}

		// set the ingredient count
		setIntField(*ingredient, ms_fidDraftSchematicSimpleIngredientCount,
			source.ingredients[i]->count);

		setObjectArrayElement(*ingredients, i, *ingredient);
	}
	if (i != count)
	{
		return LocalRef::cms_nullPtr;
	}
	setObjectField(*slot, ms_fidDraftSchematicSlotIngredients, *ingredients);

	return slot;
}	// JavaLibrary::convert(const Crafting::IngredientSlot &)

/**
 * Converts a C++ DraftSchematicObject to a Java draft_schematic.
 *
 * @param source		the DraftSchematicObject to convert
 *
 * @return a new draft_schematic object
 */
jobject JavaLibrary::convert(const DraftSchematicObject & source)
{
int i;

	LocalRefPtr target = allocObject(ms_clsDraftSchematic);
	if (target == LocalRef::cms_nullPtr)
		return 0;

	// set the ingredients/slots
	int numSlots = source.getSlotsCount();
	LocalObjectArrayRefPtr slots = createNewObjectArray(numSlots, ms_clsDraftSchematicSlot);
	if (slots == LocalObjectArrayRef::cms_nullPtr)
		return 0;

	ServerDraftSchematicObjectTemplate::IngredientSlot slot;
	for (i = 0; i < numSlots; ++i)
	{
		if (!source.getSlot(slot, i))
		{
			WARNING(true, ("JavaLibrary::convert draft data: unable to get slot %d "
				"from draft schematic %s", i, source.getObjectTemplateName()));
			break;
		}

		if (slot.options.empty())
		{
			WARNING(true, ("JavaLibrary::convert draft data: slot %d in draft "
				"schematic %s has no options", i, source.getObjectTemplateName()));
			break;
		}
		if (slot.options[0].ingredients.empty())
		{
			WARNING(true, ("JavaLibrary::convert draft data: slot %d in draft "
				"schematic %s has no ingredients", i, source.getObjectTemplateName()));
			break;
		}

		JavaString appearanceName(slot.appearance);
		if (appearanceName.getValue() == 0)
			break;
		JavaString ingredientName(slot.options[0].ingredients[0].ingredient);
		if (ingredientName.getValue() == 0)
			break;
		LocalRefPtr slotName;
		if (!ScriptConversion::convert(slot.name, slotName))
			break;

		LocalRefPtr jslot = allocObject(ms_clsDraftSchematicSlot);
		if (jslot == LocalRef::cms_nullPtr)
			break;

		setObjectField(*jslot, ms_fidDraftSchematicSlotName, *slotName);
		setObjectField(*jslot, ms_fidDraftSchematicSlotIngredientName, ingredientName);
		setObjectField(*jslot, ms_fidDraftSchematicSlotIngredients, LocalRef(0));
		setObjectField(*jslot, ms_fidDraftSchematicSlotAppearance, appearanceName);
		setIntField(*jslot, ms_fidDraftSchematicSlotOption, 0);
		setIntField(*jslot, ms_fidDraftSchematicSlotIngredientType, slot.options[0].ingredientType);
		setIntField(*jslot, ms_fidDraftSchematicSlotAmountRequired, slot.options[0].ingredients[0].count);
		setFloatField(*jslot, ms_fidDraftSchematicSlotComplexity, slot.complexity);
		setObjectArrayElement(*slots, i, *jslot);
	}
	if (i != numSlots)
	{
		return 0;
	}
	setObjectField(*target, ms_fidDraftSchematicSlots, *slots);

	// set the attributes
	int numAttributes = source.getAttribsCount();
	LocalObjectArrayRefPtr attribs = createNewObjectArray(numAttributes, JavaLibrary::getClsDraftSchematicAttrib());
	if (attribs == LocalObjectArrayRef::cms_nullPtr)
		return 0;
	int minValue = 0;
	int maxValue = 0;
	for (i = 0; i < numAttributes; ++i)
	{
		const SharedDraftSchematicObjectTemplate::SchematicAttribute & attrib =
			source.getAttrib(i);
		source.getAttribMinMax(i, minValue, maxValue);

		LocalRefPtr attribName;
		if (!ScriptConversion::convert(attrib.name, attribName))
			break;

		LocalRefPtr jattrib = allocObject(JavaLibrary::getClsDraftSchematicAttrib());
		if (jattrib == LocalRef::cms_nullPtr)
			break;

		setObjectField(*jattrib, JavaLibrary::getFidDraftSchematicAttribName(), *attribName);
		setFloatField(*jattrib, JavaLibrary::getFidDraftSchematicAttribMinValue(), static_cast<float>(minValue));
		setFloatField(*jattrib, JavaLibrary::getFidDraftSchematicAttribMaxValue(), static_cast<float>(maxValue));
		setFloatField(*jattrib, JavaLibrary::getFidDraftSchematicAttribResourceMaxValue(), 0);
		setFloatField(*jattrib, JavaLibrary::getFidDraftSchematicAttribCurrentValue(), 0);
		setObjectArrayElement(*attribs, i, *jattrib);
	}
	if (i != numAttributes)
	{
		return 0;
	}
	setObjectField(*target, JavaLibrary::getFidDraftSchematicAttribs(), *attribs);

	// set the created item template crc value
	const ServerObjectTemplate * craftedTemplate = source.getCraftedObjectTemplate();
	if (craftedTemplate == nullptr)
	{
		WARNING(true, ("JavaLibrary::convert DraftSchematicObject got nullptr crafted "
			"template for draft schematic %s", source.getObjectTemplateName()));
		return 0;
	}
	setIntField(*target, ms_fidDraftSchematicObjectTemplateCreated, craftedTemplate->getCrcName().getCrc());

	// set the script value
	const ServerDraftSchematicObjectTemplate * const sourceSchematicTemplate =
		safe_cast<const ServerDraftSchematicObjectTemplate *>(source.getObjectTemplate());
	NOT_NULL(sourceSchematicTemplate);
	if (sourceSchematicTemplate == nullptr)
	{
		// emergency case for release mode
		return 0;
	}
	const int numScripts = sourceSchematicTemplate->getManufactureScriptsCount();
	LocalObjectArrayRefPtr scripts = createNewObjectArray(numScripts, JavaLibrary::getClsString());
	if (scripts == LocalObjectArrayRef::cms_nullPtr)
	{
		WARNING(true, ("JavaLibrary::convert DraftSchematicObject could not allocate "
			"script array for draft schematic %s", source.getObjectTemplateName()));
		return 0;
	}
	for (i = 0; i < numScripts; ++i)
	{
		JavaString script(sourceSchematicTemplate->getManufactureScripts(i));
		setObjectArrayElement(*scripts, i, script);
	}
	setObjectField(*target, ms_fidDraftSchematicScripts, *scripts);

	return target->getReturnValue();
}	// JavaLibrary::convert(const DraftSchematicObject &)


//========================================================================
// class JavaLibrary JNI crafting callback methods
//========================================================================

/**
 * Starts a crafting session.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param player		the player who will be crafting
 * @param station		the design station a player will be using
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsCraftingNamespace::startCraftingSession(JNIEnv *env, jobject self, jlong player, jlong station)
{
	UNREF(env);
	UNREF(self);

	CreatureObject* playerObj = nullptr;
	if (!JavaLibrary::getObject(player, playerObj))
		return JNI_FALSE;

	TangibleObject* stationObj = nullptr;
	if (!JavaLibrary::getObject(station, stationObj))
		return JNI_FALSE;

	if (!stationObj->startCraftingSession(*playerObj))
		return JNI_FALSE;

	return JNI_TRUE;
}	// JavaLibrary::startCraftingSession

/**
 * Ends a crafting session.
 *
 * @param env			Java environment
 * @param self			class calling this function
 * @param crafter		the player that was crafting
 * @param tool			the crafting tool that was being used
 * @param prototype		the prototype that was created (may be nullptr)
 *
 * @return true on success, false on fail
 */
jboolean JNICALL ScriptMethodsCraftingNamespace::endCraftingSession(JNIEnv *env, jobject self, jlong crafter, jlong tool, jlong prototype)
{
	UNREF(env);
	UNREF(self);
	UNREF(tool);

	// we're only do things if there's a prototype
	if (prototype == 0)
		return JNI_TRUE;

	CreatureObject * crafterObj = nullptr;
	if (!JavaLibrary::getObject(crafter, crafterObj))
		return JNI_FALSE;

	TangibleObject * prototypeObj = nullptr;
	if (!JavaLibrary::getObject(prototype, prototypeObj))
		return JNI_FALSE;

	// tell the crafter to create the prototype
	ObserveTracker::onCraftingEndCraftingSession(*crafterObj, *prototypeObj);

	return JNI_TRUE;
}	// JavaLibrary::endCraftingSession

/**
 * Sets the crafting level and station of a player's crafting session.
 *
 * @param env				Java environment
 * @param self				class calling this function
 * @param player			the player who's crafting
 * @param craftingLevel		the crafting level of the session
 * @param station			the crafting station of the session
 *
 * @return true if the data was set, false on error
 */
jboolean JNICALL ScriptMethodsCraftingNamespace::setCraftingLevelAndStation(JNIEnv *env, jobject self, jlong player, jint craftingLevel, jlong station)
{
	UNREF(self);

	CreatureObject* creatureObject = nullptr;
	if (!JavaLibrary::getObject(player, creatureObject))
		return JNI_FALSE;
	PlayerObject * playerObject = PlayerCreatureController::getPlayerObject(creatureObject);
	if (playerObject == nullptr)
		return JNI_FALSE;

	if (craftingLevel >= 0 && craftingLevel <= Crafting::MAX_CRAFTING_LEVEL)
		playerObject->setCraftingLevel(craftingLevel);

	if (station != 0)
	{
		const TangibleObject * stationObject = nullptr;
		const NetworkId stationId(station);
		if (stationId != NetworkId::cms_invalid)
		{
			stationObject = dynamic_cast<const TangibleObject *>(ServerWorld::findObjectByNetworkId(stationId));
			if (stationObject == nullptr)
				return JNI_FALSE;
		}
		playerObject->setCraftingStation(stationObject);
	}

	return JNI_TRUE;
}	// JavaLibrary::setCraftingLevelAndStation

/**
 * Gets the crafting level of a player's crafting session.
 *
 * @param env				Java environment
 * @param self				class calling this function
 * @param player			the player who's crafting
 *
 * @return the crafting level, or -1 on error
 */
jint JNICALL ScriptMethodsCraftingNamespace::getCraftingLevel(JNIEnv *env, jobject self, jlong player)
{
	UNREF(self);

	const CreatureObject* creatureObject = nullptr;
	if (!JavaLibrary::getObject(player, creatureObject))
		return -1;
	const PlayerObject * playerObject = PlayerCreatureController::getPlayerObject(creatureObject);
	if (playerObject == nullptr)
		return -1;

	return playerObject->getCraftingLevel();
}	// JavaLibrary::getCraftingLevel

/**
 * Gets the crafting station of a player's crafting session.
 *
 * @param env				Java environment
 * @param self				class calling this function
 * @param player			the player who's crafting
 *
 * @return the crafting station, or 0 on error
 */
jlong JNICALL ScriptMethodsCraftingNamespace::getCraftingStation(JNIEnv *env, jobject self, jlong player)
{
	UNREF(self);

	const CreatureObject* creatureObject = nullptr;
	if (!JavaLibrary::getObject(player, creatureObject))
		return 0;
	const PlayerObject * playerObject = PlayerCreatureController::getPlayerObject(creatureObject);
	if (playerObject == nullptr)
		return 0;

	return (playerObject->getCraftingStation()).getValue();
}	// JavaLibrary::getCraftingStation

/**
 * Sends a list of draft schematics to a player that he may select to craft an item.
 *
 * @param env		    Java environment
 * @param self		    class calling this function
 * @param player		the player trying to use the station
 * @param schematics	list of schematic template name crcs the player may use to craft an item
 *
 * @return true on success, false if there was an error
 **/
jboolean JNICALL ScriptMethodsCraftingNamespace::sendUseableDraftSchematics(JNIEnv *env, jobject self, jlong player, jintArray schematics)
{
	UNREF(self);

	CreatureObject* creatureObject = nullptr;
	if (!JavaLibrary::getObject(player, creatureObject))
		return JNI_FALSE;
	PlayerObject * playerObject = PlayerCreatureController::getPlayerObject(creatureObject);
	if (playerObject == nullptr)
	{
		DEBUG_WARNING(true, ("JavaLibrary::sendUseableDraftSchematics non-player "
			"object %s\n", creatureObject->getNetworkId().getValueString().c_str()));
		return JNI_FALSE;
	}

	int count = env->GetArrayLength(schematics);
	if (count == 0)
		return JNI_FALSE;

	jint * schematicsArray = env->GetIntArrayElements(schematics, nullptr);
	if (schematicsArray != nullptr)
	{
		std::vector<uint32> schematicCrcs(schematicsArray, &schematicsArray[count]);
		playerObject->sendUseableDraftSchematics(schematicCrcs);
		env->ReleaseIntArrayElements(schematics, schematicsArray, JNI_ABORT);
	}
	else
	{
		std::vector<uint32> schematicCrcs;
		playerObject->sendUseableDraftSchematics(schematicCrcs);
	}

	return JNI_TRUE;
}	// JavaLibrary::sendUseableDraftSchematics

/**
 * Sets an attribute for a manufacturing schematic.
 *
 * @param env							Java environment
 * @param self							class calling this function
 * @param manufacturingSchematic		the schematic
 * @param attribute						the attribute info to set
 * @param experiment					flag that these are experimental attributes
 *
 * @return true on success, false if there was an error
 */
jboolean JNICALL ScriptMethodsCraftingNamespace::setSchematicAttribute(JNIEnv *env, jobject self, jlong manufacturingSchematic, jobject attribute, jboolean experiment)
{
	UNREF(self);

	if (manufacturingSchematic == 0 || attribute == 0)
		return JNI_FALSE;

	ManufactureObjectInterface * schematic = nullptr;
	if (!JavaLibrary::getObject(manufacturingSchematic, schematic))
		return JNI_FALSE;

	LocalRefPtr attrName = getObjectField(LocalRefParam(attribute), JavaLibrary::getFidDraftSchematicAttribName());
	jfloat value = env->GetFloatField(attribute, JavaLibrary::getFidDraftSchematicAttribCurrentValue());
	jfloat resourceValue = env->GetFloatField(attribute, JavaLibrary::getFidDraftSchematicAttribResourceMaxValue());

	if (attrName == LocalRef::cms_nullPtr)
		return JNI_FALSE;

	StringId name;
	if (!ScriptConversion::convert(*attrName, name))
		return JNI_FALSE;

	if (experiment)
	{
		schematic->setExperimentAttribute(name, value);
		jfloat minValue = env->GetFloatField(attribute, JavaLibrary::getFidDraftSchematicAttribMinValue());
		jfloat maxValue = env->GetFloatField(attribute, JavaLibrary::getFidDraftSchematicAttribMaxValue());
		schematic->setExperimentAttributeLimits(name, minValue, maxValue, resourceValue);
	}
	else
	{
		schematic->setAttribute(name, value);
		schematic->setResourceMaxAttribute(name, resourceValue);
	}
	return JNI_TRUE;
}	// JavaLibrary::setSchematicAttribute

/**
 * Sets attributes for a manufacturing schematic.
 *
 * @param env							Java environment
 * @param self							class calling this function
 * @param manufacturingSchematic		the schematic
 * @param attributes					list of attribute info to set
 * @param experiment					flag that these are experimental attributes
 *
 * @return true on success, false if there was an error
 */
jboolean JNICALL ScriptMethodsCraftingNamespace::setSchematicAttributes(JNIEnv *env, jobject self, jlong manufacturingSchematic, jobjectArray attributes, jboolean experiment)
{
	UNREF(self);

	if (manufacturingSchematic == 0 || attributes == 0)
		return JNI_FALSE;

	ManufactureObjectInterface * schematic = nullptr;
	if (!JavaLibrary::getObject(manufacturingSchematic, schematic))
		return JNI_FALSE;

	StringId name;
	int count = env->GetArrayLength(attributes);
	for (int i = 0; i < count; ++i)
	{
		LocalRefPtr attribute = getObjectArrayElement(LocalObjectArrayRefParam(attributes), i);
		if (attribute != LocalRef::cms_nullPtr)
		{
			LocalRefPtr attrName = getObjectField(*attribute, JavaLibrary::getFidDraftSchematicAttribName());
			jfloat value = getFloatField(*attribute, JavaLibrary::getFidDraftSchematicAttribCurrentValue());
			jfloat resourceValue = getFloatField(*attribute, JavaLibrary::getFidDraftSchematicAttribResourceMaxValue());

			if (attrName != LocalRef::cms_nullPtr)
			{
				if (ScriptConversion::convert(*attrName, name))
				{
					if (experiment)
					{
						schematic->setExperimentAttribute(name, value);
						jfloat minValue = getFloatField(*attribute, JavaLibrary::getFidDraftSchematicAttribMinValue());
						jfloat maxValue = getFloatField(*attribute, JavaLibrary::getFidDraftSchematicAttribMaxValue());
						schematic->setExperimentAttributeLimits(name,
							minValue, maxValue, resourceValue);
					}
					else
					{
						schematic->setAttribute(name, value);
						schematic->setResourceMaxAttribute(name, resourceValue);
					}
				}
			}
		}
	}

	return JNI_TRUE;
}	// JavaLibrary::setSchematicAttributes

/**
 * Gets an attribute of a manufacturing schematic.
 *
 * @param env							Java environment
 * @param self							class calling this function
 * @param manufacturingSchematic		the schematic
 * @param name							name of the attribute to get
 * @param experiment					flag that these are experimental attributes
 *
 * @return the attribute, or nullptr on error
 */
jobject JNICALL ScriptMethodsCraftingNamespace::getSchematicAttribute(JNIEnv *env, jobject self, jlong manufacturingSchematic, jobject name, jboolean experiment)
{
	UNREF(self);

	if (manufacturingSchematic == 0 || name == 0)
		return 0;

	ManufactureObjectInterface * schematic = nullptr;
	if (!JavaLibrary::getObject(manufacturingSchematic, schematic))
		return 0;

	const DraftSchematicObject * draft = DraftSchematicObject::getSchematic(
		schematic->getDraftSchematic());
	if (draft == nullptr)
		return 0;

	StringId nameId;
	if (!ScriptConversion::convert(name, nameId))
		return 0;

	if (experiment)
	{
		return JavaLibrary::createExperimentAttribute(*schematic, nameId)->getReturnValue();
	}
	else
	{
		return JavaLibrary::createObjectAttribute(*schematic, *draft, draft->getAttribIndex(nameId))->getReturnValue();
	}
}	// JavaLibrary::getSchematicAttribute

/**
 * Gets a select list of the attributes of a manufacturing schematic.
 *
 * @param env							Java environment
 * @param self							class calling this function
 * @param manufacturingSchematic		the schematic
 * @param names							names of the attributes to get
 * @param experiment					flag that these are experimental attributes
 *
 * @return the attributes, or nullptr on error
 */
jobjectArray JNICALL ScriptMethodsCraftingNamespace::getSchematicAttributes(JNIEnv *env, jobject self, jlong manufacturingSchematic, jobjectArray names, jboolean experiment)
{
	UNREF(self);

	if (manufacturingSchematic == 0)
		return 0;

	ManufactureObjectInterface * schematic = nullptr;
	if (!JavaLibrary::getObject(manufacturingSchematic, schematic))
		return 0;

	const DraftSchematicObject * draft = DraftSchematicObject::getSchematic(
		schematic->getDraftSchematic());
	if (draft == nullptr)
		return 0;

	int numAttribs = env->GetArrayLength(names);

	// create the attribute array
	LocalObjectArrayRefPtr attributes = createNewObjectArray(numAttribs, JavaLibrary::getClsDraftSchematicAttrib());
	if (attributes == LocalObjectArrayRef::cms_nullPtr)
		return 0;

	int i;
	StringId attribName;
	for (i = 0; i < numAttribs; ++i)
	{
		LocalRefPtr name = getObjectArrayElement(LocalObjectArrayRefParam(names), i);
		if (!ScriptConversion::convert(*name, attribName))
			break;

		// create the attribute object
		LocalRefPtr attribute;
		if (experiment)
		{
			attribute = JavaLibrary::createExperimentAttribute(*schematic, attribName);
		}
		else
		{
			attribute = JavaLibrary::createObjectAttribute(*schematic, *draft, draft->getAttribIndex(
				attribName));
		}
		if (attribute == LocalRef::cms_nullPtr)
			break;

		setObjectArrayElement(*attributes, i, *attribute);
	}

	if (i != numAttribs)
	{
		attributes = LocalObjectArrayRef::cms_nullPtr;
	}

	return attributes->getReturnValue();
}	// JavaLibrary::getSchematicAttributes

/**
 * Gets all the attributes of a manufacturing schematic.
 *
 * @param env							Java environment
 * @param self							class calling this function
 * @param manufacturingSchematic		the schematic
 * @param experiment					flag that these are experimental attributes
 *
 * @return the attributes, or nullptr on error
 */
jobjectArray JNICALL ScriptMethodsCraftingNamespace::getAllSchematicAttributes(JNIEnv *env, jobject self, jlong manufacturingSchematic, jboolean experiment)
{
	UNREF(env);
	UNREF(self);

	if (manufacturingSchematic == 0)
		return 0;

	ManufactureObjectInterface * schematic = nullptr;
	if (!JavaLibrary::getObject(manufacturingSchematic, schematic))
		return 0;

	const DraftSchematicObject * draft = DraftSchematicObject::getSchematic(
		schematic->getDraftSchematic());
	if (draft == nullptr)
		return 0;

	// set the attributes
	int numAttribs = 0;
	const std::vector<StringId> & experimentNames = schematic->getExperimentAttributeNames();
	if (experiment)
		numAttribs = experimentNames.size();
	else
		numAttribs = draft->getAttribsCount();
	LocalObjectArrayRefPtr attributes = createNewObjectArray(numAttribs, JavaLibrary::getClsDraftSchematicAttrib());
	if (attributes == LocalObjectArrayRef::cms_nullPtr)
		return 0;

	for (int i = 0; i < numAttribs; ++i)
	{
		// create the attribute object
		LocalRefPtr attribute;
		if (experiment)
		{
			attribute = JavaLibrary::createExperimentAttribute(*schematic, experimentNames[i]);
		}
		else
		{
			attribute = JavaLibrary::createObjectAttribute(*schematic, *draft, i);
		}

		setObjectArrayElement(*attributes, i, *attribute);
	}

	return attributes->getReturnValue();
}	// JavaLibrary::getAllSchematicAttributes

//----------------------------------------------------------------------

/**
 * Returns a draft_schematic structure filled in with info about requested
 * experimental attributes. The info will consist of the experimentation attribute
 * data, the object attributes associated with them, and a mapping between the two.
 *
 * @param env							Java environment
 * @param self							class calling this function
 * @param manufacturingSchematic		the schematic to get the data from
 * @param attributeNames				the experimental attributes we're interested about
 *
 * @return the draft_schematic object, or nullptr on error
 */
jobject JNICALL ScriptMethodsCraftingNamespace::getSchematicForExperimentalAttributes(JNIEnv *env, jobject self, jlong manufacturingSchematic, jobjectArray attributeNames)
{
int i;

	UNREF(self);

	if (manufacturingSchematic == 0 || attributeNames == 0)
		return 0;

	const ManufactureObjectInterface * manfSchematic = nullptr;
	if (!JavaLibrary::getObject(manufacturingSchematic, manfSchematic))
		return 0;

	const DraftSchematicObject * draftSchematic = DraftSchematicObject::getSchematic(
		manfSchematic->getDraftSchematic());
	if (draftSchematic == nullptr)
		return 0;

	// create a draft_schematic object to return
	LocalRefPtr target = allocObject(JavaLibrary::getClsDraftSchematic());
	if (target == LocalRef::cms_nullPtr)
		return 0;

	// create the experimental attributes array
	int numExperimentAttribs = env->GetArrayLength(attributeNames);
	LocalObjectArrayRefPtr experimentalAttribs = createNewObjectArray(numExperimentAttribs,
		JavaLibrary::getClsDraftSchematicAttrib());
	if (experimentalAttribs == LocalObjectArrayRef::cms_nullPtr)
		return 0;

	// create the object attributes array
	int numObjectAttribs = draftSchematic->getAttribsCount();
	LocalObjectArrayRefPtr objectAttribs = createNewObjectArray(numObjectAttribs,
		JavaLibrary::getClsDraftSchematicAttrib());
	if (objectAttribs == LocalObjectArrayRef::cms_nullPtr)
		return 0;

	// create a map of experimental attributes to object attributes
	LocalRefPtr attribMap = createNewObject(JavaLibrary::getClsHashtable(), JavaLibrary::getMidHashtable());
	if (attribMap == LocalRef::cms_nullPtr)
		return 0;

	std::vector<LocalRefPtr> objectAttribRefs;

	// create the object attributes objects
	for (i = 0; i < numObjectAttribs; ++i)
	{
		// create the attribute object
		LocalRefPtr attribute = JavaLibrary::createObjectAttribute(*manfSchematic, *draftSchematic, i);
		if (attribute == LocalRef::cms_nullPtr)
		{
			objectAttribRefs.clear();
			return 0;
		}

		setObjectArrayElement(*objectAttribs, i, *attribute);
		objectAttribRefs.push_back(attribute);
	}
	setObjectField(*target, JavaLibrary::getFidDraftSchematicAttribs(), *objectAttribs);

	// create the experimental attribute objects
	StringId experimentAttribName;
	for (i = 0; i < numExperimentAttribs; ++i)
	{
		// get the experimental attribute name
		LocalRefPtr name = getObjectArrayElement(LocalObjectArrayRefParam(attributeNames), i);
		if (name == LocalRef::cms_nullPtr)
		{
			objectAttribRefs.clear();
			return 0;
		}

		if (!ScriptConversion::convert(*name, experimentAttribName))
		{
			objectAttribRefs.clear();
			return 0;
		}

		// get the data for the experimental attribute
		LocalRefPtr attribute = JavaLibrary::createExperimentAttribute(*manfSchematic,
			experimentAttribName);
		if (attribute == LocalRef::cms_nullPtr)
		{
			objectAttribRefs.clear();
			return 0;
		}

		setObjectArrayElement(*experimentalAttribs, i, *attribute);

		// find all the object attributes associated with this experimental
		// attribute and map them to each other
		LocalObjectArrayRefPtr mappedObjectAttribs = createNewObjectArray(numObjectAttribs,
			JavaLibrary::getClsDraftSchematicAttrib());
		if (mappedObjectAttribs == LocalObjectArrayRef::cms_nullPtr)
		{
			objectAttribRefs.clear();
			return 0;
		}

		int j;
		int mappedObjectIndex = 0;
		for (j = 0; j < numObjectAttribs; ++j)
		{
			if (draftSchematic->getAttribMin(j).value != draftSchematic->getAttribMax(j).value)
			{
				const SharedDraftSchematicObjectTemplate::SchematicAttribute &
					objectAttrib = draftSchematic->getAttribMin(j);
				if (objectAttrib.experiment == experimentAttribName)
				{
					setObjectArrayElement(*mappedObjectAttribs, mappedObjectIndex++, *objectAttribRefs[j]);
				}
			}
		}
		for (j = mappedObjectIndex; j < numObjectAttribs; ++j)
		{
			setObjectArrayElement(*mappedObjectAttribs, j, LocalRef(0));
		}
		callObjectMethod(*attribMap, JavaLibrary::getMidMapPut(), attribute->getValue(), mappedObjectAttribs->getValue());
	}
	setObjectField(*target, JavaLibrary::getFidDraftSchematicExperimentalAttribs(), *experimentalAttribs);
	setObjectField(*target, JavaLibrary::getFidDraftSchematicAttribMap(), *attribMap);

	objectAttribRefs.clear();

	return target->getReturnValue();
}	// JavaLibrary::getSchematicForExperimentalAttributes

//----------------------------------------------------------------------

/**
 * Sets an approximate experiment mod for an assembled schematic so the player will have some idea of
 * the risk experimenting with an item.
 *
 * @param manufacturingSchematic	the schematic
 * @param experimentMod				approximent mod that will be applied during experimentation
 *
 * @return true on success, false if there was an error
 */
jboolean JNICALL ScriptMethodsCraftingNamespace::setSchematicExperimentMod(JNIEnv *env, jobject self, jlong manufacturingSchematic, jfloat experimentMod)
{
	UNREF(self);

	ManufactureSchematicObject * schematic = nullptr;
	if (!JavaLibrary::getObject(manufacturingSchematic, schematic))
		return JNI_FALSE;

	schematic->setExperimentMod(experimentMod);

	return JNI_TRUE;
}	// JavaLibrary::setSchematicExperimentMod

//----------------------------------------------------------------------

/**
 * Sets the appearances that will be available to the player during customization.
 *
 * @param env							Java environment
 * @param self							class calling this function
 * @param manufacturingSchematic		the schematic that will create the
 *                                          customized object
 * @param appearances                   appearance filenames that the player may choose
 *
 * @return JNI_TRUE on success, JNI_FALSE on error
 */
jboolean JNICALL ScriptMethodsCraftingNamespace::setSchematicAppearances(JNIEnv *env, jobject self, jlong manufacturingSchematic, jobjectArray appearances)
{
	UNREF(self);

	if (manufacturingSchematic == 0 || appearances == 0)
		return JNI_FALSE;

	ManufactureSchematicObject * schematic = nullptr;
	if (!JavaLibrary::getObject(manufacturingSchematic, schematic))
		return JNI_FALSE;

	std::string appearance;
	int count = env->GetArrayLength(appearances);
	for (int i = 0; i < count; ++i)
	{
		JavaString appearanceString(static_cast<jstring>(env->GetObjectArrayElement(
			appearances, i)));
		if (JavaLibrary::convert(appearanceString, appearance))
		{
			schematic->addAppearance(appearance);
		}
	}
	return JNI_TRUE;
}	// JavaLibrary::setSchematicAppearances

//----------------------------------------------------------------------

/**
 * Sets the customization parameters that a player is allowed to change, or
 * differ in their default value from the template. You do not have to set
 * fixed customization parameters that use their default value.
 *
 * @param env							Java environment
 * @param self							class calling this function
 * @param manufacturingSchematic		the schematic that will create the
 *                                          customized object
 * @param customizations				customization info
 *
 * @return JNI_TRUE on success, JNI_FALSE if there was an error
 */
jboolean JNICALL ScriptMethodsCraftingNamespace::setSchematicCustomizations(JNIEnv *env, jobject self, jlong manufacturingSchematic, jobjectArray customizations)
{
	UNREF(self);

	if (manufacturingSchematic == 0 || customizations == 0)
		return JNI_FALSE;

	ManufactureSchematicObject * schematic = nullptr;
	if (!JavaLibrary::getObject(manufacturingSchematic, schematic))
		return JNI_FALSE;

	std::string name;
	int count = env->GetArrayLength(customizations);
	for (int i = 0; i < count; ++i)
	{
		LocalRefPtr custom = getObjectArrayElement(LocalObjectArrayRefParam(customizations), i);
		if (custom == LocalRef::cms_nullPtr)
			continue;

		JavaStringPtr jname = getStringField(*custom, JavaLibrary::getFidDraftSchematicCustomName());
		if (jname == JavaString::cms_nullPtr)
			continue;
		if (!JavaLibrary::convert(*jname, name))
			return JNI_FALSE;

		int value = getIntField(*custom, JavaLibrary::getFidDraftSchematicCustomValue());
		int minValue = getIntField(*custom, JavaLibrary::getFidDraftSchematicCustomMinValue());
		int maxValue = getIntField(*custom, JavaLibrary::getFidDraftSchematicCustomMaxValue());

		schematic->setCustomization(name, value, minValue, maxValue);
	}

	return JNI_TRUE;
}	// JavaLibrary::setSchematicCustomizations

//----------------------------------------------------------------------

/**
 * Returns the number of items a manufacturing schematic can create.
 *
 * @param env							Java environment
 * @param self							class calling this function
 * @param manufacturingSchematic		the schematic
 *
 * @return the number of items, or -1 on error
 */
jint JNICALL ScriptMethodsCraftingNamespace::getSchematicItemCount(JNIEnv *env, jobject self, jlong manufacturingSchematic)
{
	UNREF(self);

	if (manufacturingSchematic == 0)
		return -1;

	const ManufactureSchematicObject * schematic = nullptr;
	if (!JavaLibrary::getObject(manufacturingSchematic, schematic))
		return -1;

	return schematic->getCount();
}	// JavaLibrary::getSchematicItemCount

//----------------------------------------------------------------------

/**
 * Sets the number of items a manufacturing schematic can create.
 *
 * @param env						Java environment
 * @param self						class calling this function
 * @param manufacturingSchematic	the schematic
 * @param count						the number of items
 *
 * @return true on success, false if there was an error
 */
jboolean JNICALL ScriptMethodsCraftingNamespace::setSchematicItemCount(JNIEnv *env, jobject self, jlong manufacturingSchematic, jint count)
{
	UNREF(self);

	if (manufacturingSchematic == 0)
		return JNI_FALSE;

	ManufactureSchematicObject * schematic = nullptr;
	if (!JavaLibrary::getObject(manufacturingSchematic, schematic))
		return JNI_FALSE;

	schematic->setCount(count);
	return JNI_TRUE;
}	// JavaLibrary::setSchematicItemCount

//----------------------------------------------------------------------

/**
 * Returns the numer of items per container a manufacturing schematic will
 * create.
 *
 * @param env							Java environment
 * @param self							class calling this function
 * @param manufacturingSchematic		the schematic
 *
 * @return the number of items, or -1 on error
 */
jint JNICALL ScriptMethodsCraftingNamespace::getSchematicItemsPerContainer(JNIEnv *env, jobject self, jlong manufacturingSchematic)
{
	UNREF(self);

	if (manufacturingSchematic == 0)
		return -1;

	const ManufactureSchematicObject * schematic = nullptr;
	if (!JavaLibrary::getObject(manufacturingSchematic, schematic))
		return -1;

	return schematic->getItemsPerContainer();
}	// JavaLibrary::getSchematicItemsPerContainer

//----------------------------------------------------------------------

/**
 * Sets the numer of items per container a manufacturing schematic will
 * create.
 *
 * @param env						Java environment
 * @param self						class calling this function
 * @param manufacturingSchematic	the schematic
 * @param count						the number of items
 *
 * @return true on success, false if there was an error
 */
jboolean JNICALL ScriptMethodsCraftingNamespace::setSchematicItemsPerContainer(JNIEnv *env, jobject self, jlong manufacturingSchematic, jint count)
{
	UNREF(self);

	if (manufacturingSchematic == 0)
		return JNI_FALSE;

	ManufactureSchematicObject * schematic = nullptr;
	if (!JavaLibrary::getObject(manufacturingSchematic, schematic))
		return JNI_FALSE;

	schematic->setItemsPerContainer(count);
	return JNI_TRUE;
}	// JavaLibrary::setSchematicItemsPerContainer

//----------------------------------------------------------------------

/**
 * Returns the time per complexity point it takes to manufacture an item with
 * a manufacturing schematic.
 *
 * @param env							Java environment
 * @param self							class calling this function
 * @param manufacturingSchematic		the schematic
 *
 * @return the time, or -1 on error
 */
jfloat JNICALL ScriptMethodsCraftingNamespace::getSchematicManufactureTime(JNIEnv *env, jobject self, jlong manufacturingSchematic)
{
	UNREF(self);

	if (manufacturingSchematic == 0)
		return -1.0f;

	const ManufactureSchematicObject * schematic = nullptr;
	if (!JavaLibrary::getObject(manufacturingSchematic, schematic))
		return -1.0f;

	return schematic->getManufactureTime();
}	// JavaLibrary::getSchematicManufactureTime

//----------------------------------------------------------------------

/**
 * Sets the time per complexity point it takes to manufacture an item with
 * a manufacturing schematic.
 *
 * @param env						Java environment
 * @param self						class calling this function
 * @param manufacturingSchematic	the schematic
 * @param time						the manufacturing time per complexity
 *
 * @return true on success, false if there was an error
 */
jboolean JNICALL ScriptMethodsCraftingNamespace::setSchematicManufactureTime(JNIEnv *env, jobject self, jlong manufacturingSchematic, jfloat time)
{
	UNREF(self);

	if (manufacturingSchematic == 0)
		return JNI_FALSE;

	ManufactureSchematicObject * schematic = nullptr;
	if (!JavaLibrary::getObject(manufacturingSchematic, schematic))
		return JNI_FALSE;

	schematic->setManufactureTime(time);
	return JNI_TRUE;
}	// JavaLibrary::setSchematicManufactureTime

//----------------------------------------------------------------------

/**
 * Sets the amount of xp the creator of an object will get when an object is "used".
 *
 * @param env		Java environment
 * @param self		class calling this function
 * @param object	the object that will grant the xp
 * @param xp        the amount of xp to grant
 *
 * @return true on success, false if there was an error
 */
jboolean JNICALL ScriptMethodsCraftingNamespace::setCreatorXp(JNIEnv *env, jobject self, jlong object, jint xp)
{
	UNREF(self);

	TangibleObject * target = nullptr;
	if (!JavaLibrary::getObject(object, target))
		return JNI_FALSE;

	target->setCreatorXp(xp);
	return JNI_TRUE;
}	// JavaLibrary::setCreatorXp

/**
 * Gets a list of ingredients needed to craft using the current manufacturing
 * schematic in a manufacturing station.
 *
 * @param env			Java environment
 * @param self			class calling this function
 * @param station		the manufacturing station id
 * @param ingredients	array to be filled in with a string array of ingredients
 */
void JNICALL ScriptMethodsCraftingNamespace::getIngredientsForManufactureStation(JNIEnv *env, jobject self, jlong station, jobjectArray ingredients)
{
	UNREF(self);

	if (station == 0 || ingredients == 0)
		return;

	const ManufactureInstallationObject * manfStation = nullptr;
	if (!JavaLibrary::getObject(station, manfStation))
		return;

	if (env->GetArrayLength(ingredients) != 1)
		return;

	const ManufactureSchematicObject * const schematic = manfStation->getSchematic();
	if (schematic == nullptr)
		return;

	static ManufactureSchematicObject::IngredientInfoVector iiv;
	iiv.clear ();

	schematic->getIngredientInfo (iiv);

	if (iiv.empty ())
		return;

	const size_t count = iiv.size();

	LocalObjectArrayRefPtr ingredientList = createNewObjectArray(count, JavaLibrary::getClsString());
	if (ingredientList == LocalObjectArrayRef::cms_nullPtr)
		return;

	char buf [128];
	const size_t buf_size = sizeof (buf);

	for (size_t i = 0; i < count; ++i)
	{
		const ManufactureSchematicObject::IngredientInfo & info = iiv [i];

		const Unicode::String & name        = info.first;
		const int               numPerItem  = info.second.second;

		if (name.empty ())
			continue;

		snprintf (buf, buf_size, ":\\>200%d", numPerItem);

		Unicode::String element = name;

		if (name [0] == '@')
			element.push_back ('\0');

		element += Unicode::narrowToWide (buf);

		JavaString jelement(element);
		setObjectArrayElement(*ingredientList, i, jelement);
	}

	setObjectArrayElement(LocalObjectArrayRefParam(ingredients), 0, *ingredientList);
}	// JavaLibrary::getIngredientsForManufactureStation

//----------------------------------------------------------------------

/**
 * Gets the id of a manufacture station's input hopper.
 *
 * @param env			Java environment
 * @param self			class calling this function
 * @param station		the station id
 *
 * @return the hopper id, or nullptr on error
 */
jlong JNICALL ScriptMethodsCraftingNamespace::getManufactureStationInputHopper(JNIEnv *env, jobject self, jlong station)
{
	UNREF(self);

	if (station == 0)
		return 0;

	const ManufactureInstallationObject * manfStation = nullptr;
	if (!JavaLibrary::getObject(station, manfStation))
		return 0;

	ServerObject * hopper = manfStation->getInputHopper();
	if (hopper == nullptr)
		return 0;

	return (hopper->getNetworkId()).getValue();
}	// JavaLibrary::getManufactureStationInputHopper

//----------------------------------------------------------------------

/**
 * Gets the id of a manufacture station's output hopper.
 *
 * @param env			Java environment
 * @param self			class calling this function
 * @param station		the station id
 *
 * @return the hopper id, or nullptr on error
 */
jlong JNICALL ScriptMethodsCraftingNamespace::getManufactureStationOutputHopper(JNIEnv *env, jobject self, jlong station)
{
	UNREF(self);

	if (station == 0)
		return 0;

	const ManufactureInstallationObject * manfStation = nullptr;
	if (!JavaLibrary::getObject(station, manfStation))
		return 0;

	ServerObject * hopper = manfStation->getOutputHopper();
	if (hopper == nullptr)
		return 0;

	return (hopper->getNetworkId()).getValue();
}	// JavaLibrary::getManufactureStationOutputHopper

//----------------------------------------------------------------------

/**
 * Gets the id and name of the schematic in a manufacture station.
 *
 * @param env			Java environment
 * @param self			class calling this function
 * @param station		the station id
 *
 * @return a string of the form <id> "*" <name>, or nullptr if the station has no schematic
 */
jstring JNICALL ScriptMethodsCraftingNamespace::getManufactureStationSchematic(JNIEnv *env, jobject self, jlong station)
{
	static const Unicode::String SEPARATOR(Unicode::narrowToWide("*"));

	UNREF(self);

	const ManufactureInstallationObject * manfStation = nullptr;
	if (!JavaLibrary::getObject(station, manfStation))
		return 0;

	const ManufactureSchematicObject * manfSchematic = manfStation->getSchematic();
	if (manfSchematic == nullptr)
		return 0;

	return JavaString(
			Unicode::narrowToWide(manfSchematic->getNetworkId().getValueString()) +
			SEPARATOR +
			manfSchematic->getObjectName()
		).getReturnValue();
}	// JavaLibrary::getManufactureStationSchematic

//----------------------------------------------------------------------

/**
 * Gets a list of manufacture schematics that could be used at a given manufacturing
 * station.
 *
 * @param env			Java environment
 * @param self			class calling this function
 * @param player		the player whose schematics to get
 * @param station		the manufacturing station
 * @param schematics	array to be filled in with a string array of schematics
 */
void JNICALL ScriptMethodsCraftingNamespace::getValidManufactureSchematicsForStation(JNIEnv *env, jobject self, jlong player, jlong station, jobjectArray schematics)
{
	static const Unicode::String SEPARATOR(Unicode::narrowToWide("*"));

	UNREF(self);

	if (player == 0 || station == 0 || schematics == 0)
		return;

	CreatureObject * playerCreature = nullptr;
	if (!JavaLibrary::getObject(player, playerCreature))
		return;

	const ManufactureInstallationObject * manfStation = nullptr;
	if (!JavaLibrary::getObject(station, manfStation))
		return;

	if (env->GetArrayLength(schematics) != 1)
		return;

	std::vector<const ManufactureSchematicObject *> manfSchematics;
	uint32 stationTypes = manfStation->getValidSchematicTypes();
	playerCreature->getManufactureSchematics(manfSchematics, stationTypes);

	int count = manfSchematics.size();
	LocalObjectArrayRefPtr schematicList = createNewObjectArray(count, JavaLibrary::getClsString());
	if (schematicList == LocalObjectArrayRef::cms_nullPtr)
		return;

	for (int i = 0; i < count; ++i)
	{
		const ManufactureSchematicObject * manfSchematic = manfSchematics[i];
		setObjectArrayElement(*schematicList, i,
			JavaString(
				Unicode::narrowToWide(manfSchematic->getNetworkId().getValueString()) +
				SEPARATOR +
				manfSchematic->getObjectName()
			));
	}

	setObjectArrayElement(LocalObjectArrayRefParam(schematics), 0, *schematicList);
}	// JavaLibrary::getValidManufactureSchematicsForStation

//----------------------------------------------------------------------

/**
 * Checks if a player has any manufacture schematics that could be used at a
 * given manufacturing station.
 *
 * @param env			Java environment
 * @param self			class calling this function
 * @param player		the player whose schematics to check
 * @param station		the manufacturing station
 *
 * @return true if the player has any schematics, false if not
 */
jboolean JNICALL ScriptMethodsCraftingNamespace::hasValidManufactureSchematicsForStation(JNIEnv *env, jobject self, jlong player, jlong station)
{
	UNREF(self);

	if (player == 0 || station == 0)
		return JNI_FALSE;

	CreatureObject * playerCreature = nullptr;
	if (!JavaLibrary::getObject(player, playerCreature))
		return JNI_FALSE;

	const ManufactureInstallationObject * manfStation = nullptr;
	if (!JavaLibrary::getObject(station, manfStation))
		return JNI_FALSE;

	std::vector<const ManufactureSchematicObject *> manfSchematics;
	playerCreature->getManufactureSchematics(manfSchematics,
		manfStation->getValidSchematicTypes());

	if (manfSchematics.empty())
		return JNI_FALSE;
	return JNI_TRUE;
}	// JavaLibrary::hasValidManufactureSchematicsForStation

//----------------------------------------------------------------------

/**
 * Transfers the manufacture schematic in a manufacturing station to a player.
 *
 * @param env			Java environment
 * @param self			class calling this function
 * @param station		the manufacturing station id
 * @param player		the the player to transfer to
 *
 * @return true if the schematic was transferred, false if not
 */
jboolean JNICALL ScriptMethodsCraftingNamespace::transferManufactureSchematicToPlayer(JNIEnv *env, jobject self, jlong station, jlong player)
{
	UNREF(self);

	const ManufactureInstallationObject * manfStation = nullptr;
	if (!JavaLibrary::getObject(station, manfStation))
		return JNI_FALSE;

	ManufactureSchematicObject * schematic = manfStation->getSchematic();
	if (schematic == nullptr)
		return JNI_TRUE;

	CreatureObject * playerCreature = nullptr;
	if (!JavaLibrary::getObject(player, playerCreature))
		return JNI_FALSE;

	ServerObject * datapad = playerCreature->getDatapad();
	if (datapad == nullptr)
		return JNI_FALSE;

	Container::ContainerErrorCode tmp = Container::CEC_Success;
	if (ContainerInterface::transferItemToVolumeContainer(*datapad, *schematic, nullptr, tmp))
		return JNI_TRUE;
	return JNI_FALSE;
}	// JavaLibrary::transferManufactureSchematicToPlayer

//----------------------------------------------------------------------

/**
 * Transfers a manufacture schematic to a manufacturing station.
 *
 * @param env			Java environment
 * @param self			class calling this function
 * @param schematic		the schematic id
 * @param station		the manufacturing station id
 *
 * @return true if the schematic was transferred, false if not
 */
jboolean JNICALL ScriptMethodsCraftingNamespace::transferManufactureSchematicToStation(JNIEnv *env, jobject self, jlong schematic, jlong station)
{
	UNREF(self);

	ManufactureSchematicObject * manfSchematic = nullptr;
	if (!JavaLibrary::getObject(schematic, manfSchematic))
		return JNI_FALSE;

	ManufactureInstallationObject * manfStation = nullptr;
	if (!JavaLibrary::getObject(station, manfStation))
		return JNI_FALSE;

	if (manfStation->addSchematic(*manfSchematic, nullptr))
		return JNI_TRUE;
	return JNI_FALSE;
}	// JavaLibrary::transferManufactureSchematicToStation

//----------------------------------------------------------------------

/**
 * Gets a list of objects that can be repaired by a given repair tool.
 *
 * @param env			Java environment
 * @param self			class calling this function
 * @param player		the player whose objects to get
 * @param tool			the repair tool
 * @param objects		array to be filled in with a string array of objects
 */
void JNICALL ScriptMethodsCraftingNamespace::getRepairableObjectsForTool(JNIEnv *env, jobject self, jlong player, jlong tool, jobjectArray objects)
{
	static const Unicode::String SEPARATOR(Unicode::narrowToWide("*"));

	if (player == 0 || tool == 0 || objects == 0)
		return;

	CreatureObject * playerCreature = nullptr;
	if (!JavaLibrary::getObject(player, playerCreature))
		return;

	const TangibleObject * toolObject = nullptr;
	if (!JavaLibrary::getObject(tool, toolObject))
		return;
	if (!toolObject->isRepairTool())
		return;
	int toolType = toolObject->getCraftingType();

	// determine if the repair tool repairs a specific GOT or a category
	bool genericTool = false;
	if ((toolType & 0x000000ff) == 0)
		genericTool = true;

	if (env->GetArrayLength(objects) != 1)
		return;

	// @todo: we need to get a list of things the player owns that are loaded,
	// so non-inventory objects can be repaired. For now, just repair inventory
	// objects

	const ServerObject * inventory = playerCreature->getInventory();
	if (inventory == nullptr)
		return;

	const VolumeContainer * inventoryContainer = ContainerInterface::getVolumeContainer(*inventory);
	if (inventoryContainer == nullptr)
		return;

	std::vector<Unicode::String> objList;
	for (ContainerConstIterator iter(inventoryContainer->begin());
		iter != inventoryContainer->end(); ++iter)
	{
		const CachedNetworkId & objId = (*iter);
		const TangibleObject * obj = safe_cast<const TangibleObject *>(objId.getObject());
		if (obj != nullptr && !obj->isCraftingTool() && !obj->isRepairTool() &&
			obj->getDamageTaken() > 0)
		{
			if ((genericTool && (toolType & obj->getGameObjectType()) != 0) ||
				(!genericTool && toolType == obj->getGameObjectType()))
			{
				objList.push_back(
					Unicode::narrowToWide(obj->getNetworkId().getValueString()) +
					SEPARATOR +
					obj->getEncodedObjectName ());
			}
		}
	}

	int count = objList.size();
	LocalObjectArrayRefPtr objectStringList = createNewObjectArray(count, JavaLibrary::getClsString());
	if (objectStringList == LocalObjectArrayRef::cms_nullPtr)
		return;

	for (int i = 0; i < count; ++i)
	{
		setObjectArrayElement(*objectStringList, i, JavaString(objList[i]));
	}

	setObjectArrayElement(LocalObjectArrayRefParam(objects), 0, *objectStringList);
}	// JavaLibrary::getRepairableObjectsForTool

//----------------------------------------------------------------------

/**
 * Returns the attribute bonuses an object grants when equipped.
 *
 * @param env			Java environment
 * @param self			class calling this function
 * @param target		the object
 *
 * @return an array with the bonus for each attribute, or nullptr on error
 */
jintArray JNICALL ScriptMethodsCraftingNamespace::getAttributeBonuses(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	const ServerObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return 0;

	std::vector<std::pair<int, int> > bonuses;
	if (object->asTangibleObject() != nullptr)
	{
		object->asTangibleObject()->getAttribBonuses(bonuses);
	}
	else if (object->asManufactureSchematicObject() != nullptr)
	{
		object->asManufactureSchematicObject()->getAttribBonuses(bonuses);
	}
	else
		return 0;

	jint bonusesArray[Attributes::NumberOfAttributes];
	{
	for (int i = 0; i < Attributes::NumberOfAttributes; ++i)
		bonusesArray[i] = 0;
	}
	{
	for (std::vector<std::pair<int, int> >::const_iterator i = bonuses.begin();
		i != bonuses.end(); ++i)
	{
		if ((*i).first >= 0 && (*i).first < Attributes::NumberOfAttributes)
			bonusesArray[(*i).first] = (*i).second;
	}
	}

	LocalIntArrayRefPtr jbonuses = createNewIntArray(Attributes::NumberOfAttributes);
	if (jbonuses == LocalIntArrayRef::cms_nullPtr)
		return 0;

	setIntArrayRegion(*jbonuses, 0, Attributes::NumberOfAttributes, &bonusesArray[0]);
	return jbonuses->getReturnValue();
}	// JavaLibrary::getAttributeBonuses

//----------------------------------------------------------------------

/**
 * Returns the attribute bonus of an object for a given attribute.
 *
 * @param env			Java environment
 * @param self			class calling this function
 * @param target		the object
 * @param attribute		the attribute to get
 *
 * @return the attribute bonus
 */
jint JNICALL ScriptMethodsCraftingNamespace::getAttributeBonus(JNIEnv *env, jobject self, jlong target, jint attribute)
{
	UNREF(self);

	if (attribute < 0 || attribute >= Attributes::NumberOfAttributes)
		return 0;

	const ServerObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return 0;

	jint bonus = 0;
	if (object->asTangibleObject() != nullptr)
	{
		bonus = object->asTangibleObject()->getAttribBonus(attribute);
	}
	else if (object->asManufactureSchematicObject() != nullptr)
	{
		bonus = object->asManufactureSchematicObject()->getAttribBonus(attribute);
	}
	return bonus;
}	// JavaLibrary::getAttributeBonus

//----------------------------------------------------------------------

/**
 * Sets the attribute bonus an object applies when equipped.
 *
 * @param env			Java environment
 * @param self			class calling this function
 * @param target		the object
 * @param attribute		the attribute to set
 * @param bonus			the attribute bonus value
 *
 * @return true on success, false on error
 */
jboolean JNICALL ScriptMethodsCraftingNamespace::setAttributeBonus(JNIEnv *env, jobject self, jlong target, jint attribute, jint bonus)
{
	UNREF(self);

	if (attribute < 0 || attribute >= Attributes::NumberOfAttributes)
		return JNI_FALSE;

	ServerObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return JNI_FALSE;

	if (object->asTangibleObject() != nullptr)
	{
		object->asTangibleObject()->setAttribBonus(attribute, bonus);
	}
	else if (object->asManufactureSchematicObject() != nullptr)
	{
		object->asManufactureSchematicObject()->setAttribBonus(attribute, bonus);
	}
	else
		return JNI_FALSE;
	return JNI_TRUE;
}	// JavaLibrary::setAttributeBonus

//----------------------------------------------------------------------

/**
 * Sets the attribute bonuses an object applies when equipped.
 *
 * @param env			Java environment
 * @param self			class calling this function
 * @param target		the object
 * @param bonuses		the attribute bonus values
 *
 * @return true on success, false on error
 */
jboolean JNICALL ScriptMethodsCraftingNamespace::setAttributeBonuses(JNIEnv *env, jobject self, jlong target, jintArray bonuses)
{
	UNREF(self);

	if (bonuses == 0)
		return JNI_FALSE;

	ServerObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return JNI_FALSE;

	jsize count = env->GetArrayLength(bonuses);
	if (count != Attributes::NumberOfAttributes)
	{
		WARNING(true, ("JavaLibrary::setAttributeBonuses passed bad array of "
			"size %u", count));
		return JNI_FALSE;
	}

	jint buffer[Attributes::NumberOfAttributes];
	env->GetIntArrayRegion(bonuses, 0, count, buffer);

	if (object->asTangibleObject() != nullptr)
	{
		TangibleObject * tangibleObject = object->asTangibleObject();
		for (jsize i = 0; i < count; ++i)
			tangibleObject->setAttribBonus(i, buffer[i]);
	}
	else if (object->asManufactureSchematicObject() != nullptr)
	{
		ManufactureSchematicObject * manufactureSchematicObject = object->asManufactureSchematicObject();
		for (jsize i = 0; i < count; ++i)
			manufactureSchematicObject->setAttribBonus(i, buffer[i]);
	}
	else
		return JNI_FALSE;

	return JNI_TRUE;
}	// JavaLibrary::setAttributeBonus

//----------------------------------------------------------------------

/**
 * Returns the skill mod bonuses an object grants when equipped.
 *
 * @param env			Java environment
 * @param self			class calling this function
 * @param target		the object
 *
 * @return a dictionary of skill mod names -> mod values, or nullptr on error
 */
jobject JNICALL ScriptMethodsCraftingNamespace::getSkillModBonuses(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	const TangibleObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return 0;

	std::vector<std::pair<std::string, int> > skillModBonuses;
	object->getSkillModBonuses(skillModBonuses);

	// put the bonuses into ScriptParams, and then convert to a dictionary
	ScriptParams params;
	int count = skillModBonuses.size();
	for (int i = 0; i < count; ++i)
	{
		params.addParam(skillModBonuses[i].second, skillModBonuses[i].first);
	}
	JavaDictionaryPtr dictionary;
	JavaLibrary::instance()->convert(params, dictionary);

	return dictionary->getReturnValue();
}	// JavaLibrary::getSkillModBonuses

//----------------------------------------------------------------------

/**
 * Returns the skill mod bonus of an object for a given skill mod.
 *
 * @param env			Java environment
 * @param self			class calling this function
 * @param target		the object
 * @param skillMod		the skill mod to get
 *
 * @return the skill mod value
 */
jint JNICALL ScriptMethodsCraftingNamespace::getSkillModBonus(JNIEnv *env, jobject self, jlong target, jstring skillMod)
{
	UNREF(self);

	JavaStringParam jskillMod(skillMod);

	const TangibleObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return 0;

	std::string skillModName;
	if (!JavaLibrary::convert(jskillMod, skillModName))
		return 0;

	return object->getSkillModBonus(skillModName);
}	// JavaLibrary::getSkillModBonus

//----------------------------------------------------------------------

/**
 * Sets the base skill mod bonus an object applies when equipped.
 *
 * @param env			Java environment
 * @param self			class calling this function
 * @param target		the object
 * @param skillMod		the skill mod to set
 * @param bonus			the skill mod bonus value
 *
 * @return true on success, false on error
 */
jboolean JNICALL ScriptMethodsCraftingNamespace::setSkillModBonus(JNIEnv *env, jobject self, jlong target, jstring skillMod, jint bonus)
{
	UNREF(self);

	JavaStringParam jskillMod(skillMod);

	TangibleObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return JNI_FALSE;

	std::string skillModName;
	if (!JavaLibrary::convert(jskillMod, skillModName))
		return JNI_FALSE;

	object->setSkillModBonus(skillModName, bonus);
	return JNI_TRUE;
}	// JavaLibrary::setSkillModBonus

//----------------------------------------------------------------------

/**
 * Sets the base skill mod bonuses an object applies when equipped.
 *
 * @param env			Java environment
 * @param self			class calling this function
 * @param target		the object
 * @param skillMod		the skill mods to set
 * @param bonus			the skill mod bonus values
 *
 * @return true on success, false on error
 */
jboolean JNICALL ScriptMethodsCraftingNamespace::setSkillModBonuses(JNIEnv *env, jobject self, jlong target, jobjectArray skillMod, jintArray bonus)
{
	UNREF(self);

	TangibleObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return JNI_FALSE;

	if (skillMod == 0 || bonus == 0)
		return JNI_FALSE;

	int skillModCount = env->GetArrayLength(skillMod);
	int bonusCount = env->GetArrayLength(bonus);
	if (skillModCount != bonusCount)
	{
		DEBUG_WARNING(true, ("[designer bug] setSkillModBonuses called with unequal "
			"array sizes"));
		return JNI_FALSE;
	}

	std::string skillName;
	const jint * bonusArray = env->GetIntArrayElements(bonus, nullptr);
	for (int i = 0; i < skillModCount; ++i)
	{
		JavaStringParam jskillName(static_cast<jstring>(env->GetObjectArrayElement(skillMod, i)));
		JavaLibrary::convert(jskillName, skillName);
		object->setSkillModBonus(skillName, bonusArray[i]);
	}
	env->ReleaseIntArrayElements(bonus, const_cast<jint *>(bonusArray), JNI_ABORT);

	return JNI_TRUE;
}	// JavaLibrary::setSkillModBonuses

//----------------------------------------------------------------------

/**
* Sets the base skill mod bonus an object applies when equipped.
*
* @param env			Java environment
* @param self			class calling this function
* @param target			the object
* @param category		the category of the skill mod bonus
*						multiple/independent bonuses can be granted
*						for a particular skill mod by putting them
*						into different categories
* @param skillMod		the skill mod to set
* @param bonus			the skill mod bonus value
*
* @return true on success, false on error
*/
jboolean JNICALL ScriptMethodsCraftingNamespace::setCategorizedSkillModBonus(JNIEnv *env, jobject self, jlong target, jstring category, jstring skillMod, jint bonus)
{
	UNREF(self);

	JavaStringParam jcategory(category);
	JavaStringParam jskillMod(skillMod);

	TangibleObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return JNI_FALSE;

	std::string categoryName;
	if (!JavaLibrary::convert(jcategory, categoryName))
		return JNI_FALSE;

	std::string skillModName;
	if (!JavaLibrary::convert(jskillMod, skillModName))
		return JNI_FALSE;

	object->setSkillModBonus(categoryName, skillModName, bonus);
	return JNI_TRUE;
}

//----------------------------------------------------------------------

/**
* Removes all skill mod bonuses in a particular category on an object.
*
* @param env			Java environment
* @param self			class calling this function
* @param target			the object
* @param category		the category of the skill mod bonus
*						multiple/independent bonuses can be granted
*						for a particular skill mod by putting them
*						into different categories
*/
void JNICALL ScriptMethodsCraftingNamespace::removeCategorizedSkillModBonuses(JNIEnv *env, jobject self, jlong target, jstring category)
{
	UNREF(self);

	JavaStringParam jcategory(category);

	TangibleObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return;

	std::string categoryName;
	if (!JavaLibrary::convert(jcategory, categoryName))
		return;

	object->removeCategorizedSkillModBonuses(categoryName);
}

//----------------------------------------------------------------------

/**
 * Returns the number of available skill mod sockets of an object.
 *
 * @param env			Java environment
 * @param self			class calling this function
 * @param target		the object
 *
 * @return the number of sockets
 */
jint JNICALL ScriptMethodsCraftingNamespace::getSkillModSockets(JNIEnv *env, jobject self, jlong target)
{
	UNREF(self);

	const TangibleObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return 0;

	return object->getSkillModSockets();
}	// JavaLibrary::getSkillModSockets

//----------------------------------------------------------------------

/**
 * Sets the number of available skill mod sockets for an object.
 *
 * @param env			Java environment
 * @param self			class calling this function
 * @param target		the object
 * @param sockets		the number of sockets
 *
 * @return true on success, false on error
 */
jboolean JNICALL ScriptMethodsCraftingNamespace::setSkillModSockets(JNIEnv *env, jobject self, jlong target, jint sockets)
{
	UNREF(self);

	TangibleObject * object = nullptr;
	if (!JavaLibrary::getObject(target, object))
		return JNI_FALSE;

	object->setSkillModSockets(sockets);
	return JNI_TRUE;
}	// JavaLibrary::setSkillModSockets

//----------------------------------------------------------------------

/**
 * Creates a new item based on a draft schematic. The item will have stats
 * based on a % value passed in (0 = worst possible stats, 100 = best possible
 * stats).
 *
 * @param env				Java environment
 * @param self				class calling this function
 * @param draftSchematic	draft schematic to make the object from
 * @param qualityPercent	% stat adjustment
 * @param container			the container to create the item in
 *
 * @return the item, or nullptr on error
 */
jlong JNICALL ScriptMethodsCraftingNamespace::makeCraftedItem(JNIEnv *env, jobject self, jstring draftSchematic, jfloat qualityPercent, jlong container)
{
	UNREF(self);

	if (draftSchematic == 0 || container == 0)
	{
		WARNING(true, ("[script bug] nullptr schematic or container passed to "
			"makeCraftedItem"));
		return 0;
	}

	JavaStringParam jDraftSchematic(draftSchematic);
	std::string draftSchematicName;
	if (!JavaLibrary::convert(jDraftSchematic, draftSchematicName))
	{
		WARNING(true, ("JavaLibrary::makeCraftedItem can't convert schematic "
			"name to string"));
		return 0;
	}

	const DraftSchematicObject * schematic = DraftSchematicObject::getSchematic(
		draftSchematicName);
	if (schematic == nullptr)
	{
		WARNING(true, ("[script bug] bad schematic name %s passed to "
			"makeCraftedItem", draftSchematicName.c_str()));
		return 0;
	}

	ServerObject * target = nullptr;
	if (!JavaLibrary::getObject(container, target))
	{
		WARNING(true, ("[script bug] bad container id passed to makeCraftedItem"));
		return 0;
	}
	Object * targetParent = ContainerInterface::getFirstParentInWorld(*target);
	if (targetParent == nullptr)
	{
		WARNING(true, ("JavaLibrary::makeCraftedItem can't find parent in world "
			"for container %s", target->getNetworkId().getValueString().c_str()));
		return 0;
	}
	Vector createPos(targetParent->getPosition_w());
	createPos.y = -100000.0f;

	// create a manf schematic and prototype
	ManufactureSchematicObject * manfSchematic = ServerWorld::createNewManufacturingSchematic(
		*schematic, createPos, false);
	if (manfSchematic == nullptr)
	{
		WARNING(true, ("JavaLibrary::makeCraftedItem: error creating manf "
			"schematic!"));
		return 0;
	}
	ServerObject * prototype = manfSchematic->manufactureObject(createPos);
	if (prototype == nullptr)
	{
		WARNING(true, ("JavaLibrary::makeCraftedItem: error creating "
			"prototype!"));
		manfSchematic->permanentlyDestroy(DeleteReasons::SetupFailed);
		return false;
	}

	if (qualityPercent < 0.0f)
		qualityPercent = 0.0f;
	else if (qualityPercent > 100.0f)
		qualityPercent = 100.0f;

	ScriptParams params;
	params.addParam(prototype->getNetworkId());
	params.addParam(*manfSchematic);
	params.addParam(qualityPercent);
	IGNORE_RETURN(manfSchematic->getScriptObject()->trigAllScripts(
		Scripting::TRIG_MAKE_CRAFTED_ITEM, params));

	manfSchematic->permanentlyDestroy(DeleteReasons::Consumed);
	Container::ContainerErrorCode error;
	if (!ContainerInterface::transferItemToVolumeContainer (*target, *prototype,
		nullptr, error, true))
	{
		WARNING(true, ("JavaLibrary::makeCraftedItem: error can't store prototype "
			"in container, error = %d", error));
		prototype->permanentlyDestroy(DeleteReasons::BadContainerTransfer);
		return 0;
	}

	return (prototype->getNetworkId()).getValue();
}	// JavaLibrary::makeCraftedItem

//----------------------------------------------------------------------

/**
 * Returns a draft_schematic structure filled in with info used by a manufacturing
 * schematic.
 *
 * @param env						Java environment
 * @param self						class calling this function
 * @param manufacturingSchematic	the schematic to get the data from
 *
 * @return the draft_schematic object, or nullptr on error
 */
jobject JNICALL ScriptMethodsCraftingNamespace::getSchematicData(JNIEnv *env, jobject self, jlong manufacturingSchematic)
{
	if (manufacturingSchematic == 0)
		return 0;

	const ManufactureSchematicObject * schematicObject = nullptr;
	if (!JavaLibrary::getObject(manufacturingSchematic, schematicObject))
		return 0;

	return JavaLibrary::convert(*schematicObject)->getReturnValue();
}	// JavaLibrary::getSchematicData

//----------------------------------------------------------------------

/**
 * Returns a draft_schematic structure filled in with info used by a draft
 * schematic.
 *
 * @param env				Java environment
 * @param self				class calling this function
 * @param draftSchematic	the schematic to get the data from
 *
 * @return the draft_schematic object, or nullptr on error
 */
jobject JNICALL ScriptMethodsCraftingNamespace::getDraftSchematicData(JNIEnv *env, jobject self,
	jstring draftSchematic)
{
	if (draftSchematic == 0)
		return 0;

	JavaStringParam jdraftSchematic(draftSchematic);
	std::string schematicName;
	if (!JavaLibrary::convert(jdraftSchematic, schematicName))
		return 0;

	const DraftSchematicObject * schematicObject = DraftSchematicObject::getSchematic(schematicName);
	if (schematicObject == nullptr)
		return 0;

	return JavaLibrary::convert(*schematicObject);
}	// JavaLibrary::getSchematicData

//----------------------------------------------------------------------

/**
 * Returns a draft_schematic structure filled in with info used by a draft
 * schematic.
 *
 * @param env				Java environment
 * @param self				class calling this function
 * @param draftSchematicCrc	the schematic to get the data from
 *
 * @return the draft_schematic object, or nullptr on error
 */
jobject JNICALL ScriptMethodsCraftingNamespace::getDraftSchematicDataCrc(JNIEnv *env, jobject self,
	jint draftSchematicCrc)
{
	if (draftSchematicCrc == 0)
		return 0;

	const DraftSchematicObject * schematicObject = DraftSchematicObject::getSchematic(draftSchematicCrc);
	if (schematicObject == nullptr)
		return 0;

	return JavaLibrary::convert(*schematicObject);
}	// JavaLibrary::getSchematicDataCrc

//----------------------------------------------------------------------

/**
 * Causes the stats of a crate of manufactured objects to get recomputed from the 
 * crate's objvars.
 * IMPORTANT: This only affects the stats of new objects that get pulled out of the
 * crate! It is up to the caller to modify the "examine" item stored in the crate.
 *
 * @param env		Java environment
 * @param self		class calling this function
 * @param crate		the crate to recompute the stats of
 */
void JNICALL ScriptMethodsCraftingNamespace::recomputeCrateAttributes(JNIEnv *env, jobject self, jlong crate)
{
	FactoryObject * factory = nullptr;
	if (!JavaLibrary::getObject(crate, factory))
		return;

	factory->calculateAttributes();
}

//----------------------------------------------------------------------
