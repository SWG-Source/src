// ======================================================================
//
// BiographyManager.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/BiographyManager.h"

#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/ServerUniverse.h"
#include "serverNetworkMessages/BiographyMessage.h"
#include "serverNetworkMessages/RequestBiographyMessage.h"
#include "UnicodeUtils.h"

#include <algorithm>
#include <map>
#include <vector>

// ======================================================================

bool BiographyManager::m_installed = false;
std::map<NetworkId, std::vector<CachedNetworkId> > *BiographyManager::m_retrievalQueue;


BiographyManager::BiographyManager()
{
}

// ----------------------------------------------------------------------

BiographyManager::~BiographyManager()
{
}

// ----------------------------------------------------------------------

void BiographyManager::install()
{
	DEBUG_FATAL(m_installed, ("BioManager already installed"));
	m_installed = true;
	m_retrievalQueue = new std::map<NetworkId, std::vector<CachedNetworkId> >;
}

// ----------------------------------------------------------------------

void BiographyManager::remove()
{
	DEBUG_FATAL(!m_installed, ("BioManager not installed"));
	m_installed = false;
	delete m_retrievalQueue;
	m_retrievalQueue=0;
}

// ----------------------------------------------------------------------

void BiographyManager::setBiography(const NetworkId &owner, const Unicode::String &bio)
{
	//DEBUG_REPORT_LOG(true, ("BiographyManager::setBiography - netId: %s\n", owner.getValueString().c_str()));
	DEBUG_FATAL(!m_installed, ("BioManager not installed"));
	
	// Trim off leading and trailing whitespace
	size_t firstNonWhitespace = bio.find_first_not_of(Unicode::narrowToWide(" \t\n"));
	size_t lastNonWhitespace = bio.find_last_not_of(Unicode::narrowToWide(" \t\n"));
	Unicode::String truncatedBio;
	if (firstNonWhitespace==bio.npos || lastNonWhitespace==bio.npos)
		DEBUG_REPORT_LOG(true,("Biography for %s discarded because it was all whitespace.\n",owner.getValueString().c_str()));
	else
		truncatedBio.assign(bio,firstNonWhitespace,lastNonWhitespace+1-firstNonWhitespace);
	
	// If the biography is larger than the databaes can handle then truncate the
	// size the databae can handle
	unsigned int const maximumBiographyLength = 1024;
	if (truncatedBio.size() > maximumBiographyLength)
	{
		WARNING(true, ("The biography for player %s is larger than the maximum allowed characters of %d, truncating.", owner.getValueString().c_str(), maximumBiographyLength));
	}

	// Send bio to the database
	BiographyMessage const msg(owner, (truncatedBio.size() > maximumBiographyLength) ? truncatedBio.substr(0, maximumBiographyLength) : truncatedBio);
	GameServer::getInstance().sendToDatabaseServer(msg);

	// Save off the bio for viewing by other players
	std::map<NetworkId, Unicode::String> const & connectedCharacterBiographyData = ServerUniverse::getConnectedCharacterBiographyData();
	std::map<NetworkId, Unicode::String>::const_iterator iterFindBiography = connectedCharacterBiographyData.find(owner);
	if ((iterFindBiography == connectedCharacterBiographyData.end()) || (iterFindBiography->second != truncatedBio))
		ServerUniverse::setConnectedCharacterBiographyData(owner, truncatedBio);
}

// ----------------------------------------------------------------------

/**
 * Called when a message containing an object's biography is received from the database.
 */
void BiographyManager::onBiographyRetrieved(const NetworkId &owner, const Unicode::String &bio)
{
	//DEBUG_REPORT_LOG(true, ("BiographyManager::onBiographyRetrieved - netId: %s\n", owner.getValueString().c_str()));

	DEBUG_FATAL(!m_installed, ("BioManager not installed"));

	// Save off the bio for viewing by other players
	std::map<NetworkId, Unicode::String> const & connectedCharacterBiographyData = ServerUniverse::getConnectedCharacterBiographyData();
	std::map<NetworkId, Unicode::String>::const_iterator iterFindBiography = connectedCharacterBiographyData.find(owner);
	if ((iterFindBiography == connectedCharacterBiographyData.end()) || (iterFindBiography->second != bio))
		ServerUniverse::setConnectedCharacterBiographyData(owner, bio);

	std::map<NetworkId, std::vector<CachedNetworkId> >::iterator i=m_retrievalQueue->find(owner);
	if (i!=m_retrievalQueue->end())
	{
		std::vector<CachedNetworkId> &targets=(*i).second;
		for (std::vector<CachedNetworkId>::iterator j=targets.begin(); j!=targets.end(); ++j)
		{
			CreatureObject *obj=dynamic_cast<CreatureObject*>((*j).getObject());
			if (obj)
			{
				obj->onBiographyRetrieved(owner,bio);
			}
		}
		m_retrievalQueue->erase(i);
	}
}

// ----------------------------------------------------------------------

/**
 * Delete the biography from the database
 */
void BiographyManager::deleteBiography(const NetworkId &owner)
{
	//DEBUG_REPORT_LOG(true, ("BiographyManager::deleteBiography - netId: %s\n", owner.getValueString().c_str()));

	DEBUG_FATAL(!m_installed, ("BioManager not installed"));
	
	BiographyMessage const msg(owner,nullptr);
	GameServer::getInstance().sendToDatabaseServer(msg);

	// Save off the bio for viewing by other players
	std::map<NetworkId, Unicode::String> const & connectedCharacterBiographyData = ServerUniverse::getConnectedCharacterBiographyData();
	std::map<NetworkId, Unicode::String>::const_iterator iterFindBiography = connectedCharacterBiographyData.find(owner);
	if ((iterFindBiography == connectedCharacterBiographyData.end()) || (!iterFindBiography->second.empty()))
		ServerUniverse::setConnectedCharacterBiographyData(owner, Unicode::String());
}

// ----------------------------------------------------------------------

/**
 * Queue up a request to retrive a biography, and give it to the CreatureObject
 * when retrieved.
 */
void BiographyManager::requestBiography(const NetworkId &owner, CreatureObject * requestedBy)
{
	//DEBUG_REPORT_LOG(true, ("BiographyManager::requestBiography - netId: %s\n", owner.getValueString().c_str()));

	DEBUG_FATAL(!m_installed, ("BioManager not installed"));

	std::map<NetworkId, Unicode::String> const & connectedCharacterBiographyData = ServerUniverse::getConnectedCharacterBiographyData();
	std::map<NetworkId, Unicode::String>::const_iterator iterFindBiography = connectedCharacterBiographyData.find(owner);
	if (iterFindBiography != connectedCharacterBiographyData.end())
	{
		if (requestedBy)
			requestedBy->onBiographyRetrieved(owner,iterFindBiography->second);
	}
	else
	{
		if (requestedBy)
		{
			std::vector<CachedNetworkId> &requesters=(*m_retrievalQueue)[owner];
			std::vector<CachedNetworkId>::const_iterator alreadyThere=std::find(requesters.begin(),requesters.end(),CachedNetworkId(*requestedBy));
			if (alreadyThere == requesters.end())
				requesters.push_back(CachedNetworkId(*requestedBy));
		}

		RequestBiographyMessage const msg(owner);
		GameServer::getInstance().sendToDatabaseServer(msg);
	}
}

// ======================================================================
