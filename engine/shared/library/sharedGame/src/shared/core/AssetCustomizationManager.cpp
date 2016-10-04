// ======================================================================
//
// AssetCustomizationManager.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/AssetCustomizationManager.h"

#include "sharedDebug/InstallTimer.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/CrcString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedMath/PaletteArgb.h"
#include "sharedMath/PaletteArgbList.h"
#include "sharedObject/BasicRangedIntCustomizationVariable.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/PaletteColorCustomizationVariable.h"

#include <stdlib.h>

// ======================================================================

#if defined(PLATFORM_WIN32)

#define PACKING_END_STRUCT

#elif defined(PLATFORM_LINUX)

#define PACKING_END_STRUCT __attribute__ ((packed))

#else

#error figure out how to handle structure packing for this platform.

#endif

// ======================================================================

namespace AssetCustomizationManagerNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifdef PLATFORM_WIN32
#pragma pack(push, 1)
#endif

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct IntRange
	{
		int32  minRangeInclusive;
		int32  maxRangeExclusive;
	} PACKING_END_STRUCT;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct VariableUsage
	{
		uint16  variableId;
		uint16  rangeId;
		uint16  defaultId;
	} PACKING_END_STRUCT;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct UsageIndexEntry
	{
		uint16  assetId;        //lint -esym(754, UsageIndexEntry::assetId) // error 754: (Info -- local structure member not referenced) // Wrong, accessed indirectly.
		uint16  listStartIndex;
		uint8   count;
	} PACKING_END_STRUCT;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct LinkIndexEntry
	{
		uint16  assetId;        //lint -esym(754, LinkIndexEntry::assetId) // error 754: (Info -- local structure member not referenced) // Wrong, accessed indirectly.
		uint16  listStartIndex;
		uint8   count;
	} PACKING_END_STRUCT;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct CrcLookupEntry
	{
		uint32  assetNameCrc;   //lint -esym(754, CrcLookupEntry::assetNameCrc) // error 754: (Info -- local structure member not referenced) // Wrong, accessed indirectly.
		uint16  assetId;
	} PACKING_END_STRUCT;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

#ifdef PLATFORM_WIN32
#pragma pack(pop)
#endif

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void                   remove();

	void                   load(Iff &iff);
	void                   load_0000(Iff &iff);

	char const            *getNameFromOffset(int offset);
	char const            *getPaletteNameFromPaletteId(int paletteId);
	char const            *getVariableNameFromVariableId(int variableId);
	int                    getDefaultValueFromDefaultId(int defaultId);
	void                   getIntRangeFromIntRangeId(int intRangeId, IntRange &intRange);
	void                   getRangeTypeInfoFromRangeId(int rangeId, bool &isPalette, int &idForRealType);
	VariableUsage const   *getVariableUsageFromId(int variableUsageId);

	UsageIndexEntry const *lookupVariableUsageIndexEntry(int assetId);
	LinkIndexEntry const  *lookupAssetLinkIndexEntry(int assetId);
	int                    lookupAssetId(CrcString const &assetName);

	void                   addVariablesForAssetAndLinks(int assetId, CustomizationData &customizationData, bool skipSharedOwnerVariables, int &addedVariableCount);

	int                    compare_uint16(void const *lhs, void const *rhs);
	int                    compare_uint32(void const *lhs, void const *rhs);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	Tag const   TAG_ACST = TAG(A, C, S, T);
	Tag const   TAG_CIDX = TAG(C, I, D, X);
	Tag const   TAG_DEFV = TAG(D, E, F, V);
	Tag const   TAG_IRNG = TAG(I, R, N, G);
	Tag const   TAG_LIDX = TAG(L, I, D, X);
	Tag const   TAG_LLST = TAG(L, L, S, T);
	Tag const   TAG_NAME = TAG(N, A, M, E);
	Tag const   TAG_PNOF = TAG(P, N, O, F);
	Tag const   TAG_RTYP = TAG(R, T, Y, P);
	Tag const   TAG_UCMP = TAG(U, C, M, P);
	Tag const   TAG_UIDX = TAG(U, I, D, X);
	Tag const   TAG_ULST = TAG(U, L, S, T);
	Tag const   TAG_VNOF = TAG(V, N, O, F);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool             s_installed;

	char            *s_nameDataBlock;
	int              s_nameDataBlockSize;

	uint16          *s_paletteIdNameOffsetMap;
	int              s_maxValidPaletteId;

	uint16          *s_variableIdNameOffsetMap;
	int              s_maxValidVariableId;

	int32           *s_defaultValueMap;
	int              s_maxValidDefaultId;

	IntRange        *s_intRangeMap;
	int              s_maxValidIntRangeId; // int range IDs map only into this int range table and are not the same thing as range IDs.

	uint16          *s_rangeTypeMap;
	int              s_maxValidRangeId;    // range IDs map into this range type map and include both palettized ranges and int-range ranges.

	VariableUsage   *s_variableUsageMap;
	int              s_maxValidVariableUsageId;

	uint16          *s_variableUsageList;
	int              s_variableUsageListEntryCount;

	UsageIndexEntry *s_usageIndex;
	int              s_usageIndexEntryCount;

	uint16          *s_linkList;
	int              s_linkListEntryCount;

	LinkIndexEntry  *s_linkIndex;
	int              s_linkIndexEntryCount;

	CrcLookupEntry  *s_crcLookupTable;
	int              s_crcLookupEntryCount;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}

