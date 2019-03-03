//========================================================================
//
// ServerManufactureSchematicObjectTemplate.cpp
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "sharedTemplate/FirstSharedTemplate.h"
#include "sharedTemplateDefinition/TemplateData.h"
#include "sharedTemplateDefinition/TemplateGlobals.h"
#include "ServerManufactureSchematicObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>

#include "sharedFoundation/CrcConstexpr.hpp"


/**
 * Class constructor.
 */
ServerManufactureSchematicObjectTemplate::ServerManufactureSchematicObjectTemplate(const std::string &filename)
//@BEGIN TFD INIT
        : ServerIntangibleObjectTemplate(filename), m_ingredientsLoaded(false), m_ingredientsAppend(false),
          m_attributesLoaded(false), m_attributesAppend(false)
//@END TFD INIT
{
}    // ServerManufactureSchematicObjectTemplate::ServerManufactureSchematicObjectTemplate

/**
 * Class destructor.
 */
ServerManufactureSchematicObjectTemplate::~ServerManufactureSchematicObjectTemplate() {
//@BEGIN TFD CLEANUP
    {
        std::vector<StructParamOT *>::iterator iter;
        for (iter = m_ingredients.begin(); iter != m_ingredients.end(); ++iter) {
            delete *iter;
            *iter = nullptr;
        }
        m_ingredients.clear();
    }
    {
        std::vector<StructParamOT *>::iterator iter;
        for (iter = m_attributes.begin(); iter != m_attributes.end(); ++iter) {
            delete *iter;
            *iter = nullptr;
        }
        m_attributes.clear();
    }
//@END TFD CLEANUP
}    // ServerManufactureSchematicObjectTemplate::~ServerManufactureSchematicObjectTemplate

/**
 * Static function used to register this template.
 */
void ServerManufactureSchematicObjectTemplate::registerMe(void) {
    ObjectTemplateList::registerTemplate(ServerManufactureSchematicObjectTemplate_tag, create);
}    // ServerManufactureSchematicObjectTemplate::registerMe

