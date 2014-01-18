// ======================================================================
//
// UnicodeBlocks.h
// copyright (c) 2001 Sony Online Entertainment
//
// This contains Unicode 3.0 compliant Blocks.
//
// ======================================================================

#ifndef INCLUDED_PlatUnicodeBlocks_H
#define INCLUDED_PlatUnicodeBlocks_H

#if WIN32
// stl warning func not inlined
#pragma warning (disable:4710)
#pragma warning (disable:4786)
#endif

#include "Unicode.h"

#include <map>
#include <vector>
#include <set>

// ======================================================================
#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{
#endif

namespace Plat_Unicode 
{

	namespace Blocks
	{
		/**
		* The enum values are precisely the beginning of the represented block range.
		* These blocks are precisely the blocks defined by Unicode 3.0.
		*/
		enum Id
		{		
			Basic_Latin    = 0x0000,
			Latin_1_Supplement    = 0x0080,
			Latin_Extended_A    = 0x0100,
			Latin_Extended_B    = 0x0180,
			IPA_Extensions    = 0x0250,
			Spacing_Modifier_Letters    = 0x02B0,
			Combining_Diacritical_Marks    = 0x0300,
			Greek    = 0x0370,
			Cyrillic    = 0x0400,
			Armenian    = 0x0530,
			Hebrew    = 0x0590,
			Arabic    = 0x0600,
			Syriac    = 0x0700,
			Thaana    = 0x0780,
			Devanagari    = 0x0900,
			Bengali    = 0x0980,
			Gurmukhi    = 0x0A00,
			Gujarati    = 0x0A80,
			Oriya    = 0x0B00,
			Tamil    = 0x0B80,
			Telugu    = 0x0C00,
			Kannada    = 0x0C80,
			Malayalam    = 0x0D00,
			Sinhala    = 0x0D80,
			Thai    = 0x0E00,
			Lao    = 0x0E80,
			Tibetan    = 0x0F00,
			Myanmar    = 0x1000,
			Georgian    = 0x10A0,
			Hangul_Jamo    = 0x1100,
			Ethiopic    = 0x1200,
			Cherokee    = 0x13A0,
			Unified_Canadian_Aboriginal_Syllabics    = 0x1400,
			Ogham    = 0x1680,
			Runic    = 0x16A0,
			Khmer    = 0x1780,
			Mongolian    = 0x1800,
			Latin_Extended_Additional    = 0x1E00,
			Greek_Extended    = 0x1F00,
			General_Punctuation    = 0x2000,
			Superscripts_and_Subscripts    = 0x2070,
			Currency_Symbols    = 0x20A0,
			Combining_Marks_for_Symbols    = 0x20D0,
			Letterlike_Symbols    = 0x2100,
			Number_Forms    = 0x2150,
			Arrows    = 0x2190,
			Mathematical_Operators    = 0x2200,
			Miscellaneous_Technical    = 0x2300,
			Control_Pictures    = 0x2400,
			Optical_Character_Recognition    = 0x2440,
			Enclosed_Alphanumerics    = 0x2460,
			Box_Drawing    = 0x2500,
			Block_Elements    = 0x2580,
			Geometric_Shapes    = 0x25A0,
			Miscellaneous_Symbols    = 0x2600,
			Dingbats    = 0x2700,
			Braille_Patterns    = 0x2800,
			CJK_Radicals_Supplement    = 0x2E80,
			Kangxi_Radicals    = 0x2F00,
			Ideographic_Description_Characters    = 0x2FF0,
			CJK_Symbols_and_Punctuation    = 0x3000,
			Hiragana    = 0x3040,
			Katakana    = 0x30A0,
			Bopomofo    = 0x3100,
			Hangul_Compatibility_Jamo    = 0x3130,
			Kanbun    = 0x3190,
			Bopomofo_Extended    = 0x31A0,
			Enclosed_CJK_Letters_and_Months    = 0x3200,
			CJK_Compatibility    = 0x3300,
			CJK_Unified_Ideographs_Extension_A    = 0x3400,
			CJK_Unified_Ideographs    = 0x4E00,
			Yi_Syllables    = 0xA000,
			Yi_Radicals    = 0xA490,
			Hangul_Syllables    = 0xAC00,
			High_Surrogates    = 0xD800,
			High_Private_Use_Surrogates    = 0xDB80,
			Low_Surrogates    = 0xDC00,
			Private_Use    = 0xE000,
			CJK_Compatibility_Ideographs    = 0xF900,
			Alphabetic_Presentation_Forms    = 0xFB00,
			Arabic_Presentation_Forms_A    = 0xFB50,
			Combining_Half_Marks    = 0xFE20,
			CJK_Compatibility_Forms    = 0xFE30,
			Small_Form_Variants    = 0xFE50,
			Arabic_Presentation_Forms_B    = 0xFE70,
			Specials_1    = 0xFEFF,
			Halfwidth_and_Fullwidth_Forms    = 0xFF00,
			Specials_2    = 0xFFF0,
			End_Block_Ids    = 0xFFFE
			
		};
		
		
		//-----------------------------------------------------------------
		
