// ======================================================================
//
// CrcString.h
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_CrcString_H
#define INCLUDED_CrcString_H

// ======================================================================

class CrcString
{
public:

	static void normalize(char *output, const char *input);

public:

	virtual ~CrcString();

	bool                 isEmpty() const;	
	uint32               getCrc() const;	

	bool           operator ==(CrcString const & rhs) const;
	bool           operator !=(CrcString const & rhs) const;
	bool DLLEXPORT operator < (CrcString const & rhs) const;
	bool           operator <=(CrcString const & rhs) const;
	bool           operator > (CrcString const & rhs) const;
	bool           operator >=(CrcString const & rhs) const;

	virtual char const * getString() const = 0;

	virtual void         clear() = 0;
	virtual void         set(char const * string, bool applyNormalize) = 0;
	virtual void         set(char const * string, uint32 crc) = 0;
	void                 set(CrcString const & rhs);

protected:

	CrcString();
	CrcString(uint32 crc);
	void calculateCrc();

private:

	CrcString &    operator =(CrcString const &);
	CrcString(CrcString const &);

protected:

	uint32      m_crc;
};

// ======================================================================

inline uint32 CrcString::getCrc() const
{
	return m_crc;
}

// ======================================================================

#endif
