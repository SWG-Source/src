// ======================================================================
//
// FileManifest.h
// Copyright 2005 Sony Online Entertainment
// All Rights Reserved.
// Author: Vijay Thakkar
//
// ======================================================================

#ifndef	_INCLUDED_VerifyAnyLockName_H
#define	_INCLUDED_VerifyAnyLockName_H

//-----------------------------------------------------------------------

#include "StringId.h"
#include "Unicode.h"

//-----------------------------------------------------------------------

class VerifyAndLockNameRequest: public GameNetworkMessage
{
public:
	explicit VerifyAndLockNameRequest(uint32 stationId, const NetworkId & characterId, const std::string& templateName, const Unicode::String& characterName, uint32 gameFeatures);
	explicit VerifyAndLockNameRequest(Archive::ReadIterator & source);
	~VerifyAndLockNameRequest();

	const uint32                           getStationId() const;
	const NetworkId &                      getCharacterId() const;
	const std::string&                     getTemplateName() const;
	const Unicode::String&                 getCharacterName() const;
	const uint32                           getGameFeatures() const;
private:
	Archive::AutoVariable<uint32>           m_stationId;
	Archive::AutoVariable<NetworkId>        m_characterId;
	Archive::AutoVariable<std::string>      m_templateName;
	Archive::AutoVariable<Unicode::String>  m_characterName;
	Archive::AutoVariable<uint32>           m_gameFeatures;

	VerifyAndLockNameRequest(const VerifyAndLockNameRequest & source);
	VerifyAndLockNameRequest & operator = (const VerifyAndLockNameRequest & other);
};

//--------------------------------------------------------------------

inline const uint32 VerifyAndLockNameRequest::getStationId() const
{
	return m_stationId.get();
}

//--------------------------------------------------------------------

inline const NetworkId & VerifyAndLockNameRequest::getCharacterId() const
{
	return m_characterId.get();
}

//--------------------------------------------------------------------

inline const std::string& VerifyAndLockNameRequest::getTemplateName() const
{
	return m_templateName.get();
}

//-----------------------------------------------------------------------

inline const Unicode::String& VerifyAndLockNameRequest::getCharacterName() const
{
	return m_characterName.get();
}

//-----------------------------------------------------------------------

inline const uint32 VerifyAndLockNameRequest::getGameFeatures() const
{
	return m_gameFeatures.get();
}

//-----------------------------------------------------------------------

class VerifyAndLockNameResponse: public GameNetworkMessage
{
public:
	explicit VerifyAndLockNameResponse(uint32 stationId, const Unicode::String &characterName, const StringId& errorMessage);
	explicit VerifyAndLockNameResponse(Archive::ReadIterator & source);
	~VerifyAndLockNameResponse();

	const uint32                            getStationId() const;
	const Unicode::String&                  getCharacterName() const;
	const StringId&                         getErrorMessage() const;
private:
	Archive::AutoVariable<uint32>           m_stationId;
	Archive::AutoVariable<Unicode::String>  m_characterName;
	Archive::AutoVariable<StringId>         m_errorMessage;

	VerifyAndLockNameResponse(const VerifyAndLockNameRequest & source);
	VerifyAndLockNameResponse & operator = (const VerifyAndLockNameRequest & other);
};

//--------------------------------------------------------------------

inline const uint32 VerifyAndLockNameResponse::getStationId() const
{
	return m_stationId.get();
}

//--------------------------------------------------------------------

inline const Unicode::String &VerifyAndLockNameResponse::getCharacterName() const
{
	return m_characterName.get();
}

//--------------------------------------------------------------------

inline const StringId &VerifyAndLockNameResponse::getErrorMessage() const
{
	return m_errorMessage.get();
}

// ======================================================================

#endif

