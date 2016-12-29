// ======================================================================
//
// Snapshot.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/Snapshot.h"

#include "serverDatabase/AbstractTableBuffer.h"
#include "serverDatabase/CustomPersistStep.h"
#include "serverDatabase/ObjectLocator.h"
#include "serverDatabase/SaveTimestampQuery.h"
#include "serverUtility/ServerClock.h"
#include "sharedDatabaseInterface/DbSession.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedLog/Log.h"

// ======================================================================

int Snapshot::ms_creationCount=0;
int Snapshot::ms_deletionCount=0;

// ======================================================================

Snapshot::Snapshot(DB::ModeQuery::Mode mode, bool useGoldDatabase) :
		m_universeAuthorityHack(false),
		m_isBeingSaved(false),
		m_useGoldDatabase(useGoldDatabase),
		m_mode(mode),
		m_timestamp(0)
{
	LOG("Snapshot",("Created snapshot"));
	++ms_creationCount;
}

// ----------------------------------------------------------------------

Snapshot::~Snapshot()
{
	for (LocatorListType::iterator i=m_locatorList.begin(); i!=m_locatorList.end(); ++i)
	{
		delete *i;
		*i = nullptr;
	}

	for (CustomStepListType::iterator j=m_customStepList.begin(); j!=m_customStepList.end(); ++j) {
		delete *j;
		*j = nullptr;
	}

	m_locatorList.clear();
	m_customStepList.clear();

	++ms_deletionCount;
	LOG("Snapshot",("Deleted snapshot.  %i outstanding, %i created, %i deleted", ms_creationCount-ms_deletionCount,ms_creationCount,ms_deletionCount));
}

// ----------------------------------------------------------------------

/**
 * Called by a worker thread.  Saves the snapshot to the database
 *
 * Saves all the buffers in the list and runs any custom steps.
 */

bool Snapshot::saveToDB(DB::Session *session)
{
	NOT_NULL(session);

	if (m_timestamp != 0 && !saveTimestamp(session)) {
		return false;
	}

	return true;
}
// ----------------------------------------------------------------------

/**
 * Called when a message is received to update an object's data.
 *
 * Identifies what package the message is part of, and invokes the
 * appropriate (virtual) function.
 */

void Snapshot::handleDeltasMessage(NetworkId & objectId, const DeltasMessage &msg)
{
	unsigned char packageId=msg.getPackageId();
	Tag typeId=msg.getTypeId();
	Archive::ReadIterator bs=msg.getPackage().begin();
	uint16 count;
	Archive::get(bs,count);
	DEBUG_REPORT_LOG(count==0,("Deltas package was empty.\n"));
	for (uint16 i=0; i< count; ++i)
	{
		unsigned short int index;
		Archive::get(bs,index);

		switch (packageId) {
 			case DeltasMessage::DELTAS_SERVER:
				decodeServerData(objectId,typeId,index,bs,false);
				break;
			case DeltasMessage::DELTAS_SHARED:
				decodeSharedData(objectId,typeId,index,bs,false);
				break;
			case DeltasMessage::DELTAS_CLIENT_SERVER:
				decodeClientData(objectId,typeId,index,bs,false);
				break;
			case DeltasMessage::DELTAS_FIRST_PARENT_CLIENT_SERVER:
				decodeParentClientData(objectId,typeId,index,bs,false);
				break;
                        case DeltasMessage::DELTAS_SERVER_NP:
                        case DeltasMessage::DELTAS_SHARED_NP:
                        case DeltasMessage::DELTAS_CLIENT_SERVER_NP:			
			case DeltasMessage::DELTAS_FIRST_PARENT_CLIENT_SERVER_NP:
			case DeltasMessage::DELTAS_UI:
				break;
			default:
				FATAL(true,("PackageId was invalid."));
				break;
		}
	}
}

// ----------------------------------------------------------------------

/**
 * Called when a message is received to update an object's data.
 *
 * Identifies what package the message is part of, and invokes the
 * appropriate (virtual) function.
 */

void Snapshot::handleBaselinesMessage(NetworkId &objectId, const BaselinesMessage &msg)
{
	unsigned char packageId=msg.getPackageId();
	Tag typeId=msg.getTypeId();
	Archive::ReadIterator bs=msg.getPackage().begin();
	uint16 count;
	Archive::get(bs,count);
	for (uint16 i=0; i< count; ++i)
	{
		switch (packageId){
			case BaselinesMessage::BASELINES_SERVER :
				decodeServerData(objectId,typeId,i,bs,true);
				break;	
			case BaselinesMessage::BASELINES_SHARED:
				decodeSharedData(objectId,typeId,i,bs,true);
				break;
			case BaselinesMessage::BASELINES_CLIENT_SERVER :
				decodeClientData(objectId,typeId,i,bs,true);
				break;
			case BaselinesMessage::BASELINES_FIRST_PARENT_CLIENT_SERVER:
				decodeParentClientData(objectId,typeId,i,bs,true);
				break;
			case BaselinesMessage::BASELINES_SERVER_NP :
			case BaselinesMessage::BASELINES_SHARED_NP :
			case BaselinesMessage::BASELINES_CLIENT_SERVER_NP :
			case BaselinesMessage::BASELINES_UI :
			case BaselinesMessage::BASELINES_FIRST_PARENT_CLIENT_SERVER_NP :
				break;	
			default:
				FATAL(true,("PackageId was not BASELINES_SERVER, BASELINES_SHARED, or BASELINES_CLIENT.\n"));
				break;
		
		}	
	}
}

// ----------------------------------------------------------------------

void Snapshot::addLocator(ObjectLocator *newLocator)
{
		
	NOT_NULL(newLocator);
	m_locatorList.push_back(newLocator);
}

// ----------------------------------------------------------------------

int Snapshot::getLocatorCount() const
{
	return m_locatorList.size();
}

// ----------------------------------------------------------------------

void Snapshot::takeTimestamp()
{
	DEBUG_FATAL(m_timestamp!=0,("takeTimestamp was invoked more than once.\n"));
	m_timestamp=ServerClock::getInstance().getGameTimeSeconds();
}

// ----------------------------------------------------------------------

bool Snapshot::saveTimestamp(DB::Session *session)
{
	DEBUG_FATAL(m_timestamp==0,("Can't save unset timestamp.\n"));
	NOT_NULL(session);

	DBQuery::SaveTimestampQuery stq(m_timestamp);
	return session->exec(&stq);
}

// ----------------------------------------------------------------------

void Snapshot::addCustomPersistStep(CustomPersistStep *newStep)
{
	NOT_NULL(newStep);
	m_customStepList.push_back(newStep);
}

// ----------------------------------------------------------------------

void Snapshot::saveCompleted()
{
	for (CustomStepListType::iterator step=m_customStepList.begin(); step !=m_customStepList.end(); ++step)
	{
		NOT_NULL(*step);
		(*step)->onComplete();
	}
}

// ======================================================================
