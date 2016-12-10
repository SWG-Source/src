// SkillManager.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_SkillManager_H
#define	_SkillManager_H

#include <unordered_map>

//---------------------------------------------------------------------
#pragma warning (disable : 4786 )

class SkillObject;
class DataTable;

//---------------------------------------------------------------------

class SkillManager
{
public:
	typedef std::unordered_map<std::string, SkillObject *> SkillMap;
	typedef std::unordered_map<std::string, uint32>        XpLimitMap;

	virtual              ~SkillManager ();

	const SkillObject *   getSkill     (const std::string & skillName);
	const SkillMap &      getSkillMap  () const;
	const SkillObject *   getRoot      ();
	uint32                getDefaultXpLimit(const std::string & experienceType);

	static SkillManager & getInstance  ();

	static void           install      ();
	static void           remove       ();

protected:
	                      SkillManager ();
private:
	                      SkillManager (const SkillManager & source);
	SkillManager &        operator =   (const SkillManager &  rhs);
	const SkillObject *   loadSkill    (const std::string & skillName);
	void                  initXpLimits ();

private:
	SkillMap             * m_skillMap;
	DataTable            * m_skillTable;
	XpLimitMap           * m_xpLimitMap;
	DataTable            * m_xpLimitTable;
	static SkillManager  * ms_instance;
	static const std::string & cms_skillsDatatableName;
};

//-----------------------------------------------------------------------

inline const SkillManager::SkillMap & SkillManager::getSkillMap() const
{
	return *(m_skillMap);
}

//---------------------------------------------------------------------

#endif	// _SkillManager_H
