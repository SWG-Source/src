// ======================================================================
//
// AuctionQueryHeadersResponseMessage.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef	_AuctionQueryHeadersResponseMessage_H
#define	_AuctionQueryHeadersResponseMessage_H

//-----------------------------------------------------------------------

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/AuctionData.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"

#include "Unicode.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

class AuctionQueryHeadersResponseMessage : public GameNetworkMessage
{
public:

	static const char * const MessageType;

	typedef std::vector<Auction::ItemDataHeader>           AuctionHeaderVector;
	typedef std::vector<Auction::PalettizedItemDataHeader> PalettizedAuctionHeaderVector;

	AuctionQueryHeadersResponseMessage(int requestId, int typeFlag, AuctionHeaderVector const &auctionData, uint16 queryOffset, bool hasMorePages);
	explicit AuctionQueryHeadersResponseMessage(Archive::ReadIterator &source);

	~AuctionQueryHeadersResponseMessage();

public: // methods

	int                         getRequestId    () const;
	int                         getTypeFlag     () const;
	AuctionHeaderVector const & getAuctionData  () const;
	uint16                      getQueryOffset  () const;
	bool                        getHasMorePages () const;

public: // types

private: 
	Archive::AutoVariable<int>                   m_requestId;
	Archive::AutoVariable<int>                   m_typeFlag;
	Archive::AutoArray<std::string>              m_stringPalette;
	Archive::AutoArray<Unicode::String>          m_wideStringPalette;
	Archive::AutoArray<Auction::PalettizedItemDataHeader>  m_palettizedAuctionData;
	std::vector<Auction::ItemDataHeader>         m_auctionData;
	Archive::AutoVariable<uint16>                m_queryOffset;
	Archive::AutoVariable<bool>                  m_hasMorePages;

	void                        palettizeAuctionData();
	void                        decodeAuctionData();
	uint16                      getPaletteIndex(const std::string &str);
	uint16                      getPaletteIndex(const Unicode::String &str);
};

// ----------------------------------------------------------------------

inline int AuctionQueryHeadersResponseMessage::getRequestId() const
{
	return m_requestId.get();
}

// ----------------------------------------------------------------------

inline int AuctionQueryHeadersResponseMessage::getTypeFlag() const
{
	return m_typeFlag.get();
}

// ----------------------------------------------------------------------

inline AuctionQueryHeadersResponseMessage::AuctionHeaderVector const & AuctionQueryHeadersResponseMessage::getAuctionData() const
{
	return m_auctionData;
}

// ----------------------------------------------------------------------

inline uint16 AuctionQueryHeadersResponseMessage::getQueryOffset() const
{
	return m_queryOffset.get();
}

// ----------------------------------------------------------------------

inline bool AuctionQueryHeadersResponseMessage::getHasMorePages() const
{
	return m_hasMorePages.get();
}

// ----------------------------------------------------------------------

namespace Archive
{

	void get(ReadIterator &source, Auction::PalettizedItemDataHeader &target);
	void put(ByteStream &target, Auction::PalettizedItemDataHeader const &source);
}

// ----------------------------------------------------------------------

#endif // _AuctionQueryHeadersResponseMessage_H

