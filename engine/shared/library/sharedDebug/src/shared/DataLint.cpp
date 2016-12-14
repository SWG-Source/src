// ============================================================================
//
// DataLint.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "sharedDebug/FirstSharedDebug.h"
#include "sharedDebug/DataLint.h"

#include "sharedFoundation/ConfigSharedFoundation.h"
#include "sharedFoundation/ExitChain.h"

#include <cstdio>
#include <set>
#include <stack>
#include <string>
#include <vector>

// ============================================================================

namespace DataLintNamespace
{
	enum Mode
	{
		M_client,
		M_server
	};

	typedef std::vector<std::string>                               StringList;
	typedef std::pair<std::string, std::pair<DataLint::AssetType, StringList> > WarningPair;
	typedef std::vector<WarningPair>                               WarningList;
	typedef bool(*AssetFunction)(char const *);

	bool                       ms_installed = false;
	DataLint::AssetType        m_currentAssetType = DataLint::AT_invalid;
	bool                       m_enabled = false;
	WarningList *              m_warningList = nullptr;
	StringList *               m_assetStack = nullptr;
	DataLint::StringPairList * m_assetList = nullptr;
	std::string                m_dataLintCategorizedAssetsFile("DataLint_CategorizedAssets.txt");
	std::string                m_dataLintUnSupportedAssetsFile("DataLint_UnSupportedAssets.txt");
	Mode                       m_mode = M_client;

	bool                       isAnAppearanceAsset(char const *text);
	bool                       isAnArrangementDescriptorAsset(char const *text);
	bool                       isALocalizedStringAsset(char const *text);
	bool                       isAnObjectTemplateAsset(char const *text);
	bool                       isAPortalProprtyAsset(char const *text);
	bool                       isAShaderTemplateAsset(char const *text);
	bool                       isASkyBoxAsset(char const *text);
	bool                       isASlotDescriptorAsset(char const *text);
	bool                       isASoundAsset(char const *text);
	bool                       isATerrainAsset(char const *text);
	bool                       isATextureAsset(char const *text);
	bool                       isATextureRendererTemplateAsset(char const *text);
	bool                       isAnUnSupportedAsset(char const *text);

	void                       fixUpSlashes(char *text);
	DataLint::StringPairList   getList(int const reserveCount, AssetFunction listTypeFunction);
	void                       writeToCategorizedFile(FILE *fp, char const *title, DataLint::AssetType const listType);
	std::string                formatErrorMessage(WarningPair &warningPair, int &currentErrorCount);
	void                       writeError(FILE *fp, WarningPair &warningPair, int &currentErrorCount);
}

using namespace DataLintNamespace;

// ============================================================================
//
// DataLint
//
// ============================================================================

void DataLint::install()
{
	DEBUG_FATAL(ms_installed, ("DataLint::install: already installed"));
	ms_installed = true;
	m_enabled = true;

	m_warningList = new WarningList();
	m_warningList->reserve(4096);

	m_assetStack = new StringList();

	m_assetList = new StringPairList();
	m_assetList->reserve(4096);

	FatalSetThrowExceptions(true);

	ExitChain::add(&remove, "DataLint::remove");
}

//-----------------------------------------------------------------------------

bool DataLint::isInstalled()
{
	return ms_installed;
}

//-----------------------------------------------------------------------------