/**
 * Creates a ServerManufactureSchematicObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate *ServerManufactureSchematicObjectTemplate::create(const std::string &filename) {
    return new ServerManufactureSchematicObjectTemplate(filename);
}    // ServerManufactureSchematicObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerManufactureSchematicObjectTemplate::getId(void) const {
    return ServerManufactureSchematicObjectTemplate_tag;
}    // ServerManufactureSchematicObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag ServerManufactureSchematicObjectTemplate::getTemplateVersion(void) const {
    return m_templateVersion;
} // ServerManufactureSchematicObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag ServerManufactureSchematicObjectTemplate::getHighestTemplateVersion(void) const {
    if (m_baseData == NULL)
        return m_templateVersion;
    const ServerManufactureSchematicObjectTemplate *base = dynamic_cast<const ServerManufactureSchematicObjectTemplate *>(m_baseData);
    if (base == NULL)
        return m_templateVersion;
    return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // ServerManufactureSchematicObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam *
ServerManufactureSchematicObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index) {
    if (strcmp(name, "itemCount") == 0) {
        if (index == 0) {
            if (deepCheck && !isParamLoaded(name, false, 0)) {
                if (getBaseTemplate() != nullptr)
                    return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
                return nullptr;
            }
            return &m_itemCount;
        }
        fprintf(stderr, "trying to access single-parameter \"itemCount\" as an array\n");
    } else
        return ServerIntangibleObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
    return nullptr;
}    //ServerManufactureSchematicObjectTemplate::getCompilerIntegerParam

FloatParam *ServerManufactureSchematicObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index) {
    return ServerIntangibleObjectTemplate::getFloatParam(name, deepCheck, index);
}    //ServerManufactureSchematicObjectTemplate::getFloatParam

BoolParam *ServerManufactureSchematicObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index) {
    return ServerIntangibleObjectTemplate::getBoolParam(name, deepCheck, index);
}    //ServerManufactureSchematicObjectTemplate::getBoolParam

StringParam *ServerManufactureSchematicObjectTemplate::getStringParam(const char *name, bool deepCheck, int index) {
    switch (runtimeCrc(name)) {
        case constcrc("draftSchematic"): {
            if (index == 0) {
                if (deepCheck && !isParamLoaded(name, false, 0)) {
                    if (getBaseTemplate() != nullptr)
                        return getBaseTemplate()->getStringParam(name, deepCheck, index);
                    return nullptr;
                }
                return &m_draftSchematic;
            }
            fprintf(stderr, "trying to access single-parameter \"draftSchematic\" as an array\n");
        }
            break;
        case constcrc("creator"): {
            if (index == 0) {
                if (deepCheck && !isParamLoaded(name, false, 0)) {
                    if (getBaseTemplate() != nullptr)
                        return getBaseTemplate()->getStringParam(name, deepCheck, index);
                    return nullptr;
                }
                return &m_creator;
            }
            fprintf(stderr, "trying to access single-parameter \"creator\" as an array\n");
        }
            break;
        default:
            return ServerIntangibleObjectTemplate::getStringParam(name, deepCheck, index);
            break;
    }
    return nullptr;
}    //ServerManufactureSchematicObjectTemplate::getStringParam

StringIdParam *ServerManufactureSchematicObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index) {
    return ServerIntangibleObjectTemplate::getStringIdParam(name, deepCheck, index);
}    //ServerManufactureSchematicObjectTemplate::getStringIdParam

VectorParam *ServerManufactureSchematicObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index) {
    return ServerIntangibleObjectTemplate::getVectorParam(name, deepCheck, index);
}    //ServerManufactureSchematicObjectTemplate::getVectorParam

DynamicVariableParam *
ServerManufactureSchematicObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index) {
    return ServerIntangibleObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}    //ServerManufactureSchematicObjectTemplate::getDynamicVariableParam

StructParamOT *ServerManufactureSchematicObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index) {
    switch (runtimeCrc(name)) {
        case constcrc("ingredients"): {
            if (index >= 0 && index < static_cast<int>(m_ingredients.size()))
                return m_ingredients[index];
            if (index == static_cast<int>(m_ingredients.size())) {
                StructParamOT *temp = new StructParamOT();
                m_ingredients.push_back(temp);
                return temp;
            }
            fprintf(stderr, "index for parameter \"ingredients\" out of bounds\n");
        }
            break;
        case constcrc("attributes"): {
            if (index >= 0 && index < static_cast<int>(m_attributes.size()))
                return m_attributes[index];
            if (index == static_cast<int>(m_attributes.size())) {
                StructParamOT *temp = new StructParamOT();
                m_attributes.push_back(temp);
                return temp;
            }
            fprintf(stderr, "index for parameter \"attributes\" out of bounds\n");
        }
            break;
        default:
            return ServerIntangibleObjectTemplate::getStructParamOT(name, deepCheck, index);
            break;
    }
    return nullptr;
}    //ServerManufactureSchematicObjectTemplate::getStructParamOT

TriggerVolumeParam *
ServerManufactureSchematicObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index) {
    return ServerIntangibleObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}    //ServerManufactureSchematicObjectTemplate::getTriggerVolumeParam

void ServerManufactureSchematicObjectTemplate::initStructParamOT(StructParamOT &param, const char *name) {
    if (param.isInitialized())
        return;
    switch (runtimeCrc(name)) {
        case constcrc("ingredients"):
            param.setValue(new _IngredientSlot(""));
            break;
        case constcrc("attributes"):
            param.setValue(new _SchematicAttribute(""));
            break;
        default:
            ServerIntangibleObjectTemplate::initStructParamOT(param, name);
            break;
    }
}    // ServerManufactureSchematicObjectTemplate::initStructParamOT

void ServerManufactureSchematicObjectTemplate::setAsEmptyList(const char *name) {
    switch (runtimeCrc(name)) {
        case constcrc("ingredients"): {
            m_ingredients.clear();
            m_ingredientsLoaded = true;
        }
            break;
        case constcrc("attributes"): {
            m_attributes.clear();
            m_attributesLoaded = true;
        }
            break;
        default:
            ServerIntangibleObjectTemplate::setAsEmptyList(name);
            break;
    }
}    // ServerManufactureSchematicObjectTemplate::setAsEmptyList

void ServerManufactureSchematicObjectTemplate::setAppend(const char *name) {
    switch (runtimeCrc(name)) {
        case constcrc("ingredients"):
            m_ingredientsAppend = true;
            break;
        case constcrc("attributes"):
            m_attributesAppend = true;
            break;
        default:
            ServerIntangibleObjectTemplate::setAppend(name);
            break;
    }
}    // ServerManufactureSchematicObjectTemplate::setAppend

bool ServerManufactureSchematicObjectTemplate::isAppend(const char *name) const {
    switch (runtimeCrc(name)) {
        case constcrc("ingredients"):
            return m_ingredientsAppend;
            break;
        case constcrc("attributes"):
            return m_attributesAppend;
            break;
        default:
            return ServerIntangibleObjectTemplate::isAppend(name);
            break;
    }
}    // ServerManufactureSchematicObjectTemplate::isAppend


int ServerManufactureSchematicObjectTemplate::getListLength(const char *name) const {
    switch (runtimeCrc(name)) {
        case constcrc("ingredients"): {
            return m_ingredients.size();
        }
            break;
        case constcrc("attributes"): {
            return m_attributes.size();
        }
            break;
        default:
            return ServerIntangibleObjectTemplate::getListLength(name);
            break;
    }
}    // ServerManufactureSchematicObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerManufactureSchematicObjectTemplate::load(Iff &file) {
    static const int MAX_NAME_SIZE = 256;
    char paramName[MAX_NAME_SIZE];

    if (file.getCurrentName() != ServerManufactureSchematicObjectTemplate_tag) {
        ServerIntangibleObjectTemplate::load(file);
        return;
    }

    file.enterForm();
    m_templateVersion = file.getCurrentName();
    if (m_templateVersion == TAG(D, E, R, V)) {
        file.enterForm();
        file.enterChunk();
        std::string baseFilename;
        file.read_string(baseFilename);
        file.exitChunk();
        const ObjectTemplate *base = ObjectTemplateList::fetch(baseFilename);
        DEBUG_WARNING(base == nullptr, ("was unable to load base template %s", baseFilename.c_str()));
        if (m_baseData == base && base != nullptr)
            base->releaseReference();
        else {
            if (m_baseData != nullptr)
                m_baseData->releaseReference();
            m_baseData = base;
        }
        file.exitForm();
        m_templateVersion = file.getCurrentName();
    }
    if (getHighestTemplateVersion() != TAG(0, 0, 0, 0)) {
        if (DataLint::isEnabled())
            DEBUG_WARNING(true, ("template %s version out of date", file.getFileName()));
    }

    file.enterForm();

    file.enterChunk();
    int paramCount = file.read_int32();
    file.exitChunk();
    for (int i = 0; i < paramCount; ++i) {
        file.enterChunk();
        file.read_string(paramName, MAX_NAME_SIZE);

        switch (runtimeCrc(paramName)) {
            case constcrc("draftSchematic"):
                m_draftSchematic.loadFromIff(file);
                break;
            case constcrc("creator"):
                m_creator.loadFromIff(file);
                break;
            case constcrc("ingredients"): {
                std::vector<StructParamOT *>::iterator iter;
                for (iter = m_ingredients.begin(); iter != m_ingredients.end(); ++iter) {
                    delete *iter;
                    *iter = nullptr;
                }
                m_ingredients.clear();
                m_ingredientsAppend = file.read_bool8();
                int listCount = file.read_int32();
                for (int j = 0; j < listCount; ++j) {
                    StructParamOT *newData = new StructParamOT;
                    newData->loadFromIff(file);
                    m_ingredients.push_back(newData);
                }
                m_ingredientsLoaded = true;
            }
                break;
            case constcrc("itemCount"):
                m_itemCount.loadFromIff(file);
                break;
            case constcrc("attributes"): {
                std::vector<StructParamOT *>::iterator iter;
                for (iter = m_attributes.begin(); iter != m_attributes.end(); ++iter) {
                    delete *iter;
                    *iter = nullptr;
                }
                m_attributes.clear();
                m_attributesAppend = file.read_bool8();
                int listCount = file.read_int32();
                for (int j = 0; j < listCount; ++j) {
                    StructParamOT *newData = new StructParamOT;
                    newData->loadFromIff(file);
                    m_attributes.push_back(newData);
                }
                m_attributesLoaded = true;
            }
                break;
        }
        file.exitChunk(true);
    }

    file.exitForm();
    ServerIntangibleObjectTemplate::load(file);
    file.exitForm();
    return;
}    // ServerManufactureSchematicObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerManufactureSchematicObjectTemplate::save(Iff &file) {
    int count;

    file.insertForm(ServerManufactureSchematicObjectTemplate_tag);
    if (m_baseTemplateName.size() != 0) {
        file.insertForm(TAG(D, E, R, V));
        file.insertChunk(TAG(X, X, X, X));
        file.insertChunkData(m_baseTemplateName.c_str(), m_baseTemplateName.size() + 1);
        file.exitChunk();
        file.exitForm();
    }
    file.insertForm(TAG(0, 0, 0, 0));
    file.allowNonlinearFunctions();

    int paramCount = 0;

    // save draftSchematic
    file.insertChunk(TAG(X, X, X, X));
    file.insertChunkString("draftSchematic");
    m_draftSchematic.saveToIff(file);
    file.exitChunk();
    ++paramCount;
    // save creator
    file.insertChunk(TAG(X, X, X, X));
    file.insertChunkString("creator");
    m_creator.saveToIff(file);
    file.exitChunk();
    ++paramCount;
    if (!m_ingredientsLoaded) {
        // mark the list as empty and extending the base list
        m_ingredientsAppend = true;
    }
    // save ingredients
    file.insertChunk(TAG(X, X, X, X));
    file.insertChunkString("ingredients");
    file.insertChunkData(&m_ingredientsAppend, sizeof(bool));
    count = m_ingredients.size();
    file.insertChunkData(&count, sizeof(count));
    {
        for (int i = 0; i < count; ++i)
            m_ingredients[i]->saveToIff(file);
    }
    file.exitChunk();
    ++paramCount;
    // save itemCount
    file.insertChunk(TAG(X, X, X, X));
    file.insertChunkString("itemCount");
    m_itemCount.saveToIff(file);
    file.exitChunk();
    ++paramCount;
    if (!m_attributesLoaded) {
        // mark the list as empty and extending the base list
        m_attributesAppend = true;
    }
    // save attributes
    file.insertChunk(TAG(X, X, X, X));
    file.insertChunkString("attributes");
    file.insertChunkData(&m_attributesAppend, sizeof(bool));
    count = m_attributes.size();
    file.insertChunkData(&count, sizeof(count));
    {
        for (int i = 0; i < count; ++i)
            m_attributes[i]->saveToIff(file);
    }
    file.exitChunk();
    ++paramCount;

    // write number of parameters
    file.goToTopOfForm();
    file.insertChunk(TAG(P, C, N, T));
    file.insertChunkData(&paramCount, sizeof(paramCount));
    file.exitChunk();

    file.exitForm(true);
    ServerIntangibleObjectTemplate::save(file);
    file.exitForm();
}    // ServerManufactureSchematicObjectTemplate::save


//=============================================================================
// class ServerManufactureSchematicObjectTemplate::_IngredientSlot

/**
 * Class constructor.
 */
