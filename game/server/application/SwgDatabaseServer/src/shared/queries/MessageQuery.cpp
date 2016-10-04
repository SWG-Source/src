// ======================================================================
//
// MessageQuery.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "SwgDatabaseServer/FirstSwgDatabaseServer.h"
#include "SwgDatabaseServer/MessageQuery.h"

#include "serverDatabase/ConfigServerDatabase.h"
#include "serverDatabase/DatabaseProcess.h"
#include "sharedLog/Log.h"

/*
   base64.cpp and base64.h

   Copyright (C) 2004-2008 René Nyffenegger

   This source code is provided 'as-is', without any express or implied
   warranty. In no event will the author be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this source code must not be misrepresented; you must not
      claim that you wrote the original source code. If you use this source code
      in a product, an acknowledgment in the product documentation would be
      appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
      misrepresented as being the original source code.

   3. This notice may not be removed or altered from any source distribution.

   René Nyffenegger rene.nyffenegger@adp-gmbh.ch
*/

static const std::string base64_chars =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
				"abcdefghijklmnopqrstuvwxyz"
				"0123456789+/";


static inline bool is_base64(unsigned char c) {
	return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
	std::string ret;
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3];
	unsigned char char_array_4[4];

	while (in_len--) {
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for(i = 0; (i <4) ; i++)
				ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}

	if (i)
	{
		for(j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
			ret += base64_chars[char_array_4[j]];

		while((i++ < 3))
			ret += '=';

	}

	return ret;

}

std::string base64_decode(std::string const& encoded_string) {
	int in_len = encoded_string.size();
	int i = 0;
	int j = 0;
	int in_ = 0;
	unsigned char char_array_4[4], char_array_3[3];
	std::string ret;

	while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
		char_array_4[i++] = encoded_string[in_]; in_++;
		if (i ==4) {
			for (i = 0; i <4; i++)
				char_array_4[i] = base64_chars.find(char_array_4[i]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
				ret += char_array_3[i];
			i = 0;
		}
	}

	if (i) {
		for (j = i; j <4; j++)
			char_array_4[j] = 0;

		for (j = 0; j <4; j++)
			char_array_4[j] = base64_chars.find(char_array_4[j]);

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
	}

	return ret;
}

//TODO:  don't save non-guaranteed messages

// ======================================================================

using namespace DBQuery;
using namespace DB;

// ----------------------------------------------------------------------

SaveMessageQuery::SaveMessageQuery() :
		m_numItems(0),
		m_enableDatabaseLogging(ConfigServerDatabase::getEnableDatabaseErrorLogging())
{
}

// ----------------------------------------------------------------------

