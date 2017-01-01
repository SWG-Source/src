// ======================================================================
//
// TransferCharacterData.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_TransferCharacterData_H
#define INCLUDED_TransferCharacterData_H

// ======================================================================

#include "serverNetworkMessages/TransferRequestMoveValidation.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedMath/Vector.h"
#include <string>

// ======================================================================

class TransferCharacterData;

namespace Archive
{
	class ReadIterator;
	void get(ReadIterator & source, TransferCharacterData & target);
}

/**
 * Data structures for passing around the list of avatars for an account.
 */
class TransferCharacterData
{
public:
	TransferCharacterData(const TransferCharacterData & rhs);
	TransferCharacterData(TransferRequestMoveValidation::TransferRequestSource transferRequestSource);

	~TransferCharacterData();

	const NetworkId &                   getCharacterId                () const;
	const NetworkId &                   getContainerId                () const;
	const std::string &                 getCustomerLocalizedLanguage  () const;
	const NetworkId &                   getDestinationCharacterId     () const;
	const std::string &                 getDestinationCharacterName   () const;
	const std::string &                 getDestinationGalaxy          () const;
	unsigned int                        getDestinationStationId       () const;
	const std::string &                 getScene                      () const;
	const std::string &                 getSourceCharacterName        () const;
	const std::string &                 getSourceGalaxy               () const;
	unsigned int                        getSourceStationId            () const;
	unsigned int                        getTrack                      () const;
	unsigned int                        getTransactionId              () const;
	const Vector &                      getStartingCoordinates        () const;
	std::string                         toString                      () const;
	const std::string &                 getCustomizationData          () const;
	const std::vector<unsigned char> &  getScriptDictionaryData       () const;
	const std::string &                 getObjectTemplateName         () const;
	uint32                              getObjectTemplateCrc          () const;
	float                               getScaleFactor                () const;
	const std::string &                 getHairTemplateName           () const;
	const std::string &                 getHairAppearanceData         () const;
	const std::string &                 getProfession                 () const;
	const Unicode::String &             getBiography                  () const;
	bool                                getIsValidName                () const;
	bool                                getIsMoveRequest              () const;
	bool                                getWithItems                  () const;
	const Unicode::String &             getResultDescription          () const;
	bool                                getAllowOverride              () const;
	const std::string &                 getSkillTemplate              () const;
	const std::string &                 getWorkingSkill               () const;
	TransferRequestMoveValidation::TransferRequestSource getTransferRequestSource() const;

	const unsigned int                  getCSToolId                   () const;

	void                                setCharacterId                (const NetworkId & characterId);
	void                                setContainerId                (const NetworkId & containerId);
	void                                setCustomerLocalizedLanguage  (const std::string & locale);
	void                                setDestinationCharacterId     (const NetworkId & destinationCharacterId);
	void                                setDestinationCharacterName   (const std::string & destinationCharacterName);
	void                                setDestinationGalaxy          (const std::string & destinationGalaxy);
	void                                setDestinationStationId       (const unsigned int destinationStationId);
	void                                setScene                      (const std::string & scene);
	void                                setSourceCharacterName        (const std::string & sourceCharacterName);
	void                                setSourceGalaxy               (const std::string & sourceGalaxy);
	void                                setSourceStationId            (const unsigned int sourceStationId);
	void                                setTrack                      (const unsigned int track);
	void                                setTransactionId              (const unsigned int transactionId);
	void                                setStartingCoordinates        (const Vector & startingCoordinates);
	void                                setCustomizationData          (const std::string & customizationData);
	void                                setScriptDictionaryData       (const std::vector<unsigned char> & scriptDictionaryData);
	void                                setObjectTemplateName         (const std::string & objectTemplateName);
	void                                setObjectTemplateCrc          (uint32 objectTemplateCrc);
	void                                setScaleFactor                (const float scaleFactor);
	void                                setHairTemplateName           (const std::string & hairTemplateName);
	void                                setHairAppearanceData         (const std::string & hairAppearanceData);
	void                                setProfession                 (const std::string & profession);
	void                                setBiography                  (const Unicode::String & biography);
	void                                setIsValidName                (const bool isNameValid);
	void                                setIsMoveRequest              (const bool isMoveRequest);
	void                                setWithItems                  (const bool withItems);
	void                                setResultDescription          (const Unicode::String & resultDescription);
	void                                setAllowOverride              (const bool allowOverride);
	void                                setSkillTemplate              (const std::string & skillTemplate);
	void                                setWorkingSkill               (const std::string & workingSkill);

	void                                setCSToolId                   (const unsigned int toolId);
	TransferCharacterData();
private:
	friend class Archive::AutoVariable<TransferCharacterData>;
	friend struct std::pair<std::string, TransferCharacterData>;
	friend void Archive::get(Archive::ReadIterator & source, TransferCharacterData & target);

	unsigned int                m_track;
	std::string                 m_customerLocalizedLanguage;
	std::string                 m_sourceGalaxy;
	std::string                 m_destinationGalaxy;
	std::string                 m_sourceCharacterName;
	std::string                 m_destinationCharacterName;
	unsigned int                m_sourceStationId;
	unsigned int                m_destinationStationId;
	unsigned int                m_transactionId;
	NetworkId                   m_characterId;
	NetworkId                   m_destinationCharacterId;
	NetworkId                   m_containerId;
	std::string                 m_scene;
	Vector                      m_startingCoordinates;
	std::string                 m_customizationData;
	std::vector<unsigned char>  m_scriptDictionaryData;
	std::string                 m_objectTemplateName;
	uint32                      m_objectTemplateCrc;
	float                       m_scaleFactor;
	std::string                 m_hairTemplateName;
	std::string                 m_hairAppearanceData;
	std::string                 m_profession;
	Unicode::String             m_biography;
	bool                        m_isValidName;
	bool                        m_isMoveRequest;
	bool                        m_withItems;
	Unicode::String             m_resultDescription;
	bool                        m_allowOverride;
	std::string                 m_skillTemplate;
	std::string                 m_workingSkill;
	TransferRequestMoveValidation::TransferRequestSource m_transferRequestSource;

	unsigned int                m_csToolId;
};

// ======================================================================

#endif
