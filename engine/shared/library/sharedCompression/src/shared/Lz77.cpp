// =====================================================================
//
// Lz77.cpp
//
// ala diaz
//
// adapted from the LZSS algorithm in The Data Compression Book (Nelson & Gailly)
//
// Portions copyright 1999 Bootprint Entertainment
// Portions copyright 2001-2002 Sony Online Entertainment
// All Rights Reserved.
//
// =====================================================================

#include "sharedCompression/FirstSharedCompression.h"
#include "sharedCompression/Lz77.h"

#include "sharedCompression/BitStream.h"

// =====================================================================

// This is the LZSS module, which implements an Lz77 style compression
// algorithm.  As iplemented here the default uses a 12 bit index into the
// sliding window, and a 4 bit length, which is adjusted to reflect phrase
// lengths of between 2 and 17 bytes.

// =====================================================================

const uint32 Lz77::byteEncodingCost = 9;
const uint32 Lz77::endOfStream      = 0;
const uint32 Lz77::unused           = 0;

// =====================================================================
// Construct a Lz77 compressor
//
// Remarks:
//
//   Initializes all components of the compressor.

Lz77::Lz77(uint32 _indexBitCount, uint32 _lengthBitCount)
: Compressor(),
	indexBitCount(_indexBitCount),
	lengthBitCount(_lengthBitCount),
	windowSize(1U << indexBitCount),
	rawLookAheadSize(1U << lengthBitCount),
	breakEven( (1 + indexBitCount + lengthBitCount) / byteEncodingCost ),
	lookAheadSize(rawLookAheadSize + breakEven),
	treeRoot(windowSize),
	window(new byte[windowSize]),
	tree(nullptr)
{
}

// ----------------------------------------------------------------------
/**
 * Destroy a Lz77 compressor.
 * 
 * Frees memory used by compressor.
 */

Lz77::~Lz77(void)
{
	delete [] window;
	delete [] tree;
}

// ----------------------------------------------------------------------
/**
 * Compress from buffer to buffer.
 * 
 * Creates the buffer i/o streams and compresses.
 */

int Lz77::compress(const void *inputBuffer, int inputSize, void *outputBuffer, int outputSize)
{
	ByteBuffer in(const_cast<void *>(inputBuffer), inputSize, true);
	BitBuffer  out(outputBuffer, outputSize, false);

	compress(in, out);
	return out.getOffset();
}

// ----------------------------------------------------------------------
/**
 * Compress from file to file.
 * 
 * Creates the file i/o streams and compresses.
 */

void Lz77::compress(const char *inputName, const char *outputName)
{
	ByteFile in(inputName, true);
	BitFile  out(outputName, false);

	compress(in, out);
}

// ----------------------------------------------------------------------
/**
 * Expand from buffer to buffer.
 * 
 * Creates the buffer i/o streams and expands.
 */

int Lz77::expand(const void *inputBuffer, int inputSize, void *outputBuffer, int outputSize)
{
	BitBuffer  in(const_cast<void *>(inputBuffer), inputSize, true);
	ByteBuffer out(outputBuffer, outputSize, false);

	expand(in, out);
	return out.getOffset();
}

// ----------------------------------------------------------------------
/**
 * Expand from file to file.
 * 
 * Creates the file i/o streams and expands.
 */

void Lz77::expand(const char *inputName, const char *outputName)
{
	BitFile  in(inputName, true);
	ByteFile out(outputName, false);

	expand(in, out);
}

// ----------------------------------------------------------------------
/**
 * Initialize the tree.
 * 
 * Reset all nodes of the tree and set node r as the child of the root.
 * The tree structure contains the binary tree of all the strings in
 * the window sorted in order.
 */

void Lz77::initTree(uint32 r)
{
	memset(tree, 0, static_cast<int>(sizeof(Node) * (windowSize + 1)));

	tree[treeRoot].largeChild = r;
	tree[r].parent = treeRoot;
	tree[r].largeChild = unused;
	tree[r].smallChild = unused;
}

