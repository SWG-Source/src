//======================================================================
//
// PlayerCreationManager.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h" 
#include "sharedGame/PlayerCreationManager.h"

#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/Tag.h"
#include "sharedFile/Iff.h"
#include "sharedSkillSystem/SkillManager.h"
#include "sharedSkillSystem/SkillObject.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include "swgSharedUtility/Attributes.def"

#include <map>
#include <vector>

//=====================================================================

std::map<std::string, std::vector<int> >                  PlayerCreationManager::m_racialModifiers;
std::map<std::string, std::vector<int> >                  PlayerCreationManager::m_professionModifiers;
std::map<std::string, std::vector<std::pair<int, int> > > PlayerCreationManager::m_racialMinMaxes;
std::map<std::string, int >                               PlayerCreationManager::m_racialTotals;

//=====================================================================

namespace PlayerCreationManagerNamespace
{
	bool s_installed          = false;
	bool s_useServerTemplates = false;

	//----------------------------------------------------------------------

	namespace Tags
	{
		const Tag PFDT = TAG(P,F,D,T);
		const Tag PRFI = TAG(P,R,F,I);
		const Tag SKLS = TAG(S,K,L,S);
		const Tag SKIL = TAG(S,K,I,L);
		const Tag PTMP = TAG(P,T,M,P);
		const Tag ITEM = TAG(I,T,E,M);
	}

	//----------------------------------------------------------------------

	struct ProfessionInfo
	{
		typedef PlayerCreationManager::EqInfo       EqInfo;
		typedef PlayerCreationManager::SkillVector  SkillVector;
		typedef PlayerCreationManager::EqVector     EqVector;

		typedef std::map<std::string, EqVector> EqMap;

		SkillVector   skills;
		EqMap         eq;

		const EqVector * getEqVector (const std::string & sharedTemplateName) const
		{
			const EqMap::const_iterator it = eq.find (sharedTemplateName);
			return it == eq.end () ? 0 : &(*it).second;
		}

		bool load (const std::string & filename);
	};
	
	//----------------------------------------------------------------------
	
	bool ProfessionInfo::load (const std::string & filename)
	{
		Iff iff;
		if (!iff.open (filename.c_str (), true))
		{
			WARNING_STRICT_FATAL (true, ("Specified PlayerCreationManager file '%s' could not be opened.", filename.c_str ()));
			return false;
		}
		
		if (!iff.enterForm (Tags::PRFI, true))
		{
			WARNING_STRICT_FATAL (true, ("PlayerCreationManager '%s' no PRFI form found.", filename.c_str ()));
			return false;
		}
		
		if (!iff.enterForm (TAG_0000, true))
		{
			WARNING_STRICT_FATAL (true, ("PlayerCreationManager '%s' no 0000 form found.", filename.c_str ()));
			return false;
		}
		
		//----------------------------------------------------------------------
		//-- load the skills
			
		if (!iff.enterForm (Tags::SKLS, true))
		{
			WARNING_STRICT_FATAL (true, ("PlayerCreationManager '%s' no 0000 form found.", filename.c_str ()));
			return false;
		}
		
		{
			skills.clear ();
			skills.reserve (iff.getNumberOfBlocksLeft ());
			while (iff.enterChunk (Tags::SKIL, true))
			{			
				skills.push_back (iff.read_stdstring ());
				iff.exitChunk (Tags::SKIL);
			}
			
			iff.exitForm (Tags::SKLS);
		}
		
		//----------------------------------------------------------------------
		//-- load the equipment

		int ptmp_count = 0;
		eq.clear ();


		for (; iff.enterForm (Tags::PTMP, true); ++ptmp_count)
		{
			std::string playerTemplateStr;
			if (iff.enterChunk (TAG_NAME, true))
			{
				playerTemplateStr = iff.read_stdstring ();
				iff.exitChunk ();
			}
			
			if (playerTemplateStr.empty ())
			{
				WARNING_STRICT_FATAL (true, ("PlayerCreationManager '%s' empty player template string in PTMP form %d.", filename.c_str (), ptmp_count));
				return false;
			}
			
			EqVector v;
			v.reserve (iff.getNumberOfBlocksLeft ());
			while (iff.enterChunk (Tags::ITEM, true))
			{
				const int32 arrangement = iff.read_int32 ();
				std::string itemShared  = iff.read_stdstring ();
				std::string itemServer  = iff.read_stdstring ();

				v.push_back (EqInfo (arrangement, itemShared, itemServer));
				iff.exitChunk ();
			}
			
			eq.insert (std::make_pair (playerTemplateStr, v));
			iff.exitForm (true);
		}

		if (ptmp_count == 0)
		{
			WARNING (true, ("PlayerCreationManager file '%s' contained no PTMP forms.", filename.c_str ()));
			return false;
		}

		return true;
	}