void DataLint::report()
{
	if (m_enabled)
	{
		// Error Assets

		DEBUG_REPORT_LOG_PRINT(true, ("Writing error files\n"));

		// Setup filenames for client versus server mode

		std::string dataLintErrorsAll((m_mode == M_client) ? "DataLint_Errors_All.txt" : "DataLintServer_Errors_All.txt");
		std::string dataLintErrorsAllFatal((m_mode == M_client) ? "DataLint_Errors_All_Fatal.txt" : "DataLintServer_Errors_All_Fatal.txt");
		std::string dataLintErrorsAllWarning((m_mode == M_client) ? "DataLint_Errors_All_Warning.txt" : "DataLintServer_Errors_All_Warning.txt");
		std::string dataLintErrorsAllObjectTemplate((m_mode == M_client) ? "DataLint_Errors_ObjectTemplate.txt" : "DataLintServer_Errors_ObjectTemplate.txt");

		// Server/client

		FILE *assetErrorFileAll = fopen(dataLintErrorsAll.c_str(), "wt");
		FILE *assetErrorFileAllFatal = fopen(dataLintErrorsAllFatal.c_str(), "wt");
		FILE *assetErrorFileAllWarning = fopen(dataLintErrorsAllWarning.c_str(), "wt");
		FILE *assetErrorFileObjectTemplate = fopen(dataLintErrorsAllObjectTemplate.c_str(), "wt");

		// Client only

		FILE *assetErrorFileAppearance = nullptr;
		FILE *assetErrorFileArrangementDescriptor = nullptr;
		FILE *assetErrorFileLocalizedStringTable = nullptr;
		FILE *assetErrorFilePortalProperty = nullptr;
		FILE *assetErrorFileShaderTemplate = nullptr;
		FILE *assetErrorFileSkyBox = nullptr;
		FILE *assetErrorFileSlotDescriptor = nullptr;
		FILE *assetErrorFileSoundTemplate = nullptr;
		FILE *assetErrorFileTerrain = nullptr;
		FILE *assetErrorFileTexture = nullptr;
		FILE *assetErrorFileTextureRenderer = nullptr;

		if (m_mode == M_client)
		{
			assetErrorFileAppearance = fopen("DataLint_Errors_Appearance.txt", "wt");
			assetErrorFileArrangementDescriptor = fopen("DataLint_Errors_ArrangementDescriptor.txt", "wt");
			assetErrorFileLocalizedStringTable = fopen("DataLint_Errors_LocalizedStringTable.txt", "wt");
			assetErrorFilePortalProperty = fopen("DataLint_Errors_PortalProperty.txt", "wt");
			assetErrorFileShaderTemplate = fopen("DataLint_Errors_ShaderTemplate.txt", "wt");
			assetErrorFileSkyBox = fopen("DataLint_Errors_SkyBox.txt", "wt");
			assetErrorFileSlotDescriptor = fopen("DataLint_Errors_SlotDescriptor.txt", "wt");
			assetErrorFileSoundTemplate = fopen("DataLint_Errors_SoundTemplate.txt", "wt");
			assetErrorFileTerrain = fopen("DataLint_Errors_Terrain.txt", "wt");
			assetErrorFileTexture = fopen("DataLint_Errors_Texture.txt", "wt");
			assetErrorFileTextureRenderer = fopen("DataLint_Errors_TextureRenderer.txt", "wt");
		}

		// Might want to think of a better way to keep track of the number of errors for each specific asset type.

		int totalErrorCountAll = 1;
		int totalErrorCountAllFatal = 1;
		int totalErrorCountAllWarning = 1;
		int totalErrorCountAppearance = 1;
		int totalErrorCountArrangementDescriptor = 1;
		int totalErrorCountLocalizedStringTable = 1;
		int totalErrorCountObjectTemplate = 1;
		int totalErrorCountPortalProperty = 1;
		int totalErrorCountShaderTemplate = 1;
		int totalErrorCountSkyBox = 1;
		int totalErrorCountSlotDescriptor = 1;
		int totalErrorCountSoundTemplate = 1;
		int totalErrorCountTerrain = 1;
		int totalErrorCountTexture = 1;
		int totalErrorCountTextureRenderer = 1;

		typedef std::set<std::string> StringSet;
		StringSet duplicateStringSet;

		if (assetErrorFileAll)
		{
			WarningList::iterator warningListIter = m_warningList->begin();

			for (; warningListIter != m_warningList->end(); ++warningListIter)
			{
				// Build a duplicate string

				std::string duplicateString(warningListIter->first);

				StringList::const_iterator stringStackIter = warningListIter->second.second.begin();

				for (; stringStackIter != warningListIter->second.second.end(); ++stringStackIter)
				{
					duplicateString += *stringStackIter;
				}

				if (duplicateStringSet.find(duplicateString) == duplicateStringSet.end())
				{
					// This is the first instance of this error, so log it

					duplicateStringSet.insert(duplicateString);
				}
				else
				{
					// We found a duplicate warning, so don't show it twice

					continue;
				}

				// Write the error to the appropriate error file

				switch (warningListIter->second.first)
				{
				case AT_appearance:
				{
					if (m_mode == M_client)
					{
						writeError(assetErrorFileAppearance, *warningListIter, totalErrorCountAppearance);
					}
				}
				break;
				case AT_arrangementDescriptor:
				{
					if (m_mode == M_client)
					{
						writeError(assetErrorFileArrangementDescriptor, *warningListIter, totalErrorCountArrangementDescriptor);
					}
				}
				break;
				case AT_localizedStringTable:
				{
					if (m_mode == M_client)
					{
						writeError(assetErrorFileLocalizedStringTable, *warningListIter, totalErrorCountLocalizedStringTable);
					}
				}
				break;
				case AT_objectTemplate:
				{
					writeError(assetErrorFileObjectTemplate, *warningListIter, totalErrorCountObjectTemplate);
				}
				break;
				case AT_portalProperty:
				{
					if (m_mode == M_client)
					{
						writeError(assetErrorFilePortalProperty, *warningListIter, totalErrorCountPortalProperty);
					}
				}
				break;
				case AT_shaderTemplate:
				{
					if (m_mode == M_client)
					{
						writeError(assetErrorFileShaderTemplate, *warningListIter, totalErrorCountShaderTemplate);
					}
				}
				break;
				case AT_skyBox:
				{
					if (m_mode == M_client)
					{
						writeError(assetErrorFileSkyBox, *warningListIter, totalErrorCountSkyBox);
					}
				}
				break;
				case AT_slotDescriptor:
				{
					if (m_mode == M_client)
					{
						writeError(assetErrorFileSlotDescriptor, *warningListIter, totalErrorCountSlotDescriptor);
					}
				}
				break;
				case AT_soundTemplate:
				{
					if (m_mode == M_client)
					{
						writeError(assetErrorFileSoundTemplate, *warningListIter, totalErrorCountSoundTemplate);
					}
				}
				break;
				case AT_terrain:
				{
					if (m_mode == M_client)
					{
						writeError(assetErrorFileTerrain, *warningListIter, totalErrorCountTerrain);
					}
				}
				break;
				case AT_texture:
				{
					if (m_mode == M_client)
					{
						writeError(assetErrorFileTexture, *warningListIter, totalErrorCountTexture);
					}
				}
				break;
				case AT_textureRendererTemplate:
				{
					if (m_mode == M_client)
					{
						writeError(assetErrorFileTextureRenderer, *warningListIter, totalErrorCountTextureRenderer);
					}
				}
				break;
				default:
				{
					DEBUG_REPORT_LOG_PRINT(true, ("DataLint::remove() - Unsupported asset type: %s", warningListIter->first.c_str()));
				}
				break;
				}

				// Write the error to the master error file

				writeError(assetErrorFileAll, *warningListIter, totalErrorCountAll);

				// If this is a fatal, write to the fatal list

				if (strstr(warningListIter->first.c_str(), "FATAL:"))
				{
					writeError(assetErrorFileAllFatal, *warningListIter, totalErrorCountAllFatal);
				}

				// If a warning, write to the warning list

				if (strstr(warningListIter->first.c_str(), "WARNING:"))
				{
					writeError(assetErrorFileAllWarning, *warningListIter, totalErrorCountAllWarning);
				}
			}
		}

		// UnSupported Assets

		DEBUG_REPORT_LOG_PRINT(true, ("Writing unsupported assets list: %s\n", m_dataLintUnSupportedAssetsFile.c_str()));

		FILE *fp = fopen(m_dataLintUnSupportedAssetsFile.c_str(), "wt");

		if (fp)
		{
			StringPairList unSupportedStringPairList(getList(AT_unSupported));
			StringPairList::iterator stringPairListIter = unSupportedStringPairList.begin();
			int count = 1;

			for (; stringPairListIter != unSupportedStringPairList.end(); ++stringPairListIter)
			{
				fprintf(fp, "%3d %s @ %s\n", count, stringPairListIter->first.c_str(), stringPairListIter->second.c_str());
				++count;
			}

			fclose(fp);
		}
		else
		{
			DEBUG_REPORT_LOG_PRINT(true, ("Error writing unsupported assets list: %s\n", m_dataLintUnSupportedAssetsFile.c_str()));
		}

		// Categorized Assets

		DEBUG_REPORT_LOG_PRINT(true, ("Writing categorized files list: %s\n", m_dataLintCategorizedAssetsFile.c_str()));

		fp = fopen(m_dataLintCategorizedAssetsFile.c_str(), "wt");

		if (fp)
		{
			fprintf(fp, "This file contains a categorized listing of the linted assets in the game. The categories are as follows:\n");
			fprintf(fp, "\n");
			fprintf(fp, "1.  Appearances                (%5d)\n", getList(AT_appearance).size());
			fprintf(fp, "2.  Arrangement Descriptors    (%5d)\n", getList(AT_arrangementDescriptor).size());
			fprintf(fp, "3.  Localized String Tables    (%5d)\n", getList(AT_localizedStringTable).size());
			fprintf(fp, "4.  Object Templates           (%5d)\n", getList(AT_objectTemplate).size());
			fprintf(fp, "5.  Portal Properties          (%5d)\n", getList(AT_portalProperty).size());
			fprintf(fp, "6.  Shader Templates           (%5d)\n", getList(AT_shaderTemplate).size());
			fprintf(fp, "7.  SkyBoxes                   (%5d)\n", getList(AT_skyBox).size());
			fprintf(fp, "8.  Slot Descriptors           (%5d)\n", getList(AT_slotDescriptor).size());
			fprintf(fp, "9.  Sound Templates            (%5d)\n", getList(AT_soundTemplate).size());
			fprintf(fp, "10. Terrain                    (%5d)\n", getList(AT_terrain).size());
			fprintf(fp, "11. Textures                   (%5d)\n", getList(AT_texture).size());
			fprintf(fp, "12. Texture Renderer Templates (%5d)\n", getList(AT_textureRendererTemplate).size());
			fprintf(fp, "--------------------------------------\n");
			fprintf(fp, "13. Total Linted Assets        (%5d)\n", m_assetList->size() - getList(AT_unSupported).size());
			fprintf(fp, "\n");

			writeToCategorizedFile(fp, "Appearances", AT_appearance);
			writeToCategorizedFile(fp, "Arrangement Descriptors", AT_arrangementDescriptor);
			writeToCategorizedFile(fp, "Localized String Tables", AT_localizedStringTable);
			writeToCategorizedFile(fp, "Object Templates", AT_objectTemplate);
			writeToCategorizedFile(fp, "Portal Properties", AT_portalProperty);
			writeToCategorizedFile(fp, "Shader Templates", AT_shaderTemplate);
			writeToCategorizedFile(fp, "SkyBoxes", AT_skyBox);
			writeToCategorizedFile(fp, "Slot Descriptors", AT_slotDescriptor);
			writeToCategorizedFile(fp, "Sound Templates", AT_soundTemplate);
			writeToCategorizedFile(fp, "Terrain", AT_terrain);
			writeToCategorizedFile(fp, "Textures", AT_texture);
			writeToCategorizedFile(fp, "Texture Renderer Templates", AT_textureRendererTemplate);

			fclose(fp);
		}

		if (assetErrorFileAll) { fclose(assetErrorFileAll); }
		if (assetErrorFileAllFatal) { fclose(assetErrorFileAllFatal); }
		if (assetErrorFileAllWarning) { fclose(assetErrorFileAllWarning); }
		if (assetErrorFileObjectTemplate) { fclose(assetErrorFileObjectTemplate); }

		if (m_mode == M_client)
		{
			if (assetErrorFileAppearance) { fclose(assetErrorFileAppearance); }
			if (assetErrorFileArrangementDescriptor) { fclose(assetErrorFileArrangementDescriptor); }
			if (assetErrorFileLocalizedStringTable) { fclose(assetErrorFileLocalizedStringTable); }
			if (assetErrorFilePortalProperty) { fclose(assetErrorFilePortalProperty); }
			if (assetErrorFileShaderTemplate) { fclose(assetErrorFileShaderTemplate); }
			if (assetErrorFileSkyBox) { fclose(assetErrorFileSkyBox); }
			if (assetErrorFileSlotDescriptor) { fclose(assetErrorFileSlotDescriptor); }
			if (assetErrorFileSoundTemplate) { fclose(assetErrorFileSoundTemplate); }
			if (assetErrorFileTerrain) { fclose(assetErrorFileTerrain); }
			if (assetErrorFileTexture) { fclose(assetErrorFileTexture); }
			if (assetErrorFileTextureRenderer) { fclose(assetErrorFileTextureRenderer); }
		}
	}
}