ServerManufactureSchematicObjectTemplate::_IngredientSlot::_IngredientSlot(const std::string &filename)
        : TpfTemplate(filename) {
}    // ServerManufactureSchematicObjectTemplate::_IngredientSlot::_IngredientSlot

/**
 * Class destructor.
 */
ServerManufactureSchematicObjectTemplate::_IngredientSlot::~_IngredientSlot() {
}    // ServerManufactureSchematicObjectTemplate::_IngredientSlot::~_IngredientSlot

/**
 * Static function used to register this template.
 */
void ServerManufactureSchematicObjectTemplate::_IngredientSlot::registerMe(void) {
    ObjectTemplateList::registerTemplate(_IngredientSlot_tag, create);
}    // ServerManufactureSchematicObjectTemplate::_IngredientSlot::registerMe

/**
 * Creates a ServerManufactureSchematicObjectTemplate::_IngredientSlot template.
 *
 * @return a new instance of the template
 */
ObjectTemplate *ServerManufactureSchematicObjectTemplate::_IngredientSlot::create(const std::string &filename) {
    return new ServerManufactureSchematicObjectTemplate::_IngredientSlot(filename);
}    // ServerManufactureSchematicObjectTemplate::_IngredientSlot::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag ServerManufactureSchematicObjectTemplate::_IngredientSlot::getId(void) const {
    return _IngredientSlot_tag;
}    // ServerManufactureSchematicObjectTemplate::_IngredientSlot::getId