	//----------------------------------------------------------------------

	typedef std::pair<std::string, ProfessionInfo> ProfessionInfoPair;
	typedef std::vector<ProfessionInfoPair> ProfessionVector;
	ProfessionVector * s_profVector;

	//----------------------------------------------------------------------

	const ProfessionInfo * getProfessionInfo (const std::string & profession)
	{
		if (!s_profVector)
			return 0;

		for (ProfessionVector::const_iterator it = s_profVector->begin (); it != s_profVector->end (); ++it)
		{
			if (!_stricmp (profession.c_str (), (*it).first.c_str ()))
				return &(*it).second;
		}

		return 0;
	}
	
	//----------------------------------------------------------------------

	bool sanityCheckPath (std::string & path)
	{
		size_t pos = 0;
		bool found = false;
		while ((pos = path.find ('\\', pos)) != path.npos)
		{
			found = true;
			path [pos] = '/';
		}

		return found;
	}

	std::string sharedTemplatePre  = "object/creature/player/shared_";
	std::string sharedTemplatePost = ".iff";

	std::string s_racialModsIffName     = "datatables/creation/racial_mods.iff";
	std::string s_professionModsIffName = "datatables/creation/profession_mods.iff";

	int s_magicBaseAttribNumber = 300;
	unsigned int s_numberOfAttributes = Attributes::NumberOfAttributes;
}

using namespace PlayerCreationManagerNamespace;

//----------------------------------------------------------------------

PlayerCreationManager::EqInfo::EqInfo (int32 i, const std::string & a, const std::string & b) :
arrangement (i),
sharedTemplateName (a),
serverTemplateName (s_useServerTemplates ? b : "")
{
	if (sanityCheckPath (sharedTemplateName))
		WARNING (true, ("Bad shared template path: %s", a.c_str ()));
	
	if (s_useServerTemplates && b.empty ())
	{
		serverTemplateName = sharedTemplateName;
		
		//-- convert shared template names to server template names
		static const std::string shared_str ("/shared_");
		static const std::string shared_str_replacement ("/");
		
		const size_t sharedPos = serverTemplateName.find (shared_str);
		
		if (sharedPos != serverTemplateName.npos)
			serverTemplateName.replace (sharedPos, shared_str.size(), shared_str_replacement);
	}
	else
	{
		if (sanityCheckPath (serverTemplateName))
			WARNING (true, ("Bad server template path: %s", b.c_str ()));		
	}
}

//----------------------------------------------------------------------

std::string  PlayerCreationManager::ms_filename ("creation/profession_defaults.iff");

//----------------------------------------------------------------------

bool PlayerCreationManager::isInstalled ()
{
	return s_installed;
}

//----------------------------------------------------------------------

void PlayerCreationManager::install (bool useServerTemplates)
{
	InstallTimer const installTimer("PlayerCreationManager::install ");

	DEBUG_FATAL (s_installed, ("already installed.\n"));
	s_installed = true;
	s_useServerTemplates = useServerTemplates;

	init ();
}

//----------------------------------------------------------------------

void PlayerCreationManager::remove ()
{
	clear();
	DEBUG_FATAL (!s_installed, ("not installed.\n"));
	s_installed = false;
}

//----------------------------------------------------------------------

void PlayerCreationManager::clear ()
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));

	m_racialModifiers.clear();
	m_racialMinMaxes.clear();
	m_racialTotals.clear();
	m_professionModifiers.clear();

	delete s_profVector;
	s_profVector = 0;
}