//-----------------------------------------------------------------------------
void DataLint::remove()
{
	DEBUG_FATAL(!ms_installed, ("DataLint::remove: not installed"));
	ms_installed = false;
	m_enabled = false;

	delete m_warningList;
	delete m_assetStack;
	delete m_assetList;

	FatalSetThrowExceptions(false);
}

//-----------------------------------------------------------------------------
void DataLintNamespace::writeError(FILE *fp, WarningPair &warningPair, int &currentErrorCount)
{
	std::string formatedErrorMessage(formatErrorMessage(warningPair, currentErrorCount));

	if (fp)
	{
		fprintf(fp, "%s", formatedErrorMessage.c_str());
	}
}

//-----------------------------------------------------------------------------
std::string DataLintNamespace::formatErrorMessage(WarningPair &warningPair, int &currentErrorCount)
{
	std::string result;
	char formatedError[4096];
	char text[4096];
	sprintf(text, "%s", warningPair.first.c_str());
	fixUpSlashes(text);

	char *assetError = strstr(text, " : ");

	if (assetError != nullptr)
	{
		++assetError;
		++assetError;
		++assetError;
	}

	char *lineOfCode = &text[0];
	char *seperator = strstr(text, " : ");

	if (seperator)
	{
		*seperator = '\0';
	}

	if (assetError)
	{
		// Remove any trailing carriage returns

		int assetErrorLength = strlen(assetError);

		if (assetErrorLength > 0)
		{
			char *walkPtr = &assetError[0] + (assetErrorLength - 1);

			while ((assetErrorLength > 0) && (*walkPtr == '\n'))
			{
				--assetErrorLength;
				*walkPtr = '\0';
				--walkPtr;
			}
		}

		snprintf(formatedError, 4096, "%3d Error:        %s\n", currentErrorCount, assetError);
		result += formatedError;
	}

	if (lineOfCode)
	{
		snprintf(formatedError, 4096, "    Line of Code: %s\n", lineOfCode);
		result += formatedError;
	}

	StringList::reverse_iterator stringListIter = warningPair.second.second.rbegin();

	for (; stringListIter != warningPair.second.second.rend(); ++stringListIter)
	{
		std::string stackString(*stringListIter);

		snprintf(text, 4096, "    Loaded From:  %s\n", stackString.c_str());
		fixUpSlashes(text);

		snprintf(formatedError, 4096, "%s", text);
		result += formatedError;
	}

	result += "\n";
	++currentErrorCount;

	return result;
}

