// =====================================================================
//
// BitStream.h
//
// ala diaz
//
// copyright 1999 Bootprint Entertainment
//
// =====================================================================

#ifndef BIT_STREAM_H
#define BIT_STREAM_H

// ======================================================================
// Base class for an i/o stream of bits.

class BitStream
{
private:

	BitStream(void);	
	BitStream(const BitStream &);
	BitStream &operator =(const BitStream &);

protected:

	static const uint32 INITIAL_MASK_VALUE;

	bool  isInput;
	byte  mask;
	byte  rack;

public:

	explicit BitStream(bool inputStream);
	virtual ~BitStream(void);

	virtual int    getOffset(void) const = 0;
	virtual void   outputBits(uint32 code, uint32 count) = 0;
	virtual void   outputRack(void) = 0;
	virtual	uint32 inputBits(uint32 count) = 0;
};

// ======================================================================

// A buffer that streams bits.  The buffer is NOT created by this class, 
// it must be created externally and the location and size passed to the
// constructor.

class BitBuffer : public BitStream
{
private:

	byte *base;
	byte *current;

	int  bufSize;

private:

	BitBuffer(void);
	BitBuffer(const BitBuffer &);
	BitBuffer &operator =(const BitBuffer &);

public:

	BitBuffer(void *buffer, int size, bool inputStream);
	virtual ~BitBuffer(void);

	virtual int    getOffset(void) const;
	virtual void   outputBits(uint32 code, uint32 count);
	virtual void   outputRack(void);
	virtual uint32 inputBits(uint32 count);
};

// ======================================================================

// A file that streams bits. Mainly tools should cause the compressors to 
// use BitFile.

class BitFile : public BitStream
{
private:

	HANDLE  hFile;
	char   *name;

private:

	BitFile(void);
	BitFile(const BitFile &);
	BitFile &operator =(const BitFile &);

public:

	BitFile(const char *fileName, bool inputStream);
	virtual ~BitFile(void);

	virtual int    getOffset(void) const;
	virtual void   outputBits(uint32 code, uint32 count);
	virtual void   outputRack(void);
	virtual uint32 inputBits(uint32 count);
};

// ======================================================================
// ======================================================================

// Base class for an i/o stream of bytes.

class ByteStream
{
private:

	ByteStream(void);
	ByteStream(const ByteStream &);
	ByteStream &operator =(const ByteStream &);

protected:
	bool isInput;

public:
	explicit ByteStream(bool inputStream);
	virtual ~ByteStream(void);

	virtual int  getOffset(void) const = 0;
	virtual void output(byte b) = 0;
	virtual bool input(byte *b) = 0;
};


// ======================================================================

// A buffer that streams bytes.  The buffer is NOT created by this class,
// it must be created externally and the location and size passed to the
// constructor.

class ByteBuffer : public ByteStream
{
private:

	byte  *base;
	byte  *current;
	int    bufSize;

private:

	ByteBuffer(void);
	ByteBuffer(const ByteBuffer &);
	ByteBuffer &operator =(const ByteBuffer &);

public:
	ByteBuffer(void *buffer, int size, bool inputStream);
	virtual ~ByteBuffer(void);

	virtual int  getOffset(void) const;
	virtual void output(byte b);
	virtual bool input(byte *b);
};

// ======================================================================

// A file that streams bytes.  Mainly tools should cause the compressors 
// to use ByteFile.

class ByteFile : public ByteStream
{
private:

	HANDLE hFile;
	char  *name;

private:

	ByteFile(void);
	ByteFile(const ByteFile &);
	ByteFile &operator =(const ByteFile &);

public:

	ByteFile(const char *fileName, bool inputStream);
	virtual ~ByteFile(void);

	virtual int  getOffset(void) const;
	virtual void output(byte b);
	virtual bool input(byte *b);
};

// ======================================================================

#endif  


