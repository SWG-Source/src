// CentralGameServerMessages.h
// copyright 2001 Verant Interactive
// Author: Justin Randall

#ifndef	_INCLUDED_CentralGameServerMessages_H
#define	_INCLUDED_CentralGameServerMessages_H

//-----------------------------------------------------------------------

#include "Archive/Archive.h"
#include "StringId.h"
#include "Unicode.h"
#include "localizationArchive/StringIdArchive.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/StationId.h"
#include "sharedMath/Vector.h"
#include "sharedMathArchive/VectorArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

class CentralGameServerConnect : public GameNetworkMessage
{
public:
	CentralGameServerConnect	(	const std::string & newVolumeName, 
	                                const std::string & newClientServiceAddress, 
									const uint16 newClientServicePort,
									const std::string & newGameServiceAddress,
									const uint16 newGameServicePort);
	CentralGameServerConnect    (Archive::ReadIterator & source);
	~CentralGameServerConnect	();
	const std::string & getClientServiceAddress		(void) const;
	const uint16		getClientServicePort		(void) const;
	const std::string &	getGameServiceAddress		(void) const;
	const uint16		getGameServicePort			(void) const;
	const std::string &	getVolumeName				(void) const;
	const std::string & getBuildVersionNumber       (void) const;
private:
	Archive::AutoVariable<std::string>      clientServiceAddress;
	Archive::AutoVariable<std::string>      gameServiceAddress;
	Archive::AutoVariable<std::string>      volumeName;
	Archive::AutoVariable<uint16>           clientServicePort;
	Archive::AutoVariable<uint16>           gameServicePort;
	Archive::AutoVariable<std::string>      buildVersionNumber;

	CentralGameServerConnect();
	CentralGameServerConnect(const CentralGameServerConnect&);
	CentralGameServerConnect& operator= (const CentralGameServerConnect&);
};

//-----------------------------------------------------------------------

inline const std::string & CentralGameServerConnect::getClientServiceAddress(void) const
{
	return clientServiceAddress.get(); //lint !e1037 // const op () const is hardly ambiguous
}

//-----------------------------------------------------------------------

inline const uint16 CentralGameServerConnect::getClientServicePort(void) const
{
	return clientServicePort.get(); //lint !e1037 // const op () const is hardly ambiguous
}

//-----------------------------------------------------------------------

inline const std::string & CentralGameServerConnect::getGameServiceAddress(void) const
{
	return gameServiceAddress.get(); //lint !e1037 // const op () const is hardly ambiguous
}

//-----------------------------------------------------------------------

inline const uint16 CentralGameServerConnect::getGameServicePort(void) const
{
	return gameServicePort.get(); //lint !e1037 // const op () const is hardly ambiguous
}

//-----------------------------------------------------------------------

inline const std::string & CentralGameServerConnect::getVolumeName(void) const
{
	return volumeName.get(); //lint !e1037 // const op () const is hardly ambiguous
}

//-----------------------------------------------------------------------

inline const std::string & CentralGameServerConnect::getBuildVersionNumber(void) const
{
	return buildVersionNumber.get(); //lint !e1037 // const op () const is hardly ambiguous
}

//-----------------------------------------------------------------------
/**
	sent from the central server to a game server process advising it
	of the server's new process identifier in the cluster
*/
class CentralGameServerSetProcessId : public GameNetworkMessage
{
public:
	explicit CentralGameServerSetProcessId(const uint32 newProcessId, const uint32 clockSubtractInterval, const std::string &newClusterName);

	CentralGameServerSetProcessId(Archive::ReadIterator & source);
	~CentralGameServerSetProcessId();

	const uint32       getClockSubtractInterval  () const;
	const std::string &getClusterName            () const;
	const uint32	   getProcessId              () const;

private:
	Archive::AutoVariable<uint32> clockSubtractInterval;
	Archive::AutoVariable<std::string> clusterName;
	Archive::AutoVariable<uint32> processId;

	CentralGameServerSetProcessId();
	CentralGameServerSetProcessId(const CentralGameServerSetProcessId&);
	CentralGameServerSetProcessId& operator= (const CentralGameServerSetProcessId&);
};