//-----------------------------------------------------------------------------
void DataLint::setEnabled(bool const enabled)
{
	m_enabled = enabled;
}

//-----------------------------------------------------------------------------
bool DataLint::isEnabled()
{
	return m_enabled || ConfigSharedFoundation::getVerboseWarnings();
}

//-----------------------------------------------------------------------------
void DataLint::pushAsset(char const *assetPath)
{
	if (m_assetStack)
	{
		int const size = m_assetStack->size();
		UNREF(size);
		m_assetStack->push_back(assetPath);
	}
}

//-----------------------------------------------------------------------------
void DataLint::popAsset()
{
	if (m_assetStack)
	{
		int const size = m_assetStack->size();
		UNREF(size);
		DEBUG_FATAL(m_assetStack->empty(), ("DataLint::popAsset() - Trying to pop the asset stack when it is empty."));
		m_assetStack->pop_back();
	}
}

//-----------------------------------------------------------------------------

void DataLint::logWarning(std::string const &warning)
{
	if (m_enabled)
	{
		m_warningList->push_back(std::make_pair(warning, std::make_pair(m_currentAssetType, *m_assetStack)));
	}
}

//-----------------------------------------------------------------------------
void DataLint::clearAssetStack()
{
	if (m_assetStack)
		m_assetStack->clear();
}

