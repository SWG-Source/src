// ======================================================================
//
// UnicodeCharDataMap.h
// copyright (c) 2001 Sony Online Entertainment
//
// This contains Unicode 3.0 compliant Character data formats
// The data is loaded dynamically from the UnicodeData.txt definition file
//
// ======================================================================

#ifndef INCLUDED_UnicodeCharDataMap_H
#define INCLUDED_UnicodeCharDataMap_H

#if WIN32
// stl warning func not inlined
#pragma warning (disable:4710)
#pragma warning (disable:4786)
#endif

#include <map>
#include <set>

#include "UnicodeBlocks.h"

// ======================================================================

namespace Unicode
{
	struct CharData;
		
	//-----------------------------------------------------------------
	
	/**
	* Singleton class that allows mapping from code points to CharData objects.  This will be
	* used to change case of characters, character category testing, etc...
	* A CharacterDataMap has a set of Unicode::Block::Id's with which it is solely concerned.
	* In this way, you may instantiate many different CharDataMaps if needed.
	*/ 

	class CharDataMap
	{
	public:
		
		enum ErrorCode
		{
			ERR_SUCCESS      = 0,
			ERR_NO_FILE,
			ERR_READ_FAILED,
			ERR_CLOSE_FAILED,
			ERR_CHAR_NOT_FOUND,
			ERR_FILE_TOO_SHORT
		};
		
		typedef std::map <unicode_char_t, CharData *> Map_t;
		
		static CharDataMap & getDefaultMap ();
		static void          explicitDestroy ();
		
		                     CharDataMap ();
		                    ~CharDataMap ();
		
		void                 addBlock    (Blocks::Id id);
		void                 removeBlock (Blocks::Id id);
		void                 clearBlocks ();
		
		ErrorCode            generateMap           (const Blocks::Mapping & blockMapping, const char * filename);
		ErrorCode            generateMapFromBuffer (const Blocks::Mapping & blockMapping, const std::string & str_buf);
		
		bool                 isValid () const;
		
		const CharData *     findCharData (unicode_char_t code) const;
		
		const Map_t &        getMap () const;
		
	private:
		
		static void          initSingleton ();
		
		typedef std::set<Blocks::Id>   BlockIdSet_t;
		
		BlockIdSet_t         m_blockIds;
		CharData *           m_contiguousData;
		Map_t                m_map;
		bool                 m_dataIsValid;
		
		static CharDataMap * ms_singleton;
		
	};
	
	//-----------------------------------------------------------------
	
	/**
	* Singleton accessor.
	*/

	inline CharDataMap & CharDataMap::getDefaultMap ()
	{
		if (ms_singleton)
			return *ms_singleton;
		
		initSingleton ();
		return *ms_singleton;
	}

	//-----------------------------------------------------------------
	
	/**
	* Has the data been loaded successfully?
	*/

	inline bool CharDataMap::isValid () const
	{
		return m_dataIsValid;
	}
	
	//-----------------------------------------------------------------
	
	/**
	* Find a CharData for the given code point.
	* @return nullptr if no CharData exists for this code point
	*/

	inline const CharData *     CharDataMap::findCharData (const unicode_char_t code) const
	{
		if (isValid ())
		{
			const Map_t::const_iterator iter = m_map.find (code);
			if (iter != m_map.end ())
				return (*iter).second;
		}
		
		return 0;
	}
	
	//-----------------------------------------------------------------
	
	/**
	* Get the data mapping
	*/

	inline const CharDataMap::Map_t &        CharDataMap::getMap () const
	{
		return m_map;
	}	
}
// ======================================================================
	
#endif
