// NameErrors.h
// copyright 2001 Verant Interactive
// Author: Justin Randall

#ifndef	_INCLUDED_NameErrors_H
#define	_INCLUDED_NameErrors_H

//-----------------------------------------------------------------------

#include "StringId.h"

namespace NameErrors
{
	extern StringId nameApproved;//		ui:name_approved
	extern StringId nameDeclinedSyntax; //	ui:name_declined_syntax
	extern StringId nameDeclinedEmpty; // ui:name_declined_empty
	extern StringId nameDeclinedRaciallyInappropriate; // ui:name_declined_racially_inappropriate
	extern StringId nameDeclinedFictionallyInappropriate; // ui:name_declined_fictionally_inappropriate
	extern StringId nameDeclinedProfane; // ui:name_declined_profane
	extern StringId nameDeclinedInUse; // ui:name_declined_in_use
	extern StringId nameDeclinedReserved; // ui:name_declined_reserved
	extern StringId nameDeclinedNoTemplate; // ui:name_declined_no_template
	extern StringId nameDeclinedNotCreatureTemplate; // ui:name_declined_not_creature_template
	extern StringId nameDeclinedNoNameGenerator; // ui:name_declined_no_name_generator
	extern StringId nameDeclinedCantCreateAvatar; // ui:name_declined_cant_create_avatar
	extern StringId nameDeclinedInternalError; // ui:name_declined_internal_error
	extern StringId nameDeclinedRetry; // ui:name_declined_retry
	extern StringId nameDeclinedTooFast; // ui:name_declined_too_fast
	extern StringId nameDeclinedNotAuthorizedForSpecies; // ui:name_declined_not_authorized_for_species

	extern std::string localizationFile;
};

#endif	// _INCLUDED_NameErrors_H
