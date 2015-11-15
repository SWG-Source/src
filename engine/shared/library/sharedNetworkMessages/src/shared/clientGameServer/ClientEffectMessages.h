
// ======================================================================
//
// ClientEffectMessages.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ClientEffectMessages_H
#define INCLUDED_ClientEffectMessages_H

//-----------------------------------------------------------------------

#include "Archive/Archive.h"
#include "Unicode.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedMath/Transform.h"
#include "sharedMath/Vector.h"
#include "sharedMathArchive/TransformArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

class GameInByteStream;
class GameOutByteStream;

//-----------------------------------------------------------------------

/** The network message to play a client effect on a given object, and optionally on one of its hardpoints.
 */
class PlayClientEffectObjectMessage : public GameNetworkMessage
{
public:
	PlayClientEffectObjectMessage(const NetworkId& id, 
	                              const std::string& effectName, 
	                              const std::string& hardpoint,
								  const std::string& label);
	explicit PlayClientEffectObjectMessage (Archive::ReadIterator& source);

	const NetworkId&             getObjectId() const;
	const std::string&           getEffectName() const;
	const std::string&           getHardpoint() const;
	const std::string&           getLabel() const;

	static const char * const MESSAGE_TYPE;

private:
	//the name of the UI page to use
	Archive::AutoVariable<std::string> m_effectName;
	//the name of the UI page to use
	Archive::AutoVariable<std::string> m_hardpoint;
	//the object on which to play the effect (likely not the avatar)
	Archive::AutoVariable<NetworkId> m_objectId;
	//the optional label for this effect on this object which can be used to later stop this effect
	Archive::AutoVariable<std::string> m_label;
};

//-----------------------------------------------------------------------

/** The network message to play a client effect on a given object, and optionally on one of its hardpoints.
 */
class PlayClientEffectObjectTransformMessage : public GameNetworkMessage
{
public:
	PlayClientEffectObjectTransformMessage(const NetworkId& id, 
	                              const std::string& effectName, 
	                              Transform const & hardpoint,
								  const std::string& label);
	explicit PlayClientEffectObjectTransformMessage (Archive::ReadIterator& source);

	const NetworkId&             getObjectId() const;
	const std::string&           getEffectName() const;
	Transform const &            getTransform() const;
	const std::string&           getLabel() const;

	static const char * const MESSAGE_TYPE;

private:
	//the name of the UI page to use
	Archive::AutoVariable<std::string> m_effectName;
	//the name of the UI page to use
	Archive::AutoVariable<Transform> m_transform;
	//the object on which to play the effect (likely not the avatar)
	Archive::AutoVariable<NetworkId> m_objectId;
	//the optional label for this effect on this object which can be used to later stop this effect
	Archive::AutoVariable<std::string> m_label;
};

//-----------------------------------------------------------------------

/** The network message to play a client effect at a given location.
 */
class PlayClientEffectLocMessage : public GameNetworkMessage
{
public:
	PlayClientEffectLocMessage(const std::string& effectName, 
	                           const Vector& location, 
	                           const std::string &planet,
	                           const NetworkId & cell,
	                           float terrainDelta,
							   const std::string& label);
	explicit PlayClientEffectLocMessage (Archive::ReadIterator& source);

	const std::string&        getEffectName() const;
	Vector                    getLocation() const;
	const std::string&        getPlanet() const;
	const NetworkId&          getCell() const;
	float                     getTerrainDelta() const;
	const std::string&        getLabel() const;

	static const char * const MESSAGE_TYPE;

private:
	//the name of the UI page to use
	Archive::AutoVariable<std::string> m_effectName;
	//effect planet
	Archive::AutoVariable<std::string> m_planet;
	//effect world position
	Archive::AutoVariable<float>       m_locationX;
	Archive::AutoVariable<float>       m_locationY;
	Archive::AutoVariable<float>       m_locationZ;
	//effect cell
	Archive::AutoVariable<NetworkId>   m_cell;
	//height above/below terrain to play the effect
	Archive::AutoVariable<float>       m_terrainDelta;
	//the optional label for this effect on this object which can be used to later stop this effect
	Archive::AutoVariable<std::string> m_label;
};
//-----------------------------------------------------------------------

/** The network message to play an event on a given object, and optionally on one of its hardpoints.
 */
class PlayClientEventObjectMessage : public GameNetworkMessage
{
public:
	PlayClientEventObjectMessage(const NetworkId& id, 
	                             const std::string& eventName, 
	                             const std::string& hardpoint);
	explicit PlayClientEventObjectMessage (Archive::ReadIterator& source);

	const NetworkId&            getObjectId() const;
	const std::string&          getEventName() const;
	const std::string&          getHardpoint() const;

	static const char * const MESSAGE_TYPE;

private:
	//the name of the UI page to use
	Archive::AutoVariable<std::string> m_eventName;
	//the name of the UI page to use
	Archive::AutoVariable<std::string> m_hardpoint;
	//the object on which to play the event (likely not the avatar)
	Archive::AutoVariable<NetworkId>        m_objectId;
};

