
// CentralConnectionServerMessages.h
// copyright 2001 Verant Interactive

#ifndef	_INCLUDED_CentralConnectionServerMessages_H
#define	_INCLUDED_CentralConnectionServerMessages_H

#include "StringId.h"
#include "Unicode.h"
#include "localizationArchive/StringIdArchive.h"
#include "serverKeyShare/KeyShare.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

class NewCentralConnectionServer : public GameNetworkMessage
{
public:
	NewCentralConnectionServer  (const std::string & address, const std::string & clientServiceAddress, const std::string & chatServiceAddress, const std::string & customerServiceAddress, const uint16 clientPortPrivate, const uint16 clientPortPublic, const uint16 gamePort, const uint16 chatPort, const uint16 customerServicePort, const uint16 pingPort, const int connectionServerNumber);
	NewCentralConnectionServer  (Archive::ReadIterator & source);
	~NewCentralConnectionServer ();

	const uint16                         getChatServicePort           () const;
	const uint16                         getCustomerServicePort       () const;
	const uint16                         getClientServicePortPrivate  () const;
	const uint16                         getClientServicePortPublic   () const;
	const uint16                         getGameServicePort           () const;
	const uint16                         getPingPort                  () const;
	const int                            getConnectionServerNumber    () const;
	const std::string &                  getChatServiceAddress        () const;
	const std::string &                  getClientServiceAddress      () const;
	const std::string &                  getGameServiceAddress        () const;
	const std::string &                  getCustomerServiceAddress    () const;

private:
	Archive::AutoVariable<uint16>          chatServicePort;
	Archive::AutoVariable<uint16>          customerServicePort;
	Archive::AutoVariable<uint16>          clientServicePortPrivate;
	Archive::AutoVariable<uint16>          clientServicePortPublic;
	Archive::AutoVariable<uint16>          gameServicePort;
	Archive::AutoVariable<uint16>          m_pingPort;
	Archive::AutoVariable<int>             m_connectionServerNumber;
	Archive::AutoVariable<std::string>     serviceAddress;
	Archive::AutoVariable<std::string>     clientServiceAddress;
	Archive::AutoVariable<std::string>     chatServiceAddress;
	Archive::AutoVariable<std::string>     customerServiceAddress;
	

	NewCentralConnectionServer();
	NewCentralConnectionServer(const NewCentralConnectionServer&);
	NewCentralConnectionServer& operator= (const NewCentralConnectionServer&);
};

//-----------------------------------------------------------------------

inline const uint16 NewCentralConnectionServer::getChatServicePort() const
{
	return chatServicePort.get();
}

//-----------------------------------------------------------------------

inline const std::string & NewCentralConnectionServer::getCustomerServiceAddress() const
{
	return customerServiceAddress.get();
}

//-----------------------------------------------------------------------

inline const std::string & NewCentralConnectionServer::getClientServiceAddress() const
{
	return clientServiceAddress.get();
}

//-----------------------------------------------------------------------

inline const uint16 NewCentralConnectionServer::getCustomerServicePort() const
{
	return customerServicePort.get();
}

//-----------------------------------------------------------------------

inline const std::string & NewCentralConnectionServer::getGameServiceAddress () const
{
	return serviceAddress.get();
}

//-----------------------------------------------------------------------

inline const uint16 NewCentralConnectionServer::getClientServicePortPrivate() const
{
	return clientServicePortPrivate.get();
}

//-----------------------------------------------------------------------

inline const uint16 NewCentralConnectionServer::getClientServicePortPublic() const
{
	return clientServicePortPublic.get();
}

//-----------------------------------------------------------------------

inline const std::string & NewCentralConnectionServer::getChatServiceAddress() const
{
	return chatServiceAddress.get();
}

//-----------------------------------------------------------------------

inline const uint16 NewCentralConnectionServer::getGameServicePort () const
{
	return gameServicePort.get();
}

//----------------------------------------------------------------------

inline const uint16 NewCentralConnectionServer::getPingPort          () const
{
	return m_pingPort.get ();
}


//----------------------------------------------------------------------

inline const int NewCentralConnectionServer::getConnectionServerNumber() const
{
	return m_connectionServerNumber.get();
}

//-----------------------------------------------------------------------

//This message is sent from Central to the conenction server when they establish a connection
//To give it an id to pass around to servers that need it.
class ConnectionServerId: public GameNetworkMessage
{
public:
	explicit ConnectionServerId(int newId);
	ConnectionServerId(Archive::ReadIterator & source);
	~ConnectionServerId();

	int getId() const;

private:
	Archive::AutoVariable<int> m_id;

