//========================================================================
//
// SharedCreatureObjectTemplate.cpp
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
#include "SharedCreatureObjectTemplate.h"
#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedTemplateDefinition/ObjectTemplate.h"
#include <stdio.h>

#include "sharedFoundation/CrcConstexpr.hpp"


/**
 * Class constructor.
 */
SharedCreatureObjectTemplate::SharedCreatureObjectTemplate(const std::string &filename)
//@BEGIN TFD INIT
        : SharedTangibleObjectTemplate(filename)
//@END TFD INIT
{
}    // SharedCreatureObjectTemplate::SharedCreatureObjectTemplate

/**
 * Class destructor.
 */
SharedCreatureObjectTemplate::~SharedCreatureObjectTemplate() {
//@BEGIN TFD CLEANUP
//@END TFD CLEANUP
}    // SharedCreatureObjectTemplate::~SharedCreatureObjectTemplate

/**
 * Static function used to register this template.
 */
void SharedCreatureObjectTemplate::registerMe(void) {
    ObjectTemplateList::registerTemplate(SharedCreatureObjectTemplate_tag, create);
}    // SharedCreatureObjectTemplate::registerMe

/**
 * Creates a SharedCreatureObjectTemplate template.
 *
 * @return a new instance of the template
 */
ObjectTemplate *SharedCreatureObjectTemplate::create(const std::string &filename) {
    return new SharedCreatureObjectTemplate(filename);
}    // SharedCreatureObjectTemplate::create

/**
 * Returns the template id.
 *
 * @return the template id
 */
Tag SharedCreatureObjectTemplate::getId(void) const {
    return SharedCreatureObjectTemplate_tag;
}    // SharedCreatureObjectTemplate::getId

/**
 * Returns this template's version.
 *
 * @return the version
 */
Tag SharedCreatureObjectTemplate::getTemplateVersion(void) const {
    return m_templateVersion;
} // SharedCreatureObjectTemplate::getTemplateVersion

/**
 * Returns the highest version of this template or it's base templates.
 *
 * @return the highest version
 */
Tag SharedCreatureObjectTemplate::getHighestTemplateVersion(void) const {
    if (m_baseData == NULL)
        return m_templateVersion;
    const SharedCreatureObjectTemplate *base = dynamic_cast<const SharedCreatureObjectTemplate *>(m_baseData);
    if (base == NULL)
        return m_templateVersion;
    return std::max(m_templateVersion, base->getHighestTemplateVersion());
} // SharedCreatureObjectTemplate::getHighestTemplateVersion

//@BEGIN TFD
CompilerIntegerParam *
SharedCreatureObjectTemplate::getCompilerIntegerParam(const char *name, bool deepCheck, int index) {
    switch (runtimeCrc(name)) {
        case constcrc("gender"): {
            if (index == 0) {
                if (deepCheck && !isParamLoaded(name, false, 0)) {
                    if (getBaseTemplate() != nullptr)
                        return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
                    return nullptr;
                }
                return &m_gender;
            }
            fprintf(stderr, "trying to access single-parameter \"gender\" as an array\n");
        }
            name;
        case constcrc("niche"): {
            if (index == 0) {
                if (deepCheck && !isParamLoaded(name, false, 0)) {
                    if (getBaseTemplate() != nullptr)
                        return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
                    return nullptr;
                }
                return &m_niche;
            }
            fprintf(stderr, "trying to access single-parameter \"niche\" as an array\n");
        }
            name;
        case constcrc("species"): {
            if (index == 0) {
                if (deepCheck && !isParamLoaded(name, false, 0)) {
                    if (getBaseTemplate() != nullptr)
                        return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
                    return nullptr;
                }
                return &m_species;
            }
            fprintf(stderr, "trying to access single-parameter \"species\" as an array\n");
        }
            name;
        case constcrc("race"): {
            if (index == 0) {
                if (deepCheck && !isParamLoaded(name, false, 0)) {
                    if (getBaseTemplate() != nullptr)
                        return getBaseTemplate()->getCompilerIntegerParam(name, deepCheck, index);
                    return nullptr;
                }
                return &m_race;
            }
            fprintf(stderr, "trying to access single-parameter \"race\" as an array\n");
        }
            name;
        default:
            return SharedTangibleObjectTemplate::getCompilerIntegerParam(name, deepCheck, index);
            break;
    }
    return nullptr;
}    //SharedCreatureObjectTemplate::getCompilerIntegerParam

