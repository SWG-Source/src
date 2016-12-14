//========================================================================
//
// CraftingData.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef INCLUDED_CraftingData_H
#define INCLUDED_CraftingData_H

#include "sharedFoundation/NetworkId.h"
#include "StringId.h"

#include <memory>
#include <vector>

class CreatureObject;
class DraftSchematicObject;
class ServerCraftingDataTemplate;


//========================================================================

namespace Crafting
{
	// Crafting levels
	const int MAX_CRAFTING_LEVEL = 4;
	
	// level 0 = generic tool, no input hopper, no experiment, no manf schematic
	// level 1 = specific tool, no input hopper, no experiment, no manf schematic
	// level 2 = specific tool, public station, no input hopper
	// level 3 = specific tool, private station
	// level 4 = specific tool, private station, no input hopper
	
	//------------------------------------------------------------------------------

	// attributes associated with resources; also defined in craftinglib.scriptlib
	enum ResourceAttributes
	{
		RA_bulk,
		RA_coldResistance,
		RA_conductivity,
		RA_decayResistance,
		RA_heatResistance,
		RA_flavor,
		RA_malleability,
		RA_potentialEnergy,
		RA_quality,
		RA_shockResistance,
		RA_toughness,
		RA_volume,
		RA_numResourceAttributes
	};

	const std::string &     getResourceAttributeName         (int attr);
	const StringId &        getResourceAttributeNameStringId (int attr);

	//------------------------------------------------------------------------------

	enum CraftingStage
	{
		CS_none,
		CS_selectDraftSchematic,
		CS_assembly,
		CS_experiment,
		CS_customize,
		CS_finish
	};

	//------------------------------------------------------------------------------

	enum CraftingResult
	{
		CR_internalFailure = -1,
		CR_criticalSuccess = 0,
		CR_greatSuccess,
		CR_goodSuccess,
		CR_moderateSuccess,
		CR_success,
		CR_failure,
		CR_moderateFailure,
		CR_bigFailure,
		CR_criticalFailure,
		CR_criticalFailureNoDestroy,
		CR_numResults = CR_criticalFailureNoDestroy,
	};

	void getCraftingResultName (CraftingResult result, std::string & name);

	//------------------------------------------------------------------------------

	enum CraftingError
	{
		CE_success,
		CE_noOwner,
		CE_notAssemblyStage,
		CE_notCustomizeStage,
		CE_noDraftSchematic,
		CE_noCraftingTool,
		CE_noManfSchematic,
		CE_invalidSlot,
		CE_invalidSlotOption,
		CE_invalidIngredientSize,
		CE_slotFull,
		CE_invalidIngredient,
		CE_ingredientNotInInventory,
		CE_cantRemoveResource,
		CE_wrongResource,
		CE_damagedComponent,
		CE_cantTransferComponent,
		CE_wrongComponent,
		CE_noInventory,
		CE_badTargetHopper,
		CE_badTargetContainer,
		CE_emptySlot,
		CE_cantCreateResourceCrate,
		CE_emptySlotAssembly,
		CE_partialSlotAssembly,
		CE_noPrototype,
		CE_invalidCraftedName,
		CE_readOnlyDraftSchematic,
		CE_invalidBioLink,
		CE_stackedLoot
	};

	//----------------------------------------------------------------------
	// the following struct mirrors the ones in ServerObjectTemplate

	enum CraftingType
	{
		CT_weapon         = 0x00000001,
		CT_armor          = 0x00000002,
		CT_food           = 0x00000004,
		CT_clothing       = 0x00000008,
		CT_vehicle        = 0x00000010,
		CT_droid          = 0x00000020,
		CT_chemical       = 0x00000040,
		CT_plantBreeding  = 0x00000080,
		CT_animalBreeding = 0x00000100,
		CT_furniture      = 0x00000200,
		CT_installation   = 0x00000400,
		CT_lightsaber     = 0x00000800,
		CT_genericItem    = 0x00001000,
		CT_genetics       = 0x00002000,
		CT_mission        = 0x80000000,
		CraftingType_Last = CT_mission,
	};

