// SkillObject.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_SkillObject_H
#define	_SkillObject_H

#include <string>
#include <vector>

class DataTable;

//---------------------------------------------------------------------

class SkillObject
{
public:
	SkillObject();
	virtual ~SkillObject();
	bool load(DataTable & file, const std::string & skillName);

	typedef std::vector<const SkillObject *> SkillVector;
	typedef std::pair<int, int>                 XpPair;             // xp needed, xp limit
	typedef std::pair<std::string, XpPair>      ExperiencePair;
	typedef std::vector<ExperiencePair>      ExperienceVector;
	typedef std::pair<std::string, int>         GenericMod;	
	typedef std::vector<GenericMod>          GenericModVector;
	typedef std::vector<std::string>         StringVector;
	typedef std::pair<std::string, bool>        SpeciesFlag;
	typedef std::vector<SpeciesFlag>         SpeciesFlagVector;

	const SkillObject *                         findCategory                    () const;

	const StringVector &                        getCommandsProvided             () const;
	const StringVector &                        getSchematicsGranted            () const;
	const SkillVector &                         getNextSkillBoxes               () const;
	const ExperienceVector &                    getPrerequisiteExperienceVector () const;
	const ExperiencePair *                      getPrerequisiteExperience       () const;
	const GenericModVector &                    getPrerequisiteFactionStanding  () const;
	const SkillVector &                         getPrerequisiteSkills           () const;
	const SpeciesFlagVector &                   getPrerequisiteSpecies          () const;
	const SkillObject *                         getPrevSkill                    () const;
	const std::string &                         getSkillName                    () const;
	const GenericModVector &                    getStatisticModifiers           () const;
	const bool                                  hasCommand                      (const std::string & commandName) const;
	const bool                                  isProfession                    () const;
	const bool                                  isTitle                         () const;
	const bool                                  isSearchable                    () const;

	const SkillObject *                         findProfessionForSkill          () const;

	bool                                        dependsUponSkill                (const SkillObject & skill, bool immediatePrereqsOnly = false) const;


	struct SkillData
	{
		SkillData();
		SkillData(const SkillData & source);
		~SkillData();
		SkillData & operator = (const SkillData & rhs);

		// prerequisites
		SkillVector                                 prerequisiteSkills;
		ExperienceVector                            prerequisiteExperience;
		SpeciesFlagVector                           prerequisiteSpecies;
		GenericModVector                            prerequisiteFactionStanding;

		// descriptors
		std::string                                 skillName;
		SkillVector                                 nextSkillBoxes;
		const SkillObject *                         prevSkill;

		// effects
		StringVector                                commandsProvided;
		StringVector                                schematicsGranted;
		std::vector<std::pair<std::string, int> >   statisticModifiers;
		bool                                        isProfession;
		bool                                        isTitle;
		bool                                        isSearchable;
	};

	const SkillData & getSkillData() const;
	SkillData & getSkillData();

private:
	SkillObject(const SkillObject & source);
	SkillObject & operator = (const SkillObject &  rhs);
	SkillData skillData;

	static const std::string ms_skillLabel;
	static const std::string ms_prerequisiteSkillsLabel;
	static const std::string ms_prerequisiteExperienceTypeLabel;
	static const std::string ms_prerequisiteExperienceAmountLabel;
	static const std::string ms_prerequisiteExperienceLimitLabel;
	static const std::string ms_prerequisiteSpeciesLabel;
	static const std::string ms_commandsLabel;
	static const std::string ms_statisticsModifiersLabel;
	static const std::string ms_parentLabel;
	static const std::string ms_schematicsGrantedLabel;
	static const std::string ms_isTitleLabel;
	static const std::string ms_isProfessionLabel;
	static const std::string ms_isSearchableLabel;

	void loadPrerequisiteSkills       (DataTable & file, int skillRow);
	void loadPrerequisiteExperience   (DataTable & file, int skillRow);
	void loadPrerequisiteSpecies      (DataTable & file, int skillRow);
	void loadCommands                 (DataTable & file, int skillRow);
	void loadSchematicsGranted        (DataTable & file, int skillRow);
	void loadStatisticsModifiers      (DataTable & file, int skillRow);
	void connectLinks                 (DataTable & file, const std::string & parentName);
};

//---------------------------------------------------------------------

#endif	// _SkillObject_H
