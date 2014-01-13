
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

	const stdvector<Unicode::String>::fwd &    getDataArray () const;
	void                                       setData (const stdvector<Unicode::String>::fwd & data);
	
private:

	Archive::AutoArray<Unicode::String>        m_data;

private:
	WhoListMessage(const WhoListMessage&);
	WhoListMessage& operator= (const WhoListMessage&);
};

//-----------------------------------------------------------------------

inline const stdvector<Unicode::String>::fwd & WhoListMessage::getDataArray () const
{
	return m_data.get ();
}

//-----------------------------------------------------------------

#endif