// ----------------------------------------------------------------------
/**
 * Initialize the text window.
 * 
 * Zeroes out the text window.
 */

void Lz77::initWindow()
{
	memset(window, 0, static_cast<int>(windowSize));
}

// ----------------------------------------------------------------------
/**
 * Contract the tree.
 * 
 * This routine is used when a node is being deleted.  The link to
 * oldNode is broken by pulling the newNode in to overlay the existing
 * link.
 */

void Lz77::contractNode(uint32 oldNode, uint32 newNode)
{
	tree[newNode].parent = tree[oldNode].parent;

	if (tree[tree[oldNode].parent].largeChild == oldNode)
		tree[tree[oldNode].parent].largeChild = newNode;
	else
		tree[tree[oldNode].parent].smallChild = newNode;

	tree[oldNode].parent = unused;
}

// ----------------------------------------------------------------------
/**
 * Replace a node in the tree with another.
 * 
 * This routine is used when a node is being replaced by a node not
 * previously in the tree.
 */

void Lz77::replaceNode(uint32 oldNode, uint32 newNode)
{
	uint32 parent;

	parent = tree[oldNode].parent;

	if (tree[parent].smallChild == oldNode)
		tree[parent].smallChild = newNode;
	else
		tree[parent].largeChild = newNode;

	tree[newNode] = tree[oldNode];
	tree[tree[newNode].smallChild].parent = newNode;
	tree[tree[newNode].largeChild].parent = newNode;
	tree[oldNode].parent = unused;
}

// ----------------------------------------------------------------------
/**
 * Find the next smallest node.
 * 
 * This routine is used to find the next smallest node after the node
 * argument.  It assumes that the node has a smaller child.  We find
 * the next smallest child by going to the smallChild node, then
 * going to the end of the largeChild descendant chain.
 */

uint32 Lz77::findNextNode(uint32 node)
{
	uint32 next;

	next = tree[node].smallChild;

	while (tree[next].largeChild != unused)
	{
		next = tree[next].largeChild;
	}

	return(next);
}

// ----------------------------------------------------------------------
/**
 * Delete a string from the tree.
 * 
 * This routine performs the classic binary tree deletion algorithm.
 * If the node to be deleted has a nullptr link in either direction, we
 * just pull the non-nullptr link up one to replace the existing link.
 * If both links exist, we instead delete the next link in order, which
 * is guaranteed to have a nullptr link, then replace the node to be deleted
 * with the next link.
 */

void Lz77::deleteString(uint32 p)
{
	uint32  replacement;

	if (tree[p].parent == unused)
		return;

	if ( tree[p].largeChild == unused)
	{
		contractNode(p, tree[p].smallChild);
	}
	else
		if (tree[p].smallChild == unused)
		{
			contractNode(p, tree[p].largeChild);
		}
		else
		{
			replacement = findNextNode(p);
			deleteString(replacement);
			replaceNode(p, replacement);
		}
}

// ----------------------------------------------------------------------
/**
 * Add a string to the tree.
 * 
 * This where most of the work done by the encoder takes place.  This
 * routine is responsible for adding the new node to the binary tree.
 * It also has to find the best match among all the existing nodes in
 * the tree, and return that to the calling routine.  To make matters
 * even more complicated, if the newNode has a duplicate in the tree,
 * the oldNode is deleted, for reasons of efficiency.
 * 
 * @return The length of the best match in the existing tree.  The node where
 * the best match begins if returned in matchPosition.
 */

