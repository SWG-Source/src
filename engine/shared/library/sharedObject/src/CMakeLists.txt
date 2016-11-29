
set(SHARED_SOURCES
	shared/ObjectWatcherList.cpp
	shared/ObjectWatcherList.h

	shared/appearance/Appearance.cpp
	shared/appearance/Appearance.h
	shared/appearance/AppearanceTemplate.cpp
	shared/appearance/AppearanceTemplate.h
	shared/appearance/AppearanceTemplateList.cpp
	shared/appearance/AppearanceTemplateList.h
	shared/appearance/Hardpoint.cpp
	shared/appearance/Hardpoint.h
	shared/appearance/NoRenderAppearance.cpp
	shared/appearance/NoRenderAppearance.h
	shared/appearance/NoRenderAppearanceTemplate.cpp
	shared/appearance/NoRenderAppearanceTemplate.h
	shared/appearance/TextAppearance.cpp
	shared/appearance/TextAppearance.h
	shared/appearance/Tweakable.cpp
	shared/appearance/Tweakable.h

	shared/container/ArrangementDescriptor.cpp
	shared/container/ArrangementDescriptor.h
	shared/container/ArrangementDescriptorList.cpp
	shared/container/ArrangementDescriptorList.h
	shared/container/ContainedByProperty.cpp
	shared/container/ContainedByProperty.h
	shared/container/Container.cpp
	shared/container/Container.h
	shared/container/SlotDescriptor.cpp
	shared/container/SlotDescriptor.h
	shared/container/SlotDescriptorList.cpp
	shared/container/SlotDescriptorList.h
	shared/container/SlotIdArchive.h
	shared/container/SlotId.cpp
	shared/container/SlotId.h
	shared/container/SlotIdManager.cpp
	shared/container/SlotIdManager.h
	shared/container/SlottedContainer.cpp
	shared/container/SlottedContainer.h
	shared/container/SlottedContainmentProperty.cpp
	shared/container/SlottedContainmentProperty.h
	shared/container/SlotType.def
	shared/container/VolumeContainer.cpp
	shared/container/VolumeContainer.h
	shared/container/VolumeContainmentProperty.cpp
	shared/container/VolumeContainmentProperty.h

	shared/controller/Controller.cpp
	shared/controller/Controller.h
	shared/controller/NetworkController.cpp
	shared/controller/NetworkController.h
	shared/controller/ObjectSetMessage.def
	shared/controller/ObjectSetMessage.h

	shared/core/ConfigSharedObject.cpp
	shared/core/ConfigSharedObject.h
	shared/core/FirstSharedObject.h
	shared/core/SetupSharedObject.cpp
	shared/core/SetupSharedObject.h

	shared/customization/BasicRangedIntCustomizationVariable.cpp
	shared/customization/BasicRangedIntCustomizationVariable.h
	shared/customization/CustomizationData.cpp
	shared/customization/CustomizationData_Directory.h
	shared/customization/CustomizationData.h
	shared/customization/CustomizationData_LocalDirectory.cpp
	shared/customization/CustomizationData_LocalDirectory.h
	shared/customization/CustomizationData_RemoteDirectory.cpp
	shared/customization/CustomizationData_RemoteDirectory.h
	shared/customization/CustomizationIdManager.cpp
	shared/customization/CustomizationIdManager.h
	shared/customization/CustomizationVariable.cpp
	shared/customization/CustomizationVariable.h
	shared/customization/ObjectTemplateCustomizationDataWriter.cpp
	shared/customization/ObjectTemplateCustomizationDataWriter.h
	shared/customization/PaletteColorCustomizationVariable.cpp
	shared/customization/PaletteColorCustomizationVariable.h
	shared/customization/RangedIntCustomizationVariable.cpp
	shared/customization/RangedIntCustomizationVariable.h

	shared/dynamics/Dynamics.cpp
	shared/dynamics/Dynamics.h
	shared/dynamics/RotationDynamics.cpp
	shared/dynamics/RotationDynamics.h
	shared/dynamics/SeesawDynamics.cpp
	shared/dynamics/SeesawDynamics.h
	shared/dynamics/SimpleDynamics.cpp
	shared/dynamics/SimpleDynamics.h
	shared/dynamics/SpringDynamics.cpp
	shared/dynamics/SpringDynamics.h
	shared/dynamics/StateDynamics.cpp
	shared/dynamics/StateDynamics.h
	shared/dynamics/TrackingDynamics.cpp
	shared/dynamics/TrackingDynamics.h

	shared/lot/LotManager.cpp
	shared/lot/LotManager.h
	shared/lot/LotType.h
	shared/lot/StructureFootprint.cpp
	shared/lot/StructureFootprint.h

	shared/movement/MovementTable.cpp
	shared/movement/MovementTable.h

	shared/object/AlterResult.cpp
	shared/object/AlterResult.h
	shared/object/AlterScheduler.cpp
	shared/object/AlterScheduler.h
	shared/object/CachedNetworkIdArchive.h
	shared/object/CachedNetworkId.cpp
	shared/object/CachedNetworkId.h
	shared/object/DebugNotification.cpp
	shared/object/DebugNotification.h
	shared/object/DebugYawedFloorNotification.cpp
	shared/object/DebugYawedFloorNotification.h
	shared/object/MemoryBlockManagedObject.cpp
	shared/object/MemoryBlockManagedObject.h
	shared/object/NetworkIdManager.cpp
	shared/object/NetworkIdManager.h
	shared/object/Object.cpp
	shared/object/Object.h
	shared/object/ObjectList.cpp
	shared/object/ObjectList.h
	shared/object/ObjectNotification.cpp
	shared/object/ObjectNotification.h
	shared/object/ObjectTemplate.cpp
	shared/object/ObjectTemplate.h
	shared/object/ObjectTemplateList.cpp
	shared/object/ObjectTemplateList.h
	shared/object/ScheduleData.cpp
	shared/object/ScheduleData.h

	shared/portal/CellProperty.cpp
	shared/portal/CellProperty.h
	shared/portal/Portal.cpp
	shared/portal/Portal.h
	shared/portal/PortallizedSphereTree.h
	shared/portal/PortalProperty.cpp
	shared/portal/PortalProperty.h
	shared/portal/PortalPropertyTemplate.cpp
	shared/portal/PortalPropertyTemplate.h
	shared/portal/PortalPropertyTemplateList.cpp
	shared/portal/PortalPropertyTemplateList.h
	shared/portal/SphereGrid.h

	shared/property/CustomizationDataProperty.cpp
	shared/property/CustomizationDataProperty.h
	shared/property/LayerProperty.cpp
	shared/property/LayerProperty.h
	shared/property/Property.cpp
	shared/property/Property.h
	shared/property/PropertyId.h

	shared/world/World.cpp
	shared/world/World.h
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/FirstSharedObject.cpp
	)

	include_directories(${CMAKE_CURRENT_SOURCE_DIR}/win32)
else()
	set(PLATFORM_SOURCES "")
endif()

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/shared
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedCollision/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedDebug/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFile/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundation/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedFoundationTypes/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedGame/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedLog/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedMath/include/public
	
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedMessageDispatch/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedNetworkMessages/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedPathfinding/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedRandom/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedSynchronization/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedTerrain/include/public
	${SWG_ENGINE_SOURCE_DIR}/shared/library/sharedUtility/include/public
	${SWG_GAME_SOURCE_DIR}/shared/library/swgSharedUtility/include/public
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/archive/include
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/fileInterface/include/public
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/localization/include
	${SWG_EXTERNALS_SOURCE_DIR}/ours/library/unicode/include
)

add_library(sharedObject STATIC
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)

target_link_libraries(sharedObject
	sharedCollision
	sharedFoundation
	sharedGame
	sharedTerrain
	sharedUtility
	swgSharedUtility
)
