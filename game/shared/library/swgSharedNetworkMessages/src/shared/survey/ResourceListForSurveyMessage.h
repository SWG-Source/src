// ======================================================================
//
// ResourceListForSurveyMessage.h
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ResourceListForSurveyMessage_H
#define INCLUDED_ResourceListForSurveyMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "Unicode.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

struct ResourceList_DataItem
{
	std::string     resourceName;
	NetworkId       resourceId;
	std::string     parentClassName;
};

//-----------------------------------------------------------------------

class ResourceListForSurveyMessage : public GameNetworkMessage
{
public:

	static const char * const MessageType;

	typedef ResourceList_DataItem DataItem;

public:
	ResourceListForSurveyMessage                        (const std::vector<DataItem> &data, const std::string& type, const NetworkId& surveyToolId);
	explicit ResourceListForSurveyMessage               (Archive::ReadIterator & source);
	virtual  ~ResourceListForSurveyMessage              ();
	const std::vector<DataItem> &getData                () const;
	const std::string& getType                           () const;
	const NetworkId &getSurveyToolId                    () const;
	
private:
	Archive::AutoArray<DataItem>        m_data;
	Archive::AutoVariable<std::string>  m_surveyType;
	Archive::AutoVariable<NetworkId>    m_surveyToolId;
	
private:
	ResourceListForSurveyMessage            (const ResourceListForSurveyMessage&);
	ResourceListForSurveyMessage& operator= (const ResourceListForSurveyMessage&);
};

// ======================================================================

namespace Archive
{
	void put(ByteStream &target, const ResourceList_DataItem &data);
	void get(ReadIterator &source, ResourceList_DataItem &data);
}

// ======================================================================

#endif
