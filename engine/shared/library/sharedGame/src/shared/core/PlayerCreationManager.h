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

	typedef std::vector<std::string>            SkillVector;
	typedef std::vector<EqInfo>                 EqVector;
	typedef std::vector<std::string>            StringVector;
	typedef std::vector<int>                    AttribVector;
	typedef std::map<std::string, AttribVector> ProfessionAttribMap;

	static void                   clear ();
	static void                   init  ();
	
	static bool                   test (std::string & result, const std::vector<std::string> & templateNames);

	static void                   getProfessionVector (StringVector & sv, const std::string & categoryName);

	static bool                   getRacialModifiers     (const std::string& sharedTemplateName, std::vector<int> & /*OUT*/ modifiers);
	static bool                   getProfessionModifiers (const std::string& profession, std::vector<int> & /*OUT*/modifiers);
	static bool                   getRacialMinMaxes      (const std::string& sharedTemplateName, std::vector<std::pair<int, int> > & /*OUT*/ minMaxes);
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
	static std::map<std::string, std::vector<int> >                  m_racialModifiers;
	static std::map<std::string, std::vector<int> >                  m_professionModifiers;
	static std::map<std::string, std::vector<std::pair<int, int> > > m_racialMinMaxes;
	static std::map<std::string, int>                                   m_racialTotals;
};

//======================================================================

#endif
