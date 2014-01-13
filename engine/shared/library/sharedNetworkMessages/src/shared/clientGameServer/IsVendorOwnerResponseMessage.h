// ======================================================================
//
// IsVendorOwnerResponseMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_IsVendorOwnerResponseMessage_H
#define	_IsVendorOwnerResponseMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/AuctionErrorCodes.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkIdArchive.h"

//-----------------------------------------------------------------------

class IsVendorOwnerResponseMessage : public GameNetworkMessage
{
public:

	enum VendorOwnerResult
	{
		vor_IsOwner,
		vor_IsNotOwner,
		vor_HasNoOwner
	};

	static const char * const MessageType;

	IsVendorOwnerResponseMessage(const NetworkId &containerId, const std::string &marketName, VendorOwnerResult ownerResult, AuctionResult result, uint16 maxPageSize);
	explicit IsVendorOwnerResponseMessage(Archive::ReadIterator &source);

	~IsVendorOwnerResponseMessage();

public: // methods

	VendorOwnerResult                getOwnerResult () const;
	AuctionResult                    getResult      () const;
	const NetworkId &                getContainerId () const;
	const std::string &              getMarketName  () const;
	uint16                           getMaxPageSize () const; //returns the maximum size that a queried page will return

public: // types

private: 
	Archive::AutoVariable<int>               m_ownerResult;  
	Archive::AutoVariable<int>               m_result;   
	Archive::AutoVariable<NetworkId>         m_containerId;   
	Archive::AutoVariable<std::string>       m_marketName;   
	Archive::AutoVariable<uint16>            m_maxPageSize;   
};

// ----------------------------------------------------------------------

inline IsVendorOwnerResponseMessage::VendorOwnerResult IsVendorOwnerResponseMessage::getOwnerResult() const
{
	return static_cast<VendorOwnerResult>(m_ownerResult.get());
}

// ----------------------------------------------------------------------

inline AuctionResult IsVendorOwnerResponseMessage::getResult() const
{
	return static_cast<AuctionResult>(m_result.get());
}

// ----------------------------------------------------------------------

inline const NetworkId & IsVendorOwnerResponseMessage::getContainerId() const
{
	return m_containerId.get();
}

//----------------------------------------------------------------------

inline const std::string & IsVendorOwnerResponseMessage::getMarketName  () const
{
	return m_marketName.get ();
}

//----------------------------------------------------------------------

inline uint16 IsVendorOwnerResponseMessage::getMaxPageSize  () const
{
	return m_maxPageSize.get ();
}

// ----------------------------------------------------------------------

#endif // _IsVendorOwnerResponseMessage_H