FloatParam *SharedCreatureObjectTemplate::getFloatParam(const char *name, bool deepCheck, int index) {
    switch (runtimeCrc(name)) {
        case constcrc("acceleration"): {
            if (index >= 0 && index < 2) {
                if (deepCheck && !isParamLoaded(name, false, index)) {
                    if (getBaseTemplate() != nullptr)
                        return getBaseTemplate()->getFloatParam(name, deepCheck, index);
                    return nullptr;
                }
                return &m_acceleration[index];
            }
            fprintf(stderr, "index for parameter \"acceleration\" out of bounds\n");
        }
            name;
        case constcrc("speed"): {
            if (index >= 0 && index < 2) {
                if (deepCheck && !isParamLoaded(name, false, index)) {
                    if (getBaseTemplate() != nullptr)
                        return getBaseTemplate()->getFloatParam(name, deepCheck, index);
                    return nullptr;
                }
                return &m_speed[index];
            }
            fprintf(stderr, "index for parameter \"speed\" out of bounds\n");
        }
            name;
        case constcrc("turnRate"): {
            if (index >= 0 && index < 2) {
                if (deepCheck && !isParamLoaded(name, false, index)) {
                    if (getBaseTemplate() != nullptr)
                        return getBaseTemplate()->getFloatParam(name, deepCheck, index);
                    return nullptr;
                }
                return &m_turnRate[index];
            }
            fprintf(stderr, "index for parameter \"turnRate\" out of bounds\n");
        }
            name;
        case constcrc("slopeModAngle"): {
            if (index == 0) {
                if (deepCheck && !isParamLoaded(name, false, 0)) {
                    if (getBaseTemplate() != nullptr)
                        return getBaseTemplate()->getFloatParam(name, deepCheck, index);
                    return nullptr;
                }
                return &m_slopeModAngle;
            }
            fprintf(stderr, "trying to access single-parameter \"slopeModAngle\" as an array\n");
        }
            name;
        case constcrc("slopeModPercent"): {
            if (index == 0) {
                if (deepCheck && !isParamLoaded(name, false, 0)) {
                    if (getBaseTemplate() != nullptr)
                        return getBaseTemplate()->getFloatParam(name, deepCheck, index);
                    return nullptr;
                }
                return &m_slopeModPercent;
            }
            fprintf(stderr, "trying to access single-parameter \"slopeModPercent\" as an array\n");
        }
            name;
        case constcrc("waterModPercent"): {
            if (index == 0) {
                if (deepCheck && !isParamLoaded(name, false, 0)) {
                    if (getBaseTemplate() != nullptr)
                        return getBaseTemplate()->getFloatParam(name, deepCheck, index);
                    return nullptr;
                }
                return &m_waterModPercent;
            }
            fprintf(stderr, "trying to access single-parameter \"waterModPercent\" as an array\n");
        }
            name;
        case constcrc("stepHeight"): {
            if (index == 0) {
                if (deepCheck && !isParamLoaded(name, false, 0)) {
                    if (getBaseTemplate() != nullptr)
                        return getBaseTemplate()->getFloatParam(name, deepCheck, index);
                    return nullptr;
                }
                return &m_stepHeight;
            }
            fprintf(stderr, "trying to access single-parameter \"stepHeight\" as an array\n");
        }
            name;
        case constcrc("collisionHeight"): {
            if (index == 0) {
                if (deepCheck && !isParamLoaded(name, false, 0)) {
                    if (getBaseTemplate() != nullptr)
                        return getBaseTemplate()->getFloatParam(name, deepCheck, index);
                    return nullptr;
                }
                return &m_collisionHeight;
            }
            fprintf(stderr, "trying to access single-parameter \"collisionHeight\" as an array\n");
        }
            name;
        case constcrc("collisionRadius"): {
            if (index == 0) {
                if (deepCheck && !isParamLoaded(name, false, 0)) {
                    if (getBaseTemplate() != nullptr)
                        return getBaseTemplate()->getFloatParam(name, deepCheck, index);
                    return nullptr;
                }
                return &m_collisionRadius;
            }
            fprintf(stderr, "trying to access single-parameter \"collisionRadius\" as an array\n");
        }
            name;
        case constcrc("swimHeight"): {
            if (index == 0) {
                if (deepCheck && !isParamLoaded(name, false, 0)) {
                    if (getBaseTemplate() != nullptr)
                        return getBaseTemplate()->getFloatParam(name, deepCheck, index);
                    return nullptr;
                }
                return &m_swimHeight;
            }
            fprintf(stderr, "trying to access single-parameter \"swimHeight\" as an array\n");
        }
            name;
        case constcrc("warpTolerance"): {
            if (index == 0) {
                if (deepCheck && !isParamLoaded(name, false, 0)) {
                    if (getBaseTemplate() != nullptr)
                        return getBaseTemplate()->getFloatParam(name, deepCheck, index);
                    return nullptr;
                }
                return &m_warpTolerance;
            }
            fprintf(stderr, "trying to access single-parameter \"warpTolerance\" as an array\n");
        }
            name;
        case constcrc("collisionOffsetX"): {
            if (index == 0) {
                if (deepCheck && !isParamLoaded(name, false, 0)) {
                    if (getBaseTemplate() != nullptr)
                        return getBaseTemplate()->getFloatParam(name, deepCheck, index);
                    return nullptr;
                }
                return &m_collisionOffsetX;
            }
            fprintf(stderr, "trying to access single-parameter \"collisionOffsetX\" as an array\n");
        }
            name;
        case constcrc("collisionOffsetZ"): {
            if (index == 0) {
                if (deepCheck && !isParamLoaded(name, false, 0)) {
                    if (getBaseTemplate() != nullptr)
                        return getBaseTemplate()->getFloatParam(name, deepCheck, index);
                    return nullptr;
                }
                return &m_collisionOffsetZ;
            }
            fprintf(stderr, "trying to access single-parameter \"collisionOffsetZ\" as an array\n");
        }
            name;
        case constcrc("collisionLength"): {
            if (index == 0) {
                if (deepCheck && !isParamLoaded(name, false, 0)) {
                    if (getBaseTemplate() != nullptr)
                        return getBaseTemplate()->getFloatParam(name, deepCheck, index);
                    return nullptr;
                }
                return &m_collisionLength;
            }
            fprintf(stderr, "trying to access single-parameter \"collisionLength\" as an array\n");
        }
            name;
        case constcrc("cameraHeight"): {
            if (index == 0) {
                if (deepCheck && !isParamLoaded(name, false, 0)) {
                    if (getBaseTemplate() != nullptr)
                        return getBaseTemplate()->getFloatParam(name, deepCheck, index);
                    return nullptr;
                }
                return &m_cameraHeight;
            }
            fprintf(stderr, "trying to access single-parameter \"cameraHeight\" as an array\n");
        }
            name;
        default:
            return SharedTangibleObjectTemplate::getFloatParam(name, deepCheck, index);
            break;
    }
    return nullptr;
}    //SharedCreatureObjectTemplate::getFloatParam