using namespace AssetCustomizationManagerNamespace;

// ======================================================================
// namespace AssetCustomizationManagerNamespace
// ======================================================================

void AssetCustomizationManagerNamespace::remove()
{
	DEBUG_FATAL(!s_installed, ("AssetCustomizationManager not installed."));
	s_installed = false;

	delete[] s_nameDataBlock;
	s_nameDataBlock = nullptr;
	s_nameDataBlockSize = 0;

	delete[] s_paletteIdNameOffsetMap;
	s_paletteIdNameOffsetMap = nullptr;
	s_maxValidPaletteId = 0;

	delete[] s_variableIdNameOffsetMap;
	s_variableIdNameOffsetMap = nullptr;
	s_maxValidVariableId = 0;

	delete[] s_defaultValueMap;
	s_defaultValueMap = nullptr;
	s_maxValidDefaultId = 0;

	delete[] s_intRangeMap;
	s_intRangeMap = nullptr;
	s_maxValidIntRangeId = 0;

	delete[] s_rangeTypeMap;
	s_rangeTypeMap = nullptr;
	s_maxValidRangeId = 0;

	delete[] s_variableUsageMap;
	s_variableUsageMap = nullptr;
	s_maxValidVariableUsageId = 0;

	delete[] s_variableUsageList;
	s_variableUsageList = 0;
	s_variableUsageListEntryCount = 0;

	delete[] s_usageIndex;
	s_usageIndex = nullptr;
	s_usageIndexEntryCount = 0;

	delete[] s_linkList;
	s_linkList = nullptr;
	s_linkListEntryCount = 0;

	delete[] s_linkIndex;
	s_linkIndex = nullptr;
	s_linkIndexEntryCount = 0;

	delete[] s_crcLookupTable;
	s_crcLookupTable = nullptr;
	s_crcLookupEntryCount = 0;
}

// ----------------------------------------------------------------------

void AssetCustomizationManagerNamespace::load(Iff &iff)
{
	iff.enterForm(TAG_ACST);

	Tag const version = iff.getCurrentName();
	switch (version)
	{
	case TAG_0000:
		load_0000(iff);
		break;

	default:
	{
		char buffer[5];
		ConvertTagToString(version, buffer);
		FATAL(true, ("unsupported AssetCustomizationManager file version [%s].", buffer));
	}
	}

	iff.exitForm(TAG_ACST);
}

// ----------------------------------------------------------------------

