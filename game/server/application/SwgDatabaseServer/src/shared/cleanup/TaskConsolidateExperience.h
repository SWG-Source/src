// ======================================================================
//
// TaskConsolidateExperience.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TaskConsolidateExperience_H
#define INCLUDED_TaskConsolidateExperience_H

// ======================================================================

#include "sharedDatabaseInterface/DbQuery.h"
#include "sharedDatabaseInterface/DbTaskRequest.h"
#include "sharedDatabaseInterface/BindableNetworkId.h"

// ======================================================================

/**
 * Look for players with lots of pending experience grants, combine them
 * into a single grant with the sum of the experience.
 *
 * A time limit can be specified, in which case this task will do as much
 * as it can before the time limit expires.
 */
class TaskConsolidateExperience : public DB::TaskRequest
{
  public:
	TaskConsolidateExperience(float timeLimit);
	~TaskConsolidateExperience();
	
  public:
	virtual bool        process             (DB::Session *session);
	virtual void        onComplete          ();

  private:
	class FindLotsOfMessagesQuery : public DB::Query
	{
	  public:
		FindLotsOfMessagesQuery();
		
		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;

		NetworkId getNetworkId() const;

	  private:
		DB::BindableNetworkId object_id;
		
	  private:
		FindLotsOfMessagesQuery(const FindLotsOfMessagesQuery&);
		FindLotsOfMessagesQuery& operator=(const FindLotsOfMessagesQuery&);
	};

	class GetExperienceQuery : public DB::Query
	{
	  public:
		GetExperienceQuery(const NetworkId &objectId);
		
		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;

		const std::string getData() const;

	  private:
		DB::BindableNetworkId object_id;
		DB::BindableString<1000> grant_data;
		
	  private:
		GetExperienceQuery(const GetExperienceQuery&);
		GetExperienceQuery& operator=(const GetExperienceQuery&);
	};

	class DeleteExperienceQuery : public DB::Query
	{
	  public:
		DeleteExperienceQuery(const NetworkId &targetObject);
		
		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;

	  private:
		DB::BindableNetworkId object_id;
		
	  private:
		DeleteExperienceQuery(const DeleteExperienceQuery&);
		DeleteExperienceQuery& operator=(const DeleteExperienceQuery&);
	};
	
	class GrantExperienceQuery : public DB::Query
	{
	  public:
		GrantExperienceQuery();
		
		virtual void getSQL(std::string &sql);
		virtual bool bindParameters();
		virtual bool bindColumns();
		virtual QueryMode getExecutionMode() const;

		void setData(const NetworkId &objectId, const std::string &data);

	  private:
		DB::BindableNetworkId object_id;
		DB::BindableString<1000> experience_data;
		
	  private:
		GrantExperienceQuery(const GrantExperienceQuery&);
		GrantExperienceQuery& operator=(const GrantExperienceQuery&);
	};

  private:
	bool splitExperience(const std::string &packedData, std::string &type, int &amount) const;
	std::string makeExperienceString(const std::string &type, int amount);
			
  private:
	float m_timeLimit;
};


// ======================================================================

#endif
