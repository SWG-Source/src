// NameErrors.cpp
// copyright 2001 Verant Interactive
// Author: Justin Randall


//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "NameErrors.h"

namespace NameErrors
{
	std::string localizationFile("ui");

	StringId nameApproved(localizationFile, "name_approved");
	StringId nameApprovedModified(localizationFile, "name_approved_modified");
	StringId nameDeclinedSyntax(localizationFile, "name_declined_syntax");
	StringId nameDeclinedEmpty(localizationFile, "name_declined_empty");
	StringId nameDeclinedRaciallyInappropriate(localizationFile, "name_declined_racially_inappropriate");
	StringId nameDeclinedFictionallyInappropriate(localizationFile, "name_declined_fictionally_inappropriate");
	StringId nameDeclinedProfane(localizationFile, "name_declined_profane");
	StringId nameDeclinedInUse(localizationFile, "name_declined_in_use");
	StringId nameDeclinedReserved(localizationFile, "name_declined_reserved");
	StringId nameDeclinedNoTemplate(localizationFile, "name_declined_no_template");
	StringId nameDeclinedNotCreatureTemplate(localizationFile, "name_declined_not_creature_template");
	StringId nameDeclinedNoNameGenerator(localizationFile, "name_declined_no_name_generator");
	StringId nameDeclinedCantCreateAvatar(localizationFile, "name_declined_cant_create_avatar");
	StringId nameDeclinedInternalError(localizationFile, "name_declined_internal_error");
	StringId nameDeclinedRetry(localizationFile, "name_declined_retry");
	StringId nameDeclinedTooFast(localizationFile, "name_declined_too_fast");
	StringId nameDeclinedNotAuthorizedForSpecies(localizationFile,  "name_declined_not_authorized_for_species");
};

