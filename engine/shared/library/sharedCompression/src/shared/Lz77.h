// =====================================================================
//
// Lz77.h
//
// Portions copyright 1999 Bootprint Entertainment
// Portions copyright 2002 Sony Online Entertainment
// All Rights Reserved.
//
// =====================================================================

#ifndef INCLUDED_Lz77_H
#define INCLUDED_Lz77_H

// =====================================================================

class ByteStream;
class BitStream;

#include "sharedCompression/Compressor.h"

// =====================================================================

class Lz77 : public Compressor
{
public:

	explicit Lz77(uint32 _indexBitCount = 12, uint32 _lengthBitCount = 4);
	virtual ~Lz77();

	virtual int  compress(const void *inputBuffer, int inputSize, void *outputBuffer, int outputSize);
	virtual int  expand  (const void *inputBuffer, int inputSize, void *outputBuffer, int outputSize);

	virtual void compress(const char *inputFile, const char *outputFile);
	virtual void expand  (const char *inputFile, const char *outputFile);

protected:

	void compress(ByteStream &input, BitStream &output);
	void expand(BitStream &input, ByteStream & output);

private:

	struct Node
	{
		uint32 parent;
		uint32 smallChild;
		uint32 largeChild;
	};

private:

	uint32  modWindow(uint32 a);
	void    initTree(uint32 r);
	void    initWindow();
	void    contractNode(uint32 oldNode, uint32 newNode);
	void    replaceNode(uint32 oldNode, uint32 newNode);
	uint32  findNextNode(uint32 node);
	void    deleteString(uint32 p);
	uint32  addString(uint32 newNode, uint32 *matchPosition);

private:

	// Cost in bits to store 1 byte
	static const uint32 byteEncodingCost;

	// Symbol signifying the end of the data stream
	static const uint32 endOfStream;

	// Index to an always-unused node in the tree
	static const uint32 unused;

private:

	// # of bits allocated to indices into the text window
	const uint32 indexBitCount;

	// # of bits allocated for the length of an encode phrase
	const uint32 lengthBitCount;

	// size of the text window
	const uint32 windowSize;

	// pre-adjusted size of look ahead buffer
	const uint32 rawLookAheadSize;

	// threshold where phrase encoding produces compression over storing
	const uint32 breakEven;

	// size of the look-ahead window
	const uint32 lookAheadSize;

	// permanent root of the tree belonging to no string
	const uint32 treeRoot;

private:

	byte *window;
	Node *tree;

private:

	Lz77(const Lz77&);
	Lz77 &operator =(const Lz77&);
};

// =====================================================================
/**
 * Protect window indices from overflowing and wrap them.
 * 
 * @return The index after bounds wrapping.
 */

inline uint32 Lz77::modWindow(uint32 a)
{
	return ((a) & (windowSize - 1));
}

// =====================================================================

#endif
