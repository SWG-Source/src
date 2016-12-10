// ======================================================================
//
// ResourceListForSurveyMessage.cpp
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#include "swgSharedNetworkMessages/FirstSwgSharedNetworkMessages.h"
#include "swgSharedNetworkMessages/ResourceListForSurveyMessage.h"

// ======================================================================

const char * const ResourceListForSurveyMessage::MessageType = "ResourceListForSurveyMessage";

//----------------------------------------------------------------------

ResourceListForSurveyMessage::ResourceListForSurveyMessage (const std::vector<DataItem> &data, const std::string& type, const NetworkId& surveyToolId) :
		GameNetworkMessage(MessageType),
		m_data(),
		m_surveyType(),
		m_surveyToolId()
{
	m_data.set(data);
	m_surveyType.set(type);
	m_surveyToolId.set(surveyToolId);
	addVariable(m_data);
	addVariable(m_surveyType);
	addVariable(m_surveyToolId);
}

//-----------------------------------------------------------------------

ResourceListForSurveyMessage::ResourceListForSurveyMessage(Archive::ReadIterator & source) :
		GameNetworkMessage(MessageType),
		m_data()
{
	addVariable(m_data);
	addVariable(m_surveyType);
	addVariable(m_surveyToolId);
	unpack(source);
}

//----------------------------------------------------------------------

ResourceListForSurveyMessage::~ResourceListForSurveyMessage()
{
}

//----------------------------------------------------------------------

const std::vector<ResourceListForSurveyMessage::DataItem> &ResourceListForSurveyMessage::getData() const
{
	return m_data.get();
}

//----------------------------------------------------------------------

const std::string& ResourceListForSurveyMessage::getType() const
{
	return m_surveyType.get();
}

//----------------------------------------------------------------------

const NetworkId &ResourceListForSurveyMessage::getSurveyToolId() const
{
	return m_surveyToolId.get();
}

// =====================================================================

void Archive::put(ByteStream &target, const ResourceList_DataItem &data)
{
	put(target,data.resourceName);
	put(target,data.resourceId);
	put(target,data.parentClassName);
}

void Archive::get(ReadIterator &source, ResourceList_DataItem &data)
{
	get(source,data.resourceName);
	get(source,data.resourceId);
	get(source,data.parentClassName);
}

// ======================================================================
