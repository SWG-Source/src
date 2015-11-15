// CentralCommandChannelMessages.cpp
// copyright 2001 Verant Interactive
// Author: Justin Randall


//-----------------------------------------------------------------------

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverKeyShare/KeyShare.h"
#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "sharedFoundation/ApplicationVersion.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

CentralGameServerConnect::CentralGameServerConnect(const std::string & newVolumeName,
												   const std::string & newClientServiceAddress,
												   const uint16 newClientServicePort,
												   const std::string & newGameServiceAddress,
												   const uint16 newGameServicePort) :
GameNetworkMessage("CentralGameServerConnect"),
clientServiceAddress(newClientServiceAddress),
gameServiceAddress(newGameServiceAddress),
volumeName(newVolumeName),
clientServicePort(newClientServicePort),
gameServicePort(newGameServicePort),
buildVersionNumber(ApplicationVersion::getInternalVersion())
{
	addVariable(clientServiceAddress);
	addVariable(gameServiceAddress);
	addVariable(volumeName);
	addVariable(clientServicePort);
	addVariable(gameServicePort);
	addVariable(buildVersionNumber);
}

//-----------------------------------------------------------------------

CentralGameServerConnect::CentralGameServerConnect(Archive::ReadIterator & source) :
GameNetworkMessage("CentralGameServerConnect"),
clientServiceAddress(""),
gameServiceAddress(""),
volumeName(""),
clientServicePort(0),
gameServicePort(0),
buildVersionNumber("")
{
	addVariable(clientServiceAddress);
	addVariable(gameServiceAddress);
	addVariable(volumeName);
	addVariable(clientServicePort);
	addVariable(gameServicePort);
	addVariable(buildVersionNumber);
	unpack(source);
}

//-----------------------------------------------------------------------

CentralGameServerConnect::~CentralGameServerConnect()
{
}

//-----------------------------------------------------------------------

CentralGameServerSetProcessId::CentralGameServerSetProcessId(const uint32 newProcessId, const uint32 newClockSubtractInterval, const std::string & c) :
GameNetworkMessage("CentralGameServerSetProcessId"),
clockSubtractInterval(newClockSubtractInterval),
clusterName(c),
processId(newProcessId)
{
	addVariable(processId);
	addVariable(clockSubtractInterval);
	addVariable(clusterName);
}

//-----------------------------------------------------------------------

CentralGameServerSetProcessId::CentralGameServerSetProcessId(Archive::ReadIterator & source) :
GameNetworkMessage("CentralGameServerSetProcessId"),
clockSubtractInterval(0),
clusterName(),
processId()
{
	addVariable(processId);
	addVariable(clockSubtractInterval);
	addVariable(clusterName);
	unpack(source);
}

//-----------------------------------------------------------------------

CentralGameServerSetProcessId::~CentralGameServerSetProcessId()
{
}

//-----------------------------------------------------------------------

CentralGameServerProxyObject::CentralGameServerProxyObject(const NetworkId &newObjectId, 
		                                                   const char * const newTemplateName,
	                                                       const uint32 newGameServerProcessId,
														   const std::string & newGameServerAddress,
														   const unsigned short newGameServerPort) :
GameNetworkMessage("CentralGameServerProxyObject"),
gameServerAddress(newGameServerAddress),
gameServerPort(newGameServerPort),
gameServerProcessId(newGameServerProcessId),
objectId(newObjectId),
templateName(newTemplateName)
{
	addVariable(gameServerAddress);
	addVariable(gameServerPort);
	addVariable(gameServerProcessId);
	addVariable(objectId);
	addVariable(templateName);
}

//-----------------------------------------------------------------------

CentralGameServerProxyObject::CentralGameServerProxyObject(Archive::ReadIterator & source) :
GameNetworkMessage("CentralGameServerProxyObject"),
gameServerAddress(),
gameServerPort(),
gameServerProcessId(),
templateName()
{
	addVariable(gameServerAddress);
	addVariable(gameServerPort);
	addVariable(gameServerProcessId);
	addVariable(objectId);
	addVariable(templateName);
	unpack(source);
}

//-----------------------------------------------------------------------

CentralGameServerProxyObject::~CentralGameServerProxyObject()
{
}

