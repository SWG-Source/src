// ClientCentralMessages.h
// copyright 2001 Verant Interactive
// Author: Justin Randall

#ifndef	_INCLUDED_ClientCentralMessages_H
#define	_INCLUDED_ClientCentralMessages_H

//-----------------------------------------------------------------------

#include "StringId.h"
#include "Unicode.h"
#include "localizationArchive/StringIdArchive.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

class ClientIdMsg : public GameNetworkMessage
{
public:
	ClientIdMsg(const unsigned char * const newToken, const uint16 newTokenSize, uint32 gameBitsToClear);
	explicit ClientIdMsg(Archive::ReadIterator & source);
	~ClientIdMsg();
	const uint32                getGameBitsToClear() const;
	const unsigned char * const getToken     (void) const;
	const std::string &         getVersion   () const;
	const uint16                getTokenSize (void) const;

private:
	Archive::AutoVariable<uint32>        m_gameBitsToClear;
	Archive::AutoArray<unsigned char>    token;
	Archive::AutoVariable<std::string>   version;
	unsigned char *                      tokenData;

	ClientIdMsg();
	ClientIdMsg(const ClientIdMsg&);
	ClientIdMsg& operator= (const ClientIdMsg&);
};

//-----------------------------------------------------------------------

inline const uint32 ClientIdMsg::getGameBitsToClear() const
{
	return m_gameBitsToClear.get();
}

//-----------------------------------------------------------------------

inline const unsigned char * const ClientIdMsg::getToken(void) const
{
	return tokenData;
}

//-----------------------------------------------------------------------

inline const std::string & ClientIdMsg::getVersion() const
{
	return version.get();
}

//-----------------------------------------------------------------------

inline const uint16 ClientIdMsg::getTokenSize(void) const
{
	const std::vector<unsigned char> & t = token.get();
	return static_cast<uint16>(t.size());
}

//-----------------------------------------------------------------------

struct EnumerateCharacterId_Chardata
{
	enum CharacterType {CT_normal=1,CT_jedi=2,CT_spectral=3};
		
	Unicode::String m_name;
	int m_objectTemplateId;
	NetworkId m_networkId;
	uint32 m_clusterId;
	int m_characterType;

	EnumerateCharacterId_Chardata();
	EnumerateCharacterId_Chardata(const Unicode::String &name, int objectTemplateId, const NetworkId &networkId, uint32 clusterId, CharacterType characterType);
};

//-----------------------------------------------------------------------

class EnumerateCharacterId : public GameNetworkMessage
{
  public:
	typedef EnumerateCharacterId_Chardata Chardata;

  public:
	explicit EnumerateCharacterId(const std::vector<Chardata> &data);
	explicit EnumerateCharacterId(Archive::ReadIterator & source);
	~EnumerateCharacterId();
	
	const std::vector<Chardata> &getData() const;
	void addCharacter(const Unicode::String &name, const std::string &objectTemplate, const NetworkId &networkId);
	
  private:
	Archive::AutoArray<Chardata> m_data;

	EnumerateCharacterId();
	EnumerateCharacterId(const EnumerateCharacterId&);
	EnumerateCharacterId& operator= (const EnumerateCharacterId&);
};

// ----------------------------------------------------------------------

namespace Archive
{
	inline void get(ReadIterator & source, EnumerateCharacterId_Chardata &c)
	{
		get(source,c.m_name);
		get(source,c.m_objectTemplateId);
		get(source,c.m_networkId);
		get(source,c.m_clusterId);
		get(source,c.m_characterType);
	}

	inline void put(ByteStream & target, EnumerateCharacterId_Chardata const &c)
	{
		put(target,c.m_name);
		put(target,c.m_objectTemplateId);
		put(target,c.m_networkId);
		put(target,c.m_clusterId);
		put(target,c.m_characterType);
	}
}

// ----------------------------------------------------------------------

class SelectCharacter : public GameNetworkMessage
{
public:
	explicit SelectCharacter(const NetworkId &id);
	explicit SelectCharacter(Archive::ReadIterator & source);
	~SelectCharacter();

	const NetworkId & getId (void) const;

private:
	Archive::AutoVariable<NetworkId> m_id;

	SelectCharacter();
	SelectCharacter(const SelectCharacter&);
	SelectCharacter& operator= (const SelectCharacter&);
};

//-----------------------------------------------------------------------

inline const NetworkId & SelectCharacter::getId(void) const
{
	return m_id.get();
}

//-----------------------------------------------------------------------

class ClientCreateCharacter : public GameNetworkMessage
{
public:
	ClientCreateCharacter(const Unicode::String & characterName, const std::string & templateName, float scaleFactor, const std::string & startingLocation, const std::string& customAppearanceData, const std::string & hairTemplateName, const std::string & hairAppearanceData, const std::string & profession, bool jedi, const Unicode::String & biography, bool useNewbieTutorial, const std::string & skillTemplate, const std::string & workingSkill);
	explicit ClientCreateCharacter(Archive::ReadIterator & source);
	~ClientCreateCharacter();

