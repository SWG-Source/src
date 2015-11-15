// ======================================================================
//
// ClientEffectMessages.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ClientEffectMessages.h"

#include "Archive/AutoByteStream.h"
#include "Archive/ByteStream.h"
#include "UnicodeUtils.h"
#include "sharedFoundation/NetworkId.h"
#include "unicodeArchive/UnicodeArchive.h"

// ======================================================================

const char * const PlayClientEffectObjectMessage::MESSAGE_TYPE = "PlayClientEffectObjectMessage";
const char * const PlayClientEffectObjectTransformMessage::MESSAGE_TYPE = "PlayClientEffectObjectTransformMessage";
const char * const PlayClientEffectLocMessage::MESSAGE_TYPE    = "PlayClientEffectLocMessage";
const char * const StopClientEffectObjectByLabelMessage::MESSAGE_TYPE     = "StopClientEffectObjectByLabelMessage";
const char * const PlayClientEventObjectMessage::MESSAGE_TYPE  = "PlayClientEventObjectMessage";
const char * const PlayClientEventObjectTransformMessage::MESSAGE_TYPE = "PlayClientEventObjectTransformMessage";
const char * const PlayClientEventLocMessage::MESSAGE_TYPE     = "PlayClientEventLocMessage";
const char * const CreateClientProjectileMessage::MESSAGE_TYPE = "CreateClientProjectileMessage";
const char * const CreateClientProjectileObjectToObjectMessage::MESSAGE_TYPE = "CreateClientProjectileObjectToObjectMessage";
const char * const CreateClientProjectileLocationToObjectMessage::MESSAGE_TYPE = "CreateClientProjectileLocationToObjectMessage";
const char * const CreateClientProjectileObjectToLocationMessage::MESSAGE_TYPE = "CreateClientProjectileObjectToLocationMessage";

// ======================================================================

PlayClientEffectObjectMessage::PlayClientEffectObjectMessage(const NetworkId& id, const std::string& effectName, const std::string& hardpoint, const std::string& label)
: GameNetworkMessage(MESSAGE_TYPE),
  m_effectName(effectName),
  m_hardpoint(hardpoint),
  m_objectId(id),
  m_label(label)
{
	addVariable(m_effectName);
	addVariable(m_hardpoint);
	addVariable(m_objectId);
	addVariable(m_label);
}

//-----------------------------------------------------------------------

PlayClientEffectObjectMessage::PlayClientEffectObjectMessage(Archive::ReadIterator& source)
: GameNetworkMessage(MESSAGE_TYPE),
  m_effectName(),
  m_hardpoint(),
  m_objectId(),
  m_label()
{
	addVariable (m_effectName);
	addVariable (m_hardpoint);
	addVariable (m_objectId);
	addVariable (m_label);
	unpack (source);
}

//-----------------------------------------------------------------------

const NetworkId&        PlayClientEffectObjectMessage::getObjectId() const
{
	return m_objectId.get();
}

//-----------------------------------------------------------------------

const std::string&      PlayClientEffectObjectMessage::getEffectName() const
{
	return m_effectName.get();
}

//-----------------------------------------------------------------------

const std::string&      PlayClientEffectObjectMessage::getHardpoint() const
{
	return m_hardpoint.get();
}

const std::string&      PlayClientEffectObjectMessage::getLabel() const
{
	return m_label.get();
}

//----------------------------------------------------------------------
//-- PlayClientEffectObjectTransformMessage
//----------------------------------------------------------------------

PlayClientEffectObjectTransformMessage::PlayClientEffectObjectTransformMessage(const NetworkId& id,
																			   const std::string& effectName,
																			   Transform const & transform,
																			   const std::string& label) :
GameNetworkMessage(MESSAGE_TYPE),
m_effectName(effectName),
m_transform(transform),
m_objectId(id),
m_label(label)
{
	addVariable(m_effectName);
	addVariable(m_transform);
	addVariable(m_objectId);
	addVariable(m_label);
}

