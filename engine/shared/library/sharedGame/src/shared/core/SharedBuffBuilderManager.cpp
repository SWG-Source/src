// ======================================================================
//
// SharedBuffBuilderManager.cpp
// copyright (c) 2006 Sony Online Entertainment
//
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/SharedBuffBuilderManager.h"

#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Os.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include "UnicodeUtils.h"

#include <algorithm>
#include <map>

// ======================================================================

namespace SharedBuffBuilderManagerNamespace
{
	std::map<NetworkId, SharedBuffBuilderManager::Session> ms_sessions;
	bool ms_installed = false;
	const std::string ms_invalidString("invalid");
	const std::string ms_reactiveSecondChanceComponentName = "reactive_second_chance";
}

SharedBuffBuilderManager::BuffBuilderDataType * SharedBuffBuilderManager::ms_buffBuilderData=nullptr;
using namespace SharedBuffBuilderManagerNamespace;

// ======================================================================

SharedBuffBuilderManagerSession::SharedBuffBuilderManagerSession()
: bufferId(),
  recipientId(),
  startingTime(0),
  bufferRequiredCredits(0),
  accepted(false),
  buffComponents()
{
}

//----------------------------------------------------------------------

SharedBuffBuilderManagerSession::SharedBuffBuilderManagerSession(BuffBuilderChangeMessage const & message)
: bufferId(message.getBufferId()),
  recipientId(message.getRecipientId()),
  startingTime(message.getStartingTime()),
  bufferRequiredCredits(message.getBufferRequiredCredits()),
  accepted(message.getAccepted()),
  buffComponents(message.getBuffComponents())
{
}

bool SharedBuffBuilderManagerSession::addBuffComponent(std::string const& name, int expertiseModifier)
{
	const int maxValue = SharedBuffBuilderManager::getMaxTimesAppliedForRecordName(name);
	int value = 1;

	if(buffComponents.find(name) != buffComponents.end())
	{
		if(buffComponents[name].first < maxValue)
		{
			value = buffComponents[name].first + 1;
		}
		else
		{
			return false;
		}
	}

	buffComponents[name].first = value;
	buffComponents[name].second = expertiseModifier;
	return true;
}

void SharedBuffBuilderManagerSession::removeBuffComponent(std::string const& name)
{
	std::map<std::string, std::pair<int,int> >::iterator pos;
	pos = buffComponents.find(name);
	if(pos != buffComponents.end())
	{
		if(pos->second.first > 1)
		{
			buffComponents[name].first = pos->second.first - 1;
		}
		else
		{
			buffComponents.erase(pos);
		}
	}
}

void SharedBuffBuilderManagerSession::clearBuffComponents()
{
	buffComponents.clear();	
}

int SharedBuffBuilderManagerSession::getBuffComponentValue(std::string const& name)
{
	if(buffComponents.find(name) != buffComponents.end())
	{
		return buffComponents[name].first;
	}

	return 0;
}

// ======================================================================

void SharedBuffBuilderManager::install ()
{
	InstallTimer const installTimer("SharedBuffBuilderManager::install ");

	DEBUG_FATAL(ms_installed, ("SharedBuffBuilderManager::install - Already Installed"));
	ms_installed = true;

	ExitChain::add(SharedBuffBuilderManager::remove, "SharedBuffBuilderManager::remove", 0, false);

	ms_buffBuilderData = new BuffBuilderDataType;
	
	{
		DataTable const * const buffBuilderDataTable = DataTableManager::getTable("datatables/buff/buff_builder.iff", true);
		FATAL(!buffBuilderDataTable,("buff_builder data table could not be opened."));
		int const numRows = buffBuilderDataTable->getNumRows();
		for (int row=0; row<numRows; ++row)
		{
			BuffBuilderDataTypeRecord data;

			data.m_name              = Unicode::getTrim(buffBuilderDataTable->getStringValue("NAME",row));
			data.m_category          = Unicode::getTrim(buffBuilderDataTable->getStringValue("CATEGORY",row));
			data.m_affects           = Unicode::getTrim(buffBuilderDataTable->getStringValue("AFFECTS",row));
			data.m_maxTimesApplied   = buffBuilderDataTable->getIntValue("MAX_TIMES_APPLIED",row);
			data.m_cost              = buffBuilderDataTable->getIntValue("COST",row);
			data.m_affectAmount      = buffBuilderDataTable->getIntValue("AFFECT_AMOUNT",row);
			data.m_requiredExpertise = Unicode::getTrim(buffBuilderDataTable->getStringValue("REQUIRED_EXPERTISE",row));

			(*ms_buffBuilderData)[data.m_name] = data;
		}
		DataTableManager::close("datatables/buff/buff_builder.iff");
	}
}