	ConnectionServerId();
	ConnectionServerId(const ConnectionServerId&);
	ConnectionServerId& operator= (const ConnectionServerId&);
};

//-----------------------------------------------------------------------

inline int ConnectionServerId::getId() const
{
	return m_id.get();
}

//-----------------------------------------------------------------------

class ConnectionKeyPush : public GameNetworkMessage
{
public:
	explicit ConnectionKeyPush	(const KeyShare::Key& sourceKey);
	ConnectionKeyPush             (Archive::ReadIterator & source);
	~ConnectionKeyPush();
	const KeyShare::Key&		getKey(void) const;
private:
	AutoVariableKeyShare		key;

	ConnectionKeyPush();
	ConnectionKeyPush(const ConnectionKeyPush&);
	ConnectionKeyPush& operator= (const ConnectionKeyPush&);
};

//-----------------------------------------------------------------------

inline const KeyShare::Key & ConnectionKeyPush::getKey(void) const
{
	return key;
}
//-----------------------------------------------------------------------

class LoggedInMessage : public GameNetworkMessage
{
  public:
    LoggedInMessage  (uint32 accountNumber);
    LoggedInMessage  (Archive::ReadIterator & source);
    ~LoggedInMessage ();

    uint32 getAccountNumber(void) const;

  private:
    Archive::AutoVariable<uint32> m_accountNumber;
	
    LoggedInMessage();
    LoggedInMessage(const LoggedInMessage&);
    LoggedInMessage& operator= (const LoggedInMessage&);
};

// ----------------------------------------------------------------------

inline uint32 LoggedInMessage::getAccountNumber(void) const
{
    return m_accountNumber.get();
}

//-----------------------------------------------------------------------

class ConnEnumerateCharacterId : public GameNetworkMessage
{
public:
	ConnEnumerateCharacterId(uint32 newAccountNumber, const Unicode::String & newName, const std::string & newLocation, const std::string & newObjectTemplate);
	explicit ConnEnumerateCharacterId(Archive::ReadIterator & source);
	~ConnEnumerateCharacterId();
        
        
        const uint32            getAccountNumber  (void) const;
        const std::string &     getLocation       (void) const;
	const Unicode::String & getName           (void) const;
	const std::string &     getObjectTemplate (void) const;

private:
        Archive::AutoVariable<uint32>          accountNumber;
	Archive::AutoVariable<std::string>     location;
	Archive::AutoVariable<Unicode::String> name;
	Archive::AutoVariable<std::string>     objectTemplate;

	ConnEnumerateCharacterId();
	ConnEnumerateCharacterId(const ConnEnumerateCharacterId&);
	ConnEnumerateCharacterId& operator= (const ConnEnumerateCharacterId&);
};

//-----------------------------------------------------------------------

inline const uint32 ConnEnumerateCharacterId::getAccountNumber(void) const
{
    return accountNumber.get();
}

//-----------------------------------------------------------------------

inline const std::string & ConnEnumerateCharacterId::getLocation(void) const
{
	return location.get();
}

//-----------------------------------------------------------------------

inline const Unicode::String & ConnEnumerateCharacterId::getName(void) const
{
	return name.get();
}

//-----------------------------------------------------------------------

inline const std::string & ConnEnumerateCharacterId::getObjectTemplate(void) const
{
	return objectTemplate.get();
}

//-----------------------------------------------------------------------

class ConnectionCreateCharacter : public GameNetworkMessage
{
public:
	ConnectionCreateCharacter(uint32 stationId, const Unicode::String & characterName, const std::string & templateName, float scaleFactor, const std::string & startingLocation, const std::string & newAppeareanceData, const std::string & hairTemplateName, const std::string & hairAppearanceData, const std::string & profession, bool jedi, const Unicode::String & biography, bool useNewbieTutorial, const std::string & skillTemplate, const std::string & workingSkill, bool noRateLimit, bool isForCharacterTransfer, uint32 gameFeatures);
	explicit ConnectionCreateCharacter(Archive::ReadIterator & source);
	~ConnectionCreateCharacter();
	
	const float             getScaleFactor        () const;
	const std::string &     getAppearanceData()   const;
	const Unicode::String & getCharacterName()    const;
	const std::string &     getHairTemplateName() const;
	const std::string &     getHairAppearanceData () const;
	const std::string &     getStartingLocation() const;
	const uint32            getStationId()        const;
	const std::string &     getTemplateName()     const;
	const std::string &     getProfession()       const;
	const bool              getJedi()             const;
	const Unicode::String & getBiography()        const;
	const bool              getUseNewbieTutorial() const;
	const std::string &     getSkillTemplate()     const;
	const std::string &     getWorkingSkill()      const;
	const bool              getNoRateLimit()      const;
	const bool              getIsForCharacterTransfer() const;
	const uint32            getGameFeatures() const;
	
private:
	