//-----------------------------------------------------------------------

inline const uint32 CentralGameServerSetProcessId::getClockSubtractInterval() const
{
	return clockSubtractInterval.get();
}

//-----------------------------------------------------------------------

inline const std::string &CentralGameServerSetProcessId::getClusterName(void) const
{
	return clusterName.get();
}

//-----------------------------------------------------------------------

inline const uint32 CentralGameServerSetProcessId::getProcessId(void) const
{
	return processId.get(); //lint !e1037 // const op () const is hardly ambiguous
}

//-----------------------------------------------------------------------

class CentralGameServerProxyObject : public GameNetworkMessage
{
public:
	CentralGameServerProxyObject(const NetworkId &newObjectId, 
		                         const char * const newTemplateName,
	                             const uint32 newGameServerProcessId,
								 const std::string & newGameServerAddress,
								 const unsigned short newGameServerPort);
	CentralGameServerProxyObject(Archive::ReadIterator & source);
	~CentralGameServerProxyObject();

	const std::string &  getGameServerAddress() const;
	const uint16         getGameServerPort() const;
	const uint32         getGameServerProcessId() const;
	const NetworkId      getObjectId() const;
	const std::string &  getTemplateName() const;
private:
	Archive::AutoVariable<std::string>   gameServerAddress;
	Archive::AutoVariable<uint16>        gameServerPort;
	Archive::AutoVariable<uint32>        gameServerProcessId;
	Archive::AutoVariable<NetworkId>           objectId;
	Archive::AutoVariable<std::string>   templateName;

	CentralGameServerProxyObject();
	CentralGameServerProxyObject(const CentralGameServerProxyObject&);
	CentralGameServerProxyObject& operator= (const CentralGameServerProxyObject&);
};

//-----------------------------------------------------------------------

inline const std::string & CentralGameServerProxyObject::getGameServerAddress(void) const
{
	return gameServerAddress.get(); //lint !e1037 // const op () const is hardly ambiguous
}

//-----------------------------------------------------------------------

inline const uint16 CentralGameServerProxyObject::getGameServerPort() const
{
	return gameServerPort.get(); //lint !e1037 // const op () const is hardly ambiguous
}

//-----------------------------------------------------------------------

inline const uint32 CentralGameServerProxyObject::getGameServerProcessId(void) const
{
	return gameServerProcessId.get(); //lint !e1037 // const op () const is hardly ambiguous
}

//-----------------------------------------------------------------------

inline const NetworkId CentralGameServerProxyObject::getObjectId(void) const
{
	return objectId.get();
}

//-----------------------------------------------------------------------

inline const std::string & CentralGameServerProxyObject::getTemplateName(void) const
{
	return templateName.get();
}

//-----------------------------------------------------------------------

class CentralGameServerDbProcessServerProcessId : public GameNetworkMessage
{
public:
	CentralGameServerDbProcessServerProcessId(const uint32 newServerProcessId, const uint32 gameTime);
	CentralGameServerDbProcessServerProcessId(Archive::ReadIterator & source);
	~CentralGameServerDbProcessServerProcessId();

	const uint32  getSubtractInterval () const;
	const uint32  getServerProcessId  () const;
private:
	Archive::AutoVariable<uint32>  serverProcessId;
	Archive::AutoVariable<uint32>  gameTime;

	CentralGameServerDbProcessServerProcessId();
	CentralGameServerDbProcessServerProcessId(const CentralGameServerDbProcessServerProcessId&);
	CentralGameServerDbProcessServerProcessId& operator= (const CentralGameServerDbProcessServerProcessId&);
};

//-----------------------------------------------------------------------

inline const uint32 CentralGameServerDbProcessServerProcessId::getServerProcessId(void) const
{
	return serverProcessId.get();
}

//-----------------------------------------------------------------------

inline const uint32 CentralGameServerDbProcessServerProcessId::getSubtractInterval() const
{
	return gameTime.get();
}

//-----------------------------------------------------------------------

