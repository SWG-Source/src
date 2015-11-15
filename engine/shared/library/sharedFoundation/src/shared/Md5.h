// ======================================================================
//
// Md5.h
//
// copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_Md5_H
#define INCLUDED_Md5_H

// ======================================================================

class Md5
{
public:

	class Value
	{
		friend class Md5;

	public:

		enum
		{
			cms_dataSize = 16
		};

	public:

		Value();
		Value(Value const &rhs);

		bool operator ==(Value const &rhs) const;
		bool operator !=(Value const &rhs) const;
		void format(char *buffer, int bufferLength) const;

		void *getData();

	private:

		uint8 m_data[cms_dataSize];
	};

public:

	static Value calculate(const char *string);
	static Value calculate(const void *data, int len);

public:

	Md5();

	void  reset();	
	void  update(void const *data, int len);
	void  finish();
	Value getValue() const;

private:

	static void   wordsToBytes(const uint32* words, size_t wordCnt, uint8* bytes);
	static void   bytesToWords(const uint8* bytes, size_t byteCnt, uint32* words);
	static uint32 rotate_left(uint32 ival, uint32 shft);
	static uint32 round1(uint32 i, uint32 j, uint32 k, uint32 l, uint32 i1, uint32 j1, uint32 k1);
	static uint32 round2(uint32 i, uint32 j, uint32 k, uint32 l, uint32 i1, uint32 j1, uint32 k1);
	static uint32 round3(uint32 i, uint32 j, uint32 k, uint32 l, uint32 i1, uint32 j1, uint32 k1);
	static uint32 round4(uint32 i, uint32 j, uint32 k, uint32 l, uint32 i1, uint32 j1, uint32 k1);

	void	        transformBlock(void const *data);

private:

	enum
	{
		BUFFER_SIZE = 64
	};

	uint32 m_data[4];
	byte   m_buffer[BUFFER_SIZE];
	int    m_bufferedBytes;
	int    m_totalLength;
	bool   m_finished;

public:

	Md5(Md5 const &);
	Md5 & operator =(Md5 const &);
};

// ======================================================================

inline void * Md5::Value::getData()
{
	return m_data;
}

// ======================================================================

#endif