void AssetCustomizationManagerNamespace::load_0000(Iff &iff)
{
	iff.enterForm(TAG_0000);

	//-- Read name data as one big block.
	iff.enterChunk(TAG_NAME);

	s_nameDataBlockSize = iff.getChunkLengthLeft();
	s_nameDataBlock = new char[static_cast<size_t>(s_nameDataBlockSize)];
	iff.read_char(s_nameDataBlockSize, s_nameDataBlock);

	iff.exitChunk(TAG_NAME);

	//-- Read palette id to palette name offset block.  Palette IDs start at 1.
	iff.enterChunk(TAG_PNOF);

	s_maxValidPaletteId = iff.getChunkLengthLeft(sizeof(uint16));
	s_paletteIdNameOffsetMap = new uint16[static_cast<size_t>(s_maxValidPaletteId)];
	iff.read_uint16(s_maxValidPaletteId, s_paletteIdNameOffsetMap);

	iff.exitChunk(TAG_PNOF);

	//-- Read variable id to variable name offset block.  Variable IDs start at 1.
	iff.enterChunk(TAG_VNOF);

	s_maxValidVariableId = iff.getChunkLengthLeft(sizeof(uint16));
	s_variableIdNameOffsetMap = new uint16[static_cast<size_t>(s_maxValidVariableId)];
	iff.read_uint16(s_maxValidVariableId, s_variableIdNameOffsetMap);

	iff.exitChunk(TAG_VNOF);

	//-- Read default value id.
	iff.enterChunk(TAG_DEFV);

	s_maxValidDefaultId = iff.getChunkLengthLeft(sizeof(int32));
	s_defaultValueMap = new int32[static_cast<size_t>(s_maxValidDefaultId)];
	iff.read_int32(s_maxValidDefaultId, s_defaultValueMap);

	iff.exitChunk(TAG_DEFV);

	//-- Read the integer range table.
	iff.enterChunk(TAG_IRNG);

	s_maxValidIntRangeId = iff.getChunkLengthLeft(sizeof(IntRange));
	s_intRangeMap = new IntRange[static_cast<size_t>(s_maxValidIntRangeId)];
	iff.read_int32(2 * s_maxValidIntRangeId, reinterpret_cast<int32*>(s_intRangeMap)); //lint !e740 // unusual pointer cast. // Yes; its fine and is worth the bug savings over handling directly as array of int32.

	iff.exitChunk(TAG_IRNG);

	//-- Read the range type table.
	iff.enterChunk(TAG_RTYP);

	s_maxValidRangeId = iff.getChunkLengthLeft(sizeof(uint16));
	s_rangeTypeMap = new uint16[static_cast<size_t>(s_maxValidRangeId)];
	iff.read_uint16(s_maxValidRangeId, s_rangeTypeMap);

	iff.exitChunk(TAG_RTYP);

	//-- Read the variable usage composition table.
	iff.enterChunk(TAG_UCMP);

	s_maxValidVariableUsageId = iff.getChunkLengthLeft(sizeof(VariableUsage));
	s_variableUsageMap = new VariableUsage[static_cast<size_t>(s_maxValidVariableUsageId)];
	iff.read_uint16(3 * s_maxValidVariableUsageId, reinterpret_cast<uint16*>(s_variableUsageMap));

	iff.exitChunk(TAG_UCMP);

	//-- Read variable usage list.
	iff.enterChunk(TAG_ULST);

	s_variableUsageListEntryCount = iff.getChunkLengthLeft(sizeof(uint16));
	s_variableUsageList = new uint16[static_cast<size_t>(s_variableUsageListEntryCount)];
	iff.read_uint16(s_variableUsageListEntryCount, s_variableUsageList);

	iff.exitChunk(TAG_ULST);

	//-- Read variable usage index.
	iff.enterChunk(TAG_UIDX);

	s_usageIndexEntryCount = iff.getChunkLengthLeft(sizeof(UsageIndexEntry));
	s_usageIndex = new UsageIndexEntry[static_cast<size_t>(s_usageIndexEntryCount)];
	iff.read_uint8(isizeof(UsageIndexEntry) * s_usageIndexEntryCount, reinterpret_cast<uint8*>(s_usageIndex));

	iff.exitChunk(TAG_UIDX);

	//-- Read asset linkage list.
	iff.enterChunk(TAG_LLST);

	s_linkListEntryCount = iff.getChunkLengthLeft(sizeof(uint16));
	s_linkList = new uint16[static_cast<size_t>(s_linkListEntryCount)];
	iff.read_uint16(s_linkListEntryCount, s_linkList);

	iff.exitChunk(TAG_LLST);

	//-- Read asset linkage index.
	iff.enterChunk(TAG_LIDX);

	s_linkIndexEntryCount = iff.getChunkLengthLeft(sizeof(LinkIndexEntry));
	s_linkIndex = new LinkIndexEntry[static_cast<size_t>(s_linkIndexEntryCount)];
	iff.read_uint8(isizeof(LinkIndexEntry) * s_linkIndexEntryCount, reinterpret_cast<uint8*>(s_linkIndex));

	iff.exitChunk(TAG_LIDX);

	//-- Read crc name -> asset id lookup table.
	iff.enterChunk(TAG_CIDX);

	s_crcLookupEntryCount = iff.getChunkLengthLeft(sizeof(CrcLookupEntry));
	s_crcLookupTable = new CrcLookupEntry[static_cast<size_t>(s_crcLookupEntryCount)];
	iff.read_uint8(isizeof(CrcLookupEntry) * s_crcLookupEntryCount, reinterpret_cast<uint8*>(s_crcLookupTable));

	iff.exitChunk(TAG_CIDX);

	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

char const *AssetCustomizationManagerNamespace::getNameFromOffset(int offset)
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, offset, s_nameDataBlockSize);
	return s_nameDataBlock + offset;
}