//-----------------------------------------------------------------------------
void DataLint::addFilePath(char const *filePath)
{
	if (filePath == nullptr || (strlen(filePath) <= 0))
	{
		return;
	}

	if (m_assetList != nullptr)
	{
		char text[4096];
		sprintf(text, "%s", filePath);

		char *seperator = strstr(text, " @ ");
		char *longPath = seperator;
		++longPath;
		++longPath;
		++longPath;
		char *shortPath = &text[0];
		*seperator = '\0';

		m_assetList->push_back(std::make_pair(shortPath, longPath));
	}
}

//-----------------------------------------------------------------------------
void DataLint::removeFilePath(char const *filePath)
{
	if (m_assetList != nullptr)
	{
		StringPairList::iterator stringPairListIter = m_assetList->begin();

		for (; stringPairListIter != m_assetList->end(); ++stringPairListIter)
		{
			char const *first = stringPairListIter->first.c_str();

			if (strcmp(first, filePath) == 0)
			{
				m_assetList->erase(stringPairListIter);
			}
		}
	}
}

//-----------------------------------------------------------------------------
bool DataLintNamespace::isAnAppearanceAsset(char const *text)
{
	bool const appearanceAsset = (strstr(text, "appearance/") != nullptr);
	bool const portalPropertyAsset = (strstr(text, ".pob") != nullptr);

	return (appearanceAsset && !portalPropertyAsset);
}

