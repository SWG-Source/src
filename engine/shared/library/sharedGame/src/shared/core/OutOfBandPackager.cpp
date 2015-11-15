// OutOfBandPackager.cpp
// Copyright 2000-02, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

//-----------------------------------------------------------------------

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/OutOfBandPackager.h"

#include "Archive/Archive.h"
#include "Archive/ByteStream.h"
#include "Unicode.h"
#include "UnicodeUtils.h"
#include "localizationArchive/StringIdArchive.h"
#include "sharedGame/AuctionToken.h"
#include "sharedGame/AuctionTokenArchive.h"
#include "sharedGame/OutOfBand.h"
#include "sharedGame/OutOfBandPackager.h"
#include "sharedGame/ProsePackage.h"
#include "sharedGame/ProsePackageArchive.h"
#include "sharedGame/WaypointData.h"
#include "sharedObject/ObjectTemplate.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

void OutOfBandPackager::pack(const Archive::ByteStream & source, const unsigned char typeId, const int position, Unicode::String & target)
{
	Archive::ByteStream package;
	Archive::put(package, typeId);
	Archive::put(package, position);
	package.put(source.getBuffer(), source.getSize());
	
	//-- pad out the buffer to fit perfectly in a unicode string
	const int cbufSize                         = package.getSize ();

	int skipRemainder                          = cbufSize % sizeof (Unicode::unicode_char_t);

	for (int i = 0; i < skipRemainder; ++i)
		Archive::put(package, static_cast<unsigned char> (0));

	const Unicode::unicode_char_t * const ubuf = reinterpret_cast<const Unicode::unicode_char_t *>(package.getBuffer ());
	const int ubufSize                         = package.getSize () / sizeof (Unicode::unicode_char_t);

	DEBUG_FATAL (ubufSize * sizeof (Unicode::unicode_char_t) != package.getSize (), ("bad packing logic"));

	//-- note the size of the skip remainder padding so the unpacking function can advance over it
	target.append (1, static_cast<unsigned short>(skipRemainder));
	target.append (ubuf, ubuf + ubufSize);
}

//----------------------------------------------------------------------

void OutOfBandPackager::pack                 (const AttributeVector & attribs, int position, Unicode::String & targetOob)
{
	Archive::ByteStream bs;
	Archive::put (bs, attribs);
	OutOfBandPackager::pack(bs, OutOfBandPackager::OT_objectAttributes, position, targetOob);
}

//----------------------------------------------------------------------

void OutOfBandPackager::pack                 (const AuctionToken & token,  int position, Unicode::String & targetOob)
{
	Archive::ByteStream bs;
	Archive::put (bs, token);
	OutOfBandPackager::pack(bs, OutOfBandPackager::OT_auctionToken, position, targetOob);
}

//----------------------------------------------------------------------

void OutOfBandPackager::pack    (const ProsePackage & pp, int position, Unicode::String & targetOob)
{
	Archive::ByteStream bs;
	Archive::put(bs, pp);

	OutOfBandPackager::pack(bs, OutOfBandPackager::OT_prosePackage, position, targetOob);
}

//----------------------------------------------------------------------

void OutOfBandPackager::pack (const WaypointDataBase & waypointData,  int position, Unicode::String & targetOob)
{
	Archive::ByteStream bs;
	Archive::put (bs, waypointData);
	OutOfBandPackager::pack(bs, OutOfBandPackager::OT_waypointData, position, targetOob);
}

//----------------------------------------------------------------------

void OutOfBandPackager::pack (const StringId & stringId,          int position, Unicode::String & targetOob)
{
	Archive::ByteStream bs;
	Archive::put (bs, stringId);
	OutOfBandPackager::pack(bs, OutOfBandPackager::OT_stringId, position, targetOob);
}

//----------------------------------------------------------------------

void OutOfBandPackager::pack (const std::string & str,          int position, Unicode::String & targetOob)
{
	Archive::ByteStream bs;
	Archive::put (bs, str);
	OutOfBandPackager::pack(bs, OutOfBandPackager::OT_string, position, targetOob);
}