//-----------------------------------------------------------------------
CentralGameServerDbProcessServerProcessId::CentralGameServerDbProcessServerProcessId(const uint32 newServerProcessId, const uint32 newGameTime) :
GameNetworkMessage("CentralGameServerDbProcessServerProcessId"),
serverProcessId(newServerProcessId),
gameTime(newGameTime)
{
	addVariable(serverProcessId);
	addVariable(gameTime);
}

//-----------------------------------------------------------------------

CentralGameServerDbProcessServerProcessId::CentralGameServerDbProcessServerProcessId(Archive::ReadIterator & source) :
GameNetworkMessage("CentralGameServerDbProcessServerProcessId"),
serverProcessId(0)
{
	addVariable(serverProcessId);
	addVariable(gameTime);
	unpack(source);
}

//-----------------------------------------------------------------------

CentralGameServerDbProcessServerProcessId::~CentralGameServerDbProcessServerProcessId()
{
}

//-----------------------------------------------------------------------

CentralCreateCharacter::CentralCreateCharacter(const uint32 stationId, const Unicode::String & characterName, const std::string & templateName, float scaleFactor, const std::string & planetName, const Vector & coordinates, const NetworkId & cellId, const std::string & appearanceData, const std::string & hairTemplateName, const std::string & hairAppearanceData, const std::string & profession, const Unicode::String & biography, bool useNewbieTutorial, const std::string & skillTemplate, const std::string & workingSkill, bool jedi, uint32 gameFeatures) :
		GameNetworkMessage("CentralCreateCharacter"),
		m_appearanceData(appearanceData),
		m_cellId(cellId),
		m_coordinates(coordinates),
		m_characterName(characterName),
		m_planetName(planetName),
		m_stationId(stationId),
		m_hairTemplateName(hairTemplateName),
		m_hairAppearanceData (hairAppearanceData),
		m_templateName(templateName),
		m_profession(profession),
		m_scaleFactor(scaleFactor),
		m_biography(biography),
		m_useNewbieTutorial(useNewbieTutorial),
		m_skillTemplate(skillTemplate),
		m_workingSkill(workingSkill),
		m_jedi(jedi),
		m_gameFeatures(gameFeatures)
{
	addVariable(m_appearanceData);
	addVariable(m_cellId);
	addVariable(m_coordinates);
	addVariable(m_characterName);
	addVariable(m_planetName);
	addVariable(m_stationId);
	addVariable(m_hairTemplateName);
	addVariable(m_hairAppearanceData);
	addVariable(m_templateName);
	addVariable(m_profession);
	addVariable(m_scaleFactor);
	addVariable(m_biography);
	addVariable(m_useNewbieTutorial);
	addVariable(m_skillTemplate);
	addVariable(m_workingSkill);
	addVariable(m_jedi);
	addVariable(m_gameFeatures);
}

//-----------------------------------------------------------------------

CentralCreateCharacter::CentralCreateCharacter(Archive::ReadIterator & source) :
		GameNetworkMessage("CentralCreateCharacter"),
		m_appearanceData(),
		m_cellId(),
		m_coordinates(),
		m_characterName(),
		m_planetName(),
		m_stationId(),
		m_hairTemplateName(),
		m_hairAppearanceData (),
		m_templateName(),
		m_profession(),
		m_scaleFactor(1.0f),
		m_biography(),
		m_useNewbieTutorial(false),
		m_skillTemplate(),
		m_workingSkill(),
		m_jedi(false),
		m_gameFeatures(0)
{
	addVariable(m_appearanceData);
	addVariable(m_cellId);
	addVariable(m_coordinates);
	addVariable(m_characterName);
	addVariable(m_planetName);
	addVariable(m_stationId);
	addVariable(m_hairTemplateName);
	addVariable(m_hairAppearanceData);
	addVariable(m_templateName);
	addVariable(m_profession);
	addVariable(m_scaleFactor);
	addVariable(m_biography);
	addVariable(m_useNewbieTutorial);
	addVariable(m_skillTemplate);
	addVariable(m_workingSkill);
	addVariable(m_jedi);
	addVariable(m_gameFeatures);
	unpack(source);
}

