// ======================================================================
//
// PurgeCompleteCustomPersistStep.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverDatabase/FirstServerDatabase.h"
#include "serverDatabase/PurgeCompleteCustomPersistStep.h"

#include "serverDatabase/DatabaseProcess.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"

// ======================================================================

PurgeCompleteCustomPersistStep::PurgeCompleteCustomPersistStep(StationId stationId) :
		m_stationId(stationId)
{
}

// ----------------------------------------------------------------------

PurgeCompleteCustomPersistStep::~PurgeCompleteCustomPersistStep()
{
}

// ----------------------------------------------------------------------

bool PurgeCompleteCustomPersistStep::beforePersist(DB::Session *session)
{
	UNREF(session);
	return true;
}

// ----------------------------------------------------------------------

bool PurgeCompleteCustomPersistStep::afterPersist(DB::Session *session)
{
	UNREF(session);
	return true;
}

// ----------------------------------------------------------------------

void PurgeCompleteCustomPersistStep::onComplete()
{
	GenericValueTypeMessage<StationId> msg("PurgeCompleteMessage", m_stationId);
	DatabaseProcess::getInstance().sendToCentralServer(msg,true);
}

// ======================================================================