//-----------------------------------------------------------------------

/** The network message to play a client effect on a given object, and optionally on one of its hardpoints.
*/
class PlayClientEventObjectTransformMessage : public GameNetworkMessage
{
public:
	PlayClientEventObjectTransformMessage(const NetworkId& id, 
		const std::string& eventName, 
		Transform const & hardpoint);
	explicit PlayClientEventObjectTransformMessage (Archive::ReadIterator& source);
	
	const NetworkId&             getObjectId() const;
	const std::string&           getEventName() const;
	Transform const &            getTransform() const;
	
	static const char * const MESSAGE_TYPE;
	
private:
	//the name of the UI page to use
	Archive::AutoVariable<std::string> m_eventName;
	//the name of the UI page to use
	Archive::AutoVariable<Transform> m_transform;
	//the object on which to play the effect (likely not the avatar)
	Archive::AutoVariable<NetworkId> m_objectId;
};

//-----------------------------------------------------------------------

/** The network message to play an event at a given location.
*/
class PlayClientEventLocMessage : public GameNetworkMessage
{
public:
	PlayClientEventLocMessage(const std::string& eventSource, 
	                          const std::string& eventDest, 
	                          const Vector& position, 
	                          const std::string &planet,
	                          const NetworkId & cell,
	                          float terrainDelta);
	explicit PlayClientEventLocMessage(Archive::ReadIterator& source);

	const std::string&       getEventSourceName() const;
	const std::string&       getEventDestName() const;
	Vector                   getLocation() const;
	const std::string&       getPlanet() const;
	const NetworkId &        getCell() const;
	float                    getTerrainDelta() const;

	static const char * const MESSAGE_TYPE;

private:
	//the name of the source event type
	Archive::AutoVariable<std::string> m_eventSourceName;
	//the name of the dest event type
	Archive::AutoVariable<std::string> m_eventDestName;
	//event planet
	Archive::AutoVariable<std::string> m_planet;
	//effect world position
	Archive::AutoVariable<float>       m_locationX;
	Archive::AutoVariable<float>       m_locationY;
	Archive::AutoVariable<float>       m_locationZ;
	//effect cell position
	Archive::AutoVariable<NetworkId>   m_cell;
	//height above/below terrain to play the effect
	Archive::AutoVariable<float>       m_terrainDelta;
};

//-----------------------------------------------------------------------

/** The network message to play a client effect on a given object, and optionally on one of its hardpoints.
 */
class StopClientEffectObjectByLabelMessage : public GameNetworkMessage
{
public:
	StopClientEffectObjectByLabelMessage(const NetworkId& id, 
								  const std::string& label,
								  const bool softTerminate);
	explicit StopClientEffectObjectByLabelMessage (Archive::ReadIterator& source);

	const NetworkId&             getObjectId() const;
	const std::string&           getLabel() const;
	const bool                   getSoftTerminate() const;

	static const char * const MESSAGE_TYPE;

private:
	//the object on which is playing the effect
	Archive::AutoVariable<NetworkId> m_objectId;
	//the label for of the effect to stop
	Archive::AutoVariable<std::string> m_label;
	//whether or not to immediately kill the effect
	Archive::AutoVariable<bool> m_softTerminate;
};

//-----------------------------------------------------------------------

/** The network message to create a client projectile.
*/
class CreateClientProjectileMessage : public GameNetworkMessage
{
public:
	CreateClientProjectileMessage(const std::string& weaponObjectTemplateName, 
		Vector const & start,
		NetworkId const & startCell,
		Vector const & end,
		float speed,
		float expiration,
		bool trail,
		uint32 trailArgb);
	explicit CreateClientProjectileMessage(Archive::ReadIterator& source);

	std::string const & getWeaponObjectTemplateName() const;
	Vector getStartLocation() const;
	Vector getEndLocation() const;
	NetworkId const & getStartCell() const;
	float getSpeed() const;
	float getExpiration() const;
	bool getTrail() const;
	uint32 getTrailArgb() const;

	static char const * const MESSAGE_TYPE;

private:
	Archive::AutoVariable<std::string> m_weaponObjectTemplateName;

	// start position
	Archive::AutoVariable<float>       m_startX;
	Archive::AutoVariable<float>       m_startY;
	Archive::AutoVariable<float>       m_startZ;

	// start cell
	Archive::AutoVariable<NetworkId>   m_startCell;

	// end position
	Archive::AutoVariable<float>       m_endX;
	Archive::AutoVariable<float>       m_endY;
	Archive::AutoVariable<float>       m_endZ;

	// projectile parameters
	Archive::AutoVariable<float>       m_speed;
	Archive::AutoVariable<float>       m_expiration;
	Archive::AutoVariable<bool>        m_trail;
	Archive::AutoVariable<uint32>      m_trailArgb;
};
//-----------------------------------------------------------------------