BoolParam *SharedCreatureObjectTemplate::getBoolParam(const char *name, bool deepCheck, int index) {
    switch (runtimeCrc(name)) {
        case constcrc("postureAlignToTerrain"): {
            if (index >= 0 && index < 15) {
                if (deepCheck && !isParamLoaded(name, false, index)) {
                    if (getBaseTemplate() != nullptr)
                        return getBaseTemplate()->getBoolParam(name, deepCheck, index);
                    return nullptr;
                }
                return &m_postureAlignToTerrain[index];
            }
            fprintf(stderr, "index for parameter \"postureAlignToTerrain\" out of bounds\n");
        }
            break;
        default:
            return SharedTangibleObjectTemplate::getBoolParam(name, deepCheck, index);
            break;
    }
    return nullptr;
}    //SharedCreatureObjectTemplate::getBoolParam

StringParam *SharedCreatureObjectTemplate::getStringParam(const char *name, bool deepCheck, int index) {
    switch (runtimeCrc(name)) {
        case constcrc("animationMapFilename"): {
            if (index == 0) {
                if (deepCheck && !isParamLoaded(name, false, 0)) {
                    if (getBaseTemplate() != nullptr)
                        return getBaseTemplate()->getStringParam(name, deepCheck, index);
                    return nullptr;
                }
                return &m_animationMapFilename;
            }
            fprintf(stderr, "trying to access single-parameter \"animationMapFilename\" as an array\n");
        }
            break;
        case constcrc("movementDatatable"): {
            if (index == 0) {
                if (deepCheck && !isParamLoaded(name, false, 0)) {
                    if (getBaseTemplate() != nullptr)
                        return getBaseTemplate()->getStringParam(name, deepCheck, index);
                    return nullptr;
                }
                return &m_movementDatatable;
            }
            fprintf(stderr, "trying to access single-parameter \"movementDatatable\" as an array\n");
        }
            break;
        default:
            return SharedTangibleObjectTemplate::getStringParam(name, deepCheck, index);
            break;
    }
    return nullptr;
}    //SharedCreatureObjectTemplate::getStringParam