// ----------------------------------------------------------------------

char const *AssetCustomizationManagerNamespace::getPaletteNameFromPaletteId(int paletteId)
{
	VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(1, paletteId, s_maxValidPaletteId);

	//-- Get name offset from palette id.  Palette ids start at 1 and must be shifted down one
	//   index to lookup offset data.
	int const nameOffset = static_cast<int>(s_paletteIdNameOffsetMap[paletteId - 1]);
	return getNameFromOffset(nameOffset);
}

// ----------------------------------------------------------------------

char const *AssetCustomizationManagerNamespace::getVariableNameFromVariableId(int variableId)
{
	VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(1, variableId, s_maxValidVariableId);

	//-- Get name offset from variable id.  Variable ids start at 1 and must be shifted down one
	//   index to lookup offset data.
	int const nameOffset = static_cast<int>(s_variableIdNameOffsetMap[variableId - 1]);
	return getNameFromOffset(nameOffset);
}

// ----------------------------------------------------------------------

int AssetCustomizationManagerNamespace::getDefaultValueFromDefaultId(int defaultId)
{
	VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(1, defaultId, s_maxValidDefaultId);

	//-- Get default value from default id.  Default ids start at 1 and must be shifted down one
	//   index to lookup value data.
	return static_cast<int>(s_defaultValueMap[defaultId - 1]);
}

// ----------------------------------------------------------------------

void AssetCustomizationManagerNamespace::getIntRangeFromIntRangeId(int intRangeId, IntRange &intRange)
{
	VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(1, intRangeId, s_maxValidIntRangeId);

	//-- Get int range from id.  Int range ids start at 1 and need to be shifted down one
	//   index to lookup int range data.
	intRange = s_intRangeMap[intRangeId - 1];
}

// ----------------------------------------------------------------------

void AssetCustomizationManagerNamespace::getRangeTypeInfoFromRangeId(int rangeId, bool &isPalette, int &idForRealType)
{
	VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(1, rangeId, s_maxValidRangeId);

	//-- Get range type from range id.  Range ids start at 1 and need to be shifted down one
	//   index to lookup int range type info.
	uint16 const rangeType = s_rangeTypeMap[rangeId - 1];
	isPalette = ((rangeType & 0x8000) != 0);
	idForRealType = (rangeType & 0x7fff);
}

// ----------------------------------------------------------------------

VariableUsage const *AssetCustomizationManagerNamespace::getVariableUsageFromId(int variableUsageId)
{
	VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(1, variableUsageId, s_maxValidVariableUsageId);

	//-- Get variable usage from id.  Variable usage ids start at 1 and need to be shifted down one
	//   when accessing table data.
	return s_variableUsageMap + (variableUsageId - 1);
}

// ----------------------------------------------------------------------

AssetCustomizationManagerNamespace::UsageIndexEntry const *AssetCustomizationManagerNamespace::lookupVariableUsageIndexEntry(int assetId)
{
	VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(1, assetId, s_crcLookupEntryCount);

	//-- Do a binary search on the UsageIndexEntry list.
	uint16 const key = static_cast<uint16>(assetId);
	return static_cast<UsageIndexEntry*>(bsearch(&key, s_usageIndex, static_cast<size_t>(s_usageIndexEntryCount), sizeof(UsageIndexEntry), compare_uint16));
}

