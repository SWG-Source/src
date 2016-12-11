// ======================================================================
//
// SurveyMessage.h
// copyright (c) 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SurveyMessage_H
#define INCLUDED_SurveyMessage_H

//-----------------------------------------------------------------------

#include "sharedMath/Vector.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

struct Survey_DataItem
{
	Vector m_location;
	float  m_efficiency;
};

//-----------------------------------------------------------------------

class SurveyMessage : public GameNetworkMessage
{
public:

	static const char * const MessageType;

	typedef Survey_DataItem DataItem;
	
public:
	SurveyMessage           (const std::vector<DataItem> &data);
	explicit SurveyMessage  (Archive::ReadIterator & source);
	virtual  ~SurveyMessage ();
	const std::vector<DataItem>& getData() const;
	
private:
	
	Archive::AutoArray<DataItem> m_data;
	
private:
	SurveyMessage            (const SurveyMessage&);
	SurveyMessage& operator= (const SurveyMessage&);
};

// ======================================================================

namespace Archive
{
	void put(ByteStream &target, const Survey_DataItem &data);
	void get(ReadIterator &source, Survey_DataItem &data);
}

// ======================================================================

#endif
