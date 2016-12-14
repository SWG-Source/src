// ======================================================================
//
// SharedBuffBuilderManager.h
// copyright (c) 2006 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SharedBuffBuilderManager_H
#define INCLUDED_SharedBuffBuilderManager_H

// ======================================================================

#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/BuffBuilderChangeMessage.h"

#include <string>

//-----------------------------------------------------------------------

class SharedBuffBuilderManagerSession
{
public:
	NetworkId bufferId;
	NetworkId recipientId;
	time_t startingTime;
	int bufferRequiredCredits;
	bool accepted;
	std::map<std::string, std::pair<int,int> > buffComponents; // "buff_type", <count,modifier>

	SharedBuffBuilderManagerSession();
	SharedBuffBuilderManagerSession(BuffBuilderChangeMessage const & message);

	bool addBuffComponent(std::string const& name, int expertiseModifier);
	void removeBuffComponent(std::string const& name);
	void clearBuffComponents();
	int getBuffComponentValue(std::string const& name);
};

class SharedBuffBuilderManager
{
public:

	typedef SharedBuffBuilderManagerSession Session;

	struct BuffBuilderDataTypeRecord
	{

	public:
		BuffBuilderDataTypeRecord();

	public:
		std::string m_name;
		std::string m_category;
		std::string m_affects;
		int m_maxTimesApplied;
		int m_cost;
		int m_affectAmount;
		std::string m_requiredExpertise;
	};

	typedef std::map<std::string, BuffBuilderDataTypeRecord> BuffBuilderDataType;
	static BuffBuilderDataType* ms_buffBuilderData;

public:
	static void install();
	static void startSession(Session const & session);
	static void updateSession(Session const & session);
	static void endSession(NetworkId const & designerId);
	static bool getSession(NetworkId const & designerId, Session & /*OUT*/ session);
	static bool isCurrentlyBuffing(NetworkId const & designerId);
	static bool isCurrentlyBeingBuffed(NetworkId const & recipientId);
	static time_t getTimeRemaining(NetworkId const & designerId);
	static bool isSessionValid(Session const & session);
	static void populateChangeMessage(Session const & session, BuffBuilderChangeMessage & /*OUT*/ msg);
	static int computeAdjustedAffectAmount(const std::string& recordName,int affectAmount,int expertiseModifier);

	//---
	// ms_buffBuilderData accessors
	//---

	// via recordName
	static const std::string& getCategoryNameForRecordName(const std::string& recordName);
	static const std::string& getAffectsNameForRecordName(const std::string& recordName);
	static int getMaxTimesAppliedForRecordName(const std::string& recordName);
	static int getCostForRecordName(const std::string& recordName);
	static int getAffectAmountForRecordName(const std::string& recordName);
	static const std::string& getRequiredExpertiseNameForRecordName(const std::string& recordName);

	static void getRecordNames(std::vector<std::string>& recordNames);
	static void getCategoryNames(std::vector<std::string>& categoryNames);
private:
	//disabled
	SharedBuffBuilderManager (SharedBuffBuilderManager const & rhs);
	SharedBuffBuilderManager & operator= (SharedBuffBuilderManager const & rhs);

private:
	static void remove();
};

// ======================================================================

#endif
