// ======================================================================
//
// UnicodeCharDataMap.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// This contains Unicode 3.0 compliant Character data formats
// The data is loaded dynamically from the UnicodeData.txt definition file
//
// ======================================================================

#include "FirstUnicode.h"
#include "UnicodeCharacterDataMap.h"
#include "UnicodeBlocks.h"
#include "UnicodeCharacterData.h"

#include <cstdlib>
#include <cstdio>
#include <string>
#include <cassert>

// ======================================================================

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{
#endif

namespace Plat_Unicode 
{

CharDataMap *                    CharDataMap::ms_singleton = 0;

//-----------------------------------------------------------------

CharDataMap::CharDataMap () :
m_blockIds (),
m_contiguousData (0),
m_map (),
m_dataIsValid (false)
{

}

//-----------------------------------------------------------------

CharDataMap::~CharDataMap ()
{
	m_map.clear ();
	m_blockIds.clear ();

	if (m_contiguousData)
	{
		delete[] m_contiguousData;
		m_contiguousData = 0;
	}
}

//-----------------------------------------------------------------

void CharDataMap::initSingleton ()
{
	explicitDestroy ();

	// TODO: default blocks?
	ms_singleton = new CharDataMap ();
}

//-----------------------------------------------------------------

void  CharDataMap::explicitDestroy ()
{
	delete ms_singleton;

	ms_singleton = 0;
}

//-----------------------------------------------------------------

namespace
{
	
	typedef std::map <std::string, CharData::Category> CategoryMap_t;
	
	//-----------------------------------------------------------------
	
	
	/**
	* Load a CharData from a string buffer representing a file.
	* The bufpos is set to the beginning of a line after this method returns.
	*/
	
	
	inline bool populateDataFromBuffer (size_t desiredCode,
		CharData & charData,
		const std::string & buffer,
		size_t & bufpos,
		const CategoryMap_t & cmap)
	{ 
		
		size_t endpos = 0;
		
		bool found = false;
		
		do
		{
			size_t tokenNum = 0;
			
			while ( (endpos = buffer.find ( ';', bufpos)) != std::string::npos ) //lint !e737 // loss of sign in promotion - STL bug
			{
				// code value
				if (tokenNum == 0)
				{
					const size_t code = static_cast <size_t> (strtoul (buffer.substr (bufpos, endpos - bufpos).c_str (), 0, 16));
					
					// we passed the desired line, so it must not exist
					if (code > desiredCode)
						return found;
					// not there yet
					else if (code < desiredCode)
						break;
					
					charData.m_code = static_cast<unicode_char_t> (code);
				}
				
				// General Category
				else if (tokenNum == 2)
				{
					// all category codes are currently 2 characters long
					if (endpos - bufpos > 2)
						break;
					
					const CategoryMap_t::const_iterator find_iter = cmap.find (buffer.substr (bufpos, 2));
					
					if (find_iter == cmap.end ())
					{	
						break;
					}
					else
						charData.m_category = (*find_iter).second;
				}
				
				// Uppercase mapping
				else if (charData.isLowerCase ())
				{
					if (tokenNum == 12)
					{
						if (endpos - bufpos > 1)
							charData.m_reverseCase = static_cast<unicode_char_t> (strtoul (buffer.substr (bufpos, endpos - bufpos).c_str (), 0, 16));
						
						found = true;
						break;
					}
				}
				// Lowercase mapping
				else if (tokenNum == 13)
				{
					if (endpos - bufpos > 1)
						charData.m_reverseCase = static_cast<unicode_char_t> (strtoul (buffer.substr (bufpos, endpos - bufpos).c_str (), 0, 16));
					
					found = true;
					break;
				}
				
				bufpos = endpos + 1;
				++tokenNum;
				
			}
		}
		while ( (bufpos = buffer.find ('\n', bufpos)) != std::string::npos &&             //lint !e737 // loss of sign in promotion - STL bug
			(bufpos = buffer.find_first_not_of ('\n', bufpos + 1)) != std::string::npos); //lint !e737 // loss of sign in promotion - STL bug
		
		return found;
		
	}	
}

//-----------------------------------------------------------------

/**
* Create a map of CharData's from a UnicodeData.txt standard Unicode 3.0 file, which has been packed into a buffer
*/

CharDataMap::ErrorCode CharDataMap::generateMapFromBuffer (const Blocks::Mapping & blockMapping, const std::string & str_buf)
{
	m_dataIsValid = false;

	m_map.clear ();

	//-- create the category map for use by the inline population function
	CategoryMap_t        cmap;

	{
		size_t i = 0;
		for (; CharData::ms_categoryNames [i].m_category != CharData::Cn; ++i)
		{
			cmap [CharData::ms_categoryNames [i].m_str] = CharData::ms_categoryNames [i].m_category;
		}
	}
	
	typedef std::set<unicode_char_t> IdSet_t;

	IdSet_t idSet;

	{
		for (BlockIdSet_t::const_iterator iter = m_blockIds.begin (); iter != m_blockIds.end (); ++iter)
		{
			const Blocks::Data & blockData = blockMapping.findBlock (*iter);
			
			if (blockData.m_id != Blocks::End_Block_Ids)
			{
				blockData.generateFilteredIdSet (idSet);
			}
		}
	}

	//-- read all the desired chars from file.
	//-- the file must be sorted by code

	size_t bufpos = 0;
	size_t validChars = 0;

	{
		
		for (IdSet_t::const_iterator iter = idSet.begin (); iter != idSet.end (); ++iter)
		{
			CharData * data = new CharData;
			
			assert (data); //lint !e1924 // c-style cast.  MSVC bug

			data->m_reverseCase = 0;
			
			if (populateDataFromBuffer (*iter, *data, str_buf, bufpos, cmap) == false)
			{
				delete data;
				continue;
			}
			
			else
			{
				++validChars;
				m_map[data->m_code] = data;
			}
		}
	}

	//--
	//-- pack all the char data into a contiguous block of memory
	//--
	
	delete[] m_contiguousData;
	
	m_contiguousData = new CharData [validChars];

	assert (m_contiguousData); //lint !e1924 // c-style cast.  MSVC bug
	
	size_t dataIndex = 0;
	
	for (Map_t::iterator iter = m_map.begin (); iter != m_map.end (); ++iter, ++dataIndex)
	{
		m_contiguousData [dataIndex] = *(*iter).second;
		delete (*iter).second;
		(*iter).second = &m_contiguousData [dataIndex];
	}
	
	m_dataIsValid = true;
	
	return ERR_SUCCESS;
}

//----------------------------------------------------------------------

/**
* Create a map of CharData's from a UnicodeData.txt standard Unicode 3.0 file, using cstdio
*/

CharDataMap::ErrorCode CharDataMap::generateMap (const Blocks::Mapping & blockMapping, const char * filename)
{
	//-- load data from file as a big std::string
	//-- read all the desired chars from file.
	//-- the file must be sorted by code

	FILE * fl = fopen (filename, "rb");

	if (fl == 0)
	{
		return ERR_NO_FILE;
	}

	fseek (fl, 0, SEEK_END);

	size_t fileLen = static_cast<size_t>(ftell (fl));

	fseek (fl, 0, SEEK_SET);

	char * buffer = new char [fileLen + 1];
	buffer [fileLen] = 0;

	assert (buffer); //lint !e1924 // c-style cast.  MSVC bug

	if (fread (buffer, fileLen, 1, fl) != 1)
	{
		delete[] buffer;
		fclose (fl);
		return ERR_READ_FAILED;
	}

	if (fclose (fl))
	{
		delete[] buffer;
		return ERR_CLOSE_FAILED;
	}

	std::string str_buf (buffer, fileLen);

	delete[] buffer;
	
	return generateMapFromBuffer (blockMapping, str_buf);
}

//-----------------------------------------------------------------

/**
* Add a block Id that this CharacterDataMap is concerned with.  Does not change the data map.
*/

void CharDataMap::addBlock    (Blocks::Id id)
{
	m_blockIds.insert (id);
}

//-----------------------------------------------------------------

/**
* Remove a block Id that this CharacterDataMap is concerned with.  Does not change the data map.
*/

void CharDataMap::removeBlock (Blocks::Id id)
{
	m_blockIds.erase (id);
}

//-----------------------------------------------------------------

/*
* Add a block Id that this CharacterDataMap is concerned with.  Does not change the data map.
*/

void CharDataMap::clearBlocks ()
{
	m_blockIds.clear ();
}

// ======================================================================

}; 
#ifdef EXTERNAL_DISTRO
};
#endif 