// ----------------------------------------------------------------------

AssetCustomizationManagerNamespace::LinkIndexEntry const *AssetCustomizationManagerNamespace::lookupAssetLinkIndexEntry(int assetId)
{
	VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(1, assetId, s_crcLookupEntryCount);

	//-- Do a binary search on the UsageIndexEntry list.
	uint16 const key = static_cast<uint16>(assetId);
	return static_cast<LinkIndexEntry*>(bsearch(&key, s_linkIndex, static_cast<size_t>(s_linkIndexEntryCount), sizeof(LinkIndexEntry), compare_uint16));
}

// ----------------------------------------------------------------------

int AssetCustomizationManagerNamespace::lookupAssetId(CrcString const &assetName)
{
	uint32 const key = assetName.getCrc();
	CrcLookupEntry const *entry = static_cast<CrcLookupEntry*>(bsearch(&key, s_crcLookupTable, static_cast<size_t>(s_crcLookupEntryCount), sizeof(CrcLookupEntry), compare_uint32));

	return (entry != nullptr) ? entry->assetId : 0;
}

// ----------------------------------------------------------------------

int AssetCustomizationManagerNamespace::compare_uint16(void const *lhs, void const *rhs)
{
	NOT_NULL(lhs);
	NOT_NULL(rhs);

	// return:
	//   negative if rhs > lhs,
	//   0        if rhs == lhs,
	//   positive if rhs < lhs
	return static_cast<int>(*static_cast<uint16 const*>(lhs)) - static_cast<int>(*static_cast<uint16 const*>(rhs));
}

// ----------------------------------------------------------------------

int AssetCustomizationManagerNamespace::compare_uint32(void const *lhs, void const *rhs)
{
	NOT_NULL(lhs);
	NOT_NULL(rhs);

	// return:
	//   negative if rhs > lhs,
	//   0        if rhs == lhs,
	//   positive if rhs < lhs
  //
	// Don't want to do this: I don't want to worry about how overflow influences this.
	// return static_cast<int>(*static_cast<uint16 const*>(lhs)) - static_cast<int>(*static_cast<uint16 const*>(rhs));

	uint32 const a = *static_cast<uint32 const*>(lhs);
	uint32 const b = *static_cast<uint32 const*>(rhs);

	if (a < b)
		return -1;
	else if (a > b)
		return 1;
	else
		return 0;
}

// ----------------------------------------------------------------------

