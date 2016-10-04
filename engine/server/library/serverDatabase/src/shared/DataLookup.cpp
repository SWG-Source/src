// ======================================================================
//
// DataLookup.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/DataLookup.h"

#include "UnicodeUtils.h"
#include "serverDatabase/ConfigServerDatabase.h"
#include "serverDatabase/DatabaseProcess.h"
#include "serverDatabase/GameServerConnection.h"
#include "serverDatabase/TaskCheckCharacterName.h"
#include "serverDatabase/TaskGetStructuresForPurge.h"
#include "serverDatabase/TaskGetBiography.h"
#include "serverDatabase/TaskSetBiography.h"
#include "serverNetworkMessages/BiographyMessage.h"
#include "serverNetworkMessages/CentralGameServerMessages.h"
#include "serverNetworkMessages/RequestBiographyMessage.h"
#include "serverNetworkMessages/RetrievedItemLoadMessage.h"
#include "sharedDatabaseInterface/DbTaskQueue.h"
#include "sharedFoundation/Crc.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/NameErrors.h"
#include "unicodeArchive/UnicodeArchive.h"

#include "sharedFoundation/CrcConstexpr.hpp"

//-----------------------------------------------------------------------

DataLookup *DataLookup::ms_theInstance = nullptr;

//-----------------------------------------------------------------------

void DataLookup::install()
{
	DEBUG_FATAL(ms_theInstance,("Called DataLookup::install twice.\n"));
	ms_theInstance = new DataLookup;

	ExitChain::add(&remove, "DataLookup::remove");
}

// ----------------------------------------------------------------------

void DataLookup::remove()
{
	DEBUG_FATAL(!ms_theInstance,("DataLookup was not installed.\n"));
	delete ms_theInstance;
	ms_theInstance = 0;
}

// ----------------------------------------------------------------------

DataLookup::DataLookup() :
		MessageDispatch::Receiver(),
		taskQueue(new DB::TaskQueue(1,DatabaseProcess::getInstance().getDBServer(),2)),
		m_reservations()
{
	connectToMessage("BiographyMessage");
	connectToMessage("PurgeStructuresForAccountMessage");
	connectToMessage("ReleaseNameMessage");
	connectToMessage("RequestBiographyMessage");
	connectToMessage("VerifyNameRequest");
	connectToMessage("WarnStructuresAboutPurgeMessage");
}

//-----------------------------------------------------------------------

DataLookup::~DataLookup()
{
	DEBUG_FATAL(taskQueue,("Call shutdown() before deleting DataLookup.\n"));

	while (!m_reservations.empty())
	{
		deleteReservationList(m_reservations.begin()->first);
	}
}

// ----------------------------------------------------------------------

void DataLookup::update(real uptime)
{
	UNREF(uptime);

	taskQueue->update(ConfigServerDatabase::getDefaultQueueUpdateTimeLimit());
}

// ----------------------------------------------------------------------

void DataLookup::receiveMessage(const MessageDispatch::Emitter &source, const MessageDispatch::MessageBase &message)
{
	UNREF(source);
	
	const uint32 messageType = message.getType();
	
	switch(messageType) {
		case constcrc("VerifyNameRequest") :
		{
			const GameServerConnection * g = safe_cast<const GameServerConnection *>(&source);
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			VerifyNameRequest * vnr = new VerifyNameRequest(ri);
			
			LOG("TraceCharacterCreation", ("%d received VerifyNameRequest", vnr->getStationId()));
			verifyName(g->getProcessId(), vnr);
			break;
		}
		case constcrc("ReleaseNameMessage") :
		{
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			ReleaseNameMessage rnm(ri);

			LOG("TraceCharacterCreation", ("%d received ReleaseNameMessage", rnm.getStationId()));

			releaseName(rnm.getStationId(), rnm.getCharacterId());
			break;
		}
		case constcrc("RequestBiographyMessage") :
		{
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			RequestBiographyMessage msg(ri);

			const GameServerConnection *conn = dynamic_cast<const GameServerConnection*>(&source);
			if (conn)
				getBiography(msg.getOwner(),conn->getProcessId());
			else
				DEBUG_WARNING(true,("Got RequestBiographyMessage from something that wasn't a GameServerConnection.\n"));
			
			break;
		}
		case constcrc("BiographyMessage") :
		{
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			BiographyMessage msg(ri);

			setBiography(msg.getOwner(),msg.getBio());
			break;
		}
		case constcrc("PurgeStructuresForAccountMessage") :
		{
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			GenericValueTypeMessage<StationId> msg(ri);

			getStructuresForPurge(msg.getValue(), false);
			break;
		}
		case constcrc("WarnStructuresAboutPurgeMessage") :
		{
			Archive::ReadIterator ri = static_cast<const GameNetworkMessage &>(message).getByteStream().begin();
			GenericValueTypeMessage<StationId> msg(ri);

			getStructuresForPurge(msg.getValue(), true);
			break;
		}
		default :
		{
			DEBUG_REPORT_LOG(true,("Unrecognized message recieved by DataLookup.\n"));
			break;
		}
	}
}

