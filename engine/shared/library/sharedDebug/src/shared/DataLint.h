// ============================================================================
//
// DataLint.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef DATALINT_H
#define DATALINT_H

// ============================================================================

class DataLint
{
public:

	typedef std::vector<std::pair<std::string, std::string> > StringPairList;

public:

	static void install();
	static void remove();

	static bool isInstalled();

	static void setClientMode();
	static void setServerMode();

	static void addFilePath(char const *filePath);
	static void removeFilePath(char const *filePath);

	DLLEXPORT static bool isEnabled();
	static void setEnabled(bool const enabled);

	static void clearAssetStack();
	static void logWarning(std::string const &warning);
	static void logNote(std::string const &warning);

	static void pushAsset(char const *assetPath);
	static void popAsset();

	// Dump the report files

	static void report();

	// If you add a new asset type, it needs a matching isA<AssetType> function below

	enum AssetType
	{
		AT_appearance,
		AT_arrangementDescriptor,
		AT_localizedStringTable,
		AT_objectTemplate,
		AT_portalProperty,
		AT_shaderTemplate,
		AT_skyBox,
		AT_slotDescriptor,
		AT_soundTemplate,
		AT_terrain,
		AT_texture,
		AT_textureRendererTemplate,
		AT_unSupported,
		AT_count,
		AT_invalid = -1
	};

	static StringPairList getList(AssetType const listType);
	static void           setCurrentAssetType(AssetType const assetType);
};

// ============================================================================

#endif 