CompilerIntegerParam *
ServerManufactureSchematicObjectTemplate::_IngredientSlot::getCompilerIntegerParam(const char *name, bool deepCheck,
                                                                                   int index) {
    return TpfTemplate::getCompilerIntegerParam(name, deepCheck, index);
}    //ServerManufactureSchematicObjectTemplate::_IngredientSlot::getCompilerIntegerParam

FloatParam *
ServerManufactureSchematicObjectTemplate::_IngredientSlot::getFloatParam(const char *name, bool deepCheck, int index) {
    return TpfTemplate::getFloatParam(name, deepCheck, index);
}    //ServerManufactureSchematicObjectTemplate::_IngredientSlot::getFloatParam

BoolParam *
ServerManufactureSchematicObjectTemplate::_IngredientSlot::getBoolParam(const char *name, bool deepCheck, int index) {
    return TpfTemplate::getBoolParam(name, deepCheck, index);
}    //ServerManufactureSchematicObjectTemplate::_IngredientSlot::getBoolParam

StringParam *
ServerManufactureSchematicObjectTemplate::_IngredientSlot::getStringParam(const char *name, bool deepCheck, int index) {
    return TpfTemplate::getStringParam(name, deepCheck, index);
}    //ServerManufactureSchematicObjectTemplate::_IngredientSlot::getStringParam

