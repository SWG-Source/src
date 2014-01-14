//======================================================================
//
// SkillObjectArchive.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedSkillSystem/FirstSharedSkillSystem.h"
#include "sharedSkillSystem/SkillObjectArchive.h"

#include "sharedSkillSystem/SkillObject.h"
#include "sharedSkillSystem/SkillManager.h"
#include "Archive/Archive.h"

//======================================================================

namespace Archive
{
	//----------------------------------------------------------------------

	void get(ReadIterator & source, const SkillObject *& target)
	{
		std::string name;
		Archive::get (source, name);

		if (name.empty ())
			target = 0;
		else
		{
			target = SkillManager::getInstance ().getSkill (name);
		}
	}

	//-----------------------------------------------------------------------

	void put(ByteStream & target, const SkillObject * const & source)
	{
		if (source)
			Archive::put (target, source->getSkillName ());
		else
			Archive::put (target, std::string ());
	}
}

//======================================================================