//----------------------------------------------------------------------

void SharedBuffBuilderManager::remove ()
{
	DEBUG_FATAL(!ms_installed, ("SharedImageDesignerManager::remove - Not Installed"));
	delete ms_buffBuilderData;
	ms_buffBuilderData = 0;
	ms_installed = false;
}

//----------------------------------------------------------------------

void SharedBuffBuilderManager::startSession(SharedBuffBuilderManager::Session const & session)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("SharedBuffBuilderManager::startSession SharedBuffBuilderManager not installed"));
		return; 
	}

	NetworkId const & bufferId = session.bufferId;
	if(isCurrentlyBuffing(bufferId))
	{
		endSession(bufferId);
	}
	ms_sessions[bufferId] = session;
}

//----------------------------------------------------------------------

void SharedBuffBuilderManager::updateSession(SharedBuffBuilderManager::Session const & session)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("SharedBuffBuilderManager::updateSession - SharedBuffBuilderManager not installed"));
		return; 
	}

	ms_sessions[session.bufferId] = session;
}

//----------------------------------------------------------------------

bool SharedBuffBuilderManager::getSession(NetworkId const & bufferId, SharedBuffBuilderManager::Session & /*OUT*/ session)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("SharedBuffBuilderManager::getSession - SharedBuffBuilderManager not installed"));
		return false; 
	}

	std::map<NetworkId, SharedBuffBuilderManager::Session>::const_iterator i = ms_sessions.find(bufferId);
	if(i != ms_sessions.end())
	{
		session = i->second;
		return true;
	}
	else 
		return false;
}

//----------------------------------------------------------------------

bool SharedBuffBuilderManager::isCurrentlyBuffing(NetworkId const & bufferId)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("SharedBuffBuilderManager::isCurrentlyBuffing - SharedBuffBuilderManager not installed"));
		return false; 
	}

	std::map<NetworkId, SharedBuffBuilderManager::Session>::const_iterator i = ms_sessions.find(bufferId);
	return i != ms_sessions.end();
}

//----------------------------------------------------------------------

bool SharedBuffBuilderManager::isCurrentlyBeingBuffed(NetworkId const & recipientId)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("SharedBuffBuilderManager::isCurrentlyBeingBuffed - SharedBuffBuilderManager not installed"));
		return false; 
	}

	for(std::map<NetworkId, SharedBuffBuilderManager::Session>::const_iterator i = ms_sessions.begin(); i != ms_sessions.end(); ++i)
	{
		if(i->second.recipientId == recipientId)
			return true;
	}
	return false;
}

//----------------------------------------------------------------------

void SharedBuffBuilderManager::endSession(NetworkId const & bufferId)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("SharedBuffBuilderManager::endSession - SharedBuffBuilderManager not installed"));
		return; 
	}

	if(!isCurrentlyBuffing(bufferId))
		return;

	IGNORE_RETURN(ms_sessions.erase(bufferId));
}

//----------------------------------------------------------------------

time_t SharedBuffBuilderManager::getTimeRemaining(NetworkId const & bufferId)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("SharedBuffBuilderManager::getTimeRemaining - SharedBuffBuilderManager not installed"));
		return 0; 
	}

	if(!isCurrentlyBuffing(bufferId))
		return 0;

	Session session;
	bool const result = getSession(bufferId, session);
	if(!result)
		return 0;

	if(session.startingTime == 0)
		return 0;

	time_t const elapsedTime = Os::getRealSystemTime() - session.startingTime;

	time_t endTime = 0;

	time_t timeLeft = (endTime - session.startingTime) - elapsedTime;
	if(timeLeft < 0)
		timeLeft = 0;
	return timeLeft;
}

