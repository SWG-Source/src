//------------------------------------------
// CentralConnectionServerMessages.cpp
// copyright 2001 Sony Online Entertainment
//------------------------------------------

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/CentralConnectionServerMessages.h"

//-----------------------------------------------------------------------

NewCentralConnectionServer::NewCentralConnectionServer(const std::string & address, const std::string & csa, const std::string & chatAddr, const std::string & customerAddr, const uint16 clientPortPrivate, const uint16 clientPortPublic, const uint16 gamePort, const uint16 chatPort, const uint16 _customerServicePort, const uint16 pingPort, const int connectionServerNumber) : 
GameNetworkMessage        ("NewCentralConnectionServer"),
chatServicePort           (chatPort),
customerServicePort       (_customerServicePort),
clientServicePortPrivate  (clientPortPrivate),
clientServicePortPublic   (clientPortPublic),
gameServicePort           (gamePort),
m_pingPort                (pingPort),
m_connectionServerNumber  (connectionServerNumber),
serviceAddress            (address),
clientServiceAddress      (csa),
chatServiceAddress        (chatAddr),
customerServiceAddress    (customerAddr)
{
	addVariable(chatServicePort);
	addVariable(customerServicePort);
	addVariable(clientServicePortPrivate);
	addVariable(clientServicePortPublic);
	addVariable(gameServicePort);
	addVariable(m_pingPort);
	addVariable(m_connectionServerNumber);
	addVariable(serviceAddress);
	addVariable(clientServiceAddress);
	addVariable(chatServiceAddress);
	addVariable(customerServiceAddress);
}

//-----------------------------------------------------------------------

NewCentralConnectionServer::NewCentralConnectionServer(Archive::ReadIterator & source) : 
GameNetworkMessage        ("NewCentralConnectionServer"),
chatServicePort           (0),
customerServicePort(0),
clientServicePortPrivate  (0),
clientServicePortPublic   (0),
gameServicePort           (0),
m_pingPort                (0),
m_connectionServerNumber  (0),
serviceAddress            (),
clientServiceAddress      (),
chatServiceAddress        (),
customerServiceAddress    ()
{
	addVariable(chatServicePort);
	addVariable(customerServicePort);
	addVariable(clientServicePortPrivate);
	addVariable(clientServicePortPublic);
	addVariable(gameServicePort);
	addVariable(m_pingPort);
	addVariable(m_connectionServerNumber);
	addVariable(serviceAddress);
	addVariable(clientServiceAddress);
	addVariable(chatServiceAddress);
	addVariable(customerServiceAddress);
	unpack(source);
}

//-----------------------------------------------------------------------

NewCentralConnectionServer::~NewCentralConnectionServer()
{
}

//-----------------------------------------------------------------------

ConnectionServerId::ConnectionServerId(int newId) :
		GameNetworkMessage("ConnectionServerId"),
		m_id(newId)
{
	addVariable(m_id);
}

//-----------------------------------------------------------------------
ConnectionServerId::ConnectionServerId(Archive::ReadIterator & source) :
		GameNetworkMessage("ConnectionServerId"),
		m_id(0)
{
	addVariable(m_id);
	unpack(source);
}

//-----------------------------------------------------------------------

ConnectionServerId::~ConnectionServerId()
{
}

//-----------------------------------------------------------------------

ConnectionKeyPush::ConnectionKeyPush(const KeyShare::Key & sourceKey) :
GameNetworkMessage("ConnectionKeyPush"),
key(sourceKey)
{
	addVariable(key);
}

//-----------------------------------------------------------------------

ConnectionKeyPush::ConnectionKeyPush(Archive::ReadIterator & source) :
GameNetworkMessage("ConnectionKeyPush"),
key()
{
	addVariable(key);
	unpack(source);
}

//-----------------------------------------------------------------------

ConnectionKeyPush::~ConnectionKeyPush()
{
}
//-----------------------------------------------------------------------

LoggedInMessage::LoggedInMessage(uint32 accountNumber) :
	GameNetworkMessage("LoggedInMessage"),
	m_accountNumber(accountNumber)
{
	addVariable(m_accountNumber);
}

//-----------------------------------------------------------------------

LoggedInMessage::LoggedInMessage(Archive::ReadIterator & source) :
		GameNetworkMessage("LoggedInMessage"),
		m_accountNumber()
{
	addVariable(m_accountNumber);
	unpack(source);
}

//-----------------------------------------------------------------------

LoggedInMessage::~LoggedInMessage()
{
}

//-----------------------------------------------------------------------

ConnEnumerateCharacterId::ConnEnumerateCharacterId(uint32 newAccountNumber, const Unicode::String & newName, const std::string & newLocation, const std::string & newObjectTemplate) :
GameNetworkMessage("ConnEnumerateCharacterId"),
accountNumber(newAccountNumber),
location(newLocation),
name(newName),
objectTemplate(newObjectTemplate)
{
    addVariable(accountNumber);
    addVariable(location);
    addVariable(name);
    addVariable(objectTemplate);
}

//-----------------------------------------------------------------------

ConnEnumerateCharacterId::ConnEnumerateCharacterId(Archive::ReadIterator & source) :
GameNetworkMessage("ConnEnumerateCharacterId"),
accountNumber(0),
location(),
name(),
objectTemplate()
{
    addVariable(accountNumber);
    addVariable(location);
    addVariable(name);
    addVariable(objectTemplate);
    unpack(source);
}

//-----------------------------------------------------------------------

ConnEnumerateCharacterId::~ConnEnumerateCharacterId()
{
}

