// TransferReplyMoveValidation.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/TransferReplyMoveValidation.h"

//-----------------------------------------------------------------------

TransferReplyMoveValidation::TransferReplyMoveValidation(TransferRequestMoveValidation::TransferRequestSource transferRequestSource, unsigned int track, unsigned int sourceStationId, unsigned int destinationStationId, const std::string & sourceGalaxy, const std::string & destinationGalaxy, const std::string & sourceCharacter, const NetworkId & sourceCharacterId, uint32 sourceCharacterTemplateId, const std::string & destinationCharacter, const std::string & customerLocalizedLanguage, TransferReplyMoveValidationResult result) :
GameNetworkMessage("TransferReplyMoveValidation"),
m_customerLocalizedLanguage(customerLocalizedLanguage),
m_destinationCharacter(destinationCharacter),
m_destinationGalaxy(destinationGalaxy),
m_destinationStationId(destinationStationId),
m_sourceCharacter(sourceCharacter),
m_sourceCharacterId(sourceCharacterId),
m_sourceCharacterTemplateId(sourceCharacterTemplateId),
m_sourceGalaxy(sourceGalaxy),
m_sourceStationId(sourceStationId),
m_track(track),
m_result(static_cast<int8>(result)),
m_transferRequestSource(static_cast<int8>(transferRequestSource))
{
	addVariable(m_customerLocalizedLanguage);
	addVariable(m_destinationCharacter);
	addVariable(m_destinationGalaxy);
	addVariable(m_destinationStationId);
	addVariable(m_sourceCharacter);
	addVariable(m_sourceCharacterId);
	addVariable(m_sourceCharacterTemplateId);
	addVariable(m_sourceGalaxy);
	addVariable(m_sourceStationId);
	addVariable(m_track);
	addVariable(m_result);
	addVariable(m_transferRequestSource);
}

//-----------------------------------------------------------------------

TransferReplyMoveValidation::TransferReplyMoveValidation(Archive::ReadIterator & source) :
GameNetworkMessage("TransferReplyMoveValidation"),
m_customerLocalizedLanguage(),
m_destinationCharacter(),
m_destinationGalaxy(),
m_destinationStationId(),
m_sourceCharacter(),
m_sourceCharacterId(),
m_sourceCharacterTemplateId(),
m_sourceGalaxy(),
m_sourceStationId(),
m_track(),
m_result(),
m_transferRequestSource()
{
	addVariable(m_customerLocalizedLanguage);
	addVariable(m_destinationCharacter);
	addVariable(m_destinationGalaxy);
	addVariable(m_destinationStationId);
	addVariable(m_sourceCharacter);
	addVariable(m_sourceCharacterId);
	addVariable(m_sourceCharacterTemplateId);
	addVariable(m_sourceGalaxy);
	addVariable(m_sourceStationId);
	addVariable(m_track);
	addVariable(m_result);
	addVariable(m_transferRequestSource);
	AutoByteStream::unpack(source);
}

//-----------------------------------------------------------------------

TransferReplyMoveValidation::~TransferReplyMoveValidation()
{
}

//-----------------------------------------------------------------------

const std::string & TransferReplyMoveValidation::getCustomerLocalizedLanguage() const
{
	return m_customerLocalizedLanguage.get();
}

//-----------------------------------------------------------------------

const std::string & TransferReplyMoveValidation::getDestinationCharacter() const
{
	return m_destinationCharacter.get();
}

//-----------------------------------------------------------------------

const std::string & TransferReplyMoveValidation::getDestinationGalaxy() const
{
	return m_destinationGalaxy.get();
}

//-----------------------------------------------------------------------

unsigned int TransferReplyMoveValidation::getDestinationStationId() const
{
	return m_destinationStationId.get();
}

//-----------------------------------------------------------------------

const std::string & TransferReplyMoveValidation::getSourceCharacter() const
{
	return m_sourceCharacter.get();
}

//-----------------------------------------------------------------------

const NetworkId & TransferReplyMoveValidation::getSourceCharacterId() const
{
	return m_sourceCharacterId.get();
}

//-----------------------------------------------------------------------

uint32 TransferReplyMoveValidation::getSourceCharacterTemplateId() const
{
	return m_sourceCharacterTemplateId.get();
}

//-----------------------------------------------------------------------

const std::string & TransferReplyMoveValidation::getSourceGalaxy() const
{
	return m_sourceGalaxy.get();
}

//-----------------------------------------------------------------------

unsigned int TransferReplyMoveValidation::getSourceStationId() const
{
	return m_sourceStationId.get();
}

//-----------------------------------------------------------------------
unsigned int TransferReplyMoveValidation::getTrack() const
{
	return m_track.get();
}

//-----------------------------------------------------------------------

TransferReplyMoveValidation::TransferReplyMoveValidationResult TransferReplyMoveValidation::getResult() const
{
	return static_cast<TransferReplyMoveValidationResult>(m_result.get());
}

//-----------------------------------------------------------------------

TransferRequestMoveValidation::TransferRequestSource TransferReplyMoveValidation::getTransferRequestSource() const
{
	return static_cast<TransferRequestMoveValidation::TransferRequestSource>(m_transferRequestSource.get());
}

//-----------------------------------------------------------------------
