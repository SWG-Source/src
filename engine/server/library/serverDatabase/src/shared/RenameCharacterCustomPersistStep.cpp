// ======================================================================
//
// RenameCharacterCustomPersistStep.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/RenameCharacterCustomPersistStep.h"

#include "serverDatabase/CharacterQueries.h"
#include "serverDatabase/DataLookup.h"
#include "serverDatabase/DatabaseProcess.h"
#include "serverNetworkMessages/MessageToMessage.h"
#include "serverNetworkMessages/RenameCharacterMessage.h"
#include "serverNetworkMessages/TransferCharacterData.h"
#include "serverNetworkMessages/TransferCharacterDataArchive.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"

// ======================================================================

namespace RenameCharacterCustomPersistStepNamespace
{
	std::map<NetworkId, int> s_pendingRenameCharacterCustomPersistStep;
}

using namespace RenameCharacterCustomPersistStepNamespace;

// ======================================================================

RenameCharacterCustomPersistStep::RenameCharacterCustomPersistStep(int8 renameCharacterMessageSource, uint32 stationId, const NetworkId &characterId, const Unicode::String &characterName, const Unicode::String &characterOldName, const NetworkId &requestedBy, const TransferCharacterData * requestData) :
		m_renameCharacterMessageSource(renameCharacterMessageSource),
		m_stationId(stationId),
		m_characterId(characterId),
		m_characterName(new Unicode::String(characterName)),
		m_characterOldName(new Unicode::String(characterOldName)),
		m_requestedBy(requestedBy),
		m_success(false),
		m_requestData(0)
{	
	if(requestData)
	{
		m_requestData = new TransferCharacterData(*requestData);
	}

	if (m_characterId.isValid())
	{
		std::map<NetworkId, int>::iterator iterFind = s_pendingRenameCharacterCustomPersistStep.find(m_characterId);
		if (iterFind != s_pendingRenameCharacterCustomPersistStep.end())
			++(iterFind->second);
		else
			s_pendingRenameCharacterCustomPersistStep.insert(std::make_pair(m_characterId, 1));
	}
}

// ----------------------------------------------------------------------

RenameCharacterCustomPersistStep::~RenameCharacterCustomPersistStep()
{
	delete m_characterName;
	delete m_characterOldName;
	delete m_requestData;
	m_characterName  = 0;
	m_characterOldName = 0;
	m_requestData = 0;

	if (m_characterId.isValid())
	{
		std::map<NetworkId, int>::iterator iterFind = s_pendingRenameCharacterCustomPersistStep.find(m_characterId);
		if (iterFind != s_pendingRenameCharacterCustomPersistStep.end())
		{
			if (iterFind->second <= 1)
				s_pendingRenameCharacterCustomPersistStep.erase(iterFind);
			else
				--(iterFind->second);
		}

		// if player requested, let CentralServer know that the request has
		// been completed, so CentralServer will allow the character to log back in
		if (static_cast<RenameCharacterMessageEx::RenameCharacterMessageSource>(m_renameCharacterMessageSource) == RenameCharacterMessageEx::RCMS_player_request)
		{
			GenericValueTypeMessage<std::pair<unsigned int, NetworkId> > const msg("PlayerRenameRequestCompleted", std::make_pair(static_cast<unsigned int>(m_stationId), m_characterId));
			DatabaseProcess::getInstance().sendToCentralServer(msg, true);
		}
	}
}

// ----------------------------------------------------------------------

bool RenameCharacterCustomPersistStep::beforePersist(DB::Session *session)
{
	UNREF(session);
	return true;
}

// ----------------------------------------------------------------------

bool RenameCharacterCustomPersistStep::afterPersist(DB::Session *session)
{
	DBQuery::RenameCharacter qry(m_characterId, *m_characterName, DataLookup::getInstance().normalizeName(*m_characterName));
	if (! (session->exec(&qry)))
		return false;
	m_success=qry.getResult();
	qry.done();
	return true;
}

// ----------------------------------------------------------------------