//-----------------------------------------------------------------------
ConnectionCreateCharacter::ConnectionCreateCharacter(const uint32 newStationId, const Unicode::String & newCharacterName, const std::string & newTemplateName, float scaleFactor, const std::string & newStartingLocation, const std::string & newAppearanceData, const std::string & newHairTemplateName, const std::string & hairAppearanceData, const std::string & profession, bool jedi, const Unicode::String & biography, bool useNewbieTutorial, const std::string & skillTemplate, const std::string & workingSkill, bool noRateLimit, bool isForCharacterTransfer, uint32 gameFeatures) :
		GameNetworkMessage("ConnectionCreateCharacter"),
		m_appearanceData(newAppearanceData),
		m_characterName(newCharacterName),
		m_templateName(newTemplateName),
		m_startingLocation(newStartingLocation),
		m_hairTemplateName(newHairTemplateName),
		m_hairAppearanceData (hairAppearanceData),
		m_stationId(newStationId),
		m_profession(profession),
		m_jedi(jedi),
		m_scaleFactor(scaleFactor),
		m_biography(biography),
		m_useNewbieTutorial(useNewbieTutorial),
		m_skillTemplate(skillTemplate),
		m_workingSkill(workingSkill),
		m_noRateLimit(noRateLimit),
		m_isForCharacterTransfer(isForCharacterTransfer),
		m_gameFeatures(gameFeatures)
{
	addVariable(m_appearanceData);
	addVariable(m_characterName);
	addVariable(m_templateName);
	addVariable(m_startingLocation);
	addVariable(m_hairTemplateName);
	addVariable(m_hairAppearanceData);
	addVariable(m_stationId);
	addVariable(m_profession);
	addVariable(m_jedi);
	addVariable(m_scaleFactor);
	addVariable(m_biography);
	addVariable(m_useNewbieTutorial);
	addVariable(m_skillTemplate);
	addVariable(m_workingSkill);
	addVariable(m_noRateLimit);
	addVariable(m_isForCharacterTransfer);
	addVariable(m_gameFeatures);
}

//-----------------------------------------------------------------------

ConnectionCreateCharacter::ConnectionCreateCharacter(Archive::ReadIterator & source) :
		GameNetworkMessage("ConnectionCreateCharacter"),
		m_appearanceData(),
		m_characterName(),
		m_templateName(),
		m_startingLocation(),
		m_hairTemplateName(),
		m_hairAppearanceData (),
		m_stationId(0),
		m_profession(),
		m_jedi(),
		m_scaleFactor(1.0f),
		m_biography(),
		m_useNewbieTutorial(false),
		m_skillTemplate(),
		m_workingSkill(),
		m_noRateLimit(false),
		m_isForCharacterTransfer(),
		m_gameFeatures(0)
{
	addVariable(m_appearanceData);
	addVariable(m_characterName);
	addVariable(m_templateName);
	addVariable(m_startingLocation);
	addVariable(m_hairTemplateName);
	addVariable(m_hairAppearanceData);
	addVariable(m_stationId);
	addVariable(m_profession);
	addVariable(m_jedi);
	addVariable(m_scaleFactor);
	addVariable(m_biography);
	addVariable(m_useNewbieTutorial);
	addVariable(m_skillTemplate);
	addVariable(m_workingSkill);
	addVariable(m_noRateLimit);
	addVariable(m_isForCharacterTransfer);
	addVariable(m_gameFeatures);
	unpack(source);
}

//-----------------------------------------------------------------------

ConnectionCreateCharacter::~ConnectionCreateCharacter()
{
}

//----------------------------------------------------------------------

const float ConnectionCreateCharacter::getScaleFactor        () const
{
	return m_scaleFactor.get ();
}

//-----------------------------------------------------------------------

ConnectionCreateCharacterSuccess::ConnectionCreateCharacterSuccess(uint32 suid, const NetworkId & networkId) :
        GameNetworkMessage("ConnectionCreateCharacterSuccess"),
        stationId(suid),
		m_networkId (networkId)
{
    addVariable(stationId);
    addVariable(m_networkId);
}

//-----------------------------------------------------------------------

ConnectionCreateCharacterSuccess::ConnectionCreateCharacterSuccess(Archive::ReadIterator & source) :
        GameNetworkMessage("ConnectionCreateCharacterSuccess"),
        stationId(0),
		m_networkId ()
{
    addVariable(stationId);
    addVariable(m_networkId);
    unpack(source);
}

//-----------------------------------------------------------------------

ConnectionCreateCharacterSuccess::~ConnectionCreateCharacterSuccess()
{
}

//-----------------------------------------------------------------------

ConnectionCreateCharacterFailed::ConnectionCreateCharacterFailed(uint32 newStationId, const Unicode::String &name, const StringId &errorMessage, const std::string &optionalDetailedErrorMessage) :
        GameNetworkMessage("ConnectionCreateCharacterFailed"),
				m_name(name),
				m_errorMessage(errorMessage),
				m_optionalDetailedErrorMessage(optionalDetailedErrorMessage),
        stationId(newStationId)
{
    addVariable(stationId);
		addVariable(m_name);
		addVariable(m_errorMessage);
		addVariable(m_optionalDetailedErrorMessage);
}

//-----------------------------------------------------------------------

ConnectionCreateCharacterFailed::ConnectionCreateCharacterFailed(Archive::ReadIterator & source) :
        GameNetworkMessage("ConnectionCreateCharacterFailed"),
				m_name(),
				m_errorMessage(),
				m_optionalDetailedErrorMessage(),
        stationId(0)
{
    addVariable(stationId);
		addVariable(m_name);
		addVariable(m_errorMessage);
		addVariable(m_optionalDetailedErrorMessage);
    unpack(source);
}

//-----------------------------------------------------------------------

ConnectionCreateCharacterFailed::~ConnectionCreateCharacterFailed()
{
}

// ======================================================================





