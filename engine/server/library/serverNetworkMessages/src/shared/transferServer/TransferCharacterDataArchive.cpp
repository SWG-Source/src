// ======================================================================
//
// TransferCharacterDataArchive.cpp
// copyright (c) 2001-2004 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/TransferCharacterDataArchive.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedMathArchive/VectorArchive.h"
#include "serverNetworkMessages/TransferCharacterData.h"
#include "Archive/Archive.h"

namespace Archive
{
	void get(ReadIterator & source, TransferCharacterData & target)
	{
		get(source, target.m_customerLocalizedLanguage);
		get(source, target.m_destinationCharacterName);
		get(source, target.m_destinationGalaxy);
		get(source, target.m_destinationStationId);
		get(source, target.m_sourceCharacterName);
		get(source, target.m_sourceGalaxy);
		get(source, target.m_sourceStationId);
		get(source, target.m_track);
		get(source, target.m_transactionId);
		get(source, target.m_characterId);
		get(source, target.m_containerId);
		get(source, target.m_scene);
		get(source, target.m_startingCoordinates);
		get(source, target.m_customizationData);
		get(source, target.m_scriptDictionaryData);
		get(source, target.m_objectTemplateName);
		get(source, target.m_objectTemplateCrc);
		get(source, target.m_scaleFactor);
		get(source, target.m_hairTemplateName);
		get(source, target.m_hairAppearanceData);
		get(source, target.m_profession);
		get(source, target.m_biography);
		get(source, target.m_destinationCharacterId);
		get(source, target.m_isValidName);
		get(source, target.m_isMoveRequest);
		get(source, target.m_withItems);
		get(source, target.m_resultDescription);
		get(source, target.m_allowOverride);
		get(source, target.m_csToolId);

		int8 temp;
		get(source, temp);
		target.m_transferRequestSource = static_cast<TransferRequestMoveValidation::TransferRequestSource>(temp);
	}

	void put(ByteStream & target, const TransferCharacterData & source)
	{
		put(target, source.getCustomerLocalizedLanguage());
		put(target, source.getDestinationCharacterName());
		put(target, source.getDestinationGalaxy());
		put(target, source.getDestinationStationId());
		put(target, source.getSourceCharacterName());
		put(target, source.getSourceGalaxy());
		put(target, source.getSourceStationId());
		put(target, source.getTrack());
		put(target, source.getTransactionId());
		put(target, source.getCharacterId());
		put(target, source.getContainerId());
		put(target, source.getScene());
		put(target, source.getStartingCoordinates());
		put(target, source.getCustomizationData());
		put(target, source.getScriptDictionaryData());
		put(target, source.getObjectTemplateName());
		put(target, source.getObjectTemplateCrc());
		put(target, source.getScaleFactor());
		put(target, source.getHairTemplateName());
		put(target, source.getHairAppearanceData());
		put(target, source.getProfession());
		put(target, source.getBiography());
		put(target, source.getDestinationCharacterId());
		put(target, source.getIsValidName());
		put(target, source.getIsMoveRequest());
		put(target, source.getWithItems());
		put(target, source.getResultDescription());
		put(target, source.getAllowOverride());
		put(target, source.getCSToolId());
		put(target, static_cast<int8>(source.getTransferRequestSource()));
	}
}
