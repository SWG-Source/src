// ============================================================================
//
// MatchMakingId.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/MatchMakingId.h"

#include <bitset>
#include <vector>
#include "UnicodeUtils.h"

// ============================================================================
//
// Archive
//
// ============================================================================

//-----------------------------------------------------------------------------
void Archive::get(ReadIterator &source, MatchMakingId &target)
{
	// Get the packed vector

	MatchMakingId::IntVector ints;

	get(source, ints);
	
	// Set the id from the vector

	target.setBits(ints);

	//DEBUG_REPORT_LOG(true, ("MatchMakingId::Archive::get() <id> %s\n", target.getDebugIntString().c_str()));
}

//-----------------------------------------------------------------------------
void Archive::put(ByteStream &target, const MatchMakingId &source)
{
	//DEBUG_REPORT_LOG(true, ("MatchMakingId::Archive::put() <source> %s\n", source.getDebugIntString().c_str()));

	// Pack the vector

	MatchMakingId::IntVector ints(source.getInts());

	put(target, ints);
}

// ============================================================================
//
// MatchMakingId
//
// ============================================================================

//-----------------------------------------------------------------------------
MatchMakingId::MatchMakingId()
 : m_id()
{
	reset();
}

//-----------------------------------------------------------------------------
void MatchMakingId::resetBit(Bit const bit)
{
	DEBUG_FATAL((static_cast<int>(bit) >= static_cast<int>(m_id.size())), ("Out of range bit for match making id: %d", static_cast<int>(bit)));

	if (static_cast<int>(bit) < static_cast<int>(m_id.size()))
	{
		IGNORE_RETURN(m_id.reset(static_cast<unsigned int>(bit)));
	}
}

//-----------------------------------------------------------------------------
bool MatchMakingId::isBitSet(Bit const bit) const
{
	bool result = false;

	if (static_cast<int>(bit) < static_cast<int>(m_id.size()))
	{
		result = m_id.test(static_cast<unsigned int>(bit));
	}

	return result;
}

