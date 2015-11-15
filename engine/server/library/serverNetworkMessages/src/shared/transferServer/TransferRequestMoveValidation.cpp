// TransferRequestMoveValidation.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/TransferRequestMoveValidation.h"

//-----------------------------------------------------------------------

TransferRequestMoveValidation::TransferRequestMoveValidation(TransferRequestSource transferRequestSource, unsigned int track, unsigned int sourceStationId, unsigned int destinationStationId, const std::string & sourceGalaxy, const std::string & destinationGalaxy, const std::string & sourceCharacter, const NetworkId & sourceCharacterId, uint32 sourceCharacterTemplateId, const std::string & destinationCharacter, const std::string & customerLocalizedLanguage) :
GameNetworkMessage("TransferRequestMoveValidation"),
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
	addVariable(m_transferRequestSource);
}

//-----------------------------------------------------------------------

TransferRequestMoveValidation::TransferRequestMoveValidation(Archive::ReadIterator & source) :
GameNetworkMessage("TransferRequestMoveValidation"),
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
	addVariable(m_transferRequestSource);
	AutoByteStream::unpack(source);
}

//-----------------------------------------------------------------------

TransferRequestMoveValidation::~TransferRequestMoveValidation()
{
}

//-----------------------------------------------------------------------

const std::string & TransferRequestMoveValidation::getCustomerLocalizedLanguage() const
{
	return m_customerLocalizedLanguage.get();
}

//-----------------------------------------------------------------------

const std::string & TransferRequestMoveValidation::getDestinationCharacter() const
{
	return m_destinationCharacter.get();
}

//-----------------------------------------------------------------------

const std::string & TransferRequestMoveValidation::getDestinationGalaxy() const
{
	return m_destinationGalaxy.get();
}

//-----------------------------------------------------------------------

unsigned int TransferRequestMoveValidation::getDestinationStationId() const
{
	return m_destinationStationId.get();
}

//-----------------------------------------------------------------------

const std::string & TransferRequestMoveValidation::getSourceCharacter() const
{
	return m_sourceCharacter.get();
}

//-----------------------------------------------------------------------

const NetworkId & TransferRequestMoveValidation::getSourceCharacterId() const
{
	return m_sourceCharacterId.get();
}

//-----------------------------------------------------------------------

uint32 TransferRequestMoveValidation::getSourceCharacterTemplateId() const
{
	return m_sourceCharacterTemplateId.get();
}

//-----------------------------------------------------------------------

const std::string & TransferRequestMoveValidation::getSourceGalaxy() const
{
	return m_sourceGalaxy.get();
}

//-----------------------------------------------------------------------

unsigned int TransferRequestMoveValidation::getSourceStationId() const
{
	return m_sourceStationId.get();
}

//-----------------------------------------------------------------------

unsigned int TransferRequestMoveValidation::getTrack() const
{
	return m_track.get();
}

//-----------------------------------------------------------------------

TransferRequestMoveValidation::TransferRequestSource TransferRequestMoveValidation::getTransferRequestSource() const
{
	return static_cast<TransferRequestSource>(m_transferRequestSource.get());
}

//-----------------------------------------------------------------------
