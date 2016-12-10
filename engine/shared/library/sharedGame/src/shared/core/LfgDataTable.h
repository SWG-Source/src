// ======================================================================
//
// LfgDataTable.h
// Copyright 2006 Sony Online Entertainment LLC (SOE)
// All rights reserved.
//
// ======================================================================

#ifndef INCLUDED_LfgDataTable_H
#define INCLUDED_LfgDataTable_H

#include <vector>

class BitArray;
class LfgCharacterData;

// ======================================================================

class LfgDataTable // static class
{
public:
	static void install();
	static void remove();

	enum DefaultMatchConditionType
	{
		DMCT_NA,
		DMCT_Any,
		DMCT_AnyOnly,
		DMCT_All,
		DMCT_AllOnly
	};

	// pointer to a function that, given a LfgCharacterData,
	// determines if there is an lfg "match"; every LfgNode
	// corresponding to an "internal" lfg search criteria
	// will contain the corresponding "match" function;
	// the 5 void const * params are optional information
	// that can be passed in to the "match" function, as
	// some "match" function may require additional data
	// in order to perform the "matching" and those functions
	// will cast the void const * to whatever the function
	// is expecting to be in those params
	typedef bool (*pf) (LfgCharacterData const &, void const *, void const *, void const *, void const *, void const *);

	class LfgNode
	{
	public:
		LfgNode(std::string const & pName, bool pInternalAttribute, int pMinValueBeginSlotId, int pMinValueEndSlotId, int pMaxValueBeginSlotId, int pMaxValueEndSlotId, int pMinValue, int pMaxValue, DefaultMatchConditionType pDefaultMatchCondition, LfgNode const * pParent) :
		  name(pName), internalAttribute(pInternalAttribute), minValueBeginSlotId(pMinValueBeginSlotId), minValueEndSlotId(pMinValueEndSlotId), maxValueBeginSlotId(pMaxValueBeginSlotId), maxValueEndSlotId(pMaxValueEndSlotId), minValue(pMinValue), maxValue(pMaxValue), defaultMatchCondition(pDefaultMatchCondition), actualMatchCondition(DMCT_NA), parent(pParent), anyAllGroupingParent(nullptr), children(), hasAnyInternalAttributeLeafNodeDescendants(false), hasAnyExternalAttributeLeafNodeDescendants(false), internalAttributeMatchFunction(nullptr) {};

		std::string const name;
		bool const internalAttribute;
		int const minValueBeginSlotId;
		int const minValueEndSlotId;
		int const maxValueBeginSlotId;
		int const maxValueEndSlotId;
		int const minValue;
		int const maxValue;
		DefaultMatchConditionType const defaultMatchCondition;

		// for convenience, we will use this transient/temporary
		// member to store the *ACTUAL* any/all value,
		// if applicable, that the user specified for this node;
		// ***WARNING*** don't use this value willy nilly because
		// it just contains the transient/temporary value from
		// the last time it was used; remember, this member
		// is used to store a transient/temporary value
		DefaultMatchConditionType actualMatchCondition;

		LfgNode const * const parent;
		LfgNode const * anyAllGroupingParent;
		std::vector<LfgNode const *> children;
		bool hasAnyInternalAttributeLeafNodeDescendants;
		bool hasAnyExternalAttributeLeafNodeDescendants;

		LfgDataTable::pf internalAttributeMatchFunction;

		inline bool isLeafNode() const {return children.empty();}
		inline bool isMinMaxNode() const {return (minValue > 0 && maxValue > 0);}

	private:
		LfgNode();
		LfgNode(LfgNode const &);
		LfgNode &operator =(LfgNode const &);
	};

	static std::vector<LfgNode const *> const & getTopLevelLfgNodes();
	static std::map<std::string, LfgDataTable::LfgNode const *> const & getAllLfgLeafNodes();
	static LfgNode const * getLfgNodeByName(std::string const & lfgNodeName);
	static LfgNode const * getLfgLeafNodeByName(std::string const & lfgNodeName);
	
	static void setBit(std::string const & lfgNodeName, BitArray & ba);
	static void setBit(LfgNode const & lfgNode, BitArray & ba);

	static void clearBit(std::string const & lfgNodeName, BitArray & ba);
	static void clearBit(LfgNode const & lfgNode, BitArray & ba);

	static bool testBit(std::string const & lfgNodeName, BitArray const & ba);
	static bool testBit(LfgNode const & lfgNode, BitArray const & ba);

	static void setAnyOrAllBit(std::string const & lfgNodeName, bool any, BitArray & ba);
	static void setAnyOrAllBit(LfgNode const & lfgNode, bool any, BitArray & ba);

	static bool testAnyOrAllBit(std::string const & lfgNodeName, BitArray const & ba);
	static bool testAnyOrAllBit(LfgNode const & lfgNode, BitArray const & ba);

	static void setLowHighValue(std::string const & lfgNodeName, unsigned long lowValue, unsigned long highValue, BitArray & ba);
	static void setLowHighValue(LfgNode const & lfgNode, unsigned long lowValue, unsigned long highValue, BitArray & ba);

	static void getLowHighValue(std::string const & lfgNodeName, unsigned long & lowValue, unsigned long & highValue, BitArray const & ba);
	static void getLowHighValue(LfgNode const & lfgNode, unsigned long & lowValue, unsigned long & highValue, BitArray const & ba);

	static std::string getSetLfgNodeDebugString(BitArray const & ba, bool displayAnyOrAllValue = true);

private: // disabled

	LfgDataTable();
	LfgDataTable(LfgDataTable const &);
	LfgDataTable &operator =(LfgDataTable const &);
};

// ======================================================================

#endif // INCLUDED_LfgDataTable_H