		/**
		* A simple pair representing an inclusive range of values
		*/

		struct Range
		{
			unicode_char_t m_high;
			unicode_char_t m_low;
		};

		//-----------------------------------------------------------------

		/**
		* A group of ranges that can be additive or subtractive (inclusive or non)
		*/

		struct RangeGroup
		{
			bool     m_isInclusive;
			typedef std::vector<Range> Ranges_t;
			Ranges_t m_ranges;
		};

		//-----------------------------------------------------------------
		
		/**
		* RawData is a simple aggregate data struct used to construct the more complex Data objects
		*/
		struct RawData
		{
			Id                    m_id;
			const char *          m_name;
			unicode_char_t        m_start;
			unicode_char_t        m_end;
		};
		
		/**
		* Data is the representation of a single Unicode block.
		* Including the Id, name, gross start-end range, and a set of RangeGroups (usually subtractive)
		*/
		struct Data 
		{
			Id                       m_id;
			NarrowString    m_name;
			unicode_char_t           m_start;
			unicode_char_t           m_end;

			typedef std::vector <RangeGroup>  RangeGroups_t;
			RangeGroups_t            m_rangeGroups;

			explicit                 Data (const RawData & rhs);
			                         Data ();

			const std::set<unicode_char_t> &      generateFilteredIdSet (std::set<unicode_char_t> & ) const;

			bool                     addRangeGroup (bool inclusive, const NarrowString & str);

		};
		
		//-----------------------------------------------------------------
		
		/**
		* Singleton class.
		*
		* Mapping creates a method of mapping block Ids _or_ names to Block Data objects.
		* @todo: Data objects are duplicate for each map, need to optimize.
		*/
		
		class Mapping
		{
			
		public:
			
			explicit               Mapping (const RawData dataArray[]);
			static const Mapping & getDefaultMapping ();
			static void            explicitDestroy ();
			
			typedef std::map <Id, Data>                    IdMap_t;
			typedef std::map <NarrowString, Data> NameMap_t;
			
			const Data &           findBlock (Id id) const;
			Data &                 findBlock (Id id);
			const Data &           findBlock (const NarrowString &) const;
			const Data &           findBlock (unicode_char_t code) const;
			
			const IdMap_t &        getIdMap () const;

			bool                   addBlock (const Data & data);
			
			static const RawData   ms_defaultBlockData [];
			
		private:
			
			                       Mapping ();
			                       Mapping (const Mapping & rhs);
			                       Mapping & operator= (const Mapping & rhs);
			
			static void            initSingleton ();
			
			static Mapping *       ms_singleton;
			
			IdMap_t                m_idMap;
			NameMap_t              m_nameMap;
			