void RenameCharacterCustomPersistStep::onComplete()
{
	if (! m_requestData)
	{
		if (m_success)
		{
			// send to login server (by way of central)
			RenameCharacterMessageEx msg(static_cast<RenameCharacterMessageEx::RenameCharacterMessageSource>(m_renameCharacterMessageSource), m_stationId, m_characterId, *m_characterName, *m_characterOldName, m_requestedBy);
			DatabaseProcess::getInstance().sendToCentralServer(msg,true);

			// have all the game servers update their name managers
			DatabaseProcess::getInstance().sendToAllGameServers(msg,true);

			// update name if the character is logged in
			MessageToMessage const mtm(
				MessageToPayload(m_characterId, NetworkId::cms_invalid, "C++RenameCharacter", Unicode::wideToNarrow(*m_characterName), 0, false, MessageToPayload::DT_c, NetworkId::cms_invalid, std::string(),0),
				DatabaseProcess::getInstance().getProcessId());
			DatabaseProcess::getInstance().sendToAnyGameServer(mtm);
		}
		else
		{
			// send failure message to person who requested rename
			if ((static_cast<RenameCharacterMessageEx::RenameCharacterMessageSource>(m_renameCharacterMessageSource) != RenameCharacterMessageEx::RCMS_player_request) && m_requestedBy.isValid())
			{
				// send GM message
				MessageToMessage const mtm(
					MessageToPayload(m_requestedBy, NetworkId::cms_invalid, "C++RenameFailed", FormattedString<512>().sprintf("Rename request for %s (%s) failed because the name is already in use.", m_characterId.getValueString().c_str(), Unicode::wideToNarrow(*m_characterName).c_str()), 0, false, MessageToPayload::DT_c, NetworkId::cms_invalid, std::string(), 0),
					DatabaseProcess::getInstance().getProcessId());
				DatabaseProcess::getInstance().sendToAnyGameServer(mtm);
			}
		}
	}
	else
	{
		TransferCharacterData replyData(*m_requestData);
		replyData.setIsValidName(m_success);
		
		GenericValueTypeMessage<TransferCharacterData> reply("TransferRenameCharacterReplyFromDatabase", replyData);
		DatabaseProcess::getInstance().sendToCentralServer(reply, true);
	}

	if (m_success)
	{
		if (m_requestedBy.isValid())
			LOG("CustomerService", ("Player:rename character request SUCCESS for %s (%s -> %s) for stationId %lu by %s", m_characterId.getValueString().c_str(), Unicode::wideToNarrow(*m_characterOldName).c_str(), Unicode::wideToNarrow(*m_characterName).c_str(), m_stationId, m_requestedBy.getValueString().c_str()));
		else
			LOG("CustomerService", ("Player:rename character request SUCCESS for %s (%s -> %s) for stationId %lu", m_characterId.getValueString().c_str(), Unicode::wideToNarrow(*m_characterOldName).c_str(), Unicode::wideToNarrow(*m_characterName).c_str(), m_stationId));
	}
	else
	{
		if (m_requestedBy.isValid())
			LOG("CustomerService", ("Player:rename character request FAILED for %s (%s -> %s) for stationId %lu by %s", m_characterId.getValueString().c_str(), Unicode::wideToNarrow(*m_characterOldName).c_str(), Unicode::wideToNarrow(*m_characterName).c_str(), m_stationId, m_requestedBy.getValueString().c_str()));
		else
			LOG("CustomerService", ("Player:rename character request FAILED for %s (%s -> %s) for stationId %lu", m_characterId.getValueString().c_str(), Unicode::wideToNarrow(*m_characterOldName).c_str(), Unicode::wideToNarrow(*m_characterName).c_str(), m_stationId));
	}
}

// ----------------------------------------------------------------------

bool RenameCharacterCustomPersistStep::hasPendingRenameCharacterCustomPersistStep(const NetworkId &characterId)
{
	return (s_pendingRenameCharacterCustomPersistStep.count(characterId) > 0);
}

// ======================================================================