	const float             getScaleFactor        () const;
	const std::string &     getAppearanceData     () const;
	const Unicode::String & getCharacterName      () const;
	const std::string &     getStartingLocation   () const;
	const std::string &     getTemplateName       () const;
	const std::string &     getHairTemplateName   () const;
	const std::string &     getHairAppearanceData () const;
	const std::string &     getProfession         () const;
	const bool              getJedi               () const;
	const Unicode::String & getBiography          () const;
	const bool              getUseNewbieTutorial  () const;
	const std::string &     getSkillTemplate      () const;
	const std::string &     getWorkingSkill       () const;

private:
	Archive::AutoVariable<std::string>            m_appearanceData;
	Archive::AutoVariable<Unicode::String>        m_characterName;
	Archive::AutoVariable<std::string>            m_templateName;
	Archive::AutoVariable<std::string>            m_startingLocation;
	Archive::AutoVariable<std::string>            m_hairTemplateName;
	Archive::AutoVariable<std::string>            m_hairAppearanceData;
	Archive::AutoVariable<std::string>            m_profession;
	Archive::AutoVariable<bool>                   m_jedi;
	Archive::AutoVariable<float>                  m_scaleFactor;
	Archive::AutoVariable<Unicode::String>        m_biography;
	Archive::AutoVariable<bool>                   m_useNewbieTutorial;
	Archive::AutoVariable<std::string>            m_skillTemplate;
	Archive::AutoVariable<std::string>            m_workingSkill;

	                       ClientCreateCharacter  ();
	                       ClientCreateCharacter  (const ClientCreateCharacter&);
	ClientCreateCharacter& operator=              (const ClientCreateCharacter&);
};

//-----------------------------------------------------------------------

inline const std::string & ClientCreateCharacter::getAppearanceData(void) const
{
	return m_appearanceData.get();
}

//-----------------------------------------------------------------------

inline const Unicode::String & ClientCreateCharacter::getCharacterName(void) const
{
	return m_characterName.get();
}

//-----------------------------------------------------------------------

inline const std::string & ClientCreateCharacter::getStartingLocation(void) const
{
	return m_startingLocation.get();
}

//-----------------------------------------------------------------------

inline const std::string & ClientCreateCharacter::getTemplateName(void) const
{
	return m_templateName.get();
}

//-----------------------------------------------------------------------

inline const std::string &  ClientCreateCharacter::getHairTemplateName(void) const
{
	return m_hairTemplateName.get();
}

//-----------------------------------------------------------------------

inline const std::string &  ClientCreateCharacter::getHairAppearanceData(void) const
{
	return m_hairAppearanceData.get();
}

//-----------------------------------------------------------------------

inline const std::string &  ClientCreateCharacter::getProfession(void) const
{
	return m_profession.get();
}

//-----------------------------------------------------------------------

inline const bool ClientCreateCharacter::getJedi(void) const
{
	return m_jedi.get();
}

//----------------------------------------------------------------------

inline const Unicode::String & ClientCreateCharacter::getBiography() const
{
	return m_biography.get ();
}

//----------------------------------------------------------------------

inline const bool ClientCreateCharacter::getUseNewbieTutorial() const
{
	return m_useNewbieTutorial.get();
}

//----------------------------------------------------------------------

inline const std::string &  ClientCreateCharacter::getSkillTemplate(void) const
{
	return m_skillTemplate.get();
}

//-----------------------------------------------------------------------

inline const std::string &  ClientCreateCharacter::getWorkingSkill(void) const
{
	return m_workingSkill.get();
}

//-----------------------------------------------------------------------

class ClientCreateCharacterSuccess : public GameNetworkMessage
{
public:

	static const char * const MessageType;

	explicit ClientCreateCharacterSuccess (const NetworkId & networkID);
	explicit ClientCreateCharacterSuccess(Archive::ReadIterator & source);
	~ClientCreateCharacterSuccess();

	const NetworkId & getNetworkId () const;

private:
	ClientCreateCharacterSuccess(const ClientCreateCharacterSuccess & source);
	ClientCreateCharacterSuccess & operator = (const ClientCreateCharacterSuccess & rhs);

	Archive::AutoVariable<NetworkId> m_networkId;
};

//----------------------------------------------------------------------

inline const NetworkId & ClientCreateCharacterSuccess::getNetworkId () const
{
	return m_networkId.get ();
}

//-----------------------------------------------------------------------

class ClientCreateCharacterFailed : public GameNetworkMessage
{
public:
	ClientCreateCharacterFailed(const Unicode::String &name, const StringId &errorMessage);
	explicit ClientCreateCharacterFailed(Archive::ReadIterator & source);

	const Unicode::String &getName() const;
	const StringId &getErrorMessage() const;

	~ClientCreateCharacterFailed();
private:
	Archive::AutoVariable<Unicode::String> m_name;
	Archive::AutoVariable<StringId> m_errorMessage;
	ClientCreateCharacterFailed(const ClientCreateCharacterFailed & source);
	ClientCreateCharacterFailed & operator = (const ClientCreateCharacterFailed & rhs);
};