//----------------------------------------------------------------------

void PlayerCreationManager::init  ()
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));

	initializeProfMap (ms_filename);

	loadRacialModifiers();
	loadProfessionModifiers();
	//we can only build the mins and maxes once we have all the data from the datatables for the above 2 functions
	buildRacialMinsMaxes();
}

//----------------------------------------------------------------------

bool PlayerCreationManager::initializeProfMap (const std::string & filename)
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));

	Iff iff;
	if (!iff.open (filename.c_str (), true))
	{
		WARNING_STRICT_FATAL (true, ("Specified PlayerCreationManager file '%s' could not be opened.", filename.c_str ()));
		return false;
	}

	if (!iff.enterForm (Tags::PFDT, true))
	{
		WARNING_STRICT_FATAL (true, ("PlayerCreationManager '%s' no PFDT form found.", filename.c_str ()));
		return false;
	}

	if (!iff.enterForm (TAG_0000, true))
	{
		WARNING_STRICT_FATAL (true, ("PlayerCreationManager '%s' no 0000 form found.", filename.c_str ()));
		return false;
	}

	if (s_profVector)
		s_profVector->clear ();
	else
		s_profVector = new ProfessionVector;

	int dataCount = 0;
	while (iff.enterChunk (TAG_DATA, true))
	{
		const std::string profession   = iff.read_stdstring ();
		const std::string profFilename = iff.read_stdstring ();

		ProfessionInfo pinfo;
		
		if (pinfo.load(profFilename))
			s_profVector->push_back (std::make_pair(profession, pinfo));
		else
			WARNING (true, ("Unable to load PlayerCreationManager file for profession %s: %s", profession.c_str (), profFilename.c_str ()));

		iff.exitChunk ();
		++dataCount;
	}

	if (!dataCount)
		WARNING (true, ("No DATA items found in %s", filename.c_str ()));

	return true;
}

//----------------------------------------------------------------------

/**
* The output values of attribs, skills, and eq are only guaranteed valid until
* a subsequent method call on PlayerCreationManager occurs.  Think std::string::c_str()
*
*/

bool PlayerCreationManager::getDefaults(const std::string & sharedTemplateName, const std::string & profession, AttribVector & attribs, const SkillVector *&  skills, const EqVector *& eq)
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));

	const ProfessionInfo * const pinfo = getProfessionInfo (profession);

	if (!pinfo)
	{
		WARNING (true, ("Unable to load PlayerCreationManager for profession %s", profession.c_str ()));
		return false;
	}

	getAttribsForRaceProfession(sharedTemplateName, profession, attribs);

	skills  = &pinfo->skills;

	std::string actualTemplateName = sharedTemplateName;

	if (sanityCheckPath (actualTemplateName))
		WARNING (true, ("Bad shared template path: %s", sharedTemplateName.c_str ()));

	eq      = pinfo->getEqVector(actualTemplateName);

	return true;
}

//----------------------------------------------------------------------

void PlayerCreationManager::getAttribsForRaceProfession(const std::string & sharedTemplateName, const std::string & profession, AttribVector & attribs)
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));
	attribs.clear();

	std::map<std::string, std::vector<int> >::iterator raceIt = m_racialModifiers.find(sharedTemplateName);
	if(raceIt == m_racialModifiers.end())
		return;

	std::map<std::string, std::vector<int> >::iterator profIt = m_professionModifiers.find(profession);
	if(profIt == m_professionModifiers.end())
		return;

	DEBUG_FATAL(raceIt->second.size() != s_numberOfAttributes, ("Bad number of attribs"));
	DEBUG_FATAL(profIt->second.size() != s_numberOfAttributes, ("Bad number of attribs"));
	int val = 0;
	for(unsigned int i = 0; i < s_numberOfAttributes; ++i)
	{
		val = raceIt->second[i] + profIt->second[i];
		attribs.push_back(val);
	}
}

//----------------------------------------------------------------------