/** The network message to create an object to object client projectile.
*/
class CreateClientProjectileObjectToObjectMessage : public GameNetworkMessage
{
public:
	CreateClientProjectileObjectToObjectMessage(const std::string& weaponObjectTemplateName, 
		NetworkId const & source,
		const std::string& sourceHardpoint,
		NetworkId const & startCell,
		NetworkId const & target,
		const std::string& targetHardpoint,
		float speed,
		float expiration,
		bool trail,
		uint32 trailArgb);
	explicit CreateClientProjectileObjectToObjectMessage(Archive::ReadIterator& source);

	std::string const & getWeaponObjectTemplateName() const;
	NetworkId const & getSourceObject() const;
	std::string const & getSourceHardpointName() const;
	NetworkId const & getTargetObject() const;
	std::string const & getTargetHardpointName() const;
	NetworkId const & getStartCell() const;
	float getSpeed() const;
	float getExpiration() const;
	bool getTrail() const;
	uint32 getTrailArgb() const;

	static char const * const MESSAGE_TYPE;

private:
	Archive::AutoVariable<std::string> m_weaponObjectTemplateName;

	// Source Object NetworkId
	Archive::AutoVariable<NetworkId>       m_sourceId;

	// Optional Source hardpoint name
	Archive::AutoVariable<std::string>     m_sourceHardpoint;

	// start cell
	Archive::AutoVariable<NetworkId>       m_startCell;

	// Target Object NetworkId
	Archive::AutoVariable<NetworkId>       m_targetId;

	// Optional Target hardpoint name
	Archive::AutoVariable<std::string>     m_targetHardpoint;

	// projectile parameters
	Archive::AutoVariable<float>           m_speed;
	Archive::AutoVariable<float>           m_expiration;
	Archive::AutoVariable<bool>            m_trail;
	Archive::AutoVariable<uint32>          m_trailArgb;
};
//-----------------------------------------------------------------------

/** The network message to create an location to object client projectile.
*/
class CreateClientProjectileLocationToObjectMessage : public GameNetworkMessage
{
public:
	CreateClientProjectileLocationToObjectMessage(const std::string& weaponObjectTemplateName, 
		Vector const & startLocation,
		NetworkId const & startCell,
		NetworkId const & target,
		const std::string& targetHardpoint,
		float speed,
		float expiration,
		bool trail,
		uint32 trailArgb);
	explicit CreateClientProjectileLocationToObjectMessage(Archive::ReadIterator& source);

	std::string const & getWeaponObjectTemplateName() const;
	Vector const & getStartLocation() const;
	NetworkId const & getTargetObject() const;
	std::string const & getTargetHardpointName() const;
	NetworkId const & getStartCell() const;

	float getSpeed() const;
	float getExpiration() const;
	bool getTrail() const;
	uint32 getTrailArgb() const;

	static char const * const MESSAGE_TYPE;

private:
	Archive::AutoVariable<std::string> m_weaponObjectTemplateName;

	// Source Location
	Archive::AutoVariable<Vector>          m_startLocation;

	// start cell
	Archive::AutoVariable<NetworkId>       m_startCell;

	// Target Object NetworkId
	Archive::AutoVariable<NetworkId>       m_targetId;

	// Optional Target hardpoint name
	Archive::AutoVariable<std::string>     m_targetHardpoint;

	// projectile parameters
	Archive::AutoVariable<float>           m_speed;
	Archive::AutoVariable<float>           m_expiration;
	Archive::AutoVariable<bool>            m_trail;
	Archive::AutoVariable<uint32>          m_trailArgb;
};
//-----------------------------------------------------------------------
/** The network message to create an object to location client projectile.
*/
class CreateClientProjectileObjectToLocationMessage : public GameNetworkMessage
{
public:
	CreateClientProjectileObjectToLocationMessage(const std::string& weaponObjectTemplateName, 
		NetworkId const & source,
		const std::string& sourceHardpoint,
		NetworkId const & startCell,
		Vector const & targetLocation,
		float speed,
		float expiration,
		bool trail,
		uint32 trailArgb);
	explicit CreateClientProjectileObjectToLocationMessage(Archive::ReadIterator& source);

	std::string const & getWeaponObjectTemplateName() const;
	NetworkId const & getSourceObject() const;
	std::string const & getSourceHardpointName() const;
	NetworkId const & getStartCell() const;
	Vector const & getTargetLocation() const;

	float getSpeed() const;
	float getExpiration() const;
	bool getTrail() const;
	uint32 getTrailArgb() const;

	static char const * const MESSAGE_TYPE;

private:
	Archive::AutoVariable<std::string> m_weaponObjectTemplateName;

	// Source Object NetworkId
	Archive::AutoVariable<NetworkId>       m_sourceId;

	// Optional Source hardpoint name
	Archive::AutoVariable<std::string>     m_sourceHardpoint;

	// start cell
	Archive::AutoVariable<NetworkId>       m_startCell;

	// Target Location
	Archive::AutoVariable<Vector>          m_targetLocation;

	// projectile parameters
	Archive::AutoVariable<float>           m_speed;
	Archive::AutoVariable<float>           m_expiration;
	Archive::AutoVariable<bool>            m_trail;
	Archive::AutoVariable<uint32>          m_trailArgb;
};
//-----------------------------------------------------------------------

#endif