StringIdParam *
ServerManufactureSchematicObjectTemplate::_IngredientSlot::getStringIdParam(const char *name, bool deepCheck,
                                                                            int index) {
    if (strcmp(name, "name") == 0) {
        if (index == 0) {
            if (deepCheck && !isParamLoaded(name, false, 0)) {
                if (getBaseTemplate() != nullptr)
                    return getBaseTemplate()->getStringIdParam(name, deepCheck, index);
                return nullptr;
            }
            return &m_name;
        }
        fprintf(stderr, "trying to access single-parameter \"name\" as an array\n");
    } else
        return TpfTemplate::getStringIdParam(name, deepCheck, index);
    return nullptr;
}    //ServerManufactureSchematicObjectTemplate::_IngredientSlot::getStringIdParam

VectorParam *
ServerManufactureSchematicObjectTemplate::_IngredientSlot::getVectorParam(const char *name, bool deepCheck, int index) {
    return TpfTemplate::getVectorParam(name, deepCheck, index);
}    //ServerManufactureSchematicObjectTemplate::_IngredientSlot::getVectorParam

DynamicVariableParam *
ServerManufactureSchematicObjectTemplate::_IngredientSlot::getDynamicVariableParam(const char *name, bool deepCheck,
                                                                                   int index) {
    return TpfTemplate::getDynamicVariableParam(name, deepCheck, index);
}    //ServerManufactureSchematicObjectTemplate::_IngredientSlot::getDynamicVariableParam

