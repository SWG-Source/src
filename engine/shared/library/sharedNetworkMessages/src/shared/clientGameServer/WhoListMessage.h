
// ======================================================================
//
// WhoListMessage.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_WhoListMessage_H
#define INCLUDED_WhoListMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "Unicode.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

class GameInByteStream;
class GameOutByteStream;

//-----------------------------------------------------------------------

class WhoListMessage : public GameNetworkMessage
{
public:

	static const char * const MESSAGE_TYPE;

	           WhoListMessage ();
	explicit   WhoListMessage (Archive::ReadIterator & source);
	virtual   ~WhoListMessage ();

	const std::vector<Unicode::String> &    getDataArray () const;
	void                                       setData (const std::vector<Unicode::String> & data);
	
private:

	Archive::AutoArray<Unicode::String>        m_data;

private:
	WhoListMessage(const WhoListMessage&);
	WhoListMessage& operator= (const WhoListMessage&);
};

//-----------------------------------------------------------------------

inline const std::vector<Unicode::String> & WhoListMessage::getDataArray () const
{
	return m_data.get ();
}

//-----------------------------------------------------------------

#endif