//----------------------------------------------------------------------

bool SharedBuffBuilderManager::isSessionValid(SharedBuffBuilderManager::Session const & session /*, SharedBuffBuilderManager::SkillMods const & designerSkills, std::string const & recipientSpeciesGender, CustomizationData * const customizationDataCreature, CustomizationData * const customizationDataHair*/)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("SharedBuffBuilderManager::isSessionValid - SharedBuffBuilderManager not installed"));
		return false; 
	}

	//check time left
	time_t const timeLeft = getTimeRemaining(session.bufferId);
	if(timeLeft > 0)
	{
		DEBUG_REPORT_LOG(true /* ju_todo: add a config option */, ("ImageDesigner: Designer[%s] can't change Recipient[%s], not enough time has elapsed", session.bufferId.getValueString().c_str(), session.recipientId.getValueString().c_str()));
		return false;
	}

	return true;
}

//----------------------------------------------------------------------

void SharedBuffBuilderManager::populateChangeMessage(SharedBuffBuilderManager::Session const & session, BuffBuilderChangeMessage & /*OUT*/ msg)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("SharedBuffBuilderManager::populateChangeMessage - SharedBuffBuilderManager not installed"));
		return; 
	}

	msg.setBufferId(session.bufferId);
	msg.setRecipientId(session.recipientId);
	msg.setStartingTime(session.startingTime);
	msg.setOrigin(BuffBuilderChangeMessage::O_BUFFER);
	msg.setBufferRequiredCredits(session.bufferRequiredCredits);
	msg.setAccepted(session.accepted);
	msg.setBuffComponents(session.buffComponents);
}

//----------------------------------------------------------------------

int SharedBuffBuilderManager::computeAdjustedAffectAmount(const std::string& recordName,int affectAmount,int expertiseModifier)
{
	int value = affectAmount;

	const std::string& categoryName = SharedBuffBuilderManager::getCategoryNameForRecordName(recordName);

	if(categoryName == "attributes"
		|| categoryName == "resistances"
		|| categoryName == "trade"
		)
	{
		value += ((value * expertiseModifier)/100);
	}
	else if(categoryName == "combat")
	{
		value += expertiseModifier;
	}
	else if(recordName == ms_reactiveSecondChanceComponentName)
	{
		value += expertiseModifier;
	}

	return value;
}

//----------------------------------------------------------------------

// datatable stuff

SharedBuffBuilderManager::BuffBuilderDataTypeRecord::BuffBuilderDataTypeRecord() :
m_name(),
m_category(),
m_affects(),
m_maxTimesApplied(0),
m_cost(0),
m_affectAmount(0),
m_requiredExpertise()
{
}

//----------------------------------------------------------------------

const std::string& SharedBuffBuilderManager::getCategoryNameForRecordName(const std::string& recordName)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("SharedBuffBuilderManager::getCategoryNameForRecordName - SharedBuffBuilderManager not installed"));
		return ms_invalidString; 
	}

	if(ms_installed && ms_buffBuilderData)
	{
		BuffBuilderDataType::const_iterator const i = ms_buffBuilderData->find(recordName);
		if(i != ms_buffBuilderData->end())
		{
			BuffBuilderDataTypeRecord const &data = i->second;
			return data.m_category;
		}
	}

	return ms_invalidString;
}

//----------------------------------------------------------------------

const std::string& SharedBuffBuilderManager::getAffectsNameForRecordName(const std::string& recordName)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("SharedBuffBuilderManager::getAffectsNameForRecordName - SharedBuffBuilderManager not installed"));
		return ms_invalidString; 
	}

	if(ms_installed && ms_buffBuilderData)
	{
		BuffBuilderDataType::const_iterator const i = ms_buffBuilderData->find(recordName);
		if(i != ms_buffBuilderData->end())
		{
			BuffBuilderDataTypeRecord const &data = i->second;
			return data.m_affects;
		}
	}

	return ms_invalidString;
}