class CentralCreateCharacter : public GameNetworkMessage 
{
public:
	CentralCreateCharacter(const uint32 stationId, 
	                       const Unicode::String & characterName, 
	                       const std::string & templateName, 
	                       float scaleFactor, 
	                       const std::string & planetName, 
	                       const Vector & coordinates, 
	                       const NetworkId & cellId, 
	                       const std::string & newAppearanceData, 
	                       const std::string & hairTemplateName, 
	                       const std::string & hairAppearanceData, 
	                       const std::string & profession,
						   const Unicode::String & biography,
						   const bool useNewbieTutorial,
						   const std::string & skillTemplate,
						   const std::string & workingSkill,
						   const bool jedi,
						   const uint32 gameFeatures);

	CentralCreateCharacter(Archive::ReadIterator & source);
	~CentralCreateCharacter();
	
	const float             getScaleFactor         () const;
	const std::string &     getAppearanceData      () const;
	const NetworkId &       getCell                () const;
	const Unicode::String & getCharacterName       () const;
	const Vector &          getCoordinates         () const;
	const std::string &     getHairTemplateName    () const;
	const std::string &     getHairAppearanceData  () const;
	const std::string &     getPlanetName          () const;
	const uint32            getStationId           () const;
	const std::string &     getTemplateName        () const;
	const std::string &     getProfession          () const;
	const Unicode::String & getBiography           () const;
	const bool              getUseNewbieTutorial   () const;
	const std::string &     getSkillTemplate       () const;
	const std::string &     getWorkingSkill        () const;
	const bool              getJedi                () const;
	const uint32            getGameFeatures        () const;

private:
	Archive::AutoVariable<std::string>      m_appearanceData;
	Archive::AutoVariable<NetworkId>        m_cellId;
	Archive::AutoVariable<Vector>           m_coordinates;
	Archive::AutoVariable<Unicode::String>  m_characterName;
	Archive::AutoVariable<std::string>      m_planetName;
	Archive::AutoVariable<uint32>           m_stationId;
	Archive::AutoVariable<std::string>      m_hairTemplateName;
	Archive::AutoVariable<std::string>      m_hairAppearanceData;
	Archive::AutoVariable<std::string>      m_templateName;
	Archive::AutoVariable<std::string>      m_profession;
	Archive::AutoVariable<float>            m_scaleFactor;
	Archive::AutoVariable<Unicode::String>  m_biography;
	Archive::AutoVariable<bool>             m_useNewbieTutorial;
	Archive::AutoVariable<std::string>      m_skillTemplate;
	Archive::AutoVariable<std::string>      m_workingSkill;
	Archive::AutoVariable<bool>             m_jedi;
	Archive::AutoVariable<uint32>           m_gameFeatures;
	
	CentralCreateCharacter();
	CentralCreateCharacter(const CentralCreateCharacter&);
	CentralCreateCharacter& operator= (const CentralCreateCharacter&);
};
//-----------------------------------------------------------------------

inline const std::string & CentralCreateCharacter::getAppearanceData() const
{
    return m_appearanceData.get();
}

//-----------------------------------------------------------------------

inline const NetworkId & CentralCreateCharacter::getCell() const
{
	return m_cellId.get();
}

//-----------------------------------------------------------------------

inline const Vector & CentralCreateCharacter::getCoordinates() const
{
	return m_coordinates.get();
}

//-----------------------------------------------------------------------

inline const Unicode::String & CentralCreateCharacter::getCharacterName() const
{
	return m_characterName.get();
}

//-----------------------------------------------------------------------

inline const std::string & CentralCreateCharacter::getHairTemplateName() const
{
	return m_hairTemplateName.get();
}

//-----------------------------------------------------------------------

inline const std::string & CentralCreateCharacter::getHairAppearanceData() const
{
	return m_hairAppearanceData.get();
}

//-----------------------------------------------------------------------

inline const std::string & CentralCreateCharacter::getPlanetName() const
{
	return m_planetName.get();
}

//-----------------------------------------------------------------------

inline const uint32 CentralCreateCharacter::getStationId() const
{
	return m_stationId.get();
}

//-----------------------------------------------------------------------

inline const std::string & CentralCreateCharacter::getTemplateName() const
{
	return m_templateName.get();
}