void PlayerCreationManager::loadRacialModifiers()
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));
	std::vector<int> modifiers;
	modifiers.clear();

	DataTable dataTable;
	Iff racialModsIff(s_racialModsIffName.c_str());
	dataTable.load(racialModsIff);

	for (int row = 0; row < dataTable.getNumRows(); ++row)
	{
		std::string const &maleTemplate   = dataTable.getStringValue("male_template",   row);
		std::string const &femaleTemplate = dataTable.getStringValue("female_template", row);
		int health                        = dataTable.getIntValue("health",       row);
		int constitution                  = dataTable.getIntValue("constitution", row);
		int action                        = dataTable.getIntValue("action",       row);
		int stamina                       = dataTable.getIntValue("stamina",      row);
		int mind                          = dataTable.getIntValue("mind",         row);
		int willpower                     = dataTable.getIntValue("willpower",    row);

		modifiers.push_back(health);
		modifiers.push_back(constitution);
		modifiers.push_back(action);
		modifiers.push_back(stamina);
		modifiers.push_back(mind);
		modifiers.push_back(willpower);

		std::string fullMaleTemplate   = sharedTemplatePre + maleTemplate   + sharedTemplatePost;
		std::string fullFemaleTemplate = sharedTemplatePre + femaleTemplate + sharedTemplatePost;

		DEBUG_FATAL(modifiers.size() != s_numberOfAttributes, ("Bad number of racial modifiers"));
		m_racialModifiers[fullMaleTemplate]   = modifiers;
		m_racialModifiers[fullFemaleTemplate] = modifiers;
		modifiers.clear();
	}
}

//----------------------------------------------------------------------

void PlayerCreationManager::loadProfessionModifiers()
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));
	std::vector<int> modifiers;
	modifiers.clear();

	DataTable dataTable;
	Iff racialModsIff(s_professionModsIffName.c_str());
	dataTable.load(racialModsIff);

	int max_health                        = 0;
	int max_constitution                  = 0;
	int max_action                        = 0;
	int max_stamina                       = 0;
	int max_mind                          = 0;
	int max_willpower                     = 0;

	for (int row = 0; row < dataTable.getNumRows(); ++row)
	{
		std::string const &profession     = dataTable.getStringValue("profession",   row);
		int health                        = dataTable.getIntValue   ("health",       row);
		int constitution                  = dataTable.getIntValue   ("constitution", row);
		int action                        = dataTable.getIntValue   ("action",       row);
		int stamina                       = dataTable.getIntValue   ("stamina",      row);
		int mind                          = dataTable.getIntValue   ("mind",         row);
		int willpower                     = dataTable.getIntValue   ("willpower",    row);

		//the maximum allowed value for a stat is the maximum of the profession-maxes.

		//don't figure jedi stats into the profession maxes
		//TODO if the acceptable stat window for jedi is different than other professions (and it may be), 
		  //then we will need additional code to address that new window size
		if(profession != "jedi")
		{
			//store off the maxes of these values, used as a factor of racial maximums
			if(health > max_health)
				max_health = health;
			if(constitution > max_constitution)
				max_constitution = constitution;
			if(action > max_action)
				max_action = action;
			if(stamina > max_stamina)
				max_stamina = stamina;
			if(mind > max_mind)
				max_mind = mind;
			if(willpower > max_willpower)
				max_willpower = willpower;
		}

		modifiers.push_back(health);
		modifiers.push_back(constitution);
		modifiers.push_back(action);
		modifiers.push_back(stamina);
		modifiers.push_back(mind);
		modifiers.push_back(willpower);

		DEBUG_FATAL(modifiers.size() != s_numberOfAttributes, ("Bad number of	profession modifiers"));
		m_professionModifiers[profession]   = modifiers;
		modifiers.clear();
	}
}

//----------------------------------------------------------------------

/** 
 * 
 */
