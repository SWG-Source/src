// ClientCentralMessages.cpp
// copyright 2001 Verant Interactive
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "Archive/ByteStream.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedNetworkMessages/ClientCentralMessages.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

//-- todo add SetupSharedNetworkMessages.* and ConfigSharedNetworkMessages.*
#define ConfigVersion ConfigFile::getKeyString("SharedNetworkMessages", "version", 0)

ClientIdMsg::ClientIdMsg(const unsigned char * const newToken, const uint16 newTokenSize, uint32 gameBitsToClear) :
GameNetworkMessage("ClientIdMsg"),
m_gameBitsToClear(gameBitsToClear),
token(),
version(ConfigVersion ? ConfigVersion : NetworkVersionId),  //-- todo production build should not allow this
tokenData(0)
{
	WARNING(ConfigVersion, ("Network version override detected: %s", ConfigVersion));

	tokenData = new unsigned char[newTokenSize];
	memcpy(tokenData, newToken, newTokenSize);

	std::vector<unsigned char> a;
	for(unsigned int i = 0; i < newTokenSize; ++i)
	{
		a.push_back(newToken[i]);
	}
	token.set(a);
	addVariable(m_gameBitsToClear);
	addVariable(token);
	addVariable(version);
}

//-----------------------------------------------------------------------

ClientIdMsg::ClientIdMsg(Archive::ReadIterator & source) :
GameNetworkMessage("ClientIdMsg"),
m_gameBitsToClear(0),
token(),
version(),
tokenData(0)
{
	addVariable(m_gameBitsToClear);
	addVariable(token);
	addVariable(version);
	AutoByteStream::unpack(source);
	unsigned int tokenSize = token.get().size();
	tokenData = new unsigned char[tokenSize];
	const std::vector<unsigned char> & a = token.get();
	for(unsigned int i = 0; i < tokenSize; i ++)
	{
		tokenData[i] = a[i];
	}
}

//-----------------------------------------------------------------------

ClientIdMsg::~ClientIdMsg()
{
	delete[] tokenData;
}

// ======================================================================

EnumerateCharacterId::EnumerateCharacterId(const std::vector<Chardata> &data) :
		GameNetworkMessage("EnumerateCharacterId"),
		m_data()
{
	m_data.set(data);
	addVariable(m_data);
}

// ----------------------------------------------------------------------

EnumerateCharacterId::EnumerateCharacterId(Archive::ReadIterator & source) :
		GameNetworkMessage("EnumerateCharacterId"),
		m_data()
{				
	addVariable(m_data);
	unpack(source);
}

//-----------------------------------------------------------------------

EnumerateCharacterId::~EnumerateCharacterId()
{
}

// ----------------------------------------------------------------------

EnumerateCharacterId_Chardata::EnumerateCharacterId_Chardata()
{
}

// ----------------------------------------------------------------------

EnumerateCharacterId_Chardata::EnumerateCharacterId_Chardata(const Unicode::String &name, int objectTemplateId, const NetworkId &networkId, uint32 clusterId, CharacterType characterType) :
		m_name(name),
		m_objectTemplateId(objectTemplateId),
		m_networkId(networkId),
		m_clusterId(clusterId),
		m_characterType(characterType)
{
}

// ----------------------------------------------------------------------

const std::vector<EnumerateCharacterId::Chardata> &EnumerateCharacterId::getData() const
{
	return m_data.get();
}		

// ======================================================================

SelectCharacter::SelectCharacter(const NetworkId &id) :
		GameNetworkMessage("SelectCharacter"),
		m_id(id)
{											
	addVariable(m_id);
}

//-----------------------------------------------------------------------

SelectCharacter::SelectCharacter(Archive::ReadIterator & source) :
		GameNetworkMessage("SelectCharacter"),
		m_id()
{											
	addVariable(m_id);
	unpack(source);
}

//-----------------------------------------------------------------------

SelectCharacter::~SelectCharacter()
{
}

//-----------------------------------------------------------------------