//-----------------------------------------------------------------------

inline const std::string & CentralCreateCharacter::getProfession() const
{
	return m_profession.get();
}

//-----------------------------------------------------------------------

inline const Unicode::String & CentralCreateCharacter::getBiography() const
{
	return m_biography.get();
}

//-----------------------------------------------------------------------

inline const bool CentralCreateCharacter::getUseNewbieTutorial() const
{
	return m_useNewbieTutorial.get();
}

//-----------------------------------------------------------------------

inline const std::string & CentralCreateCharacter::getSkillTemplate() const
{
	return m_skillTemplate.get();
}

//-----------------------------------------------------------------------

inline const std::string & CentralCreateCharacter::getWorkingSkill() const
{
	return m_workingSkill.get();
}

//-----------------------------------------------------------------------

inline const bool CentralCreateCharacter::getJedi() const
{
	return m_jedi.get();
}

// ----------------------------------------------------------------------

inline const uint32 CentralCreateCharacter::getGameFeatures() const
{
	return m_gameFeatures.get();
}

//-----------------------------------------------------------------------

class VerifyNameRequest: public GameNetworkMessage 
{
public:
	VerifyNameRequest(const uint32 stationId, const NetworkId & characterId, const std::string &creatureTemplate, const Unicode::String & characterName);
	VerifyNameRequest(Archive::ReadIterator & source);
	~VerifyNameRequest();

	const std::string &getCreatureTemplate() const;
	const Unicode::String & getCharacterName() const;
	const uint32            getStationId() const;
	const NetworkId &       getCharacterId() const;

private:
	Archive::AutoVariable<std::string>      m_creatureTemplate;
	Archive::AutoVariable<Unicode::String>  m_characterName;
	Archive::AutoVariable<uint32>           m_stationId;

	// 0 means requesting to verify name for character creation; otherwise, it means
	// requesting to verify name for character rename for that particular character
	Archive::AutoVariable<NetworkId>        m_characterId;

	VerifyNameRequest();
	VerifyNameRequest(const VerifyNameRequest&);
	VerifyNameRequest& operator= (const VerifyNameRequest&);
};

//-----------------------------------------------------------------------

inline const std::string &VerifyNameRequest::getCreatureTemplate() const
{
	return m_creatureTemplate.get();
}

//-----------------------------------------------------------------------

inline const Unicode::String & VerifyNameRequest::getCharacterName() const
{
	return m_characterName.get();
}

//-----------------------------------------------------------------------

inline const uint32 VerifyNameRequest::getStationId() const
{
	return m_stationId.get();
}

//-----------------------------------------------------------------------

inline const NetworkId & VerifyNameRequest::getCharacterId() const
{
	return m_characterId.get();
}

//-----------------------------------------------------------------------

class VerifyNameResponse: public GameNetworkMessage 
{
	// See NameErrors.h for acceptable error codes
public:
	VerifyNameResponse(const uint32 stationId, const NetworkId & characterId, const std::string &creatureTemplate, const Unicode::String & characterName, const StringId &errorMessage);
	VerifyNameResponse(Archive::ReadIterator & source);
	~VerifyNameResponse();
	
	const uint32 getStationId() const;
	const NetworkId & getCharacterId() const;
	const std::string &getCreatureTemplate() const;
	const Unicode::String & getName() const;
	const StringId &getErrorMessage() const;

private:
	Archive::AutoVariable<uint32>           m_stationId;
	Archive::AutoVariable<NetworkId>        m_characterId;
	Archive::AutoVariable<std::string>      m_creatureTemplate;
	Archive::AutoVariable<Unicode::String>  m_name;
	Archive::AutoVariable<StringId>         m_errorMessage;
	
	VerifyNameResponse();
	VerifyNameResponse(const VerifyNameResponse&);
	VerifyNameResponse& operator= (const VerifyNameResponse&);
};

//--------------------------------------------------------------------

inline const std::string &VerifyNameResponse::getCreatureTemplate() const
{
	return m_creatureTemplate.get();
}

//-----------------------------------------------------------------------

inline const uint32 VerifyNameResponse::getStationId() const
{
	return m_stationId.get();
}

//-----------------------------------------------------------------------

inline const NetworkId & VerifyNameResponse::getCharacterId() const
{
	return m_characterId.get();
}

//-----------------------------------------------------------------------

inline const Unicode::String & VerifyNameResponse::getName() const
{
	return m_name.get();
}

//-----------------------------------------------------------------------

inline const StringId &VerifyNameResponse::getErrorMessage() const
{
	return m_errorMessage.get();
}

//-----------------------------------------------------------------------

class ReleaseNameMessage: public GameNetworkMessage 
{
public:
	ReleaseNameMessage(const uint32 stationId, const NetworkId & characterId); // if name is empty, release all names
	ReleaseNameMessage(Archive::ReadIterator & source);
	~ReleaseNameMessage();
	
	const uint32            getStationId() const;
	const NetworkId &       getCharacterId() const;

private:
	Archive::AutoVariable<uint32>           m_stationId;
	Archive::AutoVariable<NetworkId>        m_characterId;

	ReleaseNameMessage();
	ReleaseNameMessage(const ReleaseNameMessage&);
	ReleaseNameMessage& operator= (const ReleaseNameMessage&);
};

//-----------------------------------------------------------------------

inline const uint32 ReleaseNameMessage::getStationId() const
{
	return m_stationId.get();
}

//-----------------------------------------------------------------------

inline const NetworkId & ReleaseNameMessage::getCharacterId() const
{
	return m_characterId.get();
}

//-----------------------------------------------------------------------
//  This message is sent from Persister to Central Server to indicate 
//  that a save is starting.
class DatabaseSaveStart : public GameNetworkMessage
{
public:
	DatabaseSaveStart();
	DatabaseSaveStart(Archive::ReadIterator & source);
	~DatabaseSaveStart();
};

// ----------------------------------------------------------------------

class DatabaseCreateCharacterSuccess : public GameNetworkMessage
{
public:
	DatabaseCreateCharacterSuccess(const NetworkId &objectId, StationId stationId, const Unicode::String &characterName, int templateId, bool jedi);
	DatabaseCreateCharacterSuccess(Archive::ReadIterator & source);
	~DatabaseCreateCharacterSuccess();

	const NetworkId &        getObjectId() const;
	const StationId          getStationId() const;
	const Unicode::String &  getCharacterName() const;
	const int                getTemplateId() const;
	const bool               getJedi() const;

private:
	Archive::AutoVariable<NetworkId>                m_objectId;
	Archive::AutoVariable<StationId>                m_stationId;
	Archive::AutoVariable<Unicode::String>          m_characterName;
	Archive::AutoVariable<int>                      m_templateId;
	Archive::AutoVariable<bool>                     m_jedi;

	DatabaseCreateCharacterSuccess();
	DatabaseCreateCharacterSuccess(const DatabaseCreateCharacterSuccess&);
	DatabaseCreateCharacterSuccess& operator= (const DatabaseCreateCharacterSuccess&);
};

//-----------------------------------------------------------------------

inline const NetworkId & DatabaseCreateCharacterSuccess::getObjectId(void) const
{
	return m_objectId.get();
}

// ----------------------------------------------------------------------

inline const StationId DatabaseCreateCharacterSuccess::getStationId(void) const
{
	return m_stationId.get();
}

// ----------------------------------------------------------------------

inline const Unicode::String & DatabaseCreateCharacterSuccess::getCharacterName() const
{
	return m_characterName.get();
}

// ----------------------------------------------------------------------

inline const int DatabaseCreateCharacterSuccess::getTemplateId() const
{
	return m_templateId.get();
}

// ----------------------------------------------------------------------

inline const bool DatabaseCreateCharacterSuccess:: getJedi() const
{
	return m_jedi.get();
}

//-----------------------------------------------------------------------

class GameCreateCharacterFailed : public GameNetworkMessage
{
public:
	explicit GameCreateCharacterFailed(const uint32 stationId, const Unicode::String &name, const StringId &errorCode, const std::string &optionalDetailedErrorMessage);
	GameCreateCharacterFailed(Archive::ReadIterator & source);
	~GameCreateCharacterFailed();