StringIdParam *SharedCreatureObjectTemplate::getStringIdParam(const char *name, bool deepCheck, int index) {
    return SharedTangibleObjectTemplate::getStringIdParam(name, deepCheck, index);
}    //SharedCreatureObjectTemplate::getStringIdParam

VectorParam *SharedCreatureObjectTemplate::getVectorParam(const char *name, bool deepCheck, int index) {
    return SharedTangibleObjectTemplate::getVectorParam(name, deepCheck, index);
}    //SharedCreatureObjectTemplate::getVectorParam

DynamicVariableParam *
SharedCreatureObjectTemplate::getDynamicVariableParam(const char *name, bool deepCheck, int index) {
    return SharedTangibleObjectTemplate::getDynamicVariableParam(name, deepCheck, index);
}    //SharedCreatureObjectTemplate::getDynamicVariableParam

StructParamOT *SharedCreatureObjectTemplate::getStructParamOT(const char *name, bool deepCheck, int index) {
    return SharedTangibleObjectTemplate::getStructParamOT(name, deepCheck, index);
}    //SharedCreatureObjectTemplate::getStructParamOT

TriggerVolumeParam *SharedCreatureObjectTemplate::getTriggerVolumeParam(const char *name, bool deepCheck, int index) {
    return SharedTangibleObjectTemplate::getTriggerVolumeParam(name, deepCheck, index);
}    //SharedCreatureObjectTemplate::getTriggerVolumeParam

void SharedCreatureObjectTemplate::initStructParamOT(StructParamOT &param, const char *name) {
    if (param.isInitialized())
        return;
    SharedTangibleObjectTemplate::initStructParamOT(param, name);
}    // SharedCreatureObjectTemplate::initStructParamOT

void SharedCreatureObjectTemplate::setAsEmptyList(const char *name) {
    SharedTangibleObjectTemplate::setAsEmptyList(name);
}    // SharedCreatureObjectTemplate::setAsEmptyList

void SharedCreatureObjectTemplate::setAppend(const char *name) {
    SharedTangibleObjectTemplate::setAppend(name);
}    // SharedCreatureObjectTemplate::setAppend

bool SharedCreatureObjectTemplate::isAppend(const char *name) const {
    return SharedTangibleObjectTemplate::isAppend(name);
}    // SharedCreatureObjectTemplate::isAppend