void AssetCustomizationManagerNamespace::addVariablesForAssetAndLinks(int assetId, CustomizationData &customizationData, bool skipSharedOwnerVariables, int &addedVariableCount)
{
	//-- Find variables used directly by specified asset.
	UsageIndexEntry const *usageIndexEntry = lookupVariableUsageIndexEntry(assetId);
	if (usageIndexEntry)
	{
		// This variable uses customization variables.  Process each one.
		int const startIndex = static_cast<int>(usageIndexEntry->listStartIndex);
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, startIndex, s_variableUsageListEntryCount);

		int const endIndex = startIndex + static_cast<int>(usageIndexEntry->count);
		VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(0, endIndex, s_variableUsageListEntryCount);

		for (int i = startIndex; i < endIndex; ++i)
		{
			//-- Get variable usage info.
			uint16 const variableUsageId = s_variableUsageList[i];

			VariableUsage const *variableUsage = getVariableUsageFromId(variableUsageId);
			NOT_NULL(variableUsage);

			//-- Get variable name and check if we should process further.
			char const *variablePathName = getVariableNameFromVariableId(variableUsage->variableId);
			NOT_NULL(variablePathName);

			if (skipSharedOwnerVariables && (strncmp(variablePathName, "/shared_owner/", 14) == 0))
			{
				// Skip this variable: it is a shared_owner variable and we've been directed to skip them.
				continue;
			}

			// We're going to create and add this variable if it doesn't already exist.

			//-- Check if variable already exists.  If so, we skip this and log.
			std::string const variablePathName2(variablePathName);
			if (customizationData.findVariable(variablePathName2))
			{
				// @todo: add appropriate DEBUG_REPORT_LOG
				// Skip this variable: another already exists with the same name.
				continue;
			}

			//-- Get default value.
			int const defaultValue = getDefaultValueFromDefaultId(variableUsage->defaultId);

			//-- Get variable type.
			bool isPalette = false;
			int  idForRealType = 0;
			getRangeTypeInfoFromRangeId(variableUsage->rangeId, isPalette, idForRealType);

			//-- Create variable based on type.
			CustomizationVariable *variable = nullptr;
			if (isPalette)
			{
				//-- Get palette.
				char const *const        paletteName = getPaletteNameFromPaletteId(idForRealType);
				PaletteArgb const *const palette = PaletteArgbList::fetch(TemporaryCrcString(paletteName, true));
				if (!palette)
				{
					DEBUG_WARNING(true, ("failed to load palette [%s], skipping variable [%s]", paletteName, variablePathName));
					continue;
				}

				variable = new PaletteColorCustomizationVariable(palette, defaultValue);

				//-- Release local reference.
				palette->release();
			}
			else
			{
				IntRange  intRange;
				getIntRangeFromIntRangeId(idForRealType, intRange);

				variable = new BasicRangedIntCustomizationVariable(intRange.minRangeInclusive, defaultValue, intRange.maxRangeExclusive);
			}

			//-- Add the variable to the CustomizationData.
			customizationData.addVariableTakeOwnership(variablePathName2, variable);

			++addedVariableCount;
		}
	}

	//-- Recursively call on assets that this asset is dependent upon.  These assets
	//   could be providing additional variables.
	LinkIndexEntry const *linkIndexEntry = lookupAssetLinkIndexEntry(assetId);
	if (linkIndexEntry)
	{
		// This variable uses customization variables.  Process each one.
		int const startIndex = static_cast<int>(linkIndexEntry->listStartIndex);
		VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, startIndex, s_linkListEntryCount);

		int const endIndex = startIndex + static_cast<int>(linkIndexEntry->count);
		VALIDATE_RANGE_INCLUSIVE_INCLUSIVE(0, endIndex, s_linkListEntryCount);

		for (int i = startIndex; i < endIndex; ++i)
		{
			//-- Get asset id for the linked asset, call this function recursively on it.
			uint16 const dependencyAssetId = s_linkList[i];
			addVariablesForAssetAndLinks(dependencyAssetId, customizationData, skipSharedOwnerVariables, addedVariableCount);
		}
	}
}

// ======================================================================
// class AssetCustomizationManager: PUBLIC STATIC
// ======================================================================

void AssetCustomizationManager::install(char const *filename)
{
	InstallTimer const installTimer("AssetCustomizationManager::install");

	DEBUG_FATAL(s_installed, ("AssetCustomizationManager already installed."));
	DEBUG_FATAL(!filename || !*filename, ("AssetCustomizationManager requires a valid filename for successful installation."));

	//-- Check endian-ness of platform.  This code assumes little-endian due to the way
	//   the data image is loaded directly into memory.  If we ever hit this,
	//   we can do a conversion at load time to big-endian.
	uint32 testValue = 1;
	FATAL(*reinterpret_cast<uint8*>(&testValue) != 1, ("AssetCustomizationManager: running on a non-little-endian architecture, unsupported by this class at this time."));

	Iff iff;
	bool const openResult = iff.open(filename, true);
	FATAL(!openResult, ("AssetCustomizationManager data file [%s] does not exist or failed to open.  This is likely a configuration file issue.", filename));

	load(iff);

	s_installed = true;
	ExitChain::add(remove, "AssetCustomizationManager");
}

// ----------------------------------------------------------------------

int AssetCustomizationManager::addCustomizationVariablesForAsset(CrcString const &assetName, CustomizationData &customizationData, bool skipSharedOwnerVariables)
{
	DEBUG_FATAL(!s_installed, ("AssetCustomizationManager not installed."));

	//-- Convert asset name to internal asset id.
	int const assetId = lookupAssetId(assetName);
	if (!assetId)
	{
		// Exit: there are no customization variables for this asset or its dependencies.
		return 0;
	}

	//-- Recursively add customization variables used directly by an asset and then check any child assets used by the asset.
	int addedVariableCount = 0;
	addVariablesForAssetAndLinks(assetId, customizationData, skipSharedOwnerVariables, addedVariableCount);

	return addedVariableCount;
}

// ======================================================================