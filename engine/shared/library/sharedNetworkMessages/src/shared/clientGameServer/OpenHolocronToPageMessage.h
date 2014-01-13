
// ======================================================================
//
// OpenHolocronToPageMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_OpenHolocronToPageMessage_H
#define INCLUDED_OpenHolocronToPageMessage_H

//-----------------------------------------------------------------------

#include "Archive/Archive.h"

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class GameInByteStream;
class GameOutByteStream;

//-----------------------------------------------------------------------

/** The network message to open the client's holocube window and display a particular page
 */
class OpenHolocronToPageMessage : public GameNetworkMessage
{
public:
	OpenHolocronToPageMessage(const std::string& page);
	explicit OpenHolocronToPageMessage (Archive::ReadIterator& source);

	const std::string&           getPage() const;

	static const char * const MessageType;

private:
	//the name of the page to use
	Archive::AutoVariable<std::string> m_page;
};

//-----------------------------------------------------------------------

#endif
