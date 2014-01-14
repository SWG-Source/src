//========================================================================
//
// JediConstants.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef INCLUDED_JediConstants_H
#define INCLUDED_JediConstants_H


enum JediState
{
	JS_none             = 0x00000000,
	JS_forceSensitive   = 0x00000001,
	JS_jedi             = 0x00000002,
	JS_forceRankedLight = 0x00000004,
	JS_forceRankedDark  = 0x00000008,
	JS_validStates      = 0x0000000f	// sum of all the states
};

namespace JediStateNamespace
{
	bool isEligible(JediState playerState, JediState skillState);
}

#endif	// INCLUDED_JediConstants_H

