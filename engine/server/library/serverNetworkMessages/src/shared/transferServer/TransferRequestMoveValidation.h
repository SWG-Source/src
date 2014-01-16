// TransferRequestMoveValidation.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_TransferRequestMoveValidation_H
#define	_INCLUDED_TransferRequestMoveValidation_H

//-----------------------------------------------------------------------
#include "serverNetworkMessages/AvatarList.h"
#include "serverNetworkMessages/AvatarListArchive.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include <string>

//-----------------------------------------------------------------------

class TransferRequestMoveValidation : public GameNetworkMessage
{
public:
	enum TransferRequestSource {TRS_transfer_server, TRS_console_god_command, TRS_ingame_freects_command_validate, TRS_ingame_freects_command_transfer, TRS_ingame_cts_command_validate, TRS_ingame_cts_command_transfer};

public:
	TransferRequestMoveValidation(TransferRequestSource transferRequestSource, unsigned int track, unsigned int sourceStationId, unsigned int destinationStationId, const std::string & sourceGalaxy, const std::string & destinationGalaxy, const std::string & sourceCharacter, const NetworkId & sourceCharacterId, uint32 sourceCharacterTemplateId, const std::string & destinationCharacter, const std::string & customerLocalizedLanguage);
	explicit TransferRequestMoveValidation(Archive::ReadIterator & source);
	~TransferRequestMoveValidation();

	const std::string &  getCustomerLocalizedLanguage  () const;
	const std::string &  getDestinationCharacter       () const;
	const std::string &  getDestinationGalaxy          () const;
	unsigned int         getDestinationStationId       () const;
	const std::string &  getSourceCharacter            () const;
	const NetworkId &    getSourceCharacterId          () const;
	uint32               getSourceCharacterTemplateId  () const;
	const std::string &  getSourceGalaxy               () const;
	unsigned int         getSourceStationId            () const;
	unsigned int         getTrack                      () const;
	TransferRequestSource getTransferRequestSource     () const;

private:
	TransferRequestMoveValidation & operator = (const TransferRequestMoveValidation & rhs);
	TransferRequestMoveValidation(const TransferRequestMoveValidation & source);

	Archive::AutoVariable<std::string>   m_customerLocalizedLanguage;
	Archive::AutoVariable<std::string>   m_destinationCharacter;
	Archive::AutoVariable<std::string>   m_destinationGalaxy;
	Archive::AutoVariable<unsigned int>  m_destinationStationId;
	Archive::AutoVariable<std::string>   m_sourceCharacter;
	Archive::AutoVariable<NetworkId>     m_sourceCharacterId;
	Archive::AutoVariable<uint32>        m_sourceCharacterTemplateId;
	Archive::AutoVariable<std::string>   m_sourceGalaxy;
	Archive::AutoVariable<unsigned int>  m_sourceStationId;
	Archive::AutoVariable<unsigned int>  m_track;
	Archive::AutoVariable<int8>          m_transferRequestSource;
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_TransferRequestMoveValidation_H
