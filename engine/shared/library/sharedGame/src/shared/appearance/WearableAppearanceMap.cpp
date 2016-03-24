// ======================================================================
//
// WearableAppearanceMap.cpp
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/WearableAppearanceMap.h"

#include "sharedDebug/InstallTimer.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"

#include <algorithm>
#include <string>

// ======================================================================

namespace WearableAppearanceMapNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class MapEntry
	{
	public:

		virtual ~MapEntry();

		// Key fields.
		virtual CrcString const &getSourceWearableAppearanceName() const = 0;  //lint -esym(754, *::getSourceWearableAppearanceName) // unreferenced // wrong, referenced through virtual interface.
		virtual CrcString const &getWearerAppearanceName() const = 0;          //lint -esym(754, *::getWearerAppearanceName)         // unreferenced // wrong, referenced through virtual interface.
		
		// Payload field.
		virtual CrcString const *getMappedWearableAppearanceName() const = 0;  //lint -esym(754, *::getMappedWearableAppearanceName) // unreferenced // wrong, referenced through virtual interface.
		
	protected:

		MapEntry();

	private:

		// Disabled.
		MapEntry &operator =(MapEntry const &);

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
		
	class PersistentMapEntry: public MapEntry
	{
	public:

		PersistentMapEntry(char const *sourceWearableAppearanceName, char const *wearerAppearanceName, char const *mappedWearableAppearanceName);
		virtual ~PersistentMapEntry();

		virtual CrcString const &getSourceWearableAppearanceName() const;
		virtual CrcString const &getWearerAppearanceName() const;
		virtual CrcString const *getMappedWearableAppearanceName() const;

	private:

		// Disabled.
		PersistentMapEntry();
		PersistentMapEntry(PersistentMapEntry const&);               //lint -esym(754, PersistentMapEntry::PersistentMapEntry) // local member not accessed // defensive hiding.
		PersistentMapEntry &operator =(PersistentMapEntry const &);

	private:

		PersistentCrcString const  m_sourceWearableAppearanceName;
		PersistentCrcString const  m_wearerAppearanceName;
		PersistentCrcString       *m_mappedWearableAppearanceName;

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class TemporaryMapEntry: public MapEntry
	{
	public:

		TemporaryMapEntry(CrcString const &sourceWearableAppearanceName, CrcString const &wearerAppearanceName);

		virtual CrcString const &getSourceWearableAppearanceName() const;
		virtual CrcString const &getWearerAppearanceName() const;
		virtual CrcString const *getMappedWearableAppearanceName() const;

	private:

		// Disabled.
		TemporaryMapEntry();
		TemporaryMapEntry &operator =(TemporaryMapEntry const &);

	private:

		CrcString const &m_sourceWearableAppearanceName;
		CrcString const &m_wearerAppearanceName;

	};
	
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	struct LessMapEntryComparator
	{
		bool operator ()(MapEntry const *lhs, MapEntry const *rhs) const;
	};
	
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef std::vector<MapEntry*>  MapEntryVector;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void  remove();
	int   getRequiredColumnNumberFromNamedTable(char const *filename, DataTable const *table, char const *columnName);
	void  loadTableData(char const *filename);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	std::string const cs_forbiddenWearableCellContents(":block");

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	bool            s_installed;
	MapEntryVector  s_mapEntries;

}

using namespace WearableAppearanceMapNamespace;

// ======================================================================
// class WearableAppearanceMapNamespace::MapEntry
// ======================================================================

WearableAppearanceMapNamespace::MapEntry::~MapEntry()
{
	// Placeholder for derived classes.
}

// ----------------------------------------------------------------------

WearableAppearanceMapNamespace::MapEntry::MapEntry()
{
}

// ======================================================================
// class WearableAppearanceMapNamespace::PersistentMapEntry
// ======================================================================

WearableAppearanceMapNamespace::PersistentMapEntry::PersistentMapEntry(char const *sourceWearableAppearanceName, char const *wearerAppearanceName, char const *mappedWearableAppearanceName) :
	MapEntry(),
	m_sourceWearableAppearanceName(sourceWearableAppearanceName, true),
	m_wearerAppearanceName(wearerAppearanceName, true),
	m_mappedWearableAppearanceName(nullptr)
{
	if (mappedWearableAppearanceName != nullptr)
		m_mappedWearableAppearanceName = new PersistentCrcString(mappedWearableAppearanceName, true);
}

// ----------------------------------------------------------------------

WearableAppearanceMapNamespace::PersistentMapEntry::~PersistentMapEntry()
{
	delete m_mappedWearableAppearanceName;
}

// ----------------------------------------------------------------------

CrcString const &WearableAppearanceMapNamespace::PersistentMapEntry::getSourceWearableAppearanceName() const
{
	return m_sourceWearableAppearanceName;
}

// ----------------------------------------------------------------------

CrcString const &WearableAppearanceMapNamespace::PersistentMapEntry::getWearerAppearanceName() const
{
	return m_wearerAppearanceName;
}

// ----------------------------------------------------------------------

CrcString const *WearableAppearanceMapNamespace::PersistentMapEntry::getMappedWearableAppearanceName() const
{
	return m_mappedWearableAppearanceName;
}

// ======================================================================
// class WearableAppearanceMapNamespace::TemporaryMapEntry
// ======================================================================

WearableAppearanceMapNamespace::TemporaryMapEntry::TemporaryMapEntry(CrcString const &sourceWearableAppearanceName, CrcString const &wearerAppearanceName) :
	MapEntry(),
	m_sourceWearableAppearanceName(sourceWearableAppearanceName),
	m_wearerAppearanceName(wearerAppearanceName)
{
}

// ----------------------------------------------------------------------

CrcString const &WearableAppearanceMapNamespace::TemporaryMapEntry::getSourceWearableAppearanceName() const
{
	return m_sourceWearableAppearanceName;
}

// ----------------------------------------------------------------------

CrcString const &WearableAppearanceMapNamespace::TemporaryMapEntry::getWearerAppearanceName() const
{
	return m_wearerAppearanceName;
}

// ----------------------------------------------------------------------

CrcString const *WearableAppearanceMapNamespace::TemporaryMapEntry::getMappedWearableAppearanceName() const
{
	return nullptr;
}

// ======================================================================
// struct WearableAppearanceMapNamespace::LessMapEntryComparator
// ======================================================================

bool WearableAppearanceMapNamespace::LessMapEntryComparator::operator ()(MapEntry const *lhs, MapEntry const *rhs) const
{
	NOT_NULL(lhs);
	NOT_NULL(rhs);

	// First compare by source wearable appearance name...
	CrcString const &lhsSourceWearableAppearanceName = lhs->getSourceWearableAppearanceName();
	CrcString const &rhsSourceWearableAppearanceName = rhs->getSourceWearableAppearanceName();
	
	if (lhsSourceWearableAppearanceName < rhsSourceWearableAppearanceName)
		return true;
	else if (lhsSourceWearableAppearanceName > rhsSourceWearableAppearanceName)
		return false;
	else
	{
		// ... then compare by wearer appearance name.
		return lhs->getWearerAppearanceName() < rhs->getWearerAppearanceName();
	}
}

// ======================================================================
// namespace WearableAppearanceMapNamespace
// ======================================================================

void WearableAppearanceMapNamespace::remove()
{
	DEBUG_FATAL(!s_installed, ("WearableAppearanceMap not installed."));
	s_installed = false;

	std::for_each(s_mapEntries.begin(), s_mapEntries.end(), PointerDeleter());
	s_mapEntries.clear();
}

// ----------------------------------------------------------------------

int WearableAppearanceMapNamespace::getRequiredColumnNumberFromNamedTable(char const *filename, DataTable const *table, char const *columnName)
{
	NOT_NULL(columnName);

	int const columnNumber = table->findColumnNumber(columnName);
	FATAL(columnNumber < 0, ("failed to find column name [%s] in MountValidScaleRangeTable file [%s].", columnName, filename));

	return columnNumber;
}

// ----------------------------------------------------------------------

void WearableAppearanceMapNamespace::loadTableData(char const *filename)
{
	NOT_NULL(filename);

	//-- Load the data table.
	DataTable *const table = DataTableManager::getTable(filename, true);
	FATAL(!table, ("WearableAppearanceMap data file [%s] failed to open.", filename));

	//-- Find required data column numbers.
	int const sourceWearableAppearanceNameColumnNumber = getRequiredColumnNumberFromNamedTable(filename, table, "source_wearable_appearance_name");
	int const wearerAppearanceNameColumnNumber         = getRequiredColumnNumberFromNamedTable(filename, table, "wearer_appearance_name");
	int const mappedWearableAppearanceNameColumnNumber = getRequiredColumnNumberFromNamedTable(filename, table, "mapped_wearable_appearance_name");

	//-- Loop through data, constructing entries as necessary.
	int const rowCount = table->getNumRows();
	s_mapEntries.reserve(static_cast<MapEntryVector::size_type>(rowCount));

	for (int rowIndex = 0; rowIndex < rowCount; ++rowIndex)
	{
		//-- Get entry data.
		std::string const &sourceWearableAppearanceName = table->getStringValue(sourceWearableAppearanceNameColumnNumber, rowIndex);
		std::string const &wearerAppearanceName         = table->getStringValue(wearerAppearanceNameColumnNumber, rowIndex);
		std::string const &mappedWearableAppearanceName = table->getStringValue(mappedWearableAppearanceNameColumnNumber, rowIndex);

		//-- Create map entry, add to vector.
		s_mapEntries.push_back(new PersistentMapEntry(sourceWearableAppearanceName.c_str(), wearerAppearanceName.c_str(), (mappedWearableAppearanceName == cs_forbiddenWearableCellContents) ? nullptr : mappedWearableAppearanceName.c_str()));
	}

	DataTableManager::close(filename);

	//-- Sort the list in ascending key order.
	std::sort(s_mapEntries.begin(), s_mapEntries.end(), LessMapEntryComparator());

	// @todo: search for duplicates and FATAL if this occurs.
}

// ======================================================================
// class WearableAppearanceMap::MapResult: PUBLIC
// ======================================================================

WearableAppearanceMap::MapResult::MapResult(MapResult const &rhs) :
	m_hasMapping(rhs.m_hasMapping),
	m_wearableIsForbidden(rhs.m_wearableIsForbidden),
	m_mappedWearableAppearanceName(rhs.m_mappedWearableAppearanceName) //lint !e1554 // direct copy of pointer in copy constructor // yes, it's okay, we don't own this and won't try to delete it.
{
}

// ----------------------------------------------------------------------

bool WearableAppearanceMap::MapResult::hasMapping() const
{
	return m_hasMapping;
}

// ----------------------------------------------------------------------

bool WearableAppearanceMap::MapResult::isWearableForbidden() const
{
	return m_wearableIsForbidden;
}

// ----------------------------------------------------------------------

CrcString const *WearableAppearanceMap::MapResult::getMappedWearableAppearanceName() const
{
	return m_mappedWearableAppearanceName;
}

// ======================================================================

WearableAppearanceMap::MapResult::MapResult(bool newHasMapping, bool wearableIsForbidden, CrcString const *mappedWearableAppearanceName) :
	m_hasMapping(newHasMapping),
	m_wearableIsForbidden(wearableIsForbidden),
	m_mappedWearableAppearanceName(mappedWearableAppearanceName)
{
}

// ======================================================================
// class WearableAppearanceMap
// ======================================================================

void WearableAppearanceMap::install(char const *filename)
{
	InstallTimer const installTimer("WearableAppearanceMap::install");

	DEBUG_FATAL(s_installed, ("WearableAppearanceMap already installed."));
	
	loadTableData(filename);

	s_installed = true;
	ExitChain::add(WearableAppearanceMapNamespace::remove, "WearableAppearanceMap");
}

// ----------------------------------------------------------------------

WearableAppearanceMap::MapResult WearableAppearanceMap::getMapResultForWearableAndWearer(CrcString const &sourceWearableAppearanceName, CrcString const &wearerAppearanceName)
{
	DEBUG_FATAL(!s_installed, ("WearableAppearanceMap not installed."));

	//-- Find entry.
	TemporaryMapEntry searchKey(sourceWearableAppearanceName, wearerAppearanceName);

	std::pair<MapEntryVector::iterator, MapEntryVector::iterator> findResult = std::equal_range(s_mapEntries.begin(), s_mapEntries.end(), &searchKey, LessMapEntryComparator());
	if (findResult.first == findResult.second)
	{
		// We have no mapping for this entry.  That implies the source wearable appearance name can be used as is.
		return MapResult(false, false, nullptr);
	}
	else
	{
		MapEntry const *const mapEntry = *findResult.first;
		NOT_NULL(mapEntry);
		DEBUG_WARNING(std::distance(findResult.first, findResult.second) > 1, ("WearableAppearanceMap: there are multiple entries for source wearable=[%s], wearer=[%s]. Art leads: please fix data!", mapEntry->getSourceWearableAppearanceName().getString(), mapEntry->getWearerAppearanceName().getString()));

		// We have a mapping.  Return it.
		CrcString const *const mappedWearableAppearanceName = mapEntry->getMappedWearableAppearanceName();
		return MapResult(true, mappedWearableAppearanceName == nullptr, mappedWearableAppearanceName);
	}
}

// ======================================================================
