//========================================================================
//
// CraftingData.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/CraftingData.h"

#include "localizationArchive/StringIdArchive.h"
#include "sharedGame/SharedObjectAttributes.h"
#include <map>

//----------------------------------------------------------------------

Crafting::SimpleIngredient::SimpleIngredient(const NetworkId & _ingredient, int _count, const NetworkId & _source, int _xpType) :
ingredient(_ingredient),
count(_count),
source(_source),
xpType(_xpType)
{
}

//----------------------------------------------------------------------

Crafting::IngredientSlot::IngredientSlot() :
ingredientType(IT_invalid),
name(),
ingredients(),
complexity(-1),
draftSlotOption(-1),
draftSlotIndex(-1)
{
}

//----------------------------------------------------------------------

Crafting::IngredientSlot::IngredientSlot(enum IngredientType _ingredientType,
			   const StringId & _name,
			   const std::vector<NetworkId> &slotIngredients,
			   const std::vector<int> &slotIngredientsCount,
			   float _complexity,
			   int _draftSlotOption,
			   int _draftSlotIndex) :
ingredientType(_ingredientType),
name(_name),
ingredients(),
complexity(_complexity),
draftSlotOption(_draftSlotOption),
draftSlotIndex(_draftSlotIndex)
{
	int count = slotIngredients.size();
	for (int i = 0; i < count; ++i)
		ingredients.push_back(SimpleIngredientPtr(new SimpleIngredient(slotIngredients[i], slotIngredientsCount[i], NetworkId::cms_invalid, 0)));
}

//----------------------------------------------------------------------


bool operator==(const Crafting::SimpleIngredient & x, const Crafting::SimpleIngredient & y)
{
	if (x.ingredient != y.ingredient)
		return false;

	if (x.count != y.count)
		return false;

	return true;
}

//----------------------------------------------------------------------

bool operator==(const Crafting::IngredientSlot & x, const Crafting::IngredientSlot & y)
{
	if (x.name != y.name)
		return false;

	if (x.draftSlotOption != y.draftSlotOption)
		return false;

	if (x.complexity != y.complexity)
		return false;

	Crafting::Ingredients::const_iterator xiter, yiter;
	const Crafting::Ingredients::const_iterator xend = x.ingredients.end();
	const Crafting::Ingredients::const_iterator yend = y.ingredients.end();
	for (xiter = x.ingredients.begin(), yiter = y.ingredients.begin();
		xiter != xend && yiter != yend; ++xiter, ++yiter)
	{
		if ((*xiter)->ingredient != (*yiter)->ingredient)
			return false;

		if ((*xiter)->count != (*yiter)->count)
			return false;
	}
	if (xiter != xend || yiter != yend)
		return false;

	return true;
}

//----------------------------------------------------------------------

void Crafting::getCraftingTypeNames (uint32 types, StringVector & sv)
{
	static const std::string s_craftingTypeNames [] =
	{
		"weapon",
		"armor",
		"food",
		"clothing",
		"vehicle",
		"droid",
		"chemical",
		"plant_breeding",
		"animal_breeding",
		"furniture",
		"installation",
		"lightsaber",
		"generic_item",
		"genetics",
		"mandalorian_tailor",
		"mandalorian_armorsmith",
		"mandalorian_droid_engineer",
		"space",
		"reverse_engineering",
		"misc",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"",
		"mission"
	};

	static const size_t s_numCraftingTypeNames = sizeof (s_craftingTypeNames) / sizeof (s_craftingTypeNames [0]);

	for (size_t i = 0; i < s_numCraftingTypeNames; ++i)
	{
		uint32 bit = 1 << i;

		if ((types & bit) != 0)
			sv.push_back (s_craftingTypeNames [i]);
	}
}

//----------------------------------------------------------------------

void Crafting::getCraftingResultName (CraftingResult result, std::string & name)
{
	static const std::string s_resultName_internal_failure = "internal_failure";

	static const std::string s_resultNames [CR_numResults] =
	{
		"critical_success",
		"great_success",
		"good_success",
		"moderate_success",
		"success",
		"failure",
		"moderate_failure",
		"big_failure",
		"critical_failure"
	};

	if (result == CR_internalFailure)
		name = s_resultName_internal_failure;
	else
		name = s_resultNames [static_cast<int>(result)];
}

//----------------------------------------------------------------------

const std::string & Crafting::getResourceAttributeName (int attr)
{
	static const std::string empty;

	if (attr < 0 || attr >= RA_numResourceAttributes)
		return empty;

	static const std::string s_resourceAttribteNames [RA_numResourceAttributes] =
	{
		"res_bulk",
		"res_cold_resist",
		"res_conductivity",
		"res_decay_resist",
		"res_heat_resist",
		"res_flavor",
		"res_malleability",
		"res_potential_energy",
		"res_quality",
		"res_shock_resistance",
		"res_toughness",
		"res_volume"
	};

	return s_resourceAttribteNames [attr];
}

//----------------------------------------------------------------------

const StringId & Crafting::getResourceAttributeNameStringId (int attr)
{
	typedef std::map<int, StringId> ResourceAttributeNameStringIds;
	static ResourceAttributeNameStringIds sids;

	const ResourceAttributeNameStringIds::iterator it = sids.find (attr);

	if (it != sids.end ())
		return (*it).second;

	return (sids [attr] = StringId (SharedObjectAttributes::StringTables::Name, getResourceAttributeName (attr)));
}

//----------------------------------------------------------------------