			Data                   m_errorData;
			
		};
		
		//-----------------------------------------------------------------

		/**
		* Convert a RawData into a Data object
		*/

		inline Data::Data (const RawData & rhs) :
			m_id (rhs.m_id),
			m_name (rhs.m_name),
			m_start (rhs.m_start),
			m_end (rhs.m_end),
			m_rangeGroups ()
		{
		}

		//----------------------------------------------------------------------

		/**
		* Default constructor.
		*/

		inline Data::Data () :
			m_id (End_Block_Ids),
			m_name (),
			m_start (0),
			m_end (0),
			m_rangeGroups ()
		{
		}

		//-----------------------------------------------------------------

		/**
		* Static singleton accessor
		*/
		
		inline const Mapping & Mapping::getDefaultMapping ()
		{
			if (ms_singleton)
				return *ms_singleton;
			
			initSingleton ();
			return *ms_singleton;
		}

		//-----------------------------------------------------------------

		/**
		* Add a block Data object to the mapping.  If the block already exists
		* in the mapping, the new data overwrites the old.
		*/

		inline bool Mapping::addBlock (const Data & data)
		{
			m_idMap   [data.m_id]   = data;
			m_nameMap [data.m_name] = data;
			return true;
		}

		//-----------------------------------------------------------------
		
		/**
		* Locate the block data by Id.
		* @return a Data with End_Block_Ids Id if the block was not found
		*/

		inline const Data & Mapping::findBlock (Id id) const
		{
			
			const IdMap_t::const_iterator iter = m_idMap.find (id);
			if (iter != m_idMap.end ())
				return (*iter).second;
			else
				return m_errorData;
		}
		
		//-----------------------------------------------------------------

		/**
		* Locate the block data by Id.
		* @return a Data with End_Block_Ids Id if the block was not found
		*/

		inline Data & Mapping::findBlock (Id id)
		{
			
			const IdMap_t::iterator iter = m_idMap.find (id);
			if (iter != m_idMap.end ())
				return (*iter).second;
			else
				return m_errorData;
		}
		
		//-----------------------------------------------------------------

		/**
		* Locate the block data by Name.
		* @return a Data with End_Block_Ids Id if the block was not found
		*/

		inline const Data & Mapping::findBlock (const NarrowString & name) const
		{
			
			const NameMap_t::const_iterator iter = m_nameMap.find (name);
			if (iter != m_nameMap.end ())
				return (*iter).second;
			else
				return m_errorData;
		}
		
		//-----------------------------------------------------------------

		/**
		* Locate the block data by code point.
		* @return a Data with End_Block_Ids Id if the block was not found.  This only tests if the code point is in the gross range of the located block.
		*/

		inline const Data & Mapping::findBlock (unicode_char_t code) const
		{
			
			const IdMap_t::const_iterator iter = m_idMap.lower_bound (static_cast<Id>(code));
			if (iter != m_idMap.end () && code <= (*iter).second.m_end)
				return (*iter).second;
			else
				return m_errorData;
		}
		
		//-----------------------------------------------------------------

		/**
		* Gets the Id->Data map.
		*/

		inline const Mapping::IdMap_t & Mapping::getIdMap () const
		{
			return m_idMap;
		}

		//-----------------------------------------------------------------

		/**
		* Tests a character to see if belongs in an ideograph range
		*/

		inline bool isIdeograph (unicode_char_t code)
		{
			return ((code >= static_cast<unicode_char_t>(CJK_Unified_Ideographs_Extension_A) && code < static_cast<unicode_char_t>(Yi_Syllables)) ||
					(code >= static_cast<unicode_char_t>(Hangul_Syllables) && code < static_cast<unicode_char_t>(CJK_Compatibility_Ideographs)));
		}

	} //-- namespace Blocks

}; // -- namespace Unicode
#ifdef EXTERNAL_DISTRO
};
#endif 

// ======================================================================	
#endif

