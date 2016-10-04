//======================================================================
//
// ShipComponentDataManager.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ShipComponentDataManager.h"

#include "serverGame/ShipComponentDataArmor.h"
#include "serverGame/ShipComponentDataBooster.h"
#include "serverGame/ShipComponentDataBridge.h"
#include "serverGame/ShipComponentDataCapacitor.h"
#include "serverGame/ShipComponentDataCargoHold.h"
#include "serverGame/ShipComponentDataModification.h"
#include "serverGame/ShipComponentDataDroidInterface.h"
#include "serverGame/ShipComponentDataEngine.h"
#include "serverGame/ShipComponentDataHangar.h"
#include "serverGame/ShipComponentDataReactor.h"
#include "serverGame/ShipComponentDataShield.h"
#include "serverGame/ShipComponentDataTargetingStation.h"
#include "serverGame/ShipComponentDataWeapon.h"
#include "serverGame/TangibleObject.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedGame/ShipComponentData.h"
#include "sharedGame/ShipComponentDescriptor.h"
#include "sharedGame/ShipComponentType.h"

//======================================================================

namespace ShipComponentDataManagerNamespace
{
}

using namespace ShipComponentDataManagerNamespace;

//----------------------------------------------------------------------

ShipComponentData * ShipComponentDataManager::create (TangibleObject const & component)
{
	ShipComponentDescriptor const * const shipComponentDescriptor = 
		ShipComponentDescriptor::findShipComponentDescriptorByObjectTemplate (component.getObjectTemplate ()->getCrcName ().getCrc ());

	if (shipComponentDescriptor == nullptr)
	{
		WARNING (true, ("ShipComponentDataManager [%s] [%s] is not a component", component.getNetworkId ().getValueString ().c_str (), component.getObjectTemplateName()));
		return nullptr;
	}

	ShipComponentData * const shipComponent = create (*shipComponentDescriptor);

	if (!shipComponent)
		return nullptr;

	shipComponent->readDataFromComponent (component);
	return shipComponent;
}

//----------------------------------------------------------------------

ShipComponentData * ShipComponentDataManager::create (ShipComponentDescriptor const & shipComponentDescriptor)
{
	ShipComponentData * shipComponent = nullptr;

	switch (shipComponentDescriptor.getComponentType ())
	{
	case ShipComponentType::SCT_reactor:
			shipComponent = new ShipComponentDataReactor (shipComponentDescriptor);
			break;
	case ShipComponentType::SCT_engine:
			shipComponent = new ShipComponentDataEngine (shipComponentDescriptor);
			break;
	case ShipComponentType::SCT_shield:
			shipComponent = new ShipComponentDataShield (shipComponentDescriptor);
			break;
	case ShipComponentType::SCT_armor:
			shipComponent = new ShipComponentDataArmor (shipComponentDescriptor);
			break;
	case ShipComponentType::SCT_weapon:
			shipComponent = new ShipComponentDataWeapon (shipComponentDescriptor);
			break;
	case ShipComponentType::SCT_capacitor:
			shipComponent = new ShipComponentDataCapacitor (shipComponentDescriptor);
			break;
	case ShipComponentType::SCT_booster:
			shipComponent = new ShipComponentDataBooster (shipComponentDescriptor);
			break;
	case ShipComponentType::SCT_droid_interface:
			shipComponent = new ShipComponentDataDroidInterface (shipComponentDescriptor);
			break;
	case ShipComponentType::SCT_bridge:
			shipComponent = new ShipComponentDataBridge (shipComponentDescriptor);
			break;
	case ShipComponentType::SCT_hangar:
			shipComponent = new ShipComponentDataHangar (shipComponentDescriptor);
			break;
	case ShipComponentType::SCT_targeting_station:
			shipComponent = new ShipComponentDataTargetingStation (shipComponentDescriptor);
			break;
	case ShipComponentType::SCT_cargo_hold:
			shipComponent = new ShipComponentDataCargoHold(shipComponentDescriptor);
			break;
	case ShipComponentType::SCT_modification:
			shipComponent = new ShipComponentDataModification(shipComponentDescriptor);
			break;
	default:
		WARNING (true, ("ShipComponentDataManager::create descriptor has type [%d] invalid", shipComponentDescriptor.getComponentType ()));
		return nullptr;
	}

	return shipComponent;
}

//======================================================================