bool SaveMessageQuery::setupData(DB::Session *session)
{

	if (!m_message_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
	if (!m_object_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
	if (!m_methods.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
	if (!m_packed_datas.create(session, "VAOFLONGSTRING", DatabaseProcess::getInstance().getSchema(),4000)) return false;
	if (!m_call_times.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
	if (!m_guaranteeds.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000)) return false;
	if (!m_delivery_types.create(session, "VAOFNUMBER", DatabaseProcess::getInstance().getSchema())) return false;
	return true;
}

// ----------------------------------------------------------------------

bool SaveMessageQuery::addData(const MessageToPayload &data)
{
	if (data.getMethod().size() > 50) 
	{
		WARNING(true, ("Attempt to save message \"%s\" to the MESSAGES table, which is too long", data.getMethod().c_str()));
	}
	else
	{
		if (!m_message_ids.push_back(data.getMessageId().getValueString())) return false;
		if (!m_object_ids.push_back(data.getNetworkId().getValueString())) return false;
		if (!m_methods.push_back(data.getMethod())) return false;
		std::string utf8data;
		convertMessageToUTF8(data.getPackedDataVector(), utf8data);
		WARNING(utf8data.size() > 1000,("Message had data that was over 1000 bytes when packed to UTF8.  Method:  %s,  Target:  %s,  Data:  %s",data.getMethod().c_str(), data.getNetworkId().getValueString().c_str(), utf8data.c_str()));
		if (!m_packed_datas.push_back(utf8data)) return false;
		if (!m_call_times.push_back(static_cast<int>(data.getCallTime()))) return false;
		if (!m_guaranteeds.push_back(data.getGuaranteed())) return false;
		if (!m_delivery_types.push_back(static_cast<int>(data.getDeliveryType()))) return false;

		m_numItems=m_numItems.getValue() + 1;

		if (ConfigServerDatabase::getEnableVerboseMessageLogging())
		{
			LOG("PersistedMessages",("Saving message: Method:  %s,  Target:  %s,  Data:  %s",data.getMethod().c_str(), data.getNetworkId().getValueString().c_str(), utf8data.c_str()));
		}		
	}
	return true;
}

// ----------------------------------------------------------------------

int SaveMessageQuery::getNumItems() const
{
	return m_numItems.getValue();
}

// ----------------------------------------------------------------------

void SaveMessageQuery::clearData()
{
	m_message_ids.clear();
	m_object_ids.clear();
	m_methods.clear();
	m_packed_datas.clear();
	m_call_times.clear();
	m_guaranteeds.clear();
	m_delivery_types.clear();

	m_numItems=0;

}

// ----------------------------------------------------------------------

void SaveMessageQuery::freeData()
{
	m_message_ids.free();
	m_object_ids.free();
	m_methods.free();
	m_packed_datas.free();
	m_call_times.free();
	m_guaranteeds.free();
	m_delivery_types.free();

}

// ----------------------------------------------------------------------

bool SaveMessageQuery::bindParameters()
{
	if (!bindParameter(m_message_ids)) return false;
	if (!bindParameter(m_object_ids)) return false;
	if (!bindParameter(m_methods)) return false;
	if (!bindParameter(m_packed_datas)) return false;
	if (!bindParameter(m_call_times)) return false;
	if (!bindParameter(m_guaranteeds)) return false;
	if (!bindParameter(m_delivery_types)) return false;
	if (!bindParameter(m_numItems)) return false;
	if (!bindParameter(m_enableDatabaseLogging)) return false;

	return true;
}

// ----------------------------------------------------------------------

bool SaveMessageQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

void SaveMessageQuery::getSQL(std::string &sql)
{
	sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.save_message_batch(:message_id, :object_id, :method, :data, :call_time, :guaranteed, :delivery_type, :chunk_size, :enable_db_logging); end;";
}

// ----------------------------------------------------------------------

void SaveMessageQuery::convertMessageToUTF8(const std::vector<int8> & messageData, std::string & utf8Data) const
{
	utf8Data = base64_encode(reinterpret_cast<const unsigned char*>(messageData.data()), messageData.size());
}

// ----------------------------------------------------------------------

AckMessageQuery::AckMessageQuery() :
		m_numItems(0),
		m_enableDatabaseLogging(ConfigServerDatabase::getEnableDatabaseErrorLogging())
{
}

// ----------------------------------------------------------------------

bool AckMessageQuery::setupData(DB::Session *session)
{
	return m_message_ids.create(session, "VAOFSTRING", DatabaseProcess::getInstance().getSchema(),1000);
}

// ----------------------------------------------------------------------

bool AckMessageQuery::addData(const MessageToId &MessageId)
{
	if (!m_message_ids.push_back(MessageId.getValueString()))
		return false;

	m_numItems=m_numItems.getValue() + 1;
	return true;
}

// ----------------------------------------------------------------------

int AckMessageQuery::getNumItems() const
{
	return m_numItems.getValue();
}

// ----------------------------------------------------------------------

void AckMessageQuery::clearData()
{
	m_message_ids.clear();

	m_numItems=0;

}

// ----------------------------------------------------------------------

void AckMessageQuery::freeData()
{
	m_message_ids.free();

}
// ----------------------------------------------------------------------

bool AckMessageQuery::bindParameters()
{
	if (!bindParameter(m_message_ids)) return false;
	if (!bindParameter(m_numItems)) return false;
	if (!bindParameter(m_enableDatabaseLogging)) return false;
	return true;
}

// ----------------------------------------------------------------------

bool AckMessageQuery::bindColumns()
{
	return true;
}

// ----------------------------------------------------------------------

void AckMessageQuery::getSQL(std::string &sql)
{
	sql=std::string("begin ")+DatabaseProcess::getInstance().getSchemaQualifier()+"persister.acknowledge_message_batch(:message_id, :chunk_size, :enable_db_logging); end;";
}

// ======================================================================

LoadMessagesQuery::LoadMessagesQuery(const std::string &schema) :
		m_data(ConfigServerDatabase::getDefaultFetchBatchSize()),
		m_schema(schema)
{
}

// ----------------------------------------------------------------------

DB::Query::QueryMode LoadMessagesQuery::getExecutionMode() const
{
	if (getProtocol()==DB::PROTOCOL_OCI)
		return (MODE_PLSQL_REFCURSOR);
	else
		return (MODE_SQL);
}

// ----------------------------------------------------------------------

/*
void LoadMessagesQuery::getData(std::vector<MessageToPayload> &buffer)
{
	MessageToPayload temp;
	
	size_t numRows = getNumRowsFetched();
	size_t count = 0;
	
	for (std::vector<MessageRow>::const_iterator i=m_data.begin(); i!=m_data.end(); ++i)
	{
		if (++count > numRows)
			break;
		
		std::string utf8data;
		i->packed_data.getValue(utf8data);
		std::vector<int8> packedDataVector;
		convertUTF8ToMessage(utf8data, packedDataVector);
		
		temp.set(
			i->object_id.getValue(),
			i->message_id.getValue(),
			Unicode::wideToNarrow(i->method.getValue()),
			packedDataVector,
			i->call_time.getValue(),
			i->guaranteed.getValue(),
			static_cast<MessageToPayload::DeliveryType>(i->delivery_type.getValue()));

		buffer.push_back(temp);
	}
}*/

// ----------------------------------------------------------------------

bool LoadMessagesQuery::bindParameters()
{
	return true;
}

// ----------------------------------------------------------------------

bool LoadMessagesQuery::bindColumns()
{
	size_t skipSize = reinterpret_cast<char*>(&(m_data[1])) - reinterpret_cast<char*>(&(m_data[0]));
	setColArrayMode(skipSize, m_data.size());

	if (!bindCol(m_data[0].object_id)) return false;
	if (!bindCol(m_data[0].message_id)) return false;
	if (!bindCol(m_data[0].method)) return false;
	if (!bindCol(m_data[0].packed_data)) return false;
	if (!bindCol(m_data[0].call_time)) return false;
	if (!bindCol(m_data[0].guaranteed)) return false;
	if (!bindCol(m_data[0].delivery_type)) return false;
	return true;
}

// ----------------------------------------------------------------------

void LoadMessagesQuery::getSQL(std::string &sql)
{
	sql=std::string("begin :result := ")+m_schema+"loader.load_messages(); end;";
}

// ----------------------------------------------------------------------

void LoadMessagesQuery::convertUTF8ToMessage(const std::string & utf8Data, std::vector<int8> & messageData) const
{
	auto tmp = base64_decode(utf8Data);
	messageData.insert(std::end(messageData), std::begin(tmp), std::end(tmp));
}

// ======================================================================