//----------------------------------------------------------------------

PlayClientEffectObjectTransformMessage::PlayClientEffectObjectTransformMessage (Archive::ReadIterator& source) :
GameNetworkMessage(MESSAGE_TYPE),
m_effectName(),
m_transform(),
m_objectId(),
m_label()
{
	addVariable(m_effectName);
	addVariable(m_transform);
	addVariable(m_objectId);
	addVariable(m_label);
	unpack(source);
}

//----------------------------------------------------------------------

NetworkId const & PlayClientEffectObjectTransformMessage::getObjectId() const
{
	return m_objectId.get();
}

//----------------------------------------------------------------------

std::string const & PlayClientEffectObjectTransformMessage::getEffectName() const
{
	return m_effectName.get();
}

//----------------------------------------------------------------------

Transform const & PlayClientEffectObjectTransformMessage::getTransform() const
{
	return m_transform.get();
}

//----------------------------------------------------------------------

const std::string & PlayClientEffectObjectTransformMessage::getLabel() const
{
	return m_label.get();
}

// ======================================================================

PlayClientEffectLocMessage::PlayClientEffectLocMessage(const std::string& effectName, const Vector& location, const std::string &planet, const NetworkId & cell, float terrainDelta, const std::string& label)
: GameNetworkMessage(MESSAGE_TYPE),
  m_effectName(effectName),
  m_planet(planet),
  m_locationX(location.x),
  m_locationY(location.y),
  m_locationZ(location.z),
  m_cell(cell),
  m_terrainDelta(terrainDelta),
  m_label(label)
{
	addVariable(m_effectName);
	addVariable(m_planet);
	addVariable(m_locationX);
	addVariable(m_locationY);
	addVariable(m_locationZ);
	addVariable(m_cell);
	addVariable(m_terrainDelta);
	addVariable(m_label);
}

//-----------------------------------------------------------------------

PlayClientEffectLocMessage::PlayClientEffectLocMessage(Archive::ReadIterator& source)
: GameNetworkMessage(MESSAGE_TYPE),
  m_effectName(),
  m_planet(),
  m_locationX(),
  m_locationY(),
  m_locationZ(),
  m_cell(),
  m_terrainDelta(),
  m_label()
{
	addVariable (m_effectName);
	addVariable (m_planet);
	addVariable (m_locationX);
	addVariable (m_locationY);
	addVariable (m_locationZ);
	addVariable (m_cell);
	addVariable (m_terrainDelta);
	addVariable (m_label);
	unpack (source);
}

//-----------------------------------------------------------------------

const std::string&      PlayClientEffectLocMessage::getEffectName() const
{
	return m_effectName.get();
}

//-----------------------------------------------------------------------

Vector                  PlayClientEffectLocMessage::getLocation() const
{
	return Vector(m_locationX.get(), m_locationY.get(), m_locationZ.get());
}

//-----------------------------------------------------------------------

const NetworkId &      PlayClientEffectLocMessage::getCell() const
{
	return m_cell.get();
}

//-----------------------------------------------------------------------

const std::string&      PlayClientEffectLocMessage::getPlanet() const
{
	return m_planet.get();
}

//-----------------------------------------------------------------------

float                   PlayClientEffectLocMessage::getTerrainDelta() const
{
	return m_terrainDelta.get();
}

//-----------------------------------------------------------------------

const std::string&      PlayClientEffectLocMessage::getLabel() const
{
	return m_label.get();
}

// ======================================================================

PlayClientEventObjectMessage::PlayClientEventObjectMessage(const NetworkId& id, const std::string& eventName, const std::string& hardpoint)
: GameNetworkMessage(MESSAGE_TYPE),
  m_eventName(eventName),
  m_hardpoint(hardpoint),
  m_objectId(id)
{
	addVariable(m_eventName);
	addVariable(m_hardpoint);
	addVariable(m_objectId);
}

//-----------------------------------------------------------------------

