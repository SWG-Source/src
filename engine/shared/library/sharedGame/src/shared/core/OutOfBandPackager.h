// OutOfBandPackager.h
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_OutOfBandPackager_H
#define	_INCLUDED_OutOfBandPackager_H

//-----------------------------------------------------------------------

namespace Archive
{
	class ByteStream;
}

//-----------------------------------------------------------------------

class OutOfBandBase;
class ProsePackage;
class AuctionToken;
class WaypointDataBase;

//-----------------------------------------------------------------------

class OutOfBandPackager
{
public:

	typedef std::vector<OutOfBandBase *> OutOfBandBaseVector;
	typedef std::vector<std::pair<std::string, Unicode::String> > AttributeVector;

	enum OobTypes
	{
		OT_object,
		OT_prosePackage,
		OT_auctionToken,
		OT_objectAttributes,
		OT_waypointData,
		OT_stringId,
		OT_string,
		OT_numTypes
	};

	~OutOfBandPackager();

	static void                             pack                 (const AttributeVector & attribs,    int position, Unicode::String & targetOob);
	static void                             pack                 (const AuctionToken & token,         int position, Unicode::String & targetOob);
	static void                             pack                 (const ProsePackage & pp,            int position, Unicode::String & targetOob);
	static void                             pack                 (const WaypointDataBase & waypointData, int position, Unicode::String & targetOob);
	static void                             pack                 (const StringId & stringId,          int position, Unicode::String & targetOob);
	static void                             pack                 (const std::string & str,          int position, Unicode::String & targetOob);

	static void                             pack                 (const Archive::ByteStream & source, const unsigned char typeId, const int position, Unicode::String & target);

	static OutOfBandBaseVector              unpack               (const Unicode::String & source, OobTypes type = OT_numTypes, int count = -1);

	static OutOfBandBase *                  unpackOne            (const Unicode::String & source, OobTypes type = OT_numTypes);

private:
	OutOfBandPackager();
	OutOfBandPackager(const OutOfBandPackager &);
	OutOfBandPackager & operator =  (const OutOfBandPackager &);
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_OutOfBandPackager_H
