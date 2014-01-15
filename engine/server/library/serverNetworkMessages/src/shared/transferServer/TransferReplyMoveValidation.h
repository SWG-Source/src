// TransferReplyMoveValidation.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_TransferReplyMoveValidation_H
#define	_INCLUDED_TransferReplyMoveValidation_H

//-----------------------------------------------------------------------

#include "serverNetworkMessages/TransferRequestMoveValidation.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include <string>

//-----------------------------------------------------------------------

class TransferReplyMoveValidation : public GameNetworkMessage
{
public:
	enum TransferReplyMoveValidationResult {TRMVR_can_create_regular_character, TRMVR_cannot_create_regular_character, TRMVR_destination_galaxy_invalid, TRMVR_destination_galaxy_not_connected, TRMVR_destination_galaxy_in_loading};

public:
	TransferReplyMoveValidation(TransferRequestMoveValidation::TransferRequestSource transferRequestSource, unsigned int track, unsigned int sourceStationId, unsigned int destinationStationId, const std::string & sourceGalaxy, const std::string & destinationGalaxy, const std::string & sourceCharacter, const NetworkId & sourceCharacterId, uint32 sourceCharacterTemplateId, const std::string & destinationCharacter, const std::string & customerLocalizedLanguage, TransferReplyMoveValidationResult result);
	explicit TransferReplyMoveValidation(Archive::ReadIterator & source);
	~TransferReplyMoveValidation();

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
	TransferReplyMoveValidationResult getResult        () const;
	TransferRequestMoveValidation::TransferRequestSource getTransferRequestSource() const;

private:
	TransferReplyMoveValidation & operator = (const TransferReplyMoveValidation & rhs);
	TransferReplyMoveValidation(const TransferReplyMoveValidation & source);

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
	Archive::AutoVariable<int8>          m_result;
	Archive::AutoVariable<int8>          m_transferRequestSource;
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_TransferReplyMoveValidation_H