int SharedCreatureObjectTemplate::getListLength(const char *name) const {
    switch (runtimeCrc(name)) {
        case constcrc("acceleration"): {
            return sizeof(m_acceleration) / sizeof(FloatParam);
        }
            break;
        case constcrc("speed"): {
            return sizeof(m_speed) / sizeof(FloatParam);
        }
            break;
        case constcrc("turnRate"): {
            return sizeof(m_turnRate) / sizeof(FloatParam);
        }
            break;
        case constcrc("postureAlignToTerrain"): {
            return sizeof(m_postureAlignToTerrain) / sizeof(BoolParam);
        }
            break;
        default:
            return SharedTangibleObjectTemplate::getListLength(name);
            break;
    }
}    // SharedCreatureObjectTemplate::getListLength

/**
 * Loads the template data from an iff file. We should already be in the form
 * for this template.
 *
 * @param file		file to load from
 */
void SharedCreatureObjectTemplate::load(Iff &file) {
    static const int MAX_NAME_SIZE = 256;
    char paramName[MAX_NAME_SIZE];

    if (file.getCurrentName() != SharedCreatureObjectTemplate_tag) {
        SharedTangibleObjectTemplate::load(file);
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
    if (getHighestTemplateVersion() != TAG(0, 0, 1, 3)) {
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
            case constcrc("gender"):
                m_gender.loadFromIff(file);
                break;
            case constcrc("niche"):
                m_niche.loadFromIff(file);
                break;
            case constcrc("species"):
                m_species.loadFromIff(file);
                break;
            case constcrc("race"):
                m_race.loadFromIff(file);
                break;
            case constcrc("acceleration"): {
                int listCount = file.read_int32();
                DEBUG_WARNING(listCount != 2,
                              ("Template %s: read array size of %d for array \"acceleration\" of size 2, reading values anyway", file.getFileName(), listCount));
                int j;
                for (j = 0; j < 2 && j < listCount; ++j)
                    m_acceleration[j].loadFromIff(file);
                // if there are more params for acceleration read and dump them
                for (; j < listCount; ++j) {
                    FloatParam dummy;
                    dummy.loadFromIff(file);
                }
            }
                break;
            case constcrc("speed"): {
                int listCount = file.read_int32();
                DEBUG_WARNING(listCount != 2,
                              ("Template %s: read array size of %d for array \"speed\" of size 2, reading values anyway", file.getFileName(), listCount));
                int j;
                for (j = 0; j < 2 && j < listCount; ++j)
                    m_speed[j].loadFromIff(file);
                // if there are more params for speed read and dump them
                for (; j < listCount; ++j) {
                    FloatParam dummy;
                    dummy.loadFromIff(file);
                }
            }
                break;
            case constcrc("turnRate"): {
                int listCount = file.read_int32();
                DEBUG_WARNING(listCount != 2,
                              ("Template %s: read array size of %d for array \"turnRate\" of size 2, reading values anyway", file.getFileName(), listCount));
                int j;
                for (j = 0; j < 2 && j < listCount; ++j)
                    m_turnRate[j].loadFromIff(file);
                // if there are more params for turnRate read and dump them
                for (; j < listCount; ++j) {
                    FloatParam dummy;
                    dummy.loadFromIff(file);
                }
            }
                break;
            case constcrc("animationMapFilename"):
                m_animationMapFilename.loadFromIff(file);
                break;
            case constcrc("slopeModAngle"):
                m_slopeModAngle.loadFromIff(file);
                break;
            case constcrc("slopeModPercent"):
                m_slopeModPercent.loadFromIff(file);
                break;
            case constcrc("waterModPercent"):
                m_waterModPercent.loadFromIff(file);
                break;
            case constcrc("stepHeight"):
                m_stepHeight.loadFromIff(file);
                break;
            case constcrc("collisionHeight"):
                m_collisionHeight.loadFromIff(file);
                break;
            case constcrc("collisionRadius"):
                m_collisionRadius.loadFromIff(file);
                break;
            case constcrc("movementDatatable"):
                m_movementDatatable.loadFromIff(file);
                break;
            case constcrc("postureAlignToTerrain"): {
                int listCount = file.read_int32();
                DEBUG_WARNING(listCount != 15,
                              ("Template %s: read array size of %d for array \"postureAlignToTerrain\" of size 15, reading values anyway", file.getFileName(), listCount));
                int j;
                for (j = 0; j < 15 && j < listCount; ++j)
                    m_postureAlignToTerrain[j].loadFromIff(file);
                // if there are more params for postureAlignToTerrain read and dump them
                for (; j < listCount; ++j) {
                    BoolParam dummy;
                    dummy.loadFromIff(file);
                }
            }
                break;
            case constcrc("swimHeight"):
                m_swimHeight.loadFromIff(file);
                break;
            case constcrc("warpTolerance"):
                m_warpTolerance.loadFromIff(file);
                break;
            case constcrc("collisionOffsetX"):
                m_collisionOffsetX.loadFromIff(file);
                break;
            case constcrc("collisionOffsetZ"):
                m_collisionOffsetZ.loadFromIff(file);
                break;
            case constcrc("collisionLength"):
                m_collisionLength.loadFromIff(file);
                break;
            case constcrc("cameraHeight"):
                m_cameraHeight.loadFromIff(file);
                break;
        }
        file.exitChunk(true);
    }

    file.exitForm();
    SharedTangibleObjectTemplate::load(file);
    file.exitForm();
    return;
}    // SharedCreatureObjectTemplate::load

/**
 * Saves the template data to an iff file.
 *
 * @param file			file to save to
 * @param location		file type (client or server)
 */
void SharedCreatureObjectTemplate::save(Iff &file) {
    int count;

    file.insertForm(SharedCreatureObjectTemplate_tag);
    if (m_baseTemplateName.size() != 0) {
        file.insertForm(TAG(D, E, R, V));
        file.insertChunk(TAG(X, X, X, X));
        file.insertChunkData(m_baseTemplateName.c_str(), m_baseTemplateName.size() + 1);
        file.exitChunk();
        file.exitForm();
    }
    file.insertForm(TAG(0, 0, 1, 3));
    file.allowNonlinearFunctions();

    int paramCount = 0;

    // save gender
    file.insertChunk(TAG(X, X, X, X));
    file.insertChunkString("gender");
    m_gender.saveToIff(file);
    file.exitChunk();
    ++paramCount;
    // save niche
    file.insertChunk(TAG(X, X, X, X));
    file.insertChunkString("niche");
    m_niche.saveToIff(file);
    file.exitChunk();
    ++paramCount;
    // save species
    file.insertChunk(TAG(X, X, X, X));
    file.insertChunkString("species");
    m_species.saveToIff(file);
    file.exitChunk();
    ++paramCount;
    // save race
    file.insertChunk(TAG(X, X, X, X));
    file.insertChunkString("race");
    m_race.saveToIff(file);
    file.exitChunk();
    ++paramCount;
    // save acceleration
    file.insertChunk(TAG(X, X, X, X));
    file.insertChunkString("acceleration");
    count = 2;
    file.insertChunkData(&count, sizeof(count));
    {
        for (int i = 0; i < 2; ++i)
            m_acceleration[i].saveToIff(file);
    }
    file.exitChunk();
    ++paramCount;
    // save speed
    file.insertChunk(TAG(X, X, X, X));
    file.insertChunkString("speed");
    count = 2;
    file.insertChunkData(&count, sizeof(count));
    {
        for (int i = 0; i < 2; ++i)
            m_speed[i].saveToIff(file);
    }
    file.exitChunk();
    ++paramCount;
    // save turnRate
    file.insertChunk(TAG(X, X, X, X));
    file.insertChunkString("turnRate");
    count = 2;
    file.insertChunkData(&count, sizeof(count));
    {
        for (int i = 0; i < 2; ++i)
            m_turnRate[i].saveToIff(file);
    }
    file.exitChunk();
    ++paramCount;
    // save animationMapFilename
    file.insertChunk(TAG(X, X, X, X));
    file.insertChunkString("animationMapFilename");
    m_animationMapFilename.saveToIff(file);
    file.exitChunk();
    ++paramCount;
    // save slopeModAngle
    file.insertChunk(TAG(X, X, X, X));
    file.insertChunkString("slopeModAngle");
    m_slopeModAngle.saveToIff(file);
    file.exitChunk();
    ++paramCount;
    // save slopeModPercent
    file.insertChunk(TAG(X, X, X, X));
    file.insertChunkString("slopeModPercent");
    m_slopeModPercent.saveToIff(file);
    file.exitChunk();
    ++paramCount;
    // save waterModPercent
    file.insertChunk(TAG(X, X, X, X));
    file.insertChunkString("waterModPercent");
    m_waterModPercent.saveToIff(file);
    file.exitChunk();
    ++paramCount;
    // save stepHeight
    file.insertChunk(TAG(X, X, X, X));
    file.insertChunkString("stepHeight");
    m_stepHeight.saveToIff(file);
    file.exitChunk();
    ++paramCount;
    // save collisionHeight
    file.insertChunk(TAG(X, X, X, X));
    file.insertChunkString("collisionHeight");
    m_collisionHeight.saveToIff(file);
    file.exitChunk();
    ++paramCount;
    // save collisionRadius
    file.insertChunk(TAG(X, X, X, X));
    file.insertChunkString("collisionRadius");
    m_collisionRadius.saveToIff(file);
    file.exitChunk();
    ++paramCount;
    // save movementDatatable
    file.insertChunk(TAG(X, X, X, X));
    file.insertChunkString("movementDatatable");
    m_movementDatatable.saveToIff(file);
    file.exitChunk();
    ++paramCount;
    // save postureAlignToTerrain
    file.insertChunk(TAG(X, X, X, X));
    file.insertChunkString("postureAlignToTerrain");
    count = 15;
    file.insertChunkData(&count, sizeof(count));
    {
        for (int i = 0; i < 15; ++i)
            m_postureAlignToTerrain[i].saveToIff(file);
    }
    file.exitChunk();
    ++paramCount;
    // save swimHeight
    file.insertChunk(TAG(X, X, X, X));
    file.insertChunkString("swimHeight");
    m_swimHeight.saveToIff(file);
    file.exitChunk();
    ++paramCount;
    // save warpTolerance
    file.insertChunk(TAG(X, X, X, X));
    file.insertChunkString("warpTolerance");
    m_warpTolerance.saveToIff(file);
    file.exitChunk();
    ++paramCount;
    // save collisionOffsetX
    file.insertChunk(TAG(X, X, X, X));
    file.insertChunkString("collisionOffsetX");
    m_collisionOffsetX.saveToIff(file);
    file.exitChunk();
    ++paramCount;
    // save collisionOffsetZ
    file.insertChunk(TAG(X, X, X, X));
    file.insertChunkString("collisionOffsetZ");
    m_collisionOffsetZ.saveToIff(file);
    file.exitChunk();
    ++paramCount;
    // save collisionLength
    file.insertChunk(TAG(X, X, X, X));
    file.insertChunkString("collisionLength");
    m_collisionLength.saveToIff(file);
    file.exitChunk();
    ++paramCount;
    // save cameraHeight
    file.insertChunk(TAG(X, X, X, X));
    file.insertChunkString("cameraHeight");
    m_cameraHeight.saveToIff(file);
    file.exitChunk();
    ++paramCount;

    // write number of parameters
    file.goToTopOfForm();
    file.insertChunk(TAG(P, C, N, T));
    file.insertChunkData(&paramCount, sizeof(paramCount));
    file.exitChunk();

    file.exitForm(true);
    SharedTangibleObjectTemplate::save(file);
    file.exitForm();
}    // SharedCreatureObjectTemplate::save

//@END TFD