//-----------------------------------------------------------------------------
bool DataLintNamespace::isAnArrangementDescriptorAsset(char const *text)
{
	bool const arrangementDescriptorAsset = (strstr(text, "slot/arrangement/") != nullptr);

	return arrangementDescriptorAsset;
}

//-----------------------------------------------------------------------------
bool DataLintNamespace::isALocalizedStringAsset(char const *text)
{
	bool const localizedStringAsset = (strstr(text, ".stf") != nullptr);

	return localizedStringAsset;
}

//-----------------------------------------------------------------------------
bool DataLintNamespace::isAnObjectTemplateAsset(char const *text)
{
	bool result = false;
	UNREF(text);
	bool const isInTheObjectDirectory = (strstr(text, "object/") != nullptr);
	bool const isInTheAbstractDirectory = (strstr(text, "abstract/") != nullptr);

	if (isInTheObjectDirectory || isInTheAbstractDirectory)
	{
		bool const arrangementDescriptorAsset = isAnArrangementDescriptorAsset(text);
		bool const slotDescriptorAsset = isASlotDescriptorAsset(text);

		result = (!arrangementDescriptorAsset) && (!slotDescriptorAsset);
	}

	return result;
}

//-----------------------------------------------------------------------------
bool DataLintNamespace::isAPortalProprtyAsset(char const *text)
{
	bool const portalPropertyAsset = (strstr(text, ".pob") != nullptr);

	return portalPropertyAsset;
}