//-----------------------------------------------------------------------

CentralCreateCharacter::~CentralCreateCharacter()
{
}

//----------------------------------------------------------------------

const float CentralCreateCharacter::getScaleFactor        () const
{
	return m_scaleFactor.get ();
}

//-----------------------------------------------------------------------

VerifyNameRequest::VerifyNameRequest(const uint32 stationId, const NetworkId & characterId, const std::string &creatureTemplate, const Unicode::String & characterName) :
GameNetworkMessage("VerifyNameRequest"),
m_creatureTemplate(creatureTemplate),
m_characterName(characterName),
m_stationId(stationId),
m_characterId(characterId)
{
	addVariable(m_creatureTemplate);
    addVariable(m_characterName);
    addVariable(m_stationId);
	addVariable(m_characterId);
}

//-----------------------------------------------------------------------

VerifyNameRequest::VerifyNameRequest(Archive::ReadIterator & source) :
GameNetworkMessage("VerifyNameRequest"),
m_creatureTemplate(),
m_characterName(),
m_stationId(),
m_characterId()
{
	addVariable(m_creatureTemplate);
    addVariable(m_characterName);
    addVariable(m_stationId);
	addVariable(m_characterId);

    unpack(source);
}

//-----------------------------------------------------------------------

VerifyNameRequest::~VerifyNameRequest()
{
}

//-----------------------------------------------------------------------

VerifyNameResponse::VerifyNameResponse(const uint32 stationId, const NetworkId & characterId, const std::string &creatureTemplate, const Unicode::String & characterName, const StringId &errorMessage) :
GameNetworkMessage("VerifyNameResponse"),
m_stationId(stationId),
m_characterId(characterId),
m_creatureTemplate(creatureTemplate),
m_name(characterName),
m_errorMessage(errorMessage)
{
	addVariable(m_stationId);
	addVariable(m_characterId);
	addVariable(m_creatureTemplate);
	addVariable(m_name);
	addVariable(m_errorMessage);
}

//-----------------------------------------------------------------------

VerifyNameResponse::VerifyNameResponse(Archive::ReadIterator & source) :
GameNetworkMessage("VerifyNameResponse"),
m_stationId(),
m_characterId(),
m_creatureTemplate(),
m_name(),
m_errorMessage()
{
	addVariable(m_stationId);
	addVariable(m_characterId);
	addVariable(m_creatureTemplate);
	addVariable(m_name);
	addVariable(m_errorMessage);

	unpack(source);
}

//-----------------------------------------------------------------------

VerifyNameResponse::~VerifyNameResponse()
{
}

//-----------------------------------------------------------------------

ReleaseNameMessage::ReleaseNameMessage(const uint32 stationId, const NetworkId & characterId) :
GameNetworkMessage("ReleaseNameMessage"),
m_stationId(stationId),
m_characterId(characterId)
{
    addVariable(m_stationId);
	addVariable(m_characterId);
}

//-----------------------------------------------------------------------

ReleaseNameMessage::ReleaseNameMessage(Archive::ReadIterator & source) :
GameNetworkMessage("ReleaseNameMessage"),
m_stationId(),
m_characterId()
{
    addVariable(m_stationId);
	addVariable(m_characterId);

    unpack(source);
}

//-----------------------------------------------------------------------

ReleaseNameMessage::~ReleaseNameMessage()
{
}

//----------------------------------------------------------------------
DatabaseSaveStart::DatabaseSaveStart() : GameNetworkMessage("DatabaseSaveStart")
{
}

DatabaseSaveStart::DatabaseSaveStart(Archive::ReadIterator & source) : GameNetworkMessage("DatabaseSaveStart")
{
	unpack(source);
}

DatabaseSaveStart::~DatabaseSaveStart()
{
}

//----------------------------------------------------------------------

DatabaseCreateCharacterSuccess::DatabaseCreateCharacterSuccess(const NetworkId &objectId, StationId stationId, const Unicode::String &characterName, int templateId, bool jedi) :
		GameNetworkMessage("DatabaseCreateCharacterSuccess"),
		m_objectId(objectId),
		m_stationId(stationId),
		m_characterName(characterName),
		m_templateId(templateId),
		m_jedi(jedi)
{
	addVariable(m_objectId);
	addVariable(m_stationId);
	addVariable(m_characterName);
	addVariable(m_templateId);
	addVariable(m_jedi);
}

//-----------------------------------------------------------------------

DatabaseCreateCharacterSuccess::DatabaseCreateCharacterSuccess(Archive::ReadIterator & source) :
		GameNetworkMessage("DatabaseCreateCharacterSuccess"),
		m_objectId(),
		m_stationId(),
		m_characterName(),
		m_templateId(),
		m_jedi()
{
	addVariable(m_objectId);
	addVariable(m_stationId);
	addVariable(m_characterName);
	addVariable(m_templateId);
	addVariable(m_jedi);
	unpack(source);
}

//-----------------------------------------------------------------------

DatabaseCreateCharacterSuccess::~DatabaseCreateCharacterSuccess()
{
}

//-----------------------------------------------------------------------

GameCreateCharacterFailed::GameCreateCharacterFailed(const uint32 stationId, const Unicode::String &name, const StringId &errorMessage, const std::string &optionalDetailedErrorMessage) :
GameNetworkMessage("GameCreateCharacterFailed"),
m_stationId(stationId),
m_name(name),
m_errorMessage(errorMessage),
m_optionalDetailedErrorMessage(optionalDetailedErrorMessage)
{
	addVariable(m_stationId);
	addVariable(m_name);
	addVariable(m_errorMessage);
	addVariable(m_optionalDetailedErrorMessage);
}

//-----------------------------------------------------------------------

GameCreateCharacterFailed::GameCreateCharacterFailed(Archive::ReadIterator & source) :
GameNetworkMessage("GameCreateCharacterFailed"),
m_stationId(),
m_name(),
m_errorMessage(),
m_optionalDetailedErrorMessage()
{
	addVariable(m_stationId);
	addVariable(m_name);
	addVariable(m_errorMessage);
	addVariable(m_optionalDetailedErrorMessage);

	unpack(source);
}

//-----------------------------------------------------------------------

GameCreateCharacterFailed::~GameCreateCharacterFailed()
{

}

//-----------------------------------------------------------------------

ConnectionServerAddress ::ConnectionServerAddress (const std::string & newGameAddress, const uint16 newGamePort) :
GameNetworkMessage("ConnectionServerAddress"),
gameServiceAddress(newGameAddress),
gameServicePort(newGamePort)
{
	addVariable(gameServiceAddress);
	addVariable(gameServicePort);
}

//-----------------------------------------------------------------------

ConnectionServerAddress ::ConnectionServerAddress (Archive::ReadIterator & source) :
GameNetworkMessage("ConnectionServerAddress"),
gameServiceAddress(),
gameServicePort()
{
	addVariable(gameServiceAddress);
	addVariable(gameServicePort);
	unpack(source);
}

//-----------------------------------------------------------------------

ConnectionServerAddress ::~ConnectionServerAddress ()
{
}
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
ShutdownCluster::ShutdownCluster(const uint32 timeToShutdown, const uint32 maxTime, const Unicode::String &systemMessage):
GameNetworkMessage("ShutdownCluster"),
m_timeToShutdown(timeToShutdown),
m_maxTime(maxTime),
m_systemMessage(systemMessage)
{
	addVariable(m_timeToShutdown);
	addVariable(m_maxTime);
	addVariable(m_systemMessage);
}

ShutdownCluster::ShutdownCluster( Archive::ReadIterator &source) :
GameNetworkMessage("ShutdownCluster"),
m_timeToShutdown(),
m_maxTime(),
m_systemMessage()
{
	addVariable(m_timeToShutdown);
	addVariable(m_maxTime);
	addVariable(m_systemMessage);
	unpack(source);
}

ShutdownCluster::~ShutdownCluster()
{
}

//----------------------------------------------------------------------
AbortShutdown::AbortShutdown() : GameNetworkMessage("AbortShutdown")
{
}

AbortShutdown::AbortShutdown(Archive::ReadIterator & source) : GameNetworkMessage("AbortShutdown")
{
	unpack(source);
}

AbortShutdown::~AbortShutdown()
{
}

