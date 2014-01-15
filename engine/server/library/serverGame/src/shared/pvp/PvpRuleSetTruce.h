// ======================================================================
//
// PvpRuleSetTruce.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef _PvpRuleSetTruce_H
#define _PvpRuleSetTruce_H

// ======================================================================

#include "serverGame/PvpRuleSetBase.h"

// ======================================================================

class PvpRuleSetTruce: public PvpRuleSetBase
{
public:
	virtual bool canHelp(TangibleObject const &actor, TangibleObject const &target) const;
};

// ======================================================================

#endif // _PvpRuleSetTruce_H

