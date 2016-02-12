// SkillManager.cpp
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved.
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedSkillSystem/FirstSharedSkillSystem.h"
#include "sharedSkillSystem/SkillManager.h"

#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedSkillSystem/SkillObject.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include "sharedDebug/InstallTimer.h"

#include <algorithm>
#include <unordered_map>
#include <string>

SkillManager      *SkillManager::ms_instance = nullptr;
const std::string &SkillManager::cms_skillsDatatableName = "datatables/skill/skills.iff";

//-----------------------------------------------------------------

namespace
{
	bool ensureProperSkillName (const std::string & str)
	{
		static std::string valid;
		static bool init = false;

		if (!init)
		{
			valid.reserve (38);
			int c = 0;

			for (c = 'a'; c <= 'z'; ++c)
				valid.append (1, c);

			for (c = '0'; c <= '9'; ++c)
				valid.append (1, c);

			valid.append (1, '_');
			valid.append (1, '-');
			init = true;
		}

		return str.find_first_not_of (valid) == str.npos;
	}
}

//---------------------------------------------------------------------

SkillManager::SkillManager() :
m_skillMap(new SkillMap),
m_skillTable(DataTableManager::getTable(cms_skillsDatatableName, true)/*new DataTable*/),
m_xpLimitMap(new XpLimitMap),
m_xpLimitTable(DataTableManager::getTable("datatables/skill/xp_limits.iff", true))
{
}

//---------------------------------------------------------------------

SkillManager::~SkillManager()
{
	std::for_each (m_skillMap->begin(), m_skillMap->end(), PointerDeleterPairSecond ());
	delete m_skillMap;
	delete m_xpLimitMap;
	m_skillMap = 0;
	m_skillTable = 0;
	m_xpLimitMap = 0;
	m_xpLimitTable = 0;
}

//----------------------------------------------------------------------

void SkillManager::install()
{
	InstallTimer const installTimer("SkillManager::install");

	DEBUG_FATAL (ms_instance, ("SkillManager already installed"));
	ms_instance = new SkillManager();
	ms_instance->getRoot ();
	ms_instance->initXpLimits();
	ExitChain::add (SkillManager::remove, "SkillManager");
}

//----------------------------------------------------------------------

void SkillManager::remove()
{
	DEBUG_FATAL (!ms_instance, ("SkillManager not installed"));
	delete ms_instance;
	ms_instance = nullptr;
}

//----------------------------------------------------------------------

SkillManager & SkillManager::getInstance ()
{
	//DEBUG_FATAL (!ms_instance, ("SkillManager not installed"));
	if (!ms_instance)
	{
		install();
	}
	return *ms_instance;
}

//-----------------------------------------------------------------

const SkillObject * SkillManager::getRoot ()
{
	static const std::string rootname ("skill_system_root");
	return getSkill (rootname);
}

//---------------------------------------------------------------------

const SkillObject * SkillManager::getSkill(const std::string & skillName)
{
	if (skillName.empty () || !ensureProperSkillName (skillName))
		return 0;

	const SkillMap::const_iterator f = m_skillMap->find(skillName);
	if(f == m_skillMap->end())
		return loadSkill(skillName);
	else
		return (*f).second;
}

//-----------------------------------------------------------------------

const SkillObject * SkillManager::loadSkill(const std::string & skillName)
{
	if (skillName.empty () || !ensureProperSkillName (skillName))
		return 0;

	SkillObject * result = 0;

	const SkillMap::const_iterator f = m_skillMap->find(skillName);
	if(f != m_skillMap->end())
	{
		result = f->second;
	}
	else
	{
		SkillObject * const newSkill = new SkillObject;

		m_skillMap->insert(std::make_pair (skillName, newSkill));
		result = newSkill;

		// Guard against skills table having been closed somewhere via DataTableManager.
		m_skillTable = DataTableManager::getTable(cms_skillsDatatableName, true);

		if (!newSkill->load(*m_skillTable, skillName))
		{
			WARNING (true, ("SkillManager failed to load skill [%s]", skillName.c_str ()));
			delete newSkill;
			result = 0;
			m_skillMap->erase (skillName);
//			(*skillMap) [skillName] = 0;
		}
	}

	return result;
}

//---------------------------------------------------------------------

uint32 SkillManager::getDefaultXpLimit(const std::string & experienceType)
{
	if (m_xpLimitMap != nullptr)
	{
		XpLimitMap::const_iterator result = m_xpLimitMap->find(experienceType);
		if (result != m_xpLimitMap->end())
			return (*result).second;
	}
	return static_cast<uint32>(-1);
}

//---------------------------------------------------------------------

void SkillManager::initXpLimits()
{
	if (m_xpLimitTable == nullptr)
	{
		WARNING(true, ("SkillManager::initXpLimits, m_xpLimitTable not initialized"));
		return;
	}

	if (m_xpLimitMap == nullptr)
	{
		WARNING(true, ("SkillManager::initXpLimits, m_xpLimitMap not initialized"));
		return;
	}

	int count = m_xpLimitTable->getNumRows();
	for (int i = 0; i < count; ++i)
	{
		const std::string & xpType = m_xpLimitTable->getStringValue(0, i);
		if (!xpType.empty())
		{
			if (xpType == "end")
				break;
			m_xpLimitMap->insert(std::make_pair(xpType, m_xpLimitTable->getIntValue(1, i)));
		}
	}
}

//---------------------------------------------------------------------

