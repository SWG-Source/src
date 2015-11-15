// ======================================================================
//
// TransferCharacterData.cpp
// copyright (c) 2001-2004 Sony Online Entertainment
//
// ======================================================================

#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "serverNetworkMessages/TransferCharacterData.h"
#include "Unicode.h"
#include "UnicodeUtils.h"

//-----------------------------------------------------------------------

TransferCharacterData::TransferCharacterData(const TransferCharacterData & rhs) :
m_track(rhs.m_track),
m_customerLocalizedLanguage(rhs.m_customerLocalizedLanguage),
m_sourceGalaxy(rhs.m_sourceGalaxy),
m_destinationGalaxy(rhs.m_destinationGalaxy),
m_sourceCharacterName(rhs.m_sourceCharacterName),
m_destinationCharacterName(rhs.m_destinationCharacterName),
m_sourceStationId(rhs.m_sourceStationId),
m_destinationStationId(rhs.m_destinationStationId),
m_transactionId(rhs.m_transactionId),
m_characterId(rhs.m_characterId),
m_destinationCharacterId(rhs.m_destinationCharacterId),
m_containerId(rhs.m_containerId),
m_scene(rhs.m_scene),
m_startingCoordinates(rhs.m_startingCoordinates),
m_customizationData(rhs.m_customizationData),
m_scriptDictionaryData(rhs.m_scriptDictionaryData),
m_objectTemplateName(rhs.m_objectTemplateName),
m_objectTemplateCrc(rhs.m_objectTemplateCrc),
m_scaleFactor(rhs.m_scaleFactor),
m_hairTemplateName(rhs.m_hairTemplateName),
m_hairAppearanceData(rhs.m_hairAppearanceData),
m_profession(rhs.m_profession),
m_biography(rhs.m_biography),
m_isValidName(rhs.m_isValidName),
m_isMoveRequest(rhs.m_isMoveRequest),
m_withItems(rhs.m_withItems),
m_resultDescription(rhs.m_resultDescription),
m_allowOverride(rhs.m_allowOverride),
m_skillTemplate(rhs.m_skillTemplate),
m_workingSkill(rhs.m_workingSkill),
m_transferRequestSource(rhs.m_transferRequestSource),
m_csToolId(rhs.m_csToolId)
{
}

//-----------------------------------------------------------------------

TransferCharacterData::TransferCharacterData(TransferRequestMoveValidation::TransferRequestSource transferRequestSource) :
m_track(0),
m_customerLocalizedLanguage(),
m_sourceGalaxy(),
m_destinationGalaxy(),
m_sourceCharacterName(),
m_destinationCharacterName(),
m_sourceStationId(0),
m_destinationStationId(0),
m_transactionId(0),
m_characterId(NetworkId::cms_invalid),
m_destinationCharacterId(NetworkId::cms_invalid),
m_containerId(NetworkId::cms_invalid),
m_scene(),
m_startingCoordinates(),
m_customizationData(),
m_scriptDictionaryData(),
m_objectTemplateName(),
m_objectTemplateCrc(0),
m_scaleFactor(1.0f),
m_hairTemplateName(),
m_hairAppearanceData(),
m_profession(),
m_biography(),
m_isValidName(false),
m_isMoveRequest(false),
m_withItems(false),
m_resultDescription(),
m_allowOverride(false),
m_skillTemplate(),
m_workingSkill(),
m_transferRequestSource(transferRequestSource),
m_csToolId(0)
{
}

//-----------------------------------------------------------------------

TransferCharacterData::TransferCharacterData() :
m_track(0),
m_customerLocalizedLanguage(),
m_sourceGalaxy(),
m_destinationGalaxy(),
m_sourceCharacterName(),
m_destinationCharacterName(),
m_sourceStationId(0),
m_destinationStationId(0),
m_transactionId(0),
m_characterId(NetworkId::cms_invalid),
m_destinationCharacterId(NetworkId::cms_invalid),
m_containerId(NetworkId::cms_invalid),
m_scene(),
m_startingCoordinates(),
m_customizationData(),
m_scriptDictionaryData(),
m_objectTemplateName(),
m_objectTemplateCrc(0),
m_scaleFactor(1.0f),
m_hairTemplateName(),
m_hairAppearanceData(),
m_profession(),
m_biography(),
m_isValidName(false),
m_isMoveRequest(false),
m_withItems(false),
m_resultDescription(),
m_allowOverride(false),
m_skillTemplate(),
m_workingSkill(),
m_transferRequestSource(TransferRequestMoveValidation::TRS_transfer_server),
m_csToolId(0)
{
}

//-----------------------------------------------------------------------

TransferCharacterData::~TransferCharacterData()
{
}

//-----------------------------------------------------------------------

const NetworkId & TransferCharacterData::getCharacterId() const
{
	return m_characterId;
}

//-----------------------------------------------------------------------

const NetworkId & TransferCharacterData::getContainerId() const
{
	return m_containerId;
}

//-----------------------------------------------------------------------

const std::string & TransferCharacterData::getScene() const
{
	return m_scene;
}

//-----------------------------------------------------------------------

unsigned int TransferCharacterData::getTrack() const
{
	return m_track;
}

//-----------------------------------------------------------------------

const std::string & TransferCharacterData::getCustomerLocalizedLanguage() const
{
	return m_customerLocalizedLanguage;
}

//-----------------------------------------------------------------------

const std::string & TransferCharacterData::getSourceGalaxy() const
{
	return m_sourceGalaxy;
}

//-----------------------------------------------------------------------

const std::string & TransferCharacterData::getDestinationGalaxy() const
{
	return m_destinationGalaxy;
}

//-----------------------------------------------------------------------

const std::string & TransferCharacterData::getSourceCharacterName() const
{
	return m_sourceCharacterName;
}

//-----------------------------------------------------------------------

const std::string & TransferCharacterData::getDestinationCharacterName() const
{
	return m_destinationCharacterName;
}

//-----------------------------------------------------------------------

unsigned int TransferCharacterData::getSourceStationId() const
{
	return m_sourceStationId;
}

//-----------------------------------------------------------------------

unsigned int TransferCharacterData::getDestinationStationId() const
{
	return m_destinationStationId;
}

//-----------------------------------------------------------------------

unsigned int TransferCharacterData::getTransactionId() const
{
	return m_transactionId;
}

//-----------------------------------------------------------------------

const Vector & TransferCharacterData::getStartingCoordinates() const
{
	return m_startingCoordinates;
}
	
//-----------------------------------------------------------------------

bool TransferCharacterData::getAllowOverride() const
{
	return m_allowOverride;
}

//-----------------------------------------------------------------------

const std::string & TransferCharacterData::getSkillTemplate() const
{
	return m_skillTemplate;
}

//-----------------------------------------------------------------------

const std::string & TransferCharacterData::getWorkingSkill() const
{
	return m_workingSkill;
}

//-----------------------------------------------------------------------

TransferRequestMoveValidation::TransferRequestSource TransferCharacterData::getTransferRequestSource() const
{
	return m_transferRequestSource;
}

//-----------------------------------------------------------------------

std::string TransferCharacterData::toString() const
{
	char buf[64] = {"\0"};

	std::string result;

	result += " transactionId=";
	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf) - 1, "%d", getTransactionId());
	result += buf;

	result += " track=";
	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf) - 1, "%d", getTrack());
	result += buf;

	TransferRequestMoveValidation::TransferRequestSource const transferRequestSource = getTransferRequestSource();
	if (transferRequestSource == TransferRequestMoveValidation::TRS_transfer_server)
	{
		result += " requestSource=TransferServer";
	}
	else if (transferRequestSource == TransferRequestMoveValidation::TRS_console_god_command)
	{
		result += " requestSource=GameServer_console_command";
	}
	else if (transferRequestSource == TransferRequestMoveValidation::TRS_ingame_freects_command_validate)
	{
		result += " requestSource=GameServer_freects_command_validate";
	}
	else if (transferRequestSource == TransferRequestMoveValidation::TRS_ingame_freects_command_transfer)
	{
		result += " requestSource=GameServer_freects_command_transfer";
	}
	else if (transferRequestSource == TransferRequestMoveValidation::TRS_ingame_cts_command_validate)
	{
		result += " requestSource=GameServer_cts_command_validate";
	}
	else if (transferRequestSource == TransferRequestMoveValidation::TRS_ingame_cts_command_transfer)
	{
		result += " requestSource=GameServer_cts_command_transfer";
	}
	else
	{
		result += " requestSource=";

		memset(buf, 0, sizeof(buf));
		snprintf(buf, sizeof(buf) - 1, "%d", static_cast<int>(transferRequestSource));
		result += buf;
	}

	result += " isValidName=";
	if(getIsValidName())
	{
		result += "true";
	}
	else
	{
		result += "false";
	}

	result += " isMoveRequest=";
	if(getIsMoveRequest())
	{
		result += "true";
	}
	else
	{
		result += "false";
	}

	result += " withItems=";
	if(getWithItems())
	{
		result += "true";
	}
	else
	{
		result += "false";
	}

	result += " allowOverride=";
	if(getAllowOverride())
	{
		result += "true";
	}
	else
	{
		result += "false";
	}

	result += "    {";

	result += "SOURCE: ";

	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf) - 1, "%d", getSourceStationId());
	result += buf;
	result += ", ";

	result += getSourceGalaxy();
	result += ", ";

	result += getSourceCharacterName();

	result += " (";
	result += getCharacterId().getValueString();
	result += ") ";


	result += "}    ---->    {";

	result += "DEST: ";

	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf) - 1, "%d", getDestinationStationId());
	result += buf;
	result += ", ";

	result += getDestinationGalaxy();
	result += ", ";

	result += getDestinationCharacterName();

	result += " (";
	result += getDestinationCharacterId().getValueString();
	result += ") ";

	result += "}    ";

	result += " customerLocalizedLanguage=";
	result += getCustomerLocalizedLanguage();

	result += " scene=";
	result += getScene();
	memset(buf, 0, sizeof(buf));
	const Vector & v = getStartingCoordinates();
	snprintf(buf, sizeof(buf) - 1, " (%.2f %.2f %.2f)", v.x, v.y, v.z);
    result += buf;

	result += " containerId=";
	result += getContainerId().getValueString();

	result += " objectTemplateName=";
	result += getObjectTemplateName();

	result += " objectTemplateCrc=";
	snprintf(buf, sizeof(buf) - 1, "%lu", getObjectTemplateCrc());
	result += buf;

	result += " scaleFactor=";
	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf) - 1, "%.2f", getScaleFactor());
	result += buf;

	result += " customizationData.length=";
	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf) - 1, "%d", getCustomizationData().length());
	result += buf;

	result += " scriptDictionaryData.length=";
	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf) - 1, "%d", getScriptDictionaryData().size()); 
	result += buf;

	result += " hairObjectTemplateName=";
	result += getHairTemplateName();

	result += " hairAppearanceData.length=";
	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf) -1, "%d", getHairAppearanceData().length());
	result += buf;

	result += " profession=";
	result += getProfession();
	
	result += " skillTemplate=";
	result += getSkillTemplate();

	result += " getWorkingSkill=";
	result += getWorkingSkill();
	
	result += " resultDescription=";
	result += Unicode::wideToNarrow(getResultDescription());
	
	return result;
}

//-----------------------------------------------------------------------

const std::string & TransferCharacterData::getCustomizationData() const
{
	return m_customizationData;
}

//-----------------------------------------------------------------------

const std::vector<unsigned char> & TransferCharacterData::getScriptDictionaryData() const
{
	return m_scriptDictionaryData;
}

//-----------------------------------------------------------------------

const std::string & TransferCharacterData::getObjectTemplateName() const
{
	return m_objectTemplateName;
}

//-----------------------------------------------------------------------

uint32 TransferCharacterData::getObjectTemplateCrc() const
{
	return m_objectTemplateCrc;
}

//-----------------------------------------------------------------------

float TransferCharacterData::getScaleFactor() const
{
	return m_scaleFactor;
}

//-----------------------------------------------------------------------

const std::string & TransferCharacterData::getHairTemplateName() const
{
	return m_hairTemplateName;
}

//-----------------------------------------------------------------------

const std::string & TransferCharacterData::getHairAppearanceData() const
{
	return m_hairAppearanceData;
}

//-----------------------------------------------------------------------

const std::string & TransferCharacterData::getProfession() const
{
	return m_profession;
}

//-----------------------------------------------------------------------

const Unicode::String & TransferCharacterData::getBiography() const
{
	return m_biography;
}

//-----------------------------------------------------------------------

const NetworkId & TransferCharacterData::getDestinationCharacterId() const
{
	return m_destinationCharacterId;
}

//-----------------------------------------------------------------------

bool TransferCharacterData::getIsValidName() const
{
	return m_isValidName;
}

//-----------------------------------------------------------------------

bool TransferCharacterData::getIsMoveRequest() const
{
	return m_isMoveRequest;
}

//-----------------------------------------------------------------------

bool TransferCharacterData::getWithItems() const
{
	 return m_withItems;
}

//-----------------------------------------------------------------------

const Unicode::String & TransferCharacterData::getResultDescription() const
{
	return m_resultDescription;
}

const unsigned int TransferCharacterData::getCSToolId() const
{
	return m_csToolId;
}

//-----------------------------------------------------------------------

void TransferCharacterData::setCharacterId(const NetworkId & characterId)
{
	m_characterId = characterId;
}

//-----------------------------------------------------------------------

void TransferCharacterData::setContainerId(const NetworkId & containerId)
{
	m_containerId = containerId;
}

//-----------------------------------------------------------------------

void TransferCharacterData::setCustomerLocalizedLanguage(const std::string & locale)
{
	m_customerLocalizedLanguage = locale;
}

//-----------------------------------------------------------------------

void TransferCharacterData::setDestinationCharacterId(const NetworkId & destinationCharacterId)
{
	m_destinationCharacterId = destinationCharacterId;
}

//-----------------------------------------------------------------------

void TransferCharacterData::setDestinationCharacterName(const std::string & destinationCharacterName)
{
	m_destinationCharacterName = destinationCharacterName;
}

//-----------------------------------------------------------------------

void TransferCharacterData::setDestinationGalaxy(const std::string & destinationGalaxy)
{
	m_destinationGalaxy = destinationGalaxy;
}

//-----------------------------------------------------------------------

void TransferCharacterData::setDestinationStationId(const unsigned int destinationStationId)
{
	m_destinationStationId = destinationStationId;
}

//-----------------------------------------------------------------------

void TransferCharacterData::setScene(const std::string & scene)
{
	m_scene = scene;
}

//-----------------------------------------------------------------------

void TransferCharacterData::setSourceCharacterName(const std::string & sourceCharacterName)
{
	m_sourceCharacterName = sourceCharacterName;
}

//-----------------------------------------------------------------------

void TransferCharacterData::setSourceGalaxy(const std::string & sourceGalaxy)
{
	m_sourceGalaxy = sourceGalaxy;
}

//-----------------------------------------------------------------------

void TransferCharacterData::setSourceStationId(const unsigned int sourceStationId)
{
	m_sourceStationId = sourceStationId;
}

//-----------------------------------------------------------------------

void TransferCharacterData::setTrack(const unsigned int track)
{
	m_track = track;
}

//-----------------------------------------------------------------------

void TransferCharacterData::setTransactionId(const unsigned int transactionId)
{
	m_transactionId = transactionId;
}

//-----------------------------------------------------------------------

void TransferCharacterData::setStartingCoordinates(const Vector & startingCoordinates)
{
	m_startingCoordinates = startingCoordinates;
}

//-----------------------------------------------------------------------

void TransferCharacterData::setCustomizationData(const std::string & customizationData)
{
	m_customizationData = customizationData;
}

//-----------------------------------------------------------------------

void TransferCharacterData::setScriptDictionaryData(const std::vector<unsigned char> & scriptDictionaryData)
{
	m_scriptDictionaryData = scriptDictionaryData;
}

//-----------------------------------------------------------------------

void TransferCharacterData::setObjectTemplateName(const std::string & objectTemplateName)
{
	m_objectTemplateName = objectTemplateName;
}

//-----------------------------------------------------------------------

void TransferCharacterData::setObjectTemplateCrc(uint32 objectTemplateCrc)
{
	m_objectTemplateCrc = objectTemplateCrc;
}

//-----------------------------------------------------------------------

void TransferCharacterData::setScaleFactor(const float scaleFactor)
{
	m_scaleFactor = scaleFactor;
}

//-----------------------------------------------------------------------

void TransferCharacterData::setHairTemplateName(const std::string & hairTemplateName)
{
	m_hairTemplateName = hairTemplateName;
}

//-----------------------------------------------------------------------

void TransferCharacterData::setHairAppearanceData(const std::string & hairAppearanceData)
{
	m_hairAppearanceData = hairAppearanceData;
}

//-----------------------------------------------------------------------

void TransferCharacterData::setProfession(const std::string & profession)
{
	m_profession = profession;
}

//-----------------------------------------------------------------------

void TransferCharacterData::setBiography(const Unicode::String & biography)
{
	m_biography = biography;
}

//-----------------------------------------------------------------------

void TransferCharacterData::setIsMoveRequest(const bool isMoveRequest)
{
	m_isMoveRequest = isMoveRequest;
}

//-----------------------------------------------------------------------

void TransferCharacterData::setWithItems(const bool withItems)
{
	m_withItems = withItems;
}

//-----------------------------------------------------------------------

void TransferCharacterData::setIsValidName(const bool isNameValid)
{
	m_isValidName = isNameValid;
}

//-----------------------------------------------------------------------

void TransferCharacterData::setResultDescription(const Unicode::String & description)
{
	m_resultDescription = description;
}

//-----------------------------------------------------------------------

void TransferCharacterData::setAllowOverride(const bool allowOverride)
{
	m_allowOverride = allowOverride;
}

//-----------------------------------------------------------------------

void TransferCharacterData::setSkillTemplate(const std::string & skillTemplate)
{
	m_skillTemplate = skillTemplate;
}

//-----------------------------------------------------------------------

void TransferCharacterData::setWorkingSkill(const std::string & workingSkill)
{
	m_workingSkill = workingSkill;
}

//-----------------------------------------------------------------------

void TransferCharacterData::setCSToolId(const unsigned int toolId)
{
	m_csToolId = toolId;
}

//-----------------------------------------------------------------------