// ----------------------------------------------------------------------

void DataLookup::onRetrieveItemComplete(NetworkId const &ownerId, NetworkId const &itemId) const
{
	RetrievedItemLoadMessage msg(ownerId, itemId);
	std::vector<uint32> gameServerIds;
	DatabaseProcess::getInstance().getGameServerProcessIds(gameServerIds);
	for (std::vector<uint32>::iterator i = gameServerIds.begin(); i != gameServerIds.end(); ++i)
		DatabaseProcess::getInstance().getConnectionByProcess(*i)->send(msg, true);
}

// ----------------------------------------------------------------------

void DataLookup::verifyName(uint32 gameServer, VerifyNameRequest * request)
{
	DEBUG_REPORT_LOG(true, ("verifyName: stationId %lu, process %lu, name %s: ", request->getStationId(), gameServer, Unicode::wideToNarrow(request->getCharacterName()).c_str()));
	Unicode::String name = normalizeName(request->getCharacterName());

	reservationList * rl = getReservationList(request->getStationId());

	// if request is to verify name for character rename, free up all
	// reservations that have been made for character create; there's
	// no possibility that a character create is happening if there's
	// a rename happening
	if (rl && request->getCharacterId().isValid())
	{
		releaseName(request->getStationId(), NetworkId::cms_invalid);
		rl = getReservationList(request->getStationId());
	}

	if (rl) // already have m_reservations for this character
	{
		// see if we have any equivalent m_reservations
		reservationList::iterator i;
		for (i=rl->begin(); i!=rl->end(); ++i)
		{
			// equivalent reservation; if it has been approved, 
			// response immediately; otherwise make a new reservation, 
			// but don't request a verification.  When the original
			// is approved, all matching m_reservations will be approved
			if ((i->name == name) && (request->getCharacterId() == i->request->getCharacterId()))
			{
				if (i->approved)
				{
					DEBUG_REPORT_LOG(true, ("Reserved and already approved.\n"));
					GameServerConnection * g = DatabaseProcess::getInstance().getConnectionByProcess(gameServer);
					VerifyNameResponse vnr(request->getStationId(), request->getCharacterId(), request->getCreatureTemplate(), request->getCharacterName(), NameErrors::nameApproved);
					LOG("TraceCharacterCreation", ("%d Name reservation already approved, sending VeryfyNameResponse(%d, %s, %s) to game server", request->getStationId(), request->getStationId(), request->getCreatureTemplate().c_str(), Unicode::wideToNarrow(request->getCharacterName()).c_str()));
					if (g)
						g->send(vnr, true);
					// now move the reservation to the front of the queue
					reservation temp(*i);
					IGNORE_RETURN(rl->erase(i));
					addReservation(rl, temp);
					delete request;
					return;
				}
				else
				{
					LOG("TraceCharacterCreation", ("%d Name reservation not yet approved", request->getStationId()));
					DEBUG_REPORT_LOG(true, ("Not yet approved.\n"));
					reservation temp;
					temp.gameServer = gameServer;
					temp.name = name;
					temp.approved = false;
					temp.request = request;
					addReservation(rl, temp);
					return;
				}
			}
		}
	}

	DEBUG_REPORT_LOG(true, ("No matching reservation.\n"));

	// If we get here, there are no matching m_reservations for our client. But there
	// may be m_reservations from other clients conflicting with ours.

	std::map<uint32, reservationList *>::iterator j;
	for (j=m_reservations.begin(); j != m_reservations.end(); ++j)
	{
		reservationList * otherReservationList = j->second;
		reservationList::iterator i;
		for (i = otherReservationList->begin(); i!=otherReservationList->end(); ++i)
		{
			if (i->name == name)
			{
				LOG("TraceCharacterCreation", ("Name (%s) requested by station ID %lu is already reserved by another station ID %lu and/or another character on the same station ID", Unicode::wideToNarrow(request->getCharacterName()).c_str(), request->getStationId(), j->first));
				DEBUG_REPORT_LOG(true, ("Name (%s) requested by station ID %lu is already reserved by another station ID %lu and/or another character on the same station ID\n", Unicode::wideToNarrow(request->getCharacterName()).c_str(), request->getStationId(), j->first));
				GameServerConnection * g = DatabaseProcess::getInstance().getConnectionByProcess(gameServer);
				VerifyNameResponse vnr(request->getStationId(), request->getCharacterId(), request->getCreatureTemplate(), request->getCharacterName(), NameErrors::nameDeclinedInUse);
				g->send(vnr, true);
				delete request;
				return;
			}
		}
	}

	// nothing else has this name reserved

	reservation temp;
	temp.gameServer = gameServer;
	temp.name = name;
	temp.request = request;
	temp.approved = false;

	if (!rl)
		rl = newReservationList(request->getStationId());

	addReservation(rl, temp);

	LOG("TraceCharacterCreation", ("%d Reserving name and verifying it against the DB", request->getStationId()));
	DEBUG_REPORT_LOG(true, ("Reserving and verifying\n"));

	TaskCheckCharacterName *task = new TaskCheckCharacterName(request->getStationId(), name);
	taskQueue->asyncRequest(task);	
}