//-----------------------------------------------------------------------------
std::string MatchMakingId::getDebugIntString() const
{	
	std::string result;

	for (int bit = 0; bit < getBitCount(); ++bit)
	{
		if (isBitSet(static_cast<Bit>(bit)))
		{
			char text[34];
			_itoa(bit, text, 10);
			result += text;
			result += " ";
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
bool MatchMakingId::operator ==(MatchMakingId const &rhs) const
{
	//DEBUG_REPORT_LOG(true, ("MatchMakingId::operator ==() <m_id> %s <rhs> %s\n", getDebugIntString().c_str(), rhs.getDebugIntString().c_str()));

	return (m_id == rhs.m_id);
}

//-----------------------------------------------------------------------------
bool MatchMakingId::operator !=(MatchMakingId const &rhs) const
{
	return !(*this == rhs);
}

//-----------------------------------------------------------------------------
MatchMakingId &MatchMakingId::operator =(MatchMakingId const &rhs)
{
	//DEBUG_REPORT_LOG(true, ("MatchMakingId::operator ==() <old> %s <new> %s\n", getDebugIntString().c_str(), rhs.getDebugIntString().c_str()));

	if (this != &rhs)
	{
		m_id = rhs.m_id;
	}

	return *this;
}

//-----------------------------------------------------------------------------
void MatchMakingId::reset()
{
	IGNORE_RETURN(m_id.reset());
}

//-----------------------------------------------------------------------------
MatchMakingId::IntVector MatchMakingId::getInts() const
{
	MatchMakingId::IntVector ints;

	ints.push_back(0);

	{
		int counter = 0;

		for (int i = 0; i < static_cast<int>(m_id.size()); ++i)
		{
			if (counter >= 32)
			{
				counter = 0;
				ints.push_back(0);
			}

			if (isBitSet(static_cast<Bit>(i)))
			{
				int const intsIndex = (ints.size() == 0) ? 0 : (static_cast<int>(ints.size()) - 1);

				ints[static_cast<unsigned int>(intsIndex)] |= (0x00000001 << counter);
			}

			++counter;
		}
	}

	DEBUG_FATAL((ints.size() > 4), ("Too many ints: %d", ints.size()));

	return ints;
}

//-----------------------------------------------------------------------------
void MatchMakingId::setBits(IntVector const &ints)
{
	reset();

	int const intsSize = static_cast<int>(ints.size());
	int const bitCount = intsSize * 32;
	unsigned int intsIndex = 0;
	int counter = 0;

	for (int i = 0; i < bitCount; ++i)
	{
		if (counter >= 32)
		{
			counter = 0;
			++intsIndex;
		}

		int const value = ints[intsIndex];

		if (value & (0x00000001 << counter))
		{
			setBit(static_cast<Bit>(i));
		}

		++counter;
	}
}

//-----------------------------------------------------------------------------
void MatchMakingId::toggleBit(Bit const bit)
{
	if (isBitSet(bit))
	{
		resetBit(bit);
	}
	else
	{
		setBit(bit);
	}
}

//-----------------------------------------------------------------------------
int MatchMakingId::getBitCount() const
{
	return static_cast<int>(m_id.size());
}

//-----------------------------------------------------------------------------
void MatchMakingId::setBit(Bit const bit)
{
	DEBUG_FATAL((static_cast<int>(bit) >= static_cast<int>(m_id.size())), ("Out of range bit for match making id: %d", static_cast<int>(bit)));

	if (static_cast<int>(bit) < static_cast<int>(m_id.size()))
	{
		IGNORE_RETURN(m_id.set(static_cast<unsigned int>(bit)));
	}
}

//-----------------------------------------------------------------------------
float MatchMakingId::getMatchPercent(MatchMakingId const &id) const
{
	float result = 0.0f;
	int usedBits = 0;
	int matchBits = 0;
	bool const anonymous = id.isBitSet(B_anonymous);

	if (!anonymous)
	{
		for (int i = 0; i < static_cast<int>(B_maxLoadedBits); ++i)
		{
			if (isBitSet(static_cast<Bit>(i)))
			{
				++usedBits;

				if (id.isBitSet(static_cast<Bit>(i)))
				{
					++matchBits;
				}
			}
		}
	}

	if (usedBits > 0)
	{
		result = static_cast<float>(matchBits) / static_cast<float>(usedBits);
	}

	return result;
}

//-----------------------------------------------------------------------------
int MatchMakingId::getSetBitCount() const
{
	// This sucks. I should be able to call m_id.count(), but it does not compile
	// in both debug/release for bitsets.

	int result = 0;

	for (int i = 0; i < static_cast<int>(B_maxLoadedBits); ++i)
	{
		if (isBitSet(static_cast<Bit>(i)))
		{
			++result;
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
void MatchMakingId::unPackBitString(std::string const &value)
{
	if (value.empty())
	{
		reset();
	}
	else if ((static_cast<int>(value.size()) != getBitCount()))
	{
		// String size is invalid

		DEBUG_WARNING(true, ("Invalid MatchMakingId size during unpack (%d) expecting (%d)...aborting unpack", static_cast<int>(value.size()), getBitCount()));

		reset();
	}
	else
	{
		// String size looks good

		for (unsigned int i = 0; i < value.size(); ++i)
		{
			if (value[i] == '1')
			{
				setBit(static_cast<Bit>(i));
			}
		}
	}
}

//-----------------------------------------------------------------------------
void MatchMakingId::packBitString(std::string &value) const
{
	// Build a binary string representation of the match making id

	value.reserve(static_cast<unsigned int>(getBitCount()));
	value.clear();

	for (int i = 0; i < getBitCount(); ++i)
	{
		value.push_back(isBitSet(static_cast<Bit>(i)) ? '1' : '0');
	}
}

// Unpacks a series of ints
//-----------------------------------------------------------------------------
void MatchMakingId::unPackIntString(std::string const &value)
{
	Unicode::String unicodeValue(Unicode::narrowToWide(value));

	Unicode::UnicodeStringVector result;
	IGNORE_RETURN(Unicode::tokenize(unicodeValue, result));

	unsigned int resultSize = result.size();

	if (resultSize > 1)
	{
		unsigned int const intCount = Unicode::toInt(result[0]);

		if (intCount != static_cast<unsigned int>(getBitCount() / 32))
		{
			// Int count is wrong

			DEBUG_WARNING(true, ("Invalid MatchMakingId size during unpack (%d) expecting (%d)...aborting unpack", static_cast<int>(intCount), getBitCount() / 32));

			reset();
		}
		else
		{
			// Int count looks good

			IntVector ints;
			ints.reserve(intCount);

			for (unsigned int i = 1; i <= intCount; ++i)
			{
				ints.push_back(Unicode::toInt(result[i]));
			}

			setBits(ints);
		}
	}
}

// Packs all the bits into a series of ints to conserve string space
//-----------------------------------------------------------------------------
void MatchMakingId::packIntString(std::string &value) const
{
	IntVector ints(getInts());

	std::string result;

	{
		char text[256];

		// Put the size of the vector

		sprintf(text, "%d ", ints.size());
		result += text;

		// Put each int value from the vector

		int const intsSize = static_cast<int>(ints.size());

		for (int i = 0; i < intsSize; ++i)
		{
			int const value = ints[static_cast<unsigned int>(i)];

			sprintf(text, "%d", value);
			result += text;

			if (i < intsSize - 1)
			{
				result += " ";
			}
		}
	}

	value = result;
}

// ============================================================================
