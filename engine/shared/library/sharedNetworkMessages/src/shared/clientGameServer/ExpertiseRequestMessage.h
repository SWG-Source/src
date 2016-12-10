// ======================================================================
//
// ExpertiseRequestMessage.h
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ExpertiseRequestMessage_H
#define INCLUDED_ExpertiseRequestMessage_H

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

class ExpertiseRequestMessage: public GameNetworkMessage
{
public:
	ExpertiseRequestMessage();
	explicit ExpertiseRequestMessage(Archive::ReadIterator &source);
	virtual ~ExpertiseRequestMessage();

	std::vector<std::string> const &getAddExpertisesList() const;
	void setAddExpertisesList(std::vector<std::string> const &addExpertisesList);
	bool getClearAllExpertisesFirst() const;
	void setClearAllExpertisesFirst(bool clearAllExpertisesFirst);

private:
	ExpertiseRequestMessage(ExpertiseRequestMessage const &);
	ExpertiseRequestMessage &operator=(ExpertiseRequestMessage const &);

private:

	Archive::AutoArray<std::string> m_addExpertisesList;
	Archive::AutoVariable<bool> m_clearAllExpertisesFirst;
};

// ----------------------------------------------------------------------

inline std::vector<std::string> const &ExpertiseRequestMessage::getAddExpertisesList() const
{
	return m_addExpertisesList.get();
}

// ----------------------------------------------------------------------

inline bool ExpertiseRequestMessage::getClearAllExpertisesFirst() const
{
	return m_clearAllExpertisesFirst.get();
}

// ======================================================================

#endif	// _INCLUDED_ExpertiseRequestMessage_H

