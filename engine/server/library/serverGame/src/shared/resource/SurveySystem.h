// ======================================================================
//
// SurveySystem.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SurveySystem_H
#define INCLUDED_SurveySystem_H

// ======================================================================

#include "serverGame/GameServer.h"
#include "serverGame/NonCriticalTaskQueue.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedMath/Vector.h"
#include "Singleton/Singleton2.h"

// ======================================================================

/** 
 * Singleton to manage surveys.
 */
class SurveySystem : public Singleton2<SurveySystem>
{
  public:
	void requestResourceListForSurvey  (const NetworkId &playerId, const NetworkId &surveyTool, const std::string &parentResourceClassName) const;
	void requestSurvey                 (const NetworkId &playerId, const std::string &parentResourceClassName, const std::string &resourceTypeName, const Vector &location, int surveyRange, int numPoints) const;
	
  public:
	SurveySystem                       ();
	~SurveySystem                      ();
	
  private:
	class TaskGetResourceList : public NonCriticalTaskQueue::TaskRequest
	{
	  public:
		TaskGetResourceList            (const NetworkId &playerId, const NetworkId &surveyTool, const std::string &parentResourceClassName);
		virtual ~TaskGetResourceList   ();
		virtual bool run               ();

	  private:
		NetworkId                      m_playerId;
		NetworkId                      m_surveyTool; 		
		std::string *                  m_parentResourceClassName;

	  private:
		TaskGetResourceList            (const TaskGetResourceList&);
		TaskGetResourceList & operator=(const TaskGetResourceList&);
	};

	class TaskSurvey : public NonCriticalTaskQueue::TaskRequest
	{
	  public:
		TaskSurvey                     (const NetworkId &playerId, const std::string &parentResourceClassName, const std::string &resourceTypeName, const Vector &location, int surveyRange, int numPoints);
		virtual ~TaskSurvey            ();
		virtual bool run               ();

	  private:
		const NetworkId                m_playerId;
		const std::string *            m_parentResourceClassName;
		const std::string *            m_resourceTypeName;
		Vector                         m_location;
		int                            m_surveyRange;
		int                            m_numPoints;

	  private:
		TaskSurvey                     (const TaskSurvey&);
		TaskSurvey & operator=         (const TaskSurvey&);
	};
};

// ======================================================================

#endif
