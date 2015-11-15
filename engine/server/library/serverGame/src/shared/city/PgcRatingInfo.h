// ======================================================================
//
// PgcRatingInfo.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef _PgcRatingInfo_H_
#define _PgcRatingInfo_H_

// ======================================================================

class PgcRatingInfo
{
public:
	static const size_t ms_PgcRatingDataSize = 20;

	PgcRatingInfo() : m_chroniclerName(), m_ratingCount(0), m_ratingTotal(0), m_lastRatingTime(0), m_data(ms_PgcRatingDataSize, 0) {};

	bool operator== (const PgcRatingInfo& rhs) const;
	bool operator!= (const PgcRatingInfo& rhs) const;

public:
	std::string m_chroniclerName;
	int64 m_ratingCount;
	int64 m_ratingTotal;
	int m_lastRatingTime;
	std::vector<int> m_data;
};

// ======================================================================

namespace Archive
{
	class ReadIterator;
	class ByteStream;

	void get(ReadIterator & source, PgcRatingInfo & target);
	void put(ByteStream & target, const PgcRatingInfo & source);
}

// ======================================================================

#endif // _PgcRatingInfo_H_
