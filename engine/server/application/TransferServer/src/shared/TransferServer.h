// TransferServer.h
// Copyright 2000-03, Sony Online Entertainment Inc., all rights reserved.
// Author: Justin Randall

#ifndef	_INCLUDED_TransferServer_H
#define	_INCLUDED_TransferServer_H

//-----------------------------------------------------------------------

#include "Archive/ByteStream.h"
#include <string>

//-----------------------------------------------------------------------

struct CharacterTransferData
{
	unsigned int stationId;
	std::string packedData;
	std::string fromGalaxy;
};

class TransferAccountData;
class TransferCharacterData;
class TransferReplyCharacterList;
class TransferReplyMoveValidation;
class TransferReplyNameValidation;

//-----------------------------------------------------------------------

class TransferServer
{
public:
	TransferServer();
	~TransferServer();

	static void                           unauthorizeDownload          (unsigned int stationId);
	static void                           authorizeDownload            (unsigned int stationId);
	static const CharacterTransferData *  getCharacterTransferData     (unsigned int stationId, const std::string & fromGalaxy, bool administrativeRequest);
	static void                           run                          ();
	static void                           setDone                      (bool isDone);
	static bool                           uploadCharacterTransferData  (const CharacterTransferData &, bool administrative);

	static void                           requestCharacterList         (unsigned int track, unsigned int stationId, const std::string & serverName, const std::string & customerLocalizedLanguage);
	static void                           requestMoveValidation        (unsigned int track, unsigned int sourceStationId, unsigned int destinationStationId, const std::string & sourceGalaxy, const std::string & destinationGalaxy, const std::string & sourceCharacter, const std::string & destinationCharacter, const std::string & customerLocalizedLanguage);
	static void                           cacheSourceCharacterTemplateCrc(unsigned int stationId, const std::string & galaxy, const std::string & characterName, uint32 characterTemplateCrc);

	static void                           requestMove                  (unsigned int track, const std::string & customerLocalizedLanguage, const std::string & sourceGalaxy, const std::string & destinationGalaxy, const std::string & sourceCharacter, const std::string & destinationCharacter, unsigned int sourceStationId, unsigned int destinationStationId, unsigned int transactionId, const bool withItems, const bool allowOverride);
	static void                           requestTransferAccount       (unsigned int track, unsigned int sourceStationId, unsigned int destinationStationId, unsigned int transactionId);
	static void                           getLoginLocationData         (const TransferCharacterData & characterData);
	static void                           replyCharacterList           (const TransferReplyCharacterList & characterList);
	static void                           replyValidateMove            (const TransferCharacterData &);
	static void                           replyMoveSuccess             (const TransferCharacterData & reply);
	static void                           replyTransferAccountSuccess  (const TransferAccountData & reply);
	static void                           transferCreateCharacterFailed(const TransferCharacterData & failed);
	static bool                           isRename                     (const TransferCharacterData & request);
	static void                           failedToRetrieveTransferData (const TransferCharacterData & failed);
	static void                           failedToApplyTransferData    (const TransferCharacterData & failed);
	static void                           failedToTransferAccountNoCentralConnection (const TransferAccountData & failed);
	static void                           failedToTransferAccountDestinationNotEmpty (const TransferAccountData & failed);
	static void                           failedToTransferCharacterGameConnectionClosed (const TransferCharacterData & failed);
	static void                           failedToTransferCharacterConnectionServerConnectionClosed (const unsigned int stationId);
	static void                           failedToValidateTransfer     (const TransferReplyMoveValidation & reply);
	
private:
	TransferServer & operator = (const TransferServer & rhs);
	TransferServer(const TransferServer & source);
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_TransferServer_H
