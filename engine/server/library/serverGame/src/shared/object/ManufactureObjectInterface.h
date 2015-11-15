//========================================================================
//
// ManufactureObjectInterface.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef INCLUDED_ManufactureObjectInterface_H
#define INCLUDED_ManufactureObjectInterface_H

#include "serverGame/ServerIntangibleObjectTemplate.h"


namespace Crafting
{
	struct IngredientSlot;
}

//========================================================================

/**
 * Pure abstract class that must be implemented by objects that can be used to 
 * create new crafted items
 */
class ManufactureObjectInterface
{
public:
	typedef std::map<StringId, float> AttribMap;
	typedef std::vector<StringId>     StringIdList;

	virtual uint32               getDraftSchematic() const = 0;
	virtual bool                 getSlot(int index, Crafting::IngredientSlot & data, bool flag) const = 0;
	virtual int                  getSlotsCount() const = 0;
	virtual const AttribMap &    getAttributes() const = 0;
	virtual void                 setAttribute(const ServerIntangibleObjectTemplate::SchematicAttribute & attribute) = 0;
	virtual void                 setAttribute(const StringId & name, float value) = 0;
	virtual const AttribMap &    getResourceMaxAttributes() const = 0;
	virtual void                 setResourceMaxAttribute(const StringId & name, float value) = 0;
	virtual const StringIdList & getExperimentAttributeNames() const = 0;
	virtual void                 setExperimentAttribute(const StringId & name, float value) = 0;
	virtual void                 getAllExperimentAttributeValues(const StringId & name, float & value, float & minValue, float & maxValue, float & resourceMaxValue) const = 0;
	virtual void                 setExperimentAttributeLimits(const StringId & name, float minValue, float maxValue, float resourceMaxValue) = 0;
	virtual bool                 isMakingObject(void) const = 0;

	virtual const NetworkId &    getDerivedNetworkId() const = 0;
};


//========================================================================


#endif	// INCLUDED_ManufactureObjectInterface_H