	typedef std::vector<std::string> StringVector;
	void getCraftingTypeNames (uint32 types, StringVector & sv);

	//------------------------------------------------------------------------------

	// the following structs mirror the ones in ServerCraftingDataTemplate
	// and ServerIntangibleObjectTemplate, except that SimpleIngredient references
	// a NetworkId instead of a StringId

	enum IngredientType
	{
		IT_invalid = -1,    // uninitialized IngredientSlot
		IT_none = 0,        // no item (for empty/unused slots)
		IT_item,            // a specific item (Skywalker barrel mark V)
		IT_template,        // any item created from a template (any small_blaster_barrel)
		IT_resourceType,    // a specific resource type (iron type 5)
		IT_resourceClass,   // any resource of a resource class (ferrous metal)
		IT_templateGeneric, // same as IT_template, but if multiple components are required, they don't have to be the same exact type as the first component used
		IT_schematic,		// item crafted from a draft schematic
		IT_schematicGeneric,// same as IT_schematic, but if multiple components are required, they don't have to be the same exact type as the first component used
		IngredientType_Last = IT_schematicGeneric
	};

	//------------------------------------------------------------------------------

	class SimpleIngredient
	{
	public:

		// the ingredient is the resource type id for resources, or the manf 
		// schematic id for components
		NetworkId ingredient;
		int       count;
		NetworkId source;		// who gets experience for this ingredient
		int       xpType;		// what kind of experience to grant

				 SimpleIngredient() : ingredient(), count(0), source(), xpType(0) {}
		         SimpleIngredient(const NetworkId & _ingredient, int _count, const NetworkId & _source, int _xpType);
		virtual ~SimpleIngredient() {}
	};

	//------------------------------------------------------------------------------

	class ComponentIngredient : public SimpleIngredient
	{
	public:

		std::string     templateName;
		Unicode::String componentName;
		int             sourceXp;

				 ComponentIngredient();
		virtual ~ComponentIngredient() {}
	};

	inline ComponentIngredient::ComponentIngredient() : 
		SimpleIngredient(NetworkId::cms_invalid, 1, NetworkId::cms_invalid, 0),
		templateName(),
		componentName(),
		sourceXp(0)
	{}

	//------------------------------------------------------------------------------

	typedef std::shared_ptr<SimpleIngredient> SimpleIngredientPtr;
	typedef std::vector<SimpleIngredientPtr> Ingredients;

	//------------------------------------------------------------------------------

	struct IngredientSlot
	{
		enum IngredientType               ingredientType;
		StringId                          name;
		Ingredients                       ingredients;
		float                             complexity;
		int                               draftSlotOption;
		int                               draftSlotIndex;

		IngredientSlot();

		IngredientSlot(enum IngredientType _ingredientType,
		               const StringId & _name,
					   const std::vector<NetworkId> &slotIngredients,
					   const std::vector<int> &slotIngredientsCount,
		               float _complexity,
		               int _draftSlotOption,
		               int _draftSlotIndex);
	};

	//------------------------------------------------------------------------------

	struct CustomInfo
	{
		std::string name;       // customization property name
		int         index;      // current palette/decal index
		int         minIndex;   // minimum palette/decal index
		int         maxIndex;   // maximum palette/decal index
	};

	//------------------------------------------------------------------------------

	struct CustomValue
	{
		int property;   // index of the customization property
		int value;      // value the property

		
		CustomValue() :
			property(0),
			value(0)
		{
		}

		CustomValue(int p, int v) :
			property(p),
			value(v)
		{
		}
	};

}	// namespace Crafting

//========================================================================

extern bool operator==(const Crafting::SimpleIngredient & x, const Crafting::SimpleIngredient & y);
extern bool operator==(const Crafting::IngredientSlot & x, const Crafting::IngredientSlot & y);


//========================================================================


#endif	// INCLUDED_CraftingData_H
