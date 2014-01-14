//========================================================================
//
// AttribModArchive.cpp
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/AttribModArchive.h"
#include "sharedGame/AttribMod.h"


namespace Archive
{
	void put(Archive::ByteStream &target, const AttribMod::AttribMod &mod)
	{
		put(target, mod.tag);
		put(target, mod.flags);
		if (AttribMod::isSkillMod(mod))
			put(target, mod.skill);
		else
			put(target, mod.attrib);
		put(target, mod.value);
		put(target, mod.attack);
		put(target, mod.sustain);
		put(target, mod.decay);
	}
	void get(Archive::ReadIterator &source, AttribMod::AttribMod &mod)
	{
		get(source, mod.tag);
		get(source, mod.flags);
		if (AttribMod::isSkillMod(mod))
			get(source, mod.skill);
		else
			get(source, mod.attrib);
		get(source, mod.value);
		get(source, mod.attack);
		get(source, mod.sustain);
		get(source, mod.decay);
	}
}