//--------------------------------------------------------------------

inline const Unicode::String &ClientCreateCharacterFailed::getName() const
{
	return m_name.get();
}

//--------------------------------------------------------------------

inline const StringId &ClientCreateCharacterFailed::getErrorMessage() const
{
	return m_errorMessage.get();
}

//-----------------------------------------------------------------------

class RefreshCharacterList : public GameNetworkMessage
{
public:
	RefreshCharacterList();
	explicit RefreshCharacterList(Archive::ReadIterator & source);
	~RefreshCharacterList();

private:
	RefreshCharacterList(const RefreshCharacterList & source);
	RefreshCharacterList & operator = (const RefreshCharacterList & rhs);
};

//-----------------------------------------------------------------------

class ClientRandomNameRequest: public GameNetworkMessage
{
public:
	explicit ClientRandomNameRequest(const std::string &creatureTemplate);
	explicit ClientRandomNameRequest(Archive::ReadIterator & source);
	~ClientRandomNameRequest();

	const std::string &getCreatureTemplate() const;
private:
	Archive::AutoVariable<std::string> m_creatureTemplate;

	ClientRandomNameRequest(const ClientRandomNameRequest & source);
	ClientRandomNameRequest & operator = (const ClientRandomNameRequest & other);
};

//--------------------------------------------------------------------

inline const std::string &ClientRandomNameRequest::getCreatureTemplate() const
{
	return m_creatureTemplate.get();
}

//-----------------------------------------------------------------------

class ClientVerifyAndLockNameRequest: public GameNetworkMessage
{
public:
	explicit ClientVerifyAndLockNameRequest(const std::string& templateName, const Unicode::String& characterName);
	explicit ClientVerifyAndLockNameRequest(Archive::ReadIterator& source);
	~ClientVerifyAndLockNameRequest();

	const std::string&                      getTemplateName() const;
	const Unicode::String&                  getCharacterName() const;
private:
	Archive::AutoVariable<std::string>      m_templateName;
	Archive::AutoVariable<Unicode::String>  m_characterName;

	ClientVerifyAndLockNameRequest(const ClientVerifyAndLockNameRequest& source);
	ClientVerifyAndLockNameRequest & operator = (const ClientVerifyAndLockNameRequest& other);
};

//--------------------------------------------------------------------

inline const std::string& ClientVerifyAndLockNameRequest::getTemplateName() const
{
	return m_templateName.get();
}

//-----------------------------------------------------------------------

inline const Unicode::String& ClientVerifyAndLockNameRequest::getCharacterName() const
{
	return m_characterName.get();
}

//-----------------------------------------------------------------------

class ClientRandomNameResponse: public GameNetworkMessage
{
public:
	explicit ClientRandomNameResponse(const std::string& creatureTemplate, const Unicode::String& name, const StringId& errorMessage);
	explicit ClientRandomNameResponse(Archive::ReadIterator& source);
	~ClientRandomNameResponse();

	const std::string&                      getCreatureTemplate() const;
	const Unicode::String&                  getName() const;
	const StringId&                         getErrorMessage() const;
private:
	Archive::AutoVariable<std::string>      m_creatureTemplate;
	Archive::AutoVariable<Unicode::String>  m_name;
	Archive::AutoVariable<StringId>         m_errorMessage;

	ClientRandomNameResponse(const ClientRandomNameRequest& source);
	ClientRandomNameResponse & operator = (const ClientRandomNameRequest& other);
};

//--------------------------------------------------------------------

inline const std::string& ClientRandomNameResponse::getCreatureTemplate() const
{
	return m_creatureTemplate.get();
}

//--------------------------------------------------------------------

inline const Unicode::String& ClientRandomNameResponse::getName() const
{
	return m_name.get();
}

//--------------------------------------------------------------------

inline const StringId& ClientRandomNameResponse::getErrorMessage() const
{
	return m_errorMessage.get();
}

//--------------------------------------------------------------------

class ClientVerifyAndLockNameResponse: public GameNetworkMessage
{
public:
	explicit ClientVerifyAndLockNameResponse(const Unicode::String& characterName, const StringId& errorMessage);
	explicit ClientVerifyAndLockNameResponse(Archive::ReadIterator& source);
	~ClientVerifyAndLockNameResponse();

	const Unicode::String&                  getCharacterName() const;
	const StringId&                         getErrorMessage() const;
private:
	Archive::AutoVariable<Unicode::String>  m_characterName;
	Archive::AutoVariable<StringId>         m_errorMessage;

	ClientVerifyAndLockNameResponse(const ClientVerifyAndLockNameResponse& source);
	ClientVerifyAndLockNameResponse & operator = (const ClientVerifyAndLockNameResponse& other);
};

//--------------------------------------------------------------------

inline const Unicode::String& ClientVerifyAndLockNameResponse::getCharacterName() const
{
	return m_characterName.get();
}

//-----------------------------------------------------------------------

inline const StringId& ClientVerifyAndLockNameResponse::getErrorMessage() const
{
	return m_errorMessage.get();
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ClientCentralMessages_H