	const uint32 getStationId(void) const;
	const Unicode::String &getName(void) const;
	const StringId &getErrorMessage(void) const;
	const std::string &getOptionalDetailedErrorMessage() const;
private:
	Archive::AutoVariable<uint32> m_stationId;
	Archive::AutoVariable<Unicode::String> m_name;
	Archive::AutoVariable<StringId>  m_errorMessage;
	Archive::AutoVariable<std::string> m_optionalDetailedErrorMessage;
	
	GameCreateCharacterFailed();
	GameCreateCharacterFailed(const GameCreateCharacterFailed&);
	GameCreateCharacterFailed& operator= (const GameCreateCharacterFailed&);
};

//-----------------------------------------------------------------------

inline const uint32 GameCreateCharacterFailed::getStationId(void) const
{
	return m_stationId.get();
}

//-----------------------------------------------------------------------

inline const Unicode::String &GameCreateCharacterFailed::getName(void) const
{
	return m_name.get();
}

//-----------------------------------------------------------------------

inline const StringId &GameCreateCharacterFailed::getErrorMessage(void) const
{
	return m_errorMessage.get();
}

//--------------------------------------------------------------------

inline const std::string &GameCreateCharacterFailed::getOptionalDetailedErrorMessage() const
{
	return m_optionalDetailedErrorMessage.get();
}

//-----------------------------------------------------------------------

/**
* the ConnectionServerAddress message is sent from central to game when a user server connects to the login server so the game can establish a connection with it.
*/

class ConnectionServerAddress : public GameNetworkMessage
{
public:
	ConnectionServerAddress (const std::string & newGameAddress, const uint16 newGamePort);
	ConnectionServerAddress (Archive::ReadIterator & source);
    virtual ~ConnectionServerAddress ();

    const std::string &                  getGameServiceAddress	(void) const;
    const uint16                         getGameServicePort	(void) const;

private:
	Archive::AutoVariable<std::string>   gameServiceAddress;
	Archive::AutoVariable<uint16>        gameServicePort;

	ConnectionServerAddress(const ConnectionServerAddress&);
	ConnectionServerAddress& operator= (const ConnectionServerAddress&);
};
//-----------------------------------------------------------------------

inline const std::string & ConnectionServerAddress::getGameServiceAddress (void) const
{
	return gameServiceAddress.get();
}

//-----------------------------------------------------------------------

inline const uint16 ConnectionServerAddress::getGameServicePort (void) const
{
	return gameServicePort.get();
}

//-----------------------------------------------------------------------

// Send this message to CentralServer to begin the shutdown sequence (broadcasts to the players, database save, etc)
class ShutdownCluster : public GameNetworkMessage
{
public:
	ShutdownCluster( const uint32 timeToShutdown, const uint32 maxTime, const Unicode::String &systemMessage);
	ShutdownCluster(Archive::ReadIterator &source);
	virtual ~ShutdownCluster();

	const uint32			getTimeToShutdown(void) const;
	const uint32			getMaxTime(void) const;
	const Unicode::String		getSystemMessage(void) const;

private:
	Archive::AutoVariable<uint32>			m_timeToShutdown;
	Archive::AutoVariable<uint32>			m_maxTime;
	Archive::AutoVariable<Unicode::String>		m_systemMessage;

	ShutdownCluster();
	ShutdownCluster( const ShutdownCluster& );
	ShutdownCluster& operator= (const ShutdownCluster& );
};

inline const uint32 ShutdownCluster::getTimeToShutdown(void) const
{
	return m_timeToShutdown.get();
}

inline const uint32 ShutdownCluster::getMaxTime(void) const
{
	return m_maxTime.get();
}

inline const Unicode::String ShutdownCluster::getSystemMessage(void) const
{
	return m_systemMessage.get();
}

//------------------------------------------------------------------------
//  This message is sent to Central Server to stop any shutdown sequence that is in progress
class AbortShutdown : public GameNetworkMessage
{
public:
	AbortShutdown();
	AbortShutdown(Archive::ReadIterator & source);
	~AbortShutdown();
};



#endif	// _INCLUDED_CentralGameServerMessages_H