uint32 Lz77::addString(uint32 newNode, uint32 *matchPosition)
{
	uint32  i;
	uint32  testNode;
	uint32  matchLength;
	uint32 *child;
	int     delta = 0;

	DEBUG_FATAL(!matchPosition, ("matchPosition is nullptr"));

	if (newNode == endOfStream)
		return 0;

	testNode = tree[treeRoot].largeChild;
	matchLength = 0;

	for (;;)
	{
		for (i = 0 ; i < lookAheadSize ; i++)
		{
			delta = window[modWindow(newNode + i)] -  window[modWindow(testNode + i)];

			if (delta != 0)
				break;
		}

		if (i >= matchLength)
		{
			matchLength = i;
			*matchPosition = testNode;

			if (matchLength >= lookAheadSize)
			{
				replaceNode(testNode, newNode);
				return(matchLength);
			}
		}

		if (delta >= 0)
			child = &tree[testNode].largeChild;
		else
			child = &tree[testNode].smallChild;

		if (*child == unused)
		{
			*child = newNode;
			tree[newNode].parent = testNode;
			tree[newNode].largeChild = unused;
			tree[newNode].smallChild = unused;
			return(matchLength);
		}

		testNode = *child;
	}
}

// ----------------------------------------------------------------------
/**
 * Compress a stream.
 * 
 * This is the compression routine.  It has to first load up the look
 * ahead buffer, then go into the main compression loop.  The main loop
 * decides whether to output a single character or an index/length
 * token that defines a phrase.  Once the character or phrase has been
 * sent out, another loop has to run.  The second loop reads in new
 * characters, deletes the strings that are overwritten by the new
 * character, then adds the strings that are created by the new
 * character.
 */

void Lz77::compress(ByteStream &in, BitStream &out)
{
	uint32 i;
	uint32 lookAheadBytes;
	uint32 curPosition = 1;
	uint32 replaceCount;
	uint32 matchLength = 0;
	uint32 matchPosition = 0;
	byte   c;
	bool   eos;

	if (!tree)
		tree = new Node[windowSize + 1];

	initWindow();
	initTree(curPosition);

	for (i = 0; i < lookAheadSize; i++)
	{
		eos = in.input(&c);

		if (eos)
		{
			break;
		}

		window[curPosition + i] = c;
	}

	lookAheadBytes = i;

	while (lookAheadBytes > 0)
	{
		if (matchLength > lookAheadBytes)
		{
			matchLength = lookAheadBytes;
		}

		if (matchLength <= breakEven)
		{
			replaceCount = 1;
			out.outputBits(1,1);
			out.outputBits(window[curPosition],8);
		}
		else
		{
			out.outputBits(0,1);
			out.outputBits(matchPosition, indexBitCount);
			out.outputBits(matchLength - (breakEven + 1), lengthBitCount);
			replaceCount = matchLength;
		}

		for (i = 0; i < replaceCount; i++)
		{
			deleteString(modWindow(curPosition + lookAheadSize));

			eos = in.input(&c);

			if (eos)
			{
				lookAheadBytes--;
			}
			else
			{
				window[modWindow(curPosition + lookAheadSize)] = c;
			}

			curPosition = modWindow(curPosition + 1);

			if (lookAheadBytes)
			{
				matchLength = addString(curPosition, &matchPosition);
			}
		}
	}

	out.outputBits(0,1);
	out.outputBits(endOfStream, indexBitCount);
	out.outputRack();
}

// ----------------------------------------------------------------------
/**
 * Expand a stream.
 * 
 * This is the expansion routine for the LZSS algorithm.  All it has
 * to do is read in flag bits, decide whether to read in a character or
 * a index/length pair, and take the appropriate action.
 */

void Lz77::expand(BitStream &in, ByteStream &out)
{
	uint32 i;
	byte   c;
	uint32 matchLength;
	uint32 matchPosition;
	uint32 curPosition = 1;

	initWindow();

	for(;;)
	{
		if (in.inputBits(1))
		{
			c = static_cast<byte>(in.inputBits(8));
			out.output(c);
			window[curPosition] = c;
			curPosition = modWindow(curPosition + 1);
		}
		else
		{
			matchPosition = in.inputBits(indexBitCount);

			if (matchPosition == endOfStream)
			{
				break;
			}

			matchLength = in.inputBits(lengthBitCount);
			matchLength += breakEven;

			for (i = 0; i <= matchLength; i++)
			{
				c = window[modWindow(matchPosition + i)];
				out.output(c);
				window[curPosition] = c;
				curPosition = modWindow(curPosition + 1);
			}
		}
	}
}

// =====================================================================
