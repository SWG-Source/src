// ==================================================================
//
// EnterStructurePlacementModeMessage.h
// Copyright 2002 Sony Online Entertainment
//
// ==================================================================

#ifndef	INCLUDED_EnterStructurePlacementModeMessage_H
#define	INCLUDED_EnterStructurePlacementModeMessage_H

// ==================================================================

#include "Archive/AutoDeltaByteStream.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

// ==================================================================

class EnterStructurePlacementModeMessage : public GameNetworkMessage
{
public: 

	EnterStructurePlacementModeMessage (const NetworkId& deedNetworkId, const std::string& structureSharedObjectTemplateName);
	explicit EnterStructurePlacementModeMessage (Archive::ReadIterator &source);
	virtual ~EnterStructurePlacementModeMessage();

	const NetworkId&   getDeedNetworkId () const;
	const std::string& getStructureSharedObjectTemplateName () const;

public:

	static const char* const cms_name;

private: 

	Archive::AutoVariable<NetworkId>   m_deedNetworkId;
	Archive::AutoVariable<std::string> m_structureSharedObjectTemplateName;
};

// ==================================================================

#endif 