//--------------------------------------------------------------------

void DataLookup::onCharacterNameChecked(uint32 stationId, const Unicode::String &name, int resultCode)
{
	StringId reason(NameErrors::nameApproved);
	switch (resultCode)
	{
		case 0: break;
		case 1: reason = NameErrors::nameDeclinedInUse; break;
		default: reason = NameErrors::nameDeclinedInternalError; break;
	}

	reservationList * rl = getReservationList(stationId);
	if (!rl)
	{
		DEBUG_REPORT_LOG(true, ("Received onCharacterNameChecked (%s) for non-pending name stationid %lu\n", reason.getText().c_str(), stationId));
		return;
	}

	reservationList::iterator i;
	for (i=rl->begin(); i!=rl->end(); ++i)
	{
		if (i->name == name)
		{
			LOG("TraceCharacterCreation", ("sending VerifyNameResponse(%d, %s, %s, ?) to game server", stationId, i->request->getCreatureTemplate().c_str(), Unicode::wideToNarrow(i->request->getCharacterName()).c_str()));
			VerifyNameResponse vnr(stationId, i->request->getCharacterId(), i->request->getCreatureTemplate(), i->request->getCharacterName(), reason);
			GameServerConnection * g = DatabaseProcess::getInstance().getConnectionByProcess(i->gameServer);
			if (g)
				g->send(vnr, true);
			else
				DEBUG_REPORT_LOG(true,("VerifyNameResponse for %lu dropped because GameServer has dropped connection.\n",stationId));
		}
	}

	if (reason != NameErrors::nameApproved)
	{
		LOG("TraceCharacterCreation", ("%d name released", stationId));
		DEBUG_REPORT_LOG(true, ("Name %s verified for stationId %lu: %s.\n", Unicode::wideToNarrow(name).c_str(), stationId, reason.getText().c_str()));
		releaseName(stationId, name);
	}
	else
	{
		LOG("TraceCharacterCreation", ("%d name %s verified", stationId, Unicode::wideToNarrow(name).c_str()));
		DEBUG_REPORT_LOG(true, ("Name %s verified for stationId %lu: %s, holding reservation.\n", Unicode::wideToNarrow(name).c_str(), stationId, reason.getText().c_str()));
		approveName(stationId, name);
	}
}

//--------------------------------------------------------------------

void DataLookup::releaseName(uint32 stationId, const NetworkId & characterId)
{
	reservationList * rl = getReservationList(stationId);
	if (!rl)
	{
		WARNING(true, ("Releasing names for stationId %lu, characterId %s, but no reservations found.\n", stationId, characterId.getValueString().c_str()));
		return;
	}

	reservationList newReservations;
	reservationList::iterator i;
	int erased = 0;
	for (i=rl->begin(); i!=rl->end(); ++i)
	{
		if (i->request->getCharacterId() != characterId)
		{
			newReservations.push_back(*i);
		}
		else
		{
			delete const_cast<VerifyNameRequest*>(i->request); // non-const for Lint
			++erased;
		}
	}

	DEBUG_REPORT_LOG(true, ("Releasing %d names for stationId %lu, characterId %s\n", erased, stationId, characterId.getValueString().c_str()));

	rl->swap(newReservations);
	if (rl->empty())
	{
		deleteReservationList(stationId);
	}
}

//--------------------------------------------------------------------

void DataLookup::releaseName(uint32 stationId, const Unicode::String &i_name)
{
	reservationList * rl = getReservationList(stationId);
	if (!rl)
	{
		WARNING(true, ("Releasing name [%s] for stationId %lu, but no reservations found.\n", Unicode::wideToNarrow(i_name).c_str(), stationId));
		return;
	}

	reservationList newReservations;
	reservationList::iterator i;
	int erased = 0;
	for (i=rl->begin(); i!=rl->end(); ++i)
	{
		if (i->name != i_name)
		{
			newReservations.push_back(*i);
		}
		else
		{
			delete const_cast<VerifyNameRequest*>(i->request); // non-const for Lint
			++erased;
		}
	}

	DEBUG_REPORT_LOG(true, ("Releasing %d names for stationId %lu, name (%s)\n", erased, stationId, Unicode::wideToNarrow(i_name).c_str()));

	rl->swap(newReservations);
	if (rl->empty())
	{
		deleteReservationList(stationId);
	}
}

