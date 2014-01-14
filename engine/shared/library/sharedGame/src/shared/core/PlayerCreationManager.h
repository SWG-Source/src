//======================================================================
//
// PlayerCreationManager.h
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_PlayerCreationManager_H
#define INCLUDED_PlayerCreationManager_H

//======================================================================

class PlayerCreationManager
{
public:
	static void                   install (bool useServerTemplate);
	static void                   remove ();
	static bool                   isInstalled ();
	
	struct EqInfo
	{
		int32       arrangement;
		std::string sharedTemplateName;
		std::string serverTemplateName;

		EqInfo (int32 i, const std::string & a, const std::string & b);
	};

	typedef stdvector<std::string>::fwd            SkillVector;
	typedef stdvector<EqInfo>::fwd                 EqVector;
	typedef stdvector<std::string>::fwd            StringVector;
	typedef stdvector<int>::fwd                    AttribVector;
	typedef stdmap<std::string, AttribVector>::fwd ProfessionAttribMap;

	static void                   clear ();
	static void                   init  ();
	
	static bool                   test (std::string & result, const stdvector<std::string>::fwd & templateNames);

	static void                   getProfessionVector (StringVector & sv, const std::string & categoryName);

	static bool                   getRacialModifiers     (const std::string& sharedTemplateName, stdvector<int>::fwd & /*OUT*/ modifiers);
	static bool                   getProfessionModifiers (const std::string& profession, stdvector<int>::fwd & /*OUT*/modifiers);
	static bool                   getRacialMinMaxes      (const std::string& sharedTemplateName, stdvector<std::pair<int, int> >::fwd & /*OUT*/ minMaxes);
	static bool                   getRacialTotal         (const std::string& sharedTemplateName, int & /*OUT*/ total);

protected:

	static bool                   getDefaults(const std::string & sharedTemplateName, const std::string & profession, AttribVector & attribs, const SkillVector *& skills, const EqVector *& eq);
	static void                   getAttribsForRaceProfession(const std::string & sharedTemplateName, const std::string & profession, AttribVector & attribs);
	virtual                      ~PlayerCreationManager () {}

private:
	static std::string            ms_filename;

	static bool                   initializeProfMap (const std::string & filename);

	static void                   loadRacialModifiers    ();
	static void                   loadProfessionModifiers();
	static void                   buildRacialMinsMaxes   ();

private:
	static stdmap<std::string, stdvector<int>::fwd >::fwd                  m_racialModifiers;
	static stdmap<std::string, stdvector<int>::fwd >::fwd                  m_professionModifiers;
	static stdmap<std::string, stdvector<std::pair<int, int> >::fwd >::fwd m_racialMinMaxes;
	static stdmap<std::string, int>::fwd                                   m_racialTotals;
};

//======================================================================

#endif