PlayClientEventObjectMessage::PlayClientEventObjectMessage(Archive::ReadIterator& source)
: GameNetworkMessage(MESSAGE_TYPE),
  m_eventName(),
  m_hardpoint(),
  m_objectId()
{
	addVariable (m_eventName);
	addVariable (m_hardpoint);
	addVariable (m_objectId);
	unpack (source);
}

//-----------------------------------------------------------------------

const NetworkId&        PlayClientEventObjectMessage::getObjectId() const
{
	return m_objectId.get();
}

//-----------------------------------------------------------------------

const std::string&      PlayClientEventObjectMessage::getEventName() const
{
	return m_eventName.get();
}

//-----------------------------------------------------------------------

const std::string&      PlayClientEventObjectMessage::getHardpoint() const
{
	return m_hardpoint.get();
}

//----------------------------------------------------------------------
//-- PlayClientEventObjectTransformMessage
//----------------------------------------------------------------------

PlayClientEventObjectTransformMessage::PlayClientEventObjectTransformMessage(const NetworkId& id,
																			   const std::string& eventName,
																			   Transform const & transform) :
GameNetworkMessage(MESSAGE_TYPE),
m_eventName(eventName),
m_transform(transform),
m_objectId(id)
{
	addVariable(m_eventName);
	addVariable(m_transform);
	addVariable(m_objectId);
}

//----------------------------------------------------------------------

PlayClientEventObjectTransformMessage::PlayClientEventObjectTransformMessage (Archive::ReadIterator& source) :
GameNetworkMessage(MESSAGE_TYPE),
m_eventName(),
m_transform(),
m_objectId()
{
	addVariable(m_eventName);
	addVariable(m_transform);
	addVariable(m_objectId);
	unpack(source);
}

//----------------------------------------------------------------------

NetworkId const & PlayClientEventObjectTransformMessage::getObjectId() const
{
	return m_objectId.get();
}

//----------------------------------------------------------------------

std::string const & PlayClientEventObjectTransformMessage::getEventName() const
{
	return m_eventName.get();
}

//----------------------------------------------------------------------

