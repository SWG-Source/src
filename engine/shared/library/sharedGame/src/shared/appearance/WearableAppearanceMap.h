// ======================================================================
//
// WearableAppearanceMap.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_WearableAppearanceMap_H
#define INCLUDED_WearableAppearanceMap_H

// ======================================================================

class CrcString;

// ======================================================================

class WearableAppearanceMap
{
public:

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	class MapResult
	{
		friend class WearableAppearanceMap;

	public:

		MapResult(MapResult const &rhs);

		bool             hasMapping() const;

		bool             isWearableForbidden() const;
		CrcString const *getMappedWearableAppearanceName() const;

	private:

		MapResult(bool hasMapping, bool wearableIsForbidden, CrcString const *mappedWearableAppearanceName);

		// Disabled.
		MapResult();
		MapResult &operator =(MapResult const &rhs);

	private:

		bool const             m_hasMapping;
		bool const             m_wearableIsForbidden;
		CrcString const *const m_mappedWearableAppearanceName;

	};

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

public:

	static void install(char const *filename);
	static MapResult getMapResultForWearableAndWearer(CrcString const &sourceWearableAppearanceName, CrcString const &wearerAppearanceName);

};

// ======================================================================

#endif