//-----------------------------------------------------------------------------
bool DataLintNamespace::isAShaderTemplateAsset(char const *text)
{
	bool const shaderTemplateAsset = (strstr(text, "shader/") != nullptr);

	return shaderTemplateAsset;
}

//-----------------------------------------------------------------------------
bool DataLintNamespace::isASkyBoxAsset(char const *text)
{
	bool const skyBoxAsset = (strstr(text, "skybox/") != nullptr);

	return skyBoxAsset;
}

//-----------------------------------------------------------------------------
bool DataLintNamespace::isASlotDescriptorAsset(char const *text)
{
	bool const slotDescriptorAsset = (strstr(text, "slot/descriptor/") != nullptr);

	return slotDescriptorAsset;
}

//-----------------------------------------------------------------------------
bool DataLintNamespace::isASoundAsset(char const *text)
{
	bool const soundAsset = (strstr(text, "sound/") != nullptr);

	return soundAsset;
}

//-----------------------------------------------------------------------------
bool DataLintNamespace::isATerrainAsset(char const *text)
{
	return strstr(text, ".trn") != 0;
}

//-----------------------------------------------------------------------------
bool DataLintNamespace::isATextureAsset(char const *text)
{
	bool const textureAsset = (strstr(text, ".dds") != nullptr);

	return textureAsset;
}

//-----------------------------------------------------------------------------
bool DataLintNamespace::isATextureRendererTemplateAsset(char const *text)
{
	bool const textureRendererAsset = (strstr(text, ".trt") != nullptr);

	return textureRendererAsset;
}

//-----------------------------------------------------------------------------
bool DataLintNamespace::isAnUnSupportedAsset(char const *text)
{
	bool const appearanceAsset = isAnAppearanceAsset(text);
	bool const arrangementDescriptorAsset = isAnArrangementDescriptorAsset(text);
	bool const localizedStringAsset = isALocalizedStringAsset(text);
	bool const objectTemplateAsset = isAnObjectTemplateAsset(text);
	bool const portalPropertyAsset = isAPortalProprtyAsset(text);
	bool const shaderTemplateAsset = isAShaderTemplateAsset(text);
	bool const skyBoxAsset = isASkyBoxAsset(text);
	bool const slotDescriptorAsset = isASlotDescriptorAsset(text);
	bool const soundAsset = isASoundAsset(text);
	bool const terrainAsset = isATerrainAsset(text);
	bool const textureAsset = isATextureAsset(text);
	bool const textureRendererTemplateAsset = isATextureRendererTemplateAsset(text);

	bool const result = !(appearanceAsset ||
		arrangementDescriptorAsset ||
		localizedStringAsset ||
		objectTemplateAsset ||
		portalPropertyAsset ||
		shaderTemplateAsset ||
		skyBoxAsset ||
		slotDescriptorAsset ||
		soundAsset ||
		terrainAsset ||
		textureAsset ||
		textureRendererTemplateAsset);

	return result;
}

//-----------------------------------------------------------------------------
DataLint::StringPairList DataLintNamespace::getList(int const reserveCount, AssetFunction assetTypeFunction)
{
	DEBUG_FATAL(!m_assetList, ("DataLint::getList() - m_assetList is nullptr"));

	// Create the list of terrains

	DataLint::StringPairList stringPairList;
	stringPairList.reserve(reserveCount);

	DataLint::StringPairList::iterator stringPairListIter = m_assetList->begin();

	for (; stringPairListIter != m_assetList->end(); ++stringPairListIter)
	{
		char const *first = stringPairListIter->first.c_str();
		char const *second = stringPairListIter->second.c_str();

		if (assetTypeFunction(first))
		{
			stringPairList.push_back(std::make_pair(first, second));
		}
	}

	return stringPairList;
}