ClientCreateCharacter::ClientCreateCharacter(const Unicode::String & characterName, const std::string & templateName, float scaleFactor, const std::string & startingLocation, const std::string& customAppearanceData, const std::string & hairTemplateName, const std::string & hairAppearanceData, const std::string & profession, bool jedi, const Unicode::String & biography, bool useNewbieTutorial, const std::string & skillTemplate, const std::string & workingSkill) :
GameNetworkMessage      ("ClientCreateCharacter"),
m_appearanceData        (customAppearanceData),
m_characterName         (characterName),
m_templateName          (templateName),
m_startingLocation      (startingLocation),
m_hairTemplateName      (hairTemplateName),
m_hairAppearanceData    (hairAppearanceData),
m_profession            (profession),
m_jedi                  (jedi),
m_scaleFactor           (scaleFactor),
m_biography             (biography),
m_useNewbieTutorial     (useNewbieTutorial),
m_skillTemplate         (skillTemplate),
m_workingSkill          (workingSkill)
{
	addVariable(m_appearanceData);
	addVariable(m_characterName);
	addVariable(m_templateName);
	addVariable(m_startingLocation);
	addVariable(m_hairTemplateName);
	addVariable(m_hairAppearanceData);
	addVariable(m_profession);
	addVariable(m_jedi);
	addVariable(m_scaleFactor);
	addVariable(m_biography);
	addVariable(m_useNewbieTutorial);
	addVariable(m_skillTemplate);
	addVariable(m_workingSkill);
}

//-----------------------------------------------------------------------

ClientCreateCharacter::ClientCreateCharacter(Archive::ReadIterator & source) :
GameNetworkMessage      ("ClientCreateCharacter"),
m_appearanceData        (),
m_characterName         (),
m_templateName          (),
m_startingLocation      (),
m_hairTemplateName      (),
m_hairAppearanceData    (),
m_profession            (),
m_jedi                  (),
m_scaleFactor           (1.0f),
m_biography             (),
m_useNewbieTutorial     (false),
m_skillTemplate         (),
m_workingSkill          ()
{													
	addVariable(m_appearanceData);
	addVariable(m_characterName);
	addVariable(m_templateName);
	addVariable(m_startingLocation);
	addVariable(m_hairTemplateName);
	addVariable(m_hairAppearanceData);
	addVariable(m_profession);
	addVariable(m_jedi);
	addVariable(m_scaleFactor);
	addVariable(m_biography);
	addVariable(m_useNewbieTutorial);
	addVariable(m_skillTemplate);
	addVariable(m_workingSkill);
	unpack(source);
}

//-----------------------------------------------------------------------

ClientCreateCharacter::~ClientCreateCharacter()
{
}

//----------------------------------------------------------------------

const float ClientCreateCharacter::getScaleFactor        () const
{
	return m_scaleFactor.get ();
}

//-----------------------------------------------------------------------

const char * const ClientCreateCharacterSuccess::MessageType = "ClientCreateCharacterSuccess";

//----------------------------------------------------------------------

ClientCreateCharacterSuccess::ClientCreateCharacterSuccess (const NetworkId & networkId) :
GameNetworkMessage (ClientCreateCharacterSuccess::MessageType),
m_networkId        (networkId)
{
	addVariable(m_networkId);
}

//-----------------------------------------------------------------------

ClientCreateCharacterSuccess::ClientCreateCharacterSuccess(Archive::ReadIterator & source) :
GameNetworkMessage (ClientCreateCharacterSuccess::MessageType),
m_networkId        ()
{
	addVariable(m_networkId);
	unpack(source);
}

//-----------------------------------------------------------------------

ClientCreateCharacterSuccess::~ClientCreateCharacterSuccess()
{
}

//-----------------------------------------------------------------------

ClientCreateCharacterFailed::ClientCreateCharacterFailed(const Unicode::String &name, const StringId &errorMessage) :
GameNetworkMessage("ClientCreateCharacterFailed"),
m_name(name),
m_errorMessage(errorMessage)
{
	addVariable(m_name);
	addVariable(m_errorMessage);
}

//-----------------------------------------------------------------------

ClientCreateCharacterFailed::ClientCreateCharacterFailed(Archive::ReadIterator & source) :
GameNetworkMessage("ClientCreateCharacterFailed")
{
	addVariable(m_name);
	addVariable(m_errorMessage);
	unpack(source);
}