StructParamOT *
ServerManufactureSchematicObjectTemplate::_IngredientSlot::getStructParamOT(const char *name, bool deepCheck,
                                                                            int index) {
    if (strcmp(name, "ingredient") == 0) {
        if (index == 0) {
            if (deepCheck && !isParamLoaded(name, false, 0)) {
                if (getBaseTemplate() != nullptr)
                    return getBaseTemplate()->getStructParamOT(name, deepCheck, index);
                return nullptr;
            }
            return &m_ingredient;
        }
        fprintf(stderr, "trying to access single-parameter \"ingredient\" as an array\n");
    } else
        return TpfTemplate::getStructParamOT(name, deepCheck, index);
    return nullptr;
}    //ServerManufactureSchematicObjectTemplate::_IngredientSlot::getStructParamOT

TriggerVolumeParam *
ServerManufactureSchematicObjectTemplate::_IngredientSlot::getTriggerVolumeParam(const char *name, bool deepCheck,
                                                                                 int index) {
    return TpfTemplate::getTriggerVolumeParam(name, deepCheck, index);
}    //ServerManufactureSchematicObjectTemplate::_IngredientSlot::getTriggerVolumeParam

void
ServerManufactureSchematicObjectTemplate::_IngredientSlot::initStructParamOT(StructParamOT &param, const char *name) {
    if (param.isInitialized())
        return;
    if (strcmp(name, "ingredient") == 0)
        param.setValue(new _Ingredient(""));
    else
        TpfTemplate::initStructParamOT(param, name);
}    // ServerManufactureSchematicObjectTemplate::_IngredientSlot::initStructParamOT

void ServerManufactureSchematicObjectTemplate::_IngredientSlot::setAsEmptyList(const char *name) {
    TpfTemplate::setAsEmptyList(name);
}    // ServerManufactureSchematicObjectTemplate::_IngredientSlot::setAsEmptyList

void ServerManufactureSchematicObjectTemplate::_IngredientSlot::setAppend(const char *name) {
    TpfTemplate::setAppend(name);
}    // ServerManufactureSchematicObjectTemplate::_IngredientSlot::setAppend

bool ServerManufactureSchematicObjectTemplate::_IngredientSlot::isAppend(const char *name) const {
    return TpfTemplate::isAppend(name);
}    // ServerManufactureSchematicObjectTemplate::_IngredientSlot::isAppend


int ServerManufactureSchematicObjectTemplate::_IngredientSlot::getListLength(const char *name) const {
    return TpfTemplate::getListLength(name);
}    // ServerManufactureSchematicObjectTemplate::_IngredientSlot::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void ServerManufactureSchematicObjectTemplate::_IngredientSlot::load(Iff &file) {
    static const int MAX_NAME_SIZE = 256;
    char paramName[MAX_NAME_SIZE];

    file.enterForm();

    file.enterChunk();
    int paramCount = file.read_int32();
    file.exitChunk();
    for (int i = 0; i < paramCount; ++i) {
        file.enterChunk();
        file.read_string(paramName, MAX_NAME_SIZE);

	switch(runtimeCrc(paramName)) {
		case constcrc("name"):
	            m_name.loadFromIff(file);
		    break;
	        case constcrc("ingredient"):
        	    m_ingredient.loadFromIff(file);
		    break;
	}

        file.exitChunk(true);
    }

    file.exitForm();
    UNREF(file);
}    // ServerManufactureSchematicObjectTemplate::_IngredientSlot::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void ServerManufactureSchematicObjectTemplate::_IngredientSlot::save(Iff &file) {
    int count;

    file.insertForm(_IngredientSlot_tag);

    int paramCount = 0;

    // save name
    file.insertChunk(TAG(X, X, X, X));
    file.insertChunkString("name");
    m_name.saveToIff(file);
    file.exitChunk();
    ++paramCount;
    // save ingredient
    file.insertChunk(TAG(X, X, X, X));
    file.insertChunkString("ingredient");
    m_ingredient.saveToIff(file);
    file.exitChunk();
    ++paramCount;

    // write number of parameters
    file.goToTopOfForm();
    file.insertChunk(TAG(P, C, N, T));
    file.insertChunkData(&paramCount, sizeof(paramCount));
    file.exitChunk();

    file.exitForm(true);
    UNREF(count);
}    // ServerManufactureSchematicObjectTemplate::_IngredientSlot::save

//@END TFD