//-----------------------------------------------------------------------------
DataLint::StringPairList DataLint::getList(AssetType const assetType)
{
	switch (assetType)
	{
	case AT_appearance:
	{
		return DataLintNamespace::getList(8192, isAnAppearanceAsset);
	}
	break;
	case AT_arrangementDescriptor:
	{
		return DataLintNamespace::getList(1024, isAnArrangementDescriptorAsset);
	}
	break;
	case AT_localizedStringTable:
	{
		return DataLintNamespace::getList(1024, isALocalizedStringAsset);
	}
	break;
	case AT_objectTemplate:
	{
		return DataLintNamespace::getList(8192, isAnObjectTemplateAsset);
	}
	break;
	case AT_portalProperty:
	{
		return DataLintNamespace::getList(512, isAPortalProprtyAsset);
	}
	break;
	case AT_shaderTemplate:
	{
		return DataLintNamespace::getList(4096, isAShaderTemplateAsset);
	}
	break;
	case AT_skyBox:
	{
		return DataLintNamespace::getList(256, isASkyBoxAsset);
	}
	break;
	case AT_slotDescriptor:
	{
		return DataLintNamespace::getList(1024, isASlotDescriptorAsset);
	}
	break;
	case AT_soundTemplate:
	{
		return DataLintNamespace::getList(4096, isASoundAsset);
	}
	break;
	case AT_terrain:
	{
		return DataLintNamespace::getList(32, isATerrainAsset);
	}
	break;
	case AT_texture:
	{
		return DataLintNamespace::getList(8192, isATextureAsset);
	}
	break;
	case AT_textureRendererTemplate:
	{
		return DataLintNamespace::getList(512, isATextureRendererTemplateAsset);
	}
	break;
	case AT_unSupported:
	{
		return DataLintNamespace::getList(4096, isAnUnSupportedAsset);
	}
	break;
	default:
	{
		DEBUG_FATAL(true, ("DataLint::getList() - Unknown list type."));
	}
	}

	return StringPairList();
}

//-----------------------------------------------------------------------------
void DataLintNamespace::fixUpSlashes(char *text)
{
	char *slashEater = &text[0];

	while (*slashEater != '\0')
	{
		if (*slashEater == '\\')
		{
			*slashEater = '/';
		}

		++slashEater;
	}
}

//-----------------------------------------------------------------------------
void DataLintNamespace::writeToCategorizedFile(FILE *fp, char const *title, DataLint::AssetType const assetType)
{
	DataLint::StringPairList stringPairlist(DataLint::getList(assetType));

	fprintf(fp, "-----------------%s (%d assets)-----------------\n", title, static_cast<int>(stringPairlist.size()));
	fprintf(fp, "\n");
	DataLint::StringPairList::const_iterator stringPairListIter = stringPairlist.begin();
	int count = 1;

	for (; stringPairListIter != stringPairlist.end(); ++stringPairListIter)
	{
		fprintf(fp, "%4d %s @ %s\n", count, stringPairListIter->first.c_str(), stringPairListIter->second.c_str());
		++count;
	}

	fprintf(fp, "\n");
}

//-----------------------------------------------------------------------------
void DataLint::setCurrentAssetType(AssetType const assetType)
{
	m_currentAssetType = assetType;
}

//-----------------------------------------------------------------------------
void DataLint::setClientMode()
{
	m_mode = M_client;
}

//-----------------------------------------------------------------------------
void DataLint::setServerMode()
{
	m_mode = M_server;
}

// ============================================================================