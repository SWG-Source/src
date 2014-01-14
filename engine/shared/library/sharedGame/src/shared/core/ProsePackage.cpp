//======================================================================
//
// ProsePackage.cpp
// copyright (c) 2002 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/ProsePackage.h"

//======================================================================

ProsePackage::ProsePackage (bool _complexGrammar) :
stringId     (),
actor        (),
target       (),
other        (),
digitInteger (0),
digitFloat   (0.0f),
complexGrammar (_complexGrammar)
{
}

//======================================================================
