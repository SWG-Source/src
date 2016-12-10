// ============================================================================
//
// MatchMakingId.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_MatchMakingId_H
#define INCLUDED_MatchMakingId_H

#include "Archive/AutoByteStream.h"
#include <bitset>

//-----------------------------------------------------------------------------
class MatchMakingId
{
public:

	// Please do not change the numbers on these items, the data is saved in the
	// database

	enum Bit
	{
		B_lookingForGroup = 0,
		B_helper = 1,
		B_rolePlay = 2,
		B_faction = 3,
		B_species = 4,
		B_title = 5,
		B_friend = 6,
		B_awayFromKeyBoard = 7,
		B_linkDead = 8,
		B_displayingFactionRank = 9,
		B_displayLocationInSearchResults = 10,
		B_outOfCharacter = 11, // Out of Character for Roleplayers
		B_searchableByCtsSourceGalaxy = 12,
		B_lookingForWork = 13,

		B_maxLoadedBits = 126, // The max bits that can be loaded from a file
		B_anonymous = 127
	};

	typedef std::vector<int> IntVector;

	MatchMakingId();

	void        setBit(Bit const bit);          // set to 1
	void        setBits(IntVector const &ints);

	void        toggleBit(Bit const bit);       // set to 1

	void        resetBit(Bit const bit);        // set to 0
	void        reset();                        // set all bits to 0

	bool        isBitSet(Bit const bit) const;  // is a bit 1

	int         getSetBitCount() const;         // number of bits set to 1
	int         getBitCount() const;

	IntVector   getInts() const;
	float       getMatchPercent(MatchMakingId const &id) const;

#ifdef _DEBUG
	std::string getDebugIntString() const;
#endif // _DEBUG

	void        unPackBitString(std::string const &value);
	void        packBitString(std::string &value) const;

	void        unPackIntString(std::string const &value);
	void        packIntString(std::string &value) const;

	bool           operator ==(MatchMakingId const &rhs) const;
	bool           operator !=(MatchMakingId const &rhs) const;
	MatchMakingId &operator  =(MatchMakingId const &rhs);

private:

#ifndef _DEBUG
	std::string getDebugIntString() const;
#endif // _DEBUG

	std::bitset<128> m_id;
};

//-----------------------------------------------------------------------------
namespace Archive
{
	void get(ReadIterator &source, MatchMakingId &target);
	void put(ByteStream &target, const MatchMakingId &source);
}

// ============================================================================

#endif // INCLUDED_MatchMakingId_H