	Archive::AutoVariable<std::string>           m_appearanceData;
	Archive::AutoVariable<Unicode::String>       m_characterName;
	Archive::AutoVariable<std::string>           m_templateName;
	Archive::AutoVariable<std::string>           m_startingLocation;
	Archive::AutoVariable<std::string>           m_hairTemplateName;
	Archive::AutoVariable<std::string>           m_hairAppearanceData;
	Archive::AutoVariable<uint32>                m_stationId;
	Archive::AutoVariable<std::string>           m_profession;
	Archive::AutoVariable<bool>                  m_jedi;
	Archive::AutoVariable<float>                 m_scaleFactor;
	Archive::AutoVariable<Unicode::String>       m_biography;
	Archive::AutoVariable<bool>                  m_useNewbieTutorial;
	Archive::AutoVariable<std::string>           m_skillTemplate;
	Archive::AutoVariable<std::string>           m_workingSkill;
	Archive::AutoVariable<bool>                  m_noRateLimit;
	Archive::AutoVariable<bool>                  m_isForCharacterTransfer;
	Archive::AutoVariable<uint32>                m_gameFeatures;

	ConnectionCreateCharacter();
	ConnectionCreateCharacter(const ConnectionCreateCharacter&);
	ConnectionCreateCharacter& operator= (const ConnectionCreateCharacter&);
};

//-----------------------------------------------------------------------

inline const bool ConnectionCreateCharacter::getIsForCharacterTransfer() const
{
	return m_isForCharacterTransfer.get();
}

//-----------------------------------------------------------------------

inline const std::string & ConnectionCreateCharacter::getAppearanceData(void) const
{
	return m_appearanceData.get();
}

//-----------------------------------------------------------------------

inline const Unicode::String & ConnectionCreateCharacter::getCharacterName(void) const
{
	return m_characterName.get();
}

//-----------------------------------------------------------------------

inline const std::string & ConnectionCreateCharacter::getHairTemplateName() const
{
	return m_hairTemplateName.get();
}

//-----------------------------------------------------------------------

inline const std::string & ConnectionCreateCharacter::getHairAppearanceData() const
{
	return m_hairAppearanceData.get();
}

//-----------------------------------------------------------------------

inline const std::string & ConnectionCreateCharacter::getStartingLocation(void) const
{
	return m_startingLocation.get();
}

//-----------------------------------------------------------------------
inline const uint32 ConnectionCreateCharacter::getStationId(void) const
{
    return m_stationId.get();
}
//-----------------------------------------------------------------------
inline const std::string & ConnectionCreateCharacter::getTemplateName(void) const
{
	return m_templateName.get();
}

//-----------------------------------------------------------------------

inline const std::string & ConnectionCreateCharacter::getProfession() const
{
	return m_profession.get();
}

//-----------------------------------------------------------------------

inline const bool ConnectionCreateCharacter::getJedi() const
{
	return m_jedi.get();
}

//-----------------------------------------------------------------------

inline const Unicode::String & ConnectionCreateCharacter::getBiography() const
{
	return m_biography.get();
}

//-----------------------------------------------------------------------

inline const bool ConnectionCreateCharacter::getUseNewbieTutorial() const
{
	return m_useNewbieTutorial.get();
}

// ----------------------------------------------------------------------

inline const std::string & ConnectionCreateCharacter::getSkillTemplate() const
{
	return m_skillTemplate.get();
}

//-----------------------------------------------------------------------

inline const std::string & ConnectionCreateCharacter::getWorkingSkill() const
{
	return m_workingSkill.get();
}

//-----------------------------------------------------------------------

inline const bool ConnectionCreateCharacter::getNoRateLimit() const
{
	return m_noRateLimit.get();
}

// ----------------------------------------------------------------------

inline const uint32 ConnectionCreateCharacter::getGameFeatures() const
{
	return m_gameFeatures.get();
}

//-----------------------------------------------------------------------

class ConnectionCreateCharacterSuccess : public GameNetworkMessage
{
public:
	ConnectionCreateCharacterSuccess (uint32 stationId, const NetworkId & id);
	explicit ConnectionCreateCharacterSuccess(Archive::ReadIterator & source);
	~ConnectionCreateCharacterSuccess();
	
	uint32                   getStationId () const;
	const NetworkId &        getNetworkId () const;
	
private:
	Archive::AutoVariable<uint32>                stationId;
	Archive::AutoVariable<NetworkId>             m_networkId;
	