//-----------------------------------------------------------------------

ClientCreateCharacterFailed::~ClientCreateCharacterFailed()
{
}

//-----------------------------------------------------------------------

RefreshCharacterList::RefreshCharacterList() :
GameNetworkMessage("RefreshCharacterList")
{
}

//-----------------------------------------------------------------------

RefreshCharacterList::RefreshCharacterList(Archive::ReadIterator & source) :
GameNetworkMessage("RefreshCharacterList")
{
	UNREF(source);
}

//-----------------------------------------------------------------------

RefreshCharacterList::~RefreshCharacterList()
{
}

//-----------------------------------------------------------------------

ClientRandomNameRequest::ClientRandomNameRequest(const std::string & creatureTemplate) :
GameNetworkMessage("ClientRandomNameRequest"),
m_creatureTemplate(creatureTemplate)
{
	addVariable(m_creatureTemplate);
}

//-----------------------------------------------------------------------

ClientRandomNameRequest::ClientRandomNameRequest(Archive::ReadIterator & source) :
GameNetworkMessage("ClientRandomNameRequest"),
m_creatureTemplate()
{
	addVariable(m_creatureTemplate);

	unpack(source);
}

//-----------------------------------------------------------------------

ClientRandomNameRequest::~ClientRandomNameRequest()
{
}

//-----------------------------------------------------------------------

ClientVerifyAndLockNameRequest::ClientVerifyAndLockNameRequest(const std::string & templateName, const Unicode::String & characterName) :
GameNetworkMessage("ClientVerifyAndLockNameRequest"),
m_templateName(templateName),
m_characterName(characterName)
{
	addVariable(m_templateName);
	addVariable(m_characterName);
}

//-----------------------------------------------------------------------

ClientVerifyAndLockNameRequest::ClientVerifyAndLockNameRequest(Archive::ReadIterator & source) :
GameNetworkMessage("ClientVerifyAndLockNameRequest"),
m_templateName(),
m_characterName()
{
	addVariable(m_templateName);
	addVariable(m_characterName);
	unpack(source);
}

//-----------------------------------------------------------------------

ClientVerifyAndLockNameRequest::~ClientVerifyAndLockNameRequest()
{
}

//-----------------------------------------------------------------------

ClientRandomNameResponse::ClientRandomNameResponse(const std::string & creatureTemplate, const Unicode::String &name, const StringId &errorMessage) :
GameNetworkMessage("ClientRandomNameResponse"),
m_creatureTemplate(creatureTemplate),
m_name(name),
m_errorMessage(errorMessage)
{
	addVariable(m_creatureTemplate);
	addVariable(m_name);
	addVariable(m_errorMessage);
}

//-----------------------------------------------------------------------

ClientRandomNameResponse::ClientRandomNameResponse(Archive::ReadIterator & source) :
GameNetworkMessage("ClientRandomNameResponse"),
m_creatureTemplate(),
m_name(),
m_errorMessage()
{
	addVariable(m_creatureTemplate);
	addVariable(m_name);
	addVariable(m_errorMessage);

	unpack(source);
}

//-----------------------------------------------------------------------

ClientRandomNameResponse::~ClientRandomNameResponse()
{
}

//-----------------------------------------------------------------------

ClientVerifyAndLockNameResponse::ClientVerifyAndLockNameResponse(const Unicode::String &characterName, const StringId &errorMessage) :
GameNetworkMessage("ClientVerifyAndLockNameResponse"),
m_characterName(characterName),
m_errorMessage(errorMessage)
{
	addVariable(m_characterName);
	addVariable(m_errorMessage);
}

//-----------------------------------------------------------------------

ClientVerifyAndLockNameResponse::ClientVerifyAndLockNameResponse(Archive::ReadIterator & source) :
GameNetworkMessage("ClientVerifyAndLockNameResponse"),
m_characterName(),
m_errorMessage()
{
	addVariable(m_characterName);
	addVariable(m_errorMessage);

	unpack(source);
}

//-----------------------------------------------------------------------

ClientVerifyAndLockNameResponse::~ClientVerifyAndLockNameResponse()
{
}