//-----------------------------------------------------------------------

OutOfBandPackager::OutOfBandBaseVector OutOfBandPackager::unpack(const Unicode::String & source, OobTypes type, int count)
{
	if (source.empty ())
		return OutOfBandBaseVector ();

	std::vector<OutOfBandBase *> result;
	
	const unsigned char * const buf = reinterpret_cast<const unsigned char *>(source.data());
	const uint32 buflen             = source.size () * sizeof (Unicode::unicode_char_t);

	Archive::ByteStream bs(buf, buflen);
	Archive::ReadIterator ri(bs);

	unsigned char typeId = 0;
	int position = 0;
	int numUnpacked = 0;

	try
	{
		while(ri.getSize())
		{
			if (count > 0 && numUnpacked >= count)
				break;
			
			unsigned short skipRemainder = 0;
			
			Archive::get(ri, skipRemainder);
			Archive::get(ri, typeId);
			Archive::get(ri, position);

			switch(typeId)
			{
			case OT_object:
				{
				/*
				Archive::get(ri, templateName);
				if(templateName.length())
				{
				Object * const obj = ObjectTemplate::createObject(templateName.c_str());
				if(obj)
				{
				OutOfBand<Object> * const entry = new OutOfBand<Object>(typeId, position, obj);
				result.push_back(entry);
				}
				}
					*/
				}
				break;
			case OT_prosePackage:
				{
					ProsePackage * const pp = new ProsePackage;
					Archive::get (ri, *pp);
					
					if (type != OT_numTypes && type != typeId)
						delete pp;
					else
					{
						OutOfBand<ProsePackage> * const entry = new OutOfBand<ProsePackage>(typeId, position, pp);
						result.push_back (entry);
					}
				}
				break;
			case OT_auctionToken:
				{
					AuctionToken * const token = new AuctionToken;
					Archive::get (ri, *token);
					if (type != OT_numTypes && type != typeId)
						delete token;
					else
					{
						OutOfBand<AuctionToken> * const entry = new OutOfBand<AuctionToken>(typeId, position, token);
						result.push_back (entry);
					}
				}
				break;
				
			case OT_string:
				{
					std::string * const str = new std::string;
					Archive::get (ri, *str);
					if (type != OT_numTypes && type != typeId)
						delete str;
					else
					{
						OutOfBand<std::string> * const entry = new OutOfBand<std::string>(typeId, position, str);
						result.push_back (entry);
					}
				}
				break;
				
			case OT_objectAttributes:
				{
					AttributeVector * const av = new AttributeVector;
					Archive::get (ri, *av);
					if (type != OT_numTypes && type != typeId)
						delete av;
					else
					{
						OutOfBand<AttributeVector> * const entry = new OutOfBand<AttributeVector>(typeId, position, av);
						result.push_back (entry);
					}
				}
				break;
				
			case OT_waypointData:
				{
					WaypointDataBase * const waypointData = new WaypointDataBase;
					Archive::get (ri, *waypointData);
					if (type != OT_numTypes && type != typeId)
						delete waypointData;
					else
					{
						OutOfBand<WaypointDataBase> * const entry = new OutOfBand<WaypointDataBase>(typeId, position, waypointData);
						result.push_back (entry);
					}
				}
				break;
				
			default:
				WARNING (true, ("OutOfBandPackager recieved unknown type [%d]", typeId));
				break;
			}
			
			++numUnpacked;
			
			//- advance over padding space
			if (skipRemainder)
				ri.advance (skipRemainder);
		}
	}
	catch (const Archive::ReadException & readException)
	{
		WARNING (true, ("Archive read error (%s) unpacking OOB type [%d] at position [%d]", readException.what(), typeId, position));
	}

	return result;
}

//----------------------------------------------------------------------

OutOfBandBase *  OutOfBandPackager::unpackOne  (const Unicode::String & source, OobTypes type)
{
	const OutOfBandBaseVector & v = unpack (source, type, 1);
	if (!v.empty ())
		return v.front ();

	return 0;
}

//-----------------------------------------------------------------------

