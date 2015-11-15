//------------------------------------------
// CentralConnectionServerMessages.cpp
// copyright 2001 Sony Online Entertainment
//------------------------------------------

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/RandomName.h"

//====================================================================

RandomNameRequest::RandomNameRequest(uint32 stationId, const std::string & creatureTemplate) :
GameNetworkMessage("RandomNameRequest"),
m_stationId(stationId),
m_creatureTemplate(creatureTemplate)
{
	addVariable(m_stationId);
	addVariable(m_creatureTemplate);
}

//-----------------------------------------------------------------------

RandomNameRequest::RandomNameRequest(Archive::ReadIterator & source) :
GameNetworkMessage("RandomNameRequest"),
m_stationId(),
m_creatureTemplate()
{
	addVariable(m_stationId);
	addVariable(m_creatureTemplate);

	unpack(source);
}

//-----------------------------------------------------------------------

RandomNameRequest::~RandomNameRequest()
{
}

//-----------------------------------------------------------------------

RandomNameResponse::RandomNameResponse(uint32 stationId, const std::string & creatureTemplate, const Unicode::String &name, const StringId &errorMessage) :
GameNetworkMessage("RandomNameResponse"),
m_stationId(stationId),
m_creatureTemplate(creatureTemplate),
m_name(name),
m_errorMessage(errorMessage)
{
	addVariable(m_stationId);
	addVariable(m_creatureTemplate);
	addVariable(m_name);
	addVariable(m_errorMessage);
}

//-----------------------------------------------------------------------

RandomNameResponse::RandomNameResponse(Archive::ReadIterator & source) :
GameNetworkMessage("RandomNameResponse"),
m_stationId(),
m_creatureTemplate(),
m_name(),
m_errorMessage()
{
	addVariable(m_stationId);
	addVariable(m_creatureTemplate);
	addVariable(m_name);
	addVariable(m_errorMessage);

	unpack(source);
}

//-----------------------------------------------------------------------

RandomNameResponse::~RandomNameResponse()
{
}

// ======================================================================





