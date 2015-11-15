// ======================================================================
//
// DataLookup.h
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_DataLookup_H
#define INCLUDED_DataLookup_H

// ======================================================================

#include <deque>
#include <map>

#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedFoundation/StationId.h"
#include "sharedMessageDispatch/Receiver.h"
#include "StringId.h"
#include "Unicode.h"

namespace DB
{
	class TaskQueue;
}

class VerifyNameRequest;
class GameNetworkMessage;

// ======================================================================

/**
 * Singleton that looks up specific data from the database.
 *
 * This class manages requests to fetch assorted data from the database.
 * This class is set up so that requests for various random items of data
 * don't have to compete with persistence or loading for worker thread time.
 *
 * Because this class is not synchronized in any way with Persister or
 * Loader, it should not be used to fetch data about specific in-game
 * objects.
 *
 * For example, one use of this class is to manage fetching the list of
 * prohibited character names.
 */
class DataLookup : public MessageDispatch::Receiver
{
public:
	static void install();
	static DataLookup &getInstance();

	void update          (real updateTime);
	void receiveMessage  (const MessageDispatch::Emitter & source, const MessageDispatch::MessageBase & message);

	void onRetrieveItemComplete(NetworkId const &ownerId, NetworkId const &itemId) const;

// name reservation and verification:
	void onCharacterNameChecked (uint32 stationId, const Unicode::String &name, int resultCode);
	void verifyName(uint32 gameServer, VerifyNameRequest * vrn);
	void releaseName(uint32 stationId, const NetworkId & characterId);
	void releaseName(uint32 stationId, const Unicode::String &name);
	void approveName(uint32 stationId, const Unicode::String &name);
	void releaseNamesForProcess(uint32 gameServer);
	const Unicode::String normalizeName(const Unicode::String &name) const;
	const std::string getNameByStationId(StationId stationId) const;
	const std::string getNormalizedNameByStationId(StationId stationId) const;
	bool isIdle();

	void shutdown();
	
protected:
	DB::TaskQueue *taskQueue;
	
	struct reservation
	{
	public:
		const VerifyNameRequest * request;
		Unicode::String name;
		uint32 gameServer;
		bool approved;
	};
	typedef std::deque<reservation> reservationList;
	reservationList * getReservationList(uint32 stationId);
	reservationList * newReservationList(uint32 stationId);
	std::map<uint32, reservationList *> m_reservations;
	static void addReservation(reservationList * rl, const reservation &temp);
	void deleteReservationList(uint32 stationId);

private:
	void getBiography                     (const NetworkId &owner, uint32 requestingProcess);
	void setBiography                     (const NetworkId &owner, const Unicode::String &bio);
	void getStructuresForPurge            (StationId account, bool warnOnly);

  private:
	DataLookup();
	~DataLookup();
	static void remove();
	static DataLookup *ms_theInstance;

  private:
	DataLookup &operator=(const DataLookup&); //disable
	DataLookup(const DataLookup&); //disable
};

// ----------------------------------------------------------------------

inline DataLookup &DataLookup::getInstance()
{
	DEBUG_FATAL(!ms_theInstance,("DataLookup was not installed.\n"));
	return *ms_theInstance;
}

// ======================================================================

#endif