void PlayerCreationManager::buildRacialMinsMaxes()
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));
	std::vector<std::pair<int, int> > raceMinMaxes;

	const DataTable * dt = DataTableManager::getTable(
		"datatables/creation/attribute_limits.iff", true);
	WARNING_STRICT_FATAL(dt == nullptr, ("Unable to read the "
		"attribute_limits datatable"));
	if (dt == nullptr)
		return;

	int numAttribs = dt->getNumColumns() - 1;
	for (int row = 0; row < dt->getNumRows(); ++row)
	{
		const std::string & maleTemplate   = dt->getStringValue(0, row);
		const std::string & femaleTemplate = dt->getStringValue(1, row);
		raceMinMaxes.clear();
		int i;
		for (i = 2; i < numAttribs; i += 2)
		{
			raceMinMaxes.push_back(std::make_pair(
				dt->getIntValue(i, row),
				dt->getIntValue(i + 1, row)
				));
		}
		m_racialMinMaxes[sharedTemplatePre + maleTemplate + sharedTemplatePost] = raceMinMaxes;
		m_racialMinMaxes[sharedTemplatePre + femaleTemplate + sharedTemplatePost] = raceMinMaxes;
		m_racialTotals[sharedTemplatePre + maleTemplate + sharedTemplatePost] = dt->getIntValue(i, row);
		m_racialTotals[sharedTemplatePre + femaleTemplate + sharedTemplatePost] = dt->getIntValue(i, row);
	}
}

//----------------------------------------------------------------------

bool PlayerCreationManager::getRacialModifiers(const std::string& sharedTemplateName, std::vector<int> & /*OUT*/ modifiers)
{
	DEBUG_FATAL(!s_installed, ("PlayerCreationManager not installed"));
	if(m_racialModifiers[sharedTemplateName].size() != s_numberOfAttributes)
		return false;
	modifiers = m_racialModifiers[sharedTemplateName];
	return true;
}

//----------------------------------------------------------------------

bool PlayerCreationManager::getProfessionModifiers(const std::string& profession, std::vector<int> & /*OUT*/ modifiers)
{
	DEBUG_FATAL(!s_installed, ("PlayerCreationManager not installed"));
	if(m_professionModifiers[profession].size() != s_numberOfAttributes)
		return false;
	modifiers = m_professionModifiers[profession];
	return true;
}

//----------------------------------------------------------------------

bool PlayerCreationManager::getRacialMinMaxes(const std::string& sharedTemplateName, std::vector<std::pair<int, int> >& /*OUT*/ minMaxes)
{
	DEBUG_FATAL(!s_installed, ("PlayerCreationManager not installed"));
	if(m_racialMinMaxes[sharedTemplateName].size() != s_numberOfAttributes)
		return false;
	minMaxes = m_racialMinMaxes[sharedTemplateName];
	return true;
}

//----------------------------------------------------------------------

bool PlayerCreationManager::getRacialTotal(const std::string & sharedTemplateName, int & /*OUT*/ total)
{
	DEBUG_FATAL(!s_installed, ("PlayerCreationManager not installed"));
	if(m_racialMinMaxes[sharedTemplateName].size() != s_numberOfAttributes)
		return false;
	total = m_racialTotals[sharedTemplateName];
	return true;
}

//----------------------------------------------------------------------

bool PlayerCreationManager::test (std::string & result, const std::vector<std::string> & templateNames)
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));
	UNREF (result);

	clear ();

	init ();

	UNREF (templateNames);

	return false;
}

//----------------------------------------------------------------------

void PlayerCreationManager::getProfessionVector (StringVector & sv, const std::string & categoryName)
{
	DEBUG_FATAL (!s_installed, ("not installed.\n"));
	
	sv.clear ();
	
	if (s_profVector)
	{
		sv.reserve (s_profVector->size ());
		
		for (ProfessionVector::const_iterator it = s_profVector->begin (); it != s_profVector->end (); ++it)
		{
			const std::string & skillName = (*it).first;
			const SkillObject * const skill = SkillManager::getInstance ().getSkill (skillName);
			
			if (skill)
			{
				const SkillObject * const category = skill->findCategory ();

				if (category && (categoryName.empty () || category->getSkillName () == categoryName))
				{
					sv.push_back (skillName);
				}
			}
			else
				WARNING (true, ("Invalid profession name in PlayerCreationManager: %s", skillName.c_str ()));
		}
	}
}

//======================================================================

