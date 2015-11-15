// ======================================================================
//
// PopulationListMessage.h
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_PopulationListMessage_H
#define INCLUDED_PopulationListMessage_H

// ======================================================================

#include "serverUtility/PopulationList.h"

// ======================================================================

class PopulationListMessage : public GameNetworkMessage
{
public:
	PopulationListMessage  (const PopulationList &list);
	PopulationListMessage  (Archive::ReadIterator & source);
	~PopulationListMessage ();

	const PopulationList& getList(void) const;

private:
	Archive::AutoVariable<PopulationList> m_list;        
        
	PopulationListMessage();
	PopulationListMessage(const PopulationListMessage&);
	PopulationListMessage& operator= (const PopulationListMessage&);
};

// ======================================================================

inline const PopulationList& PopulationListMessage::getList(void) const
{
	return m_list.get();
}

// ======================================================================

#endif
