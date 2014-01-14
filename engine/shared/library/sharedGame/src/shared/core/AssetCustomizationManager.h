// ======================================================================
//
// AssetCustomizationManager.h
// Copyright 2003 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_AssetCustomizationManager_H
#define INCLUDED_AssetCustomizationManager_H

// ======================================================================

class CrcString;
class CustomizationData;

// ======================================================================

class AssetCustomizationManager
{
public:

	static void install(char const *filename);

	static int addCustomizationVariablesForAsset(CrcString const &assetName, CustomizationData &customizationData, bool skipSharedOwnerVariables);

};

// ======================================================================

#endif
