// ======================================================================
//
// OpenHolocronToPageMessage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/OpenHolocronToPageMessage.h"

#include "Archive/ByteStream.h"
#include "Archive/AutoByteStream.h"

// ======================================================================

const char * const OpenHolocronToPageMessage::MessageType = "OpenHolocronToPageMessage";

// ======================================================================

OpenHolocronToPageMessage::OpenHolocronToPageMessage(const std::string& page)
: GameNetworkMessage(MessageType),
  m_page(page)
{
	addVariable(m_page);
}

//-----------------------------------------------------------------------

OpenHolocronToPageMessage::OpenHolocronToPageMessage(Archive::ReadIterator& source)
: GameNetworkMessage(MessageType),
  m_page()
{
	addVariable (m_page);
	unpack (source);
}

//-----------------------------------------------------------------------

const std::string&      OpenHolocronToPageMessage::getPage() const
{
	return m_page.get();
}

// ======================================================================

