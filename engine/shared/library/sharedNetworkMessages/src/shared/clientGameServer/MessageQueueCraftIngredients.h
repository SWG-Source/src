//========================================================================
//
// MessageQueueCraftIngredients.h
//
// copyright 2001 Sony Online Entertainment, Inc.
// All rights reserved.
//
//========================================================================


#ifndef INCLUDED_MessageQueueCraftIngredients_H
#define INCLUDED_MessageQueueCraftIngredients_H

#include "sharedFoundation/MessageQueue.h"
#include "sharedGame/CraftingData.h"
#include "sharedNetworkMessages/ControllerMessageMacros.h"

class MemoryBlockManager;
class MessageQueueCraftIngredients : public MessageQueue::Data
{
	CONTROLLER_MESSAGE_INTERFACE;

public:

	typedef Crafting::IngredientType IngredientType;

	struct Ingredient
	{
		Unicode::String name;
		union
		{
			int         type;
			int         minValue;
		};
		union
		{
			int         quantity;
			int         maxValue;
		};

		Ingredient(const Unicode::String & _name, IngredientType _type, int _quantity) :
		name(_name),
		type(_type),
		quantity(_quantity)	
		{
		}

		Ingredient(const Unicode::String & _name, int _minValue, int _maxValue) :
		name(_name),
		minValue(_minValue),
		maxValue(_maxValue)
		{
		}
	};

public:

	         MessageQueueCraftIngredients(void);
	virtual ~MessageQueueCraftIngredients();

	void               addIngredient(const Unicode::String & name, IngredientType type, int quantity);
	void               addSchematicAttribute(const Unicode::String & name, int minValue, int maxValue);
	int                getIngredientCount(void) const;
	const Ingredient & getIngredient(int index) const;

private:
	std::vector<Ingredient> m_ingredients;
};


inline MessageQueueCraftIngredients::MessageQueueCraftIngredients()
{
}	// MessageQueueCraftIngredients::MessageQueueCraftIngredients()

inline void MessageQueueCraftIngredients::addIngredient(const Unicode::String & name, IngredientType type, int quantity)
{
	m_ingredients.push_back(Ingredient(name, type, quantity));
}	// MessageQueueCraftIngredients::addIngredient

inline void MessageQueueCraftIngredients::addSchematicAttribute(const Unicode::String & name, int minValue, int maxValue)
{
	m_ingredients.push_back(Ingredient(name, minValue, maxValue));
}	// MessageQueueCraftIngredients::addSchematicAttribute

inline int MessageQueueCraftIngredients::getIngredientCount(void) const
{
	return m_ingredients.size();
}	// MessageQueueCraftIngredients::getIngredientCount

inline const MessageQueueCraftIngredients::Ingredient & MessageQueueCraftIngredients::getIngredient(int index) const
{
	DEBUG_FATAL(index < 0 || index >= static_cast<int>(m_ingredients.size()), 
		("MessageQueueCraftIngredients::getIngredient index out of range"));
	return m_ingredients.at(index);
}	// MessageQueueCraftIngredients::getIngredient


#endif	// INCLUDED_MessageQueueCraftIngredients_H