	ConnectionCreateCharacterSuccess();
	ConnectionCreateCharacterSuccess(const ConnectionCreateCharacterSuccess & source);
	ConnectionCreateCharacterSuccess & operator = (const ConnectionCreateCharacterSuccess & rhs);
};
//-----------------------------------------------------------------------

inline uint32 ConnectionCreateCharacterSuccess::getStationId() const
{
    return stationId.get();
}

//----------------------------------------------------------------------

inline const NetworkId & ConnectionCreateCharacterSuccess::getNetworkId () const
{
	return m_networkId.get ();
}

//-----------------------------------------------------------------------

class ConnectionCreateCharacterFailed : public GameNetworkMessage
{
public:
	ConnectionCreateCharacterFailed(uint32 stationId, const Unicode::String &name, const StringId &errorMessage, const std::string &optionalDetailedErrorMessage);
	explicit ConnectionCreateCharacterFailed(Archive::ReadIterator & source);
	~ConnectionCreateCharacterFailed();
	
	const Unicode::String &getName() const;
	const StringId &getErrorMessage() const;
	const std::string &getOptionalDetailedErrorMessage() const;
	uint32      getStationId() const;
	
private:
	
	Archive::AutoVariable<Unicode::String> m_name;
	Archive::AutoVariable<StringId> m_errorMessage;
	Archive::AutoVariable<std::string> m_optionalDetailedErrorMessage;
	Archive::AutoVariable<uint32>                stationId;
	
	ConnectionCreateCharacterFailed();
	ConnectionCreateCharacterFailed(const ConnectionCreateCharacterFailed & source);
	ConnectionCreateCharacterFailed & operator = (const ConnectionCreateCharacterFailed & rhs);
};

//--------------------------------------------------------------------

inline const Unicode::String &ConnectionCreateCharacterFailed::getName() const
{
	return m_name.get();
}

//--------------------------------------------------------------------

inline const StringId &ConnectionCreateCharacterFailed::getErrorMessage() const
{
	return m_errorMessage.get();
}

//--------------------------------------------------------------------

inline const std::string &ConnectionCreateCharacterFailed::getOptionalDetailedErrorMessage() const
{
	return m_optionalDetailedErrorMessage.get();
}

//-----------------------------------------------------------------------

inline uint32 ConnectionCreateCharacterFailed::getStationId() const
{
	return stationId.get();
}

//-----------------------------------------------------------------------

class ConnectionRandomNameRequest: public GameNetworkMessage
{
public:
	explicit ConnectionRandomNameRequest(uint32 stationId, const std::string &creatureTemplate);
	explicit ConnectionRandomNameRequest(Archive::ReadIterator & source);
	~ConnectionRandomNameRequest();

	const uint32 getStationId() const;
	const std::string &getCreatureTemplate() const;
private:
	Archive::AutoVariable<uint32> m_stationId;
	Archive::AutoVariable<std::string> m_creatureTemplate;

	ConnectionRandomNameRequest(const ConnectionRandomNameRequest & source);
	ConnectionRandomNameRequest & operator = (const ConnectionRandomNameRequest & other);
};

//--------------------------------------------------------------------

inline const uint32 ConnectionRandomNameRequest::getStationId() const
{
	return m_stationId.get();
}

//--------------------------------------------------------------------

inline const std::string &ConnectionRandomNameRequest::getCreatureTemplate() const
{
	return m_creatureTemplate.get();
}

//-----------------------------------------------------------------------

class ConnectionRandomNameResponse: public GameNetworkMessage
{
public:
	explicit ConnectionRandomNameResponse(uint32 stationId, const std::string &creatureTemplate, const Unicode::String &name);
	explicit ConnectionRandomNameResponse(Archive::ReadIterator & source);
	~ConnectionRandomNameResponse();

	const uint32 getStationId() const;
	const std::string &getCreatureTemplate() const;
	const Unicode::String &getName() const;
private:
	Archive::AutoVariable<uint32>          m_stationId;
	Archive::AutoVariable<std::string>     m_creatureTemplate;
	Archive::AutoVariable<Unicode::String> m_name;

	ConnectionRandomNameResponse(const ConnectionRandomNameRequest & source);
	ConnectionRandomNameResponse & operator = (const ConnectionRandomNameRequest & other);
};

//--------------------------------------------------------------------

inline const uint32 ConnectionRandomNameResponse::getStationId() const
{
	return m_stationId.get();
}

//--------------------------------------------------------------------

inline const std::string &ConnectionRandomNameResponse::getCreatureTemplate() const
{
	return m_creatureTemplate.get();
}

//--------------------------------------------------------------------

inline const Unicode::String &ConnectionRandomNameResponse::getName() const
{
	return m_name.get();
}

// ======================================================================

#endif
