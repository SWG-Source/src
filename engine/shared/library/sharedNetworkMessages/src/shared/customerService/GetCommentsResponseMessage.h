// ======================================================================
//
// GetCommentsResponseMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_GetCommentsResponseMessage_H
#define	_GetCommentsResponseMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedNetworkMessages/CustomerServiceComment.h"
#include "sharedNetworkMessages/CustomerServiceCommentArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class GetCommentsResponseMessage : public GameNetworkMessage
{
public:
	GetCommentsResponseMessage(int32 result, const std::vector<CustomerServiceComment> &comments );
	explicit GetCommentsResponseMessage(Archive::ReadIterator &source);

	~GetCommentsResponseMessage();

public: // methods

	int32                            getResult()           const;
	const std::vector<CustomerServiceComment> &   getComments()         const;

public: // types

private: 
	Archive::AutoVariable<int32>          m_result;  
	Archive::AutoArray<CustomerServiceComment>         m_comments;   
};

// ----------------------------------------------------------------------

inline const std::vector<CustomerServiceComment> & GetCommentsResponseMessage::getComments() const
{
	return m_comments.get();
}

// ----------------------------------------------------------------------

inline int32 GetCommentsResponseMessage::getResult() const
{
	return m_result.get();
}

// ----------------------------------------------------------------------

#endif // _GetCommentsResponseMessage_H