//--------------------------------------------------------------------

void DataLookup::approveName(uint32 stationId, const Unicode::String &i_name)
{
	reservationList * rl = getReservationList(stationId);
	if (!rl)
	{
		WARNING_STRICT_FATAL(true, ("Approving name for stationId %lu, but name not found.\n", stationId));
		return;
	}

	reservationList::iterator i;
	for (i=rl->begin(); i!=rl->end(); ++i)
	{
		if (i->name == i_name)
		{
			i->approved = true;
		}
	}
}

//--------------------------------------------------------------------

void DataLookup::releaseNamesForProcess(uint32 gameServer)
{
	DEBUG_REPORT_LOG(true, ("Release name m_reservations for process %lu\n", gameServer));
	std::map<uint32, reservationList *> newReservations;
	std::map<uint32, reservationList *>::iterator i;
	for (i = m_reservations.begin(); i!=m_reservations.end(); ++i)
	{
		reservationList * rl = i->second;
		reservationList newReservationListLocal;
		reservationList::iterator j;
		for (j=rl->begin(); j!=rl->end(); ++j)
		{
			if (j->gameServer != gameServer)
			{
				newReservationListLocal.push_back(*j);
			}
			else
			{
				delete const_cast<VerifyNameRequest*>(j->request); // const-cast to make Lint happy
			}
		}
		rl->swap(newReservationListLocal);
		if (rl->empty())
		{
			delete rl;
		}
		else
		{
			newReservations[i->first] = rl;
		}
	}
	m_reservations.swap(newReservations);
}

//--------------------------------------------------------------------

const Unicode::String DataLookup::normalizeName(const Unicode::String &name) const
{
	Unicode::String result = Unicode::toLower(name);
	result = Unicode::String(result, 0, result.find(' '));

	return result;
}

//--------------------------------------------------------------------

DataLookup::reservationList * DataLookup::getReservationList(uint32 stationId)
{
	std::map<uint32, reservationList *>::iterator i = m_reservations.find(stationId);
	if (i == m_reservations.end())
		return 0;
	else
		return i->second;
}

//--------------------------------------------------------------------

DataLookup::reservationList * DataLookup::newReservationList(uint32 stationId)
{
	reservationList * result = new reservationList;
	DEBUG_FATAL(m_reservations.find(stationId) != m_reservations.end(), ("DataLookup::newReservationList called, but there is already a reservationList for this stationId %lu.", stationId));
	m_reservations[stationId] = result;
	return result;
}

//--------------------------------------------------------------------

void DataLookup::addReservation(DataLookup::reservationList * rl, const DataLookup::reservation &temp)
{
	rl->push_back(temp);
	while (rl->size() > 16)
	{
		delete const_cast<VerifyNameRequest*>(rl->begin()->request);
		rl->pop_front();
	}
}

//--------------------------------------------------------------------

void DataLookup::deleteReservationList(uint32 stationId)
{
	std::map<uint32, reservationList *>::iterator rlIter = m_reservations.find(stationId);
	if (rlIter == m_reservations.end())
		return;

	reservationList * rl = rlIter->second;
	if (!rl)
	{
		WARNING_STRICT_FATAL(true, ("Reservation list for stationID %lu is nullptr", stationId));
		return;
	}
	reservationList::iterator i;
	for (i=rl->begin(); i!=rl->end(); ++i)
	{
		delete const_cast<VerifyNameRequest*>(i->request);
	}
	delete rl;
	IGNORE_RETURN(m_reservations.erase(stationId));
}

// ----------------------------------------------------------------------

void DataLookup::getBiography(const NetworkId &owner, uint32 requestingProcess)
{
	NOT_NULL(taskQueue);
	TaskGetBiography *task = new TaskGetBiography(owner,requestingProcess);
	taskQueue->asyncRequest(task);
}

// ----------------------------------------------------------------------

void DataLookup::setBiography(const NetworkId &owner, const Unicode::String &bio)
{
	NOT_NULL(taskQueue);
	TaskSetBiography *task = new TaskSetBiography(owner,bio);
	taskQueue->asyncRequest(task);
}

// ----------------------------------------------------------------------

void DataLookup::shutdown()
{
	NOT_NULL(taskQueue);
	taskQueue->cancel();
	delete taskQueue;
	taskQueue=0;
}

// ----------------------------------------------------------------------

bool DataLookup::isIdle()
{
	return taskQueue->isIdle();
}

// ----------------------------------------------------------------------

void DataLookup::getStructuresForPurge(StationId account, bool warnOnly)
{
	TaskGetStructuresForPurge *task = new TaskGetStructuresForPurge(account, warnOnly);
	taskQueue->asyncRequest(task);
}

// ======================================================================