Transform const & PlayClientEventObjectTransformMessage::getTransform() const
{
	return m_transform.get();
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
// ======================================================================

PlayClientEventLocMessage::PlayClientEventLocMessage(const std::string& eventSource, const std::string& eventDest, const Vector& location, const std::string &planet, const NetworkId & cell, float terrainDelta)
: GameNetworkMessage(MESSAGE_TYPE),
  m_eventSourceName(eventSource),
  m_eventDestName(eventDest),
  m_planet(planet),
  m_locationX(location.x),
  m_locationY(location.y),
  m_locationZ(location.z),
  m_cell(cell),
  m_terrainDelta(terrainDelta)
{
	addVariable(m_eventSourceName);
	addVariable(m_eventDestName);
	addVariable(m_planet);
	addVariable(m_locationX);
	addVariable(m_locationY);
	addVariable(m_locationZ);
	addVariable(m_cell);
	addVariable(m_terrainDelta);
}

//-----------------------------------------------------------------------

PlayClientEventLocMessage::PlayClientEventLocMessage(Archive::ReadIterator& source)
: GameNetworkMessage(MESSAGE_TYPE),
  m_eventSourceName(),
  m_eventDestName(),
  m_planet(),
  m_locationX(),
  m_locationY(),
  m_locationZ(),
  m_cell(),
  m_terrainDelta()
{
	addVariable (m_eventSourceName);
	addVariable (m_eventDestName);
	addVariable (m_planet);
	addVariable (m_locationX);
	addVariable (m_locationY);
	addVariable (m_locationZ);
	addVariable (m_cell);
	addVariable (m_terrainDelta);
	unpack (source);
}

//-----------------------------------------------------------------------

const std::string&      PlayClientEventLocMessage::getEventSourceName() const
{
	return m_eventSourceName.get();
}

//-----------------------------------------------------------------------

const std::string&      PlayClientEventLocMessage::getEventDestName() const
{
	return m_eventDestName.get();
}

//-----------------------------------------------------------------------

Vector                  PlayClientEventLocMessage::getLocation() const
{
	return Vector(m_locationX.get(), m_locationY.get(), m_locationZ.get());
}

//-----------------------------------------------------------------------

const std::string&      PlayClientEventLocMessage::getPlanet() const
{
	return m_planet.get();
}

//-----------------------------------------------------------------------

const NetworkId &      PlayClientEventLocMessage::getCell() const
{
	return m_cell.get();
}

//-----------------------------------------------------------------------

float                  PlayClientEventLocMessage::getTerrainDelta() const
{
	return m_terrainDelta.get();
}

//-----------------------------------------------------------------------

StopClientEffectObjectByLabelMessage::StopClientEffectObjectByLabelMessage(const NetworkId& id, const std::string& label, const bool softTerminate)
: GameNetworkMessage(MESSAGE_TYPE),
  m_objectId(id),
  m_label(label),
  m_softTerminate(softTerminate)
{
	addVariable(m_objectId);
	addVariable(m_label);
	addVariable (m_softTerminate);
}

//-----------------------------------------------------------------------

StopClientEffectObjectByLabelMessage::StopClientEffectObjectByLabelMessage(Archive::ReadIterator& source)
: GameNetworkMessage(MESSAGE_TYPE),
  m_objectId(),
  m_label(),
  m_softTerminate()
{
	addVariable (m_objectId);
	addVariable (m_label);
	addVariable (m_softTerminate);
	unpack (source);
}

//-----------------------------------------------------------------------

const NetworkId&        StopClientEffectObjectByLabelMessage::getObjectId() const
{
	return m_objectId.get();
}

//-----------------------------------------------------------------------

const std::string&      StopClientEffectObjectByLabelMessage::getLabel() const
{
	return m_label.get();
}

//-----------------------------------------------------------------------

const bool              StopClientEffectObjectByLabelMessage::getSoftTerminate() const
{
	return m_softTerminate.get();
}

// ======================================================================

CreateClientProjectileMessage::CreateClientProjectileMessage(const std::string & weaponObjectTemplateName, Vector const & start,
	NetworkId const & startCell, Vector const & end, float speed, float expiration, bool trail, uint32 trailArgb)
: GameNetworkMessage(MESSAGE_TYPE),
m_weaponObjectTemplateName(weaponObjectTemplateName),
m_startX(start.x),
m_startY(start.y),
m_startZ(start.z),
m_startCell(startCell),
m_endX(end.x),
m_endY(end.y),
m_endZ(end.z),
m_speed(speed),
m_expiration(expiration),
m_trail(trail),
m_trailArgb(trailArgb)
{
	addVariable(m_weaponObjectTemplateName);
	addVariable(m_startX);
	addVariable(m_startY);
	addVariable(m_startZ);
	addVariable(m_startCell);
	addVariable(m_endX);
	addVariable(m_endY);
	addVariable(m_endZ);
	addVariable(m_speed);
	addVariable(m_expiration);
	addVariable(m_trail);
	addVariable(m_trailArgb);
}

//-----------------------------------------------------------------------

CreateClientProjectileMessage::CreateClientProjectileMessage(Archive::ReadIterator& source)
: GameNetworkMessage(MESSAGE_TYPE),
m_weaponObjectTemplateName(),
m_startX(),
m_startY(),
m_startZ(),
m_startCell(),
m_endX(),
m_endY(),
m_endZ(),
m_speed(),
m_expiration(),
m_trail(),
m_trailArgb()
{
	addVariable(m_weaponObjectTemplateName);
	addVariable(m_startX);
	addVariable(m_startY);
	addVariable(m_startZ);
	addVariable(m_startCell);
	addVariable(m_endX);
	addVariable(m_endY);
	addVariable(m_endZ);
	addVariable(m_speed);
	addVariable(m_expiration);
	addVariable(m_trail);
	addVariable(m_trailArgb);
	unpack(source);
}

//-----------------------------------------------------------------------

std::string const & CreateClientProjectileMessage::getWeaponObjectTemplateName() const
{
	return m_weaponObjectTemplateName.get();
}

//-----------------------------------------------------------------------

Vector CreateClientProjectileMessage::getStartLocation() const
{
	return Vector(m_startX.get(), m_startY.get(), m_startZ.get());
}

//-----------------------------------------------------------------------

Vector CreateClientProjectileMessage::getEndLocation() const
{
	return Vector(m_endX.get(), m_endY.get(), m_endZ.get());
}

//-----------------------------------------------------------------------

NetworkId const & CreateClientProjectileMessage::getStartCell() const
{
	return m_startCell.get();
}

//-----------------------------------------------------------------------

float CreateClientProjectileMessage::getSpeed() const
{
	return m_speed.get();
}

//-----------------------------------------------------------------------

float CreateClientProjectileMessage::getExpiration() const
{
	return m_expiration.get();
}

//-----------------------------------------------------------------------

bool CreateClientProjectileMessage::getTrail() const
{
	return m_trail.get();
}

//-----------------------------------------------------------------------

uint32 CreateClientProjectileMessage::getTrailArgb() const
{
	return m_trailArgb.get();
}

//-----------------------------------------------------------------------

CreateClientProjectileObjectToObjectMessage::CreateClientProjectileObjectToObjectMessage(const std::string& weaponObjectTemplateName, NetworkId const & source, std::string const & sourceHardpoint, NetworkId const & startCell, NetworkId const & target, std::string const & targetHardpoint, float speed, float expiration, bool trail, uint32 trailArgb):
GameNetworkMessage(MESSAGE_TYPE),
m_weaponObjectTemplateName (weaponObjectTemplateName),
m_sourceId (source),
m_sourceHardpoint (sourceHardpoint),
m_startCell (startCell),
m_targetId (target),
m_targetHardpoint (targetHardpoint),
m_speed (speed),
m_expiration (expiration),
m_trail (trail),
m_trailArgb (trailArgb)
{
	addVariable(m_weaponObjectTemplateName);
	addVariable(m_sourceId);
	addVariable(m_sourceHardpoint);
	addVariable(m_targetId);
	addVariable(m_targetHardpoint);
	addVariable(m_startCell);
	addVariable(m_speed);
	addVariable(m_expiration);
	addVariable(m_trail);
	addVariable(m_trailArgb);

}

//-----------------------------------------------------------------------

CreateClientProjectileObjectToObjectMessage::CreateClientProjectileObjectToObjectMessage(Archive::ReadIterator& source)
: GameNetworkMessage(MESSAGE_TYPE),
m_weaponObjectTemplateName (),
m_sourceId (),
m_sourceHardpoint (),
m_startCell (),
m_targetId (),
m_targetHardpoint (),
m_speed (),
m_expiration (),
m_trail (),
m_trailArgb ()
{
	addVariable(m_weaponObjectTemplateName);
	addVariable(m_sourceId);
	addVariable(m_sourceHardpoint);
	addVariable(m_targetId);
	addVariable(m_targetHardpoint);
	addVariable(m_startCell);
	addVariable(m_speed);
	addVariable(m_expiration);
	addVariable(m_trail);
	addVariable(m_trailArgb);
	unpack(source);
}


//-----------------------------------------------------------------------

std::string const & CreateClientProjectileObjectToObjectMessage::getWeaponObjectTemplateName() const
{
	return m_weaponObjectTemplateName.get();
}

//-----------------------------------------------------------------------

NetworkId const & CreateClientProjectileObjectToObjectMessage::getSourceObject() const
{
	return m_sourceId.get();
}

//-----------------------------------------------------------------------

std::string const & CreateClientProjectileObjectToObjectMessage::getSourceHardpointName() const
{
	return m_sourceHardpoint.get();
}

//-----------------------------------------------------------------------

NetworkId const & CreateClientProjectileObjectToObjectMessage::getTargetObject() const
{
	return m_targetId.get();
}

//-----------------------------------------------------------------------

std::string const & CreateClientProjectileObjectToObjectMessage::getTargetHardpointName() const
{
	return m_targetHardpoint.get();
}

//-----------------------------------------------------------------------

NetworkId const & CreateClientProjectileObjectToObjectMessage::getStartCell() const
{
	return m_startCell.get();
}

//-----------------------------------------------------------------------

float CreateClientProjectileObjectToObjectMessage::getSpeed() const
{
	return m_speed.get();
}

//-----------------------------------------------------------------------

float CreateClientProjectileObjectToObjectMessage::getExpiration() const
{
	return m_expiration.get();
}

//-----------------------------------------------------------------------

bool CreateClientProjectileObjectToObjectMessage::getTrail() const
{
	return m_trail.get();
}

//-----------------------------------------------------------------------

uint32 CreateClientProjectileObjectToObjectMessage::getTrailArgb() const
{
	return m_trailArgb.get();
}

//-----------------------------------------------------------------------
CreateClientProjectileLocationToObjectMessage::CreateClientProjectileLocationToObjectMessage(const std::string& weaponObjectTemplateName, Vector const & startLocation, NetworkId const & startCell, NetworkId const & target, std::string const & targetHardpoint, float speed, float expiration, bool trail, uint32 trailArgb):
GameNetworkMessage(MESSAGE_TYPE),
m_weaponObjectTemplateName (weaponObjectTemplateName),
m_startLocation (startLocation),
m_startCell (startCell),
m_targetId (target),
m_targetHardpoint (targetHardpoint),
m_speed (speed),
m_expiration (expiration),
m_trail (trail),
m_trailArgb (trailArgb)
{
	addVariable(m_weaponObjectTemplateName);
	addVariable(m_startLocation);
	addVariable(m_targetId);
	addVariable(m_targetHardpoint);
	addVariable(m_startCell);
	addVariable(m_speed);
	addVariable(m_expiration);
	addVariable(m_trail);
	addVariable(m_trailArgb);

}

//-----------------------------------------------------------------------

CreateClientProjectileLocationToObjectMessage::CreateClientProjectileLocationToObjectMessage(Archive::ReadIterator& source)
: GameNetworkMessage(MESSAGE_TYPE),
m_weaponObjectTemplateName (),
m_startLocation (),
m_startCell (),
m_targetId (),
m_targetHardpoint (),
m_speed (),
m_expiration (),
m_trail (),
m_trailArgb ()
{
	addVariable(m_weaponObjectTemplateName);
	addVariable(m_startLocation);
	addVariable(m_targetId);
	addVariable(m_targetHardpoint);
	addVariable(m_startCell);
	addVariable(m_speed);
	addVariable(m_expiration);
	addVariable(m_trail);
	addVariable(m_trailArgb);
	unpack(source);
}


//-----------------------------------------------------------------------

std::string const & CreateClientProjectileLocationToObjectMessage::getWeaponObjectTemplateName() const
{
	return m_weaponObjectTemplateName.get();
}

//-----------------------------------------------------------------------

Vector const & CreateClientProjectileLocationToObjectMessage::getStartLocation() const
{
	return m_startLocation.get();
}

//-----------------------------------------------------------------------

NetworkId const & CreateClientProjectileLocationToObjectMessage::getTargetObject() const
{
	return m_targetId.get();
}

//-----------------------------------------------------------------------

std::string const & CreateClientProjectileLocationToObjectMessage::getTargetHardpointName() const
{
	return m_targetHardpoint.get();
}

//-----------------------------------------------------------------------

NetworkId const & CreateClientProjectileLocationToObjectMessage::getStartCell() const
{
	return m_startCell.get();
}

//-----------------------------------------------------------------------

float CreateClientProjectileLocationToObjectMessage::getSpeed() const
{
	return m_speed.get();
}

//-----------------------------------------------------------------------

float CreateClientProjectileLocationToObjectMessage::getExpiration() const
{
	return m_expiration.get();
}

//-----------------------------------------------------------------------

bool CreateClientProjectileLocationToObjectMessage::getTrail() const
{
	return m_trail.get();
}

//-----------------------------------------------------------------------

uint32 CreateClientProjectileLocationToObjectMessage::getTrailArgb() const
{
	return m_trailArgb.get();
}
//-----------------------------------------------------------------------
CreateClientProjectileObjectToLocationMessage::CreateClientProjectileObjectToLocationMessage(const std::string& weaponObjectTemplateName, NetworkId const & source, std::string const & sourceHardpoint, NetworkId const & startCell, Vector const & targetLocation, float speed, float expiration, bool trail, uint32 trailArgb):
GameNetworkMessage(MESSAGE_TYPE),
m_weaponObjectTemplateName (weaponObjectTemplateName),
m_sourceId (source),
m_sourceHardpoint (sourceHardpoint),
m_startCell (startCell),
m_targetLocation (targetLocation),
m_speed (speed),
m_expiration (expiration),
m_trail (trail),
m_trailArgb (trailArgb)
{
	addVariable(m_weaponObjectTemplateName);
	addVariable(m_sourceId);
	addVariable(m_sourceHardpoint);
	addVariable(m_startCell);
	addVariable(m_targetLocation);
	addVariable(m_speed);
	addVariable(m_expiration);
	addVariable(m_trail);
	addVariable(m_trailArgb);
}

//-----------------------------------------------------------------------

CreateClientProjectileObjectToLocationMessage::CreateClientProjectileObjectToLocationMessage(Archive::ReadIterator& source)
: GameNetworkMessage(MESSAGE_TYPE),
m_weaponObjectTemplateName (),
m_sourceId (),
m_sourceHardpoint (),
m_startCell (),
m_targetLocation (),
m_speed (),
m_expiration (),
m_trail (),
m_trailArgb ()
{
	addVariable(m_weaponObjectTemplateName);
	addVariable(m_sourceId);
	addVariable(m_sourceHardpoint);
	addVariable(m_startCell);
	addVariable(m_targetLocation);
	addVariable(m_speed);
	addVariable(m_expiration);
	addVariable(m_trail);
	addVariable(m_trailArgb);
	unpack(source);
}


//-----------------------------------------------------------------------

std::string const & CreateClientProjectileObjectToLocationMessage::getWeaponObjectTemplateName() const
{
	return m_weaponObjectTemplateName.get();
}

//-----------------------------------------------------------------------

Vector const & CreateClientProjectileObjectToLocationMessage::getTargetLocation() const
{
	return m_targetLocation.get();
}

//-----------------------------------------------------------------------

NetworkId const & CreateClientProjectileObjectToLocationMessage::getSourceObject() const
{
	return m_sourceId.get();
}

//-----------------------------------------------------------------------

std::string const & CreateClientProjectileObjectToLocationMessage::getSourceHardpointName() const
{
	return m_sourceHardpoint.get();
}

//-----------------------------------------------------------------------

NetworkId const & CreateClientProjectileObjectToLocationMessage::getStartCell() const
{
	return m_startCell.get();
}

//-----------------------------------------------------------------------

float CreateClientProjectileObjectToLocationMessage::getSpeed() const
{
	return m_speed.get();
}

//-----------------------------------------------------------------------

float CreateClientProjectileObjectToLocationMessage::getExpiration() const
{
	return m_expiration.get();
}

//-----------------------------------------------------------------------

bool CreateClientProjectileObjectToLocationMessage::getTrail() const
{
	return m_trail.get();
}

//-----------------------------------------------------------------------

uint32 CreateClientProjectileObjectToLocationMessage::getTrailArgb() const
{
	return m_trailArgb.get();
}
//-----------------------------------------------------------------------
