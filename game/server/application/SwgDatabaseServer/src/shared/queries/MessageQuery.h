// ======================================================================
//
// MessageQuery.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_MessageQuery_H
#define INCLUDED_MessageQuery_H

// ======================================================================

#include "serverNetworkMessages/MessageToPayload.h"
#include "sharedDatabaseInterface/Bindable.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"
#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbBindableVarray.h"
#include <vector>

// ======================================================================

namespace DBQuery
{

	class SaveMessageQuery : public DB::Query
	{
	  public:
		SaveMessageQuery();
	
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual void getSQL(std::string &sql);

		bool setupData(DB::Session *session);
		bool addData(const MessageToPayload &data);
		void clearData();
		void freeData();

		int getNumItems() const;

	  private:
		DB::BindableVarrayString	m_message_ids;
		DB::BindableVarrayString	m_object_ids;
		DB::BindableVarrayString	m_methods;
		DB::BindableVarrayString	m_packed_datas;
		DB::BindableVarrayNumber	m_call_times;
		DB::BindableVarrayString	m_guaranteeds;
		DB::BindableVarrayNumber	m_delivery_types;
		DB::BindableLong			m_numItems;
		DB::BindableLong			m_enableDatabaseLogging;
		
	  private:
		SaveMessageQuery(const SaveMessageQuery&); // disable
		SaveMessageQuery& operator=(const SaveMessageQuery&); // disable

		void convertMessageToUTF8(const std::vector<int8> & messageData, std::string & utf8Data) const;
	};

	class AckMessageQuery : public DB::Query
	{
	  public:
		AckMessageQuery();

		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual void getSQL(std::string &sql);

		bool setupData(DB::Session *session);
		bool addData(const MessageToId &MessageId);
		void clearData();
		void freeData();

		int getNumItems() const;

	  private:
		DB::BindableVarrayString	m_message_ids;
		DB::BindableLong			m_numItems;
		DB::BindableLong			m_enableDatabaseLogging;
				
	  private:
		AckMessageQuery(const AckMessageQuery&); // disable
		AckMessageQuery& operator=(const AckMessageQuery&); // disable
	};

	class LoadMessagesQuery : public DB::Query
	{
		LoadMessagesQuery(const LoadMessagesQuery&); // disable
		LoadMessagesQuery& operator=(const LoadMessagesQuery&); // disable

	  public:
		struct MessageRow
		{
			DB::BindableNetworkId message_id;
			DB::BindableNetworkId object_id;
			DB::BindableString<50> method;
			DB::BindableString<4000> packed_data;
			DB::BindableLong call_time;
			DB::BindableBool guaranteed;
			DB::BindableLong delivery_type;
		};

	  public:
		LoadMessagesQuery(const std::string &schema);

		const std::vector<MessageRow> & getData() const;
		
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual DB::Query::QueryMode getExecutionMode() const;
		virtual void getSQL(std::string &sql);

		void convertUTF8ToMessage(const std::string & utf8Data, std::vector<int8> & messageData) const;
				
	  private:
		std::vector<MessageRow> m_data;
		const std::string m_schema;
	};
}

// ======================================================================

inline const std::vector<DBQuery::LoadMessagesQuery::MessageRow> & DBQuery::LoadMessagesQuery::getData() const
{
	return m_data;
}
	
// ======================================================================

#endif
