// ======================================================================
//
// SurveyMessage.cpp
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#include "swgSharedNetworkMessages/FirstSwgSharedNetworkMessages.h"
#include "swgSharedNetworkMessages/SurveyMessage.h"

#include "sharedMathArchive/VectorArchive.h"

// ======================================================================

const char * const SurveyMessage::MessageType = "SurveyMessage";

//----------------------------------------------------------------------

SurveyMessage::SurveyMessage (const std::vector<DataItem> &data) :
		GameNetworkMessage(MessageType),
		m_data()
{
	m_data.set(data);
	addVariable(m_data);
}

//-----------------------------------------------------------------------

SurveyMessage::SurveyMessage(Archive::ReadIterator & source) :
		GameNetworkMessage(MessageType),
		m_data()
{
	addVariable(m_data);
	unpack(source);
}

//----------------------------------------------------------------------

SurveyMessage::~SurveyMessage()
{
}

//----------------------------------------------------------------------

const std::vector<SurveyMessage::DataItem>& SurveyMessage::getData() const
{
	return m_data.get();
}

// ======================================================================

void Archive::put(ByteStream &target, const Survey_DataItem &data)
{
	put(target,data.m_location);
	put(target,data.m_efficiency);
}

void Archive::get(ReadIterator &source, Survey_DataItem &data)
{
	get(source,data.m_location);
	get(source,data.m_efficiency);
}

// ======================================================================