//----------------------------------------------------------------------

int SharedBuffBuilderManager::getMaxTimesAppliedForRecordName(const std::string& recordName)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("SharedBuffBuilderManager::getMaxTimesAppliedForRecordName - SharedBuffBuilderManager not installed"));
		return 0; 
	}

	if(ms_installed && ms_buffBuilderData)
	{
		BuffBuilderDataType::const_iterator const i = ms_buffBuilderData->find(recordName);
		if(i != ms_buffBuilderData->end())
		{
			BuffBuilderDataTypeRecord const &data = i->second;
			return data.m_maxTimesApplied;
		}
	}

	return 0;
}

//----------------------------------------------------------------------

int SharedBuffBuilderManager::getCostForRecordName(const std::string& recordName)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("SharedBuffBuilderManager::getCostForRecordName - SharedBuffBuilderManager not installed"));
		return 0; 
	}

	if(ms_installed && ms_buffBuilderData)
	{
		BuffBuilderDataType::const_iterator const i = ms_buffBuilderData->find(recordName);
		if(i != ms_buffBuilderData->end())
		{
			BuffBuilderDataTypeRecord const &data = i->second;
			return data.m_cost;
		}
	}

	return 0;
}

//----------------------------------------------------------------------

int SharedBuffBuilderManager::getAffectAmountForRecordName(const std::string& recordName)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("SharedBuffBuilderManager::getAffectAmountForRecordName - SharedBuffBuilderManager not installed"));
		return 0; 
	}

	if(ms_installed && ms_buffBuilderData)
	{
		BuffBuilderDataType::const_iterator const i = ms_buffBuilderData->find(recordName);
		if(i != ms_buffBuilderData->end())
		{
			BuffBuilderDataTypeRecord const &data = i->second;
			return data.m_affectAmount;
		}
	}

	return 0;
}

//----------------------------------------------------------------------

const std::string& SharedBuffBuilderManager::getRequiredExpertiseNameForRecordName(const std::string& recordName)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("SharedBuffBuilderManager::getRequiredExpertiseNameForRecordName - SharedBuffBuilderManager not installed"));
		return ms_invalidString; 
	}

	if(ms_installed && ms_buffBuilderData)
	{
		BuffBuilderDataType::const_iterator const i = ms_buffBuilderData->find(recordName);
		if(i != ms_buffBuilderData->end())
		{
			BuffBuilderDataTypeRecord const &data = i->second;
			return data.m_requiredExpertise;
		}
	}

	return ms_invalidString;
}

//----------------------------------------------------------------------

void SharedBuffBuilderManager::getRecordNames(std::vector<std::string>& recordNames)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("SharedBuffBuilderManager::getRecordNames - SharedBuffBuilderManager not installed"));
		return; 
	}

	if(ms_installed && ms_buffBuilderData)
	{
		BuffBuilderDataType::iterator i;
		for(i = ms_buffBuilderData->begin(); i != ms_buffBuilderData->end(); ++i)
		{
			BuffBuilderDataTypeRecord const &data = i->second;
			recordNames.push_back(data.m_name);
		}
	}

	return;
}

//----------------------------------------------------------------------

void SharedBuffBuilderManager::getCategoryNames(std::vector<std::string>& categoryNames)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(true, ("SharedBuffBuilderManager::getCategoryNames - SharedBuffBuilderManager not installed"));
		return; 
	}

	if(ms_installed && ms_buffBuilderData)
	{
		BuffBuilderDataType::iterator i;
		std::vector<std::string>::iterator j;
		for(i = ms_buffBuilderData->begin(); i != ms_buffBuilderData->end(); ++i)
		{
			BuffBuilderDataTypeRecord const &data = i->second;
			j = find(categoryNames.begin(),categoryNames.end(),data.m_category);
			if(j == categoryNames.end())
			{
				categoryNames.push_back(data.m_category);
			}
		}
	}

	return;
}

//----------------------------------------------------------------------

