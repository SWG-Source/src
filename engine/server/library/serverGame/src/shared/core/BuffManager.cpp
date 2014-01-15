//======================================================================
//
// BuffManager.cpp
// copyright (c) 2003 Sony Online Entertainment
//
//======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/BuffManager.h"

#include "sharedFoundation/Crc.h"
#include "serverGame/CreatureObject.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/JavaLibrary.h"
#include "serverScript/ScriptParameters.h"
#include "serverUtility/ServerClock.h"
#include "sharedUtility/DataTable.h"
#include "sharedUtility/DataTableManager.h"
#include "UnicodeUtils.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedMath/Vector.h"
#include <vector>

//======================================================================

namespace BuffManagerNamespace
{
	bool m_installed = false;

	const int ms_numEffects = 5;

	const float BUFF_PERSIST_THRESHOLD_IN_SECONDS = 60.0f * 5.0f;

	DataTable *ms_buffDatatable;
	DataTable *ms_internalBuffDatatable;

	struct BuffRecord
	{
		std::string name;
		uint32 nameCrc;
		uint32 group1Crc;
		uint32 group2Crc;
		uint32 maxStacks;
		int state;
		float duration;
		std::string callback;
		bool visible;
		bool removeOnDeath;
		bool decayOnPvPDeath;
		bool playerRemovable;
		bool isCelestial;
		bool isPersistent;
		std::string effectParamString[ms_numEffects];
		uint32 effectParamStringCrc[ms_numEffects];
		float effectValue[ms_numEffects];
	};

	const std::string ms_columnName = "NAME";
	const std::string ms_columnGroup1 = "GROUP1";
	const std::string ms_columnGroup2 = "GROUP2";
	const std::string ms_columnDuration = "DURATION";
	const std::string ms_columnCallback = "CALLBACK";
	const std::string ms_columnVisible = "VISIBLE";
	const std::string ms_columnEffect1 = "EFFECT1_PARAM";
	const std::string ms_columnState = "STATE";
	const std::string ms_columnRemoveOnDeath = "REMOVE_ON_DEATH";
	const std::string ms_columnPlayerRemovable = "PLAYER_REMOVABLE";
	const std::string ms_columnDebuff = "DEBUFF";
	const std::string ms_columnIsCelestial = "IS_CELESTIAL";
	const std::string ms_columnMaxStacks = "MAX_STACKS";
	const std::string ms_columnIsPersistent = "IS_PERSISTENT";
	const std::string ms_columnDecayOnPvPDeath = "DECAY_ON_PVP_DEATH";

	std::map<uint32, BuffRecord> ms_buffRecords;

	struct EffectRecord
	{
		std::string name;
		uint32 nameCrc;
		std::string type;
		std::string subType;
		std::string other;
	};

	const std::string ms_columnInternalName = "NAME";
	const std::string ms_columnInternalType = "TYPE";
	const std::string ms_columnInternalSubType = "SUBTYPE";
	const std::string ms_columnInternalOther = "OTHER";

	std::map<uint32, EffectRecord> ms_effectRecords;

	const std::string ATTRIB_TYPE_HEALTH = "health";
	const std::string ATTRIB_TYPE_STAMINA = "stamina";
	const std::string ATTRIB_TYPE_CONSTITUTION = "constitution";
	const std::string ATTRIB_TYPE_WILLPOWER = "willpower";
	
	const std::string BUFF_HANDLER_SCRIPT = "systems.buff.buff_handler";
	const std::string BUFF_HANDLER_ADD_TRIGGER = "AddBuffHandler";
	const std::string BUFF_HANDLER_REMOVE_TRIGGER = "RemoveBuffHandler";
};

using namespace BuffManagerNamespace;


//======================================================================

void BuffManager::install()
{
	if(m_installed)
		return;
	m_installed = true;

	ms_buffDatatable = DataTableManager::getTable("datatables/buff/buff.iff", true);

	unsigned int const numRows = static_cast<unsigned int>(ms_buffDatatable->getNumRows());
	int const numColumns = ms_buffDatatable->getNumColumns();
	int nameColumn = ms_buffDatatable->findColumnNumber(ms_columnName);
	int group1Column = ms_buffDatatable->findColumnNumber(ms_columnGroup1);
	int group2Column = ms_buffDatatable->findColumnNumber(ms_columnGroup2);
	int durationColumn = ms_buffDatatable->findColumnNumber(ms_columnDuration);
	int callbackColumn = ms_buffDatatable->findColumnNumber(ms_columnCallback);
	int visibleColumn = ms_buffDatatable->findColumnNumber(ms_columnVisible);
	int effect1Column = ms_buffDatatable->findColumnNumber(ms_columnEffect1);
	int stateColumn = ms_buffDatatable->findColumnNumber(ms_columnState);
	int removeOnDeathColumn = ms_buffDatatable->findColumnNumber(ms_columnRemoveOnDeath);
	int playerRemovableColumn = ms_buffDatatable->findColumnNumber(ms_columnPlayerRemovable);
	int debuffColumn = ms_buffDatatable->findColumnNumber(ms_columnDebuff);
	int isCelestialColumn = ms_buffDatatable->findColumnNumber(ms_columnIsCelestial);
	int maxStacksColumn = ms_buffDatatable->findColumnNumber(ms_columnMaxStacks);
	int isPersistentColumn = ms_buffDatatable->findColumnNumber(ms_columnIsPersistent);
	int decayOnPvPDeath = ms_buffDatatable->findColumnNumber(ms_columnDecayOnPvPDeath);

	unsigned int i;

	for(i = 0; i < numRows; ++i)
	{
		BuffRecord buffRecord;

		buffRecord.name = ms_buffDatatable->getStringValue(nameColumn, i);
		buffRecord.nameCrc = ms_buffDatatable->getIntValue(nameColumn, i);
		buffRecord.group1Crc = ms_buffDatatable->getIntValue(group1Column, i);
		buffRecord.group2Crc = ms_buffDatatable->getIntValue(group2Column, i);
		buffRecord.duration = ms_buffDatatable->getFloatValue(durationColumn, i);
		buffRecord.callback = ms_buffDatatable->getStringValue(callbackColumn, i);
		buffRecord.visible = ms_buffDatatable->getIntValue(visibleColumn, i) != 0;
		buffRecord.state = ms_buffDatatable->getIntValue(stateColumn, i);
		buffRecord.removeOnDeath = ms_buffDatatable->getIntValue(removeOnDeathColumn, i) != 0;
		buffRecord.decayOnPvPDeath = ms_buffDatatable->getIntValue(decayOnPvPDeath, i) != 0;
		
		bool const isDebuff = ms_buffDatatable->getIntValue(debuffColumn, i) == 1;

		buffRecord.playerRemovable = (ms_buffDatatable->getIntValue(playerRemovableColumn, i) != 0) && !isDebuff;
		buffRecord.isCelestial = (ms_buffDatatable->getIntValue(isCelestialColumn, i) != 0);
		buffRecord.maxStacks = ms_buffDatatable->getIntValue(maxStacksColumn, i);
		buffRecord.isPersistent = ms_buffDatatable->getIntValue(isPersistentColumn, i) == 1;

		int curColumn = effect1Column;
		int j;
		for(j = 0; j < ms_numEffects; j++)
			buffRecord.effectParamStringCrc[j] = 0;
		for(j = 0; (j < ms_numEffects) && (curColumn < numColumns); j++)
		{
			buffRecord.effectParamString[j] = ms_buffDatatable->getStringValue(curColumn, i);
			buffRecord.effectParamStringCrc[j] = Crc::calculate(buffRecord.effectParamString[j].c_str());
			curColumn++;
			buffRecord.effectValue[j] = ms_buffDatatable->getFloatValue(curColumn, i);
			curColumn++;
		}
		
		ms_buffRecords.insert(std::make_pair(buffRecord.nameCrc, buffRecord));

	}

	ms_internalBuffDatatable = DataTableManager::getTable("datatables/buff/effect_mapping.iff", true);

	unsigned int const numInternalRows = static_cast<unsigned int>(ms_internalBuffDatatable->getNumRows());
	int internalNameColumn = ms_internalBuffDatatable->findColumnNumber(ms_columnInternalName);
	int internalTypeColumn = ms_internalBuffDatatable->findColumnNumber(ms_columnInternalType);
	int internalSubTypeColumn = ms_internalBuffDatatable->findColumnNumber(ms_columnInternalSubType);
	int internalOtherColumn = ms_internalBuffDatatable->findColumnNumber(ms_columnInternalOther);
	
	for(i = 0; i < numInternalRows; i++)
	{
		EffectRecord effectRecord;
		
		effectRecord.name = ms_internalBuffDatatable->getStringValue(internalNameColumn, i);
		effectRecord.nameCrc = Crc::calculate(effectRecord.name.c_str());
		effectRecord.type = ms_internalBuffDatatable->getStringValue(internalTypeColumn, i);
		effectRecord.subType = ms_internalBuffDatatable->getStringValue(internalSubTypeColumn, i);
		effectRecord.other = ms_internalBuffDatatable->getStringValue(internalOtherColumn, i);

		ms_effectRecords.insert(std::make_pair(effectRecord.nameCrc, effectRecord));
	}
	
}

//======================================================================

void BuffManager::remove()
{
	
}

//======================================================================

bool BuffManager::applyBuffEffects(CreatureObject *creatureObj, uint32 buffNameCrc, float duration, float dynamicValue, uint32 stackCount, NetworkId caster)
{
	return applyBuffEffectsInternal(creatureObj, buffNameCrc, duration, dynamicValue, false, stackCount, caster);
}

//======================================================================

bool BuffManager::reapplyBuffEffects(CreatureObject *creatureObj, uint32 buffNameCrc, float duration, float dynamicValue, uint32 stackCount, NetworkId caster)
{
	return applyBuffEffectsInternal(creatureObj, buffNameCrc, duration, dynamicValue, true, stackCount, caster);
}

//======================================================================

bool BuffManager::applyBuffEffectsInternal(CreatureObject *creatureObj, uint32 buffNameCrc, float duration, float dynamicValue, bool deleteFirst, uint32 stackCount, NetworkId caster)
{
	if(!creatureObj)
		return false;
	std::map<uint32, BuffRecord>::iterator it = ms_buffRecords.find(buffNameCrc);
	if(it == ms_buffRecords.end())
	{
		WARNING(true, ("Unknown buff crc %d\n", buffNameCrc));
		return false;
	}
	BuffRecord const &buffRecord = it->second;

	for(int effectNum = 0; effectNum < ms_numEffects; effectNum++)
	{
		uint32 effectCrc = buffRecord.effectParamStringCrc[effectNum];
		if(effectCrc)
		{
			std::map<uint32, EffectRecord>::iterator eit = ms_effectRecords.find(effectCrc);
			if(eit == ms_effectRecords.end())
			{
				WARNING(true, ("Unknown effect crc %d (%s)\n", effectCrc, buffRecord.effectParamString[effectNum].c_str()));
				return false;
			}
			EffectRecord const &effectRecord = eit->second;

			char tmp[512];
			snprintf(tmp, 512, "%s_%d", buffRecord.name.c_str(), effectNum);
			std::string effectName(tmp);

			if(deleteFirst)
				creatureObj->removeAttributeModifier(effectName);

			//Attach the standard buff handler script
			if(!creatureObj->getScriptObject()->hasScript(BUFF_HANDLER_SCRIPT))
				creatureObj->getScriptObject()->attachScript(BUFF_HANDLER_SCRIPT, false);

			//Construct script trigger call using type
			std::string funcName = effectRecord.type;
			funcName.append(BUFF_HANDLER_ADD_TRIGGER);

			//Call through with the effect name, duration, and value
			if (creatureObj->getScriptObject()->hasFunction(funcName))
			{
				ScriptParams params;
				params.addParam(effectName.c_str());
				params.addParam(effectRecord.subType.c_str());			
				//0.0f is the magic value that says use the datatable value
				params.addParam(duration == 0.0f ? buffRecord.duration : duration);
				float value = buffRecord.effectValue[effectNum];
				if((effectNum == 0) && (dynamicValue != 0.0f))
					value = dynamicValue;
				params.addParam(value);
				params.addParam(buffRecord.name.c_str());
				params.addParam(caster);

				creatureObj->getScriptObject()->callScriptBuffHandler(funcName, params);
			}

			//Detach buff handler script
			//NOTE: we can't do this until a lower-level bug in obj_id.java involving pending script
			// attach and detach commands is fixed.
			//creatureObj->getScriptObject()->detachScript(BUFF_HANDLER_SCRIPT);		
		}
	}
	return true;
}
	
//======================================================================

bool BuffManager::removeBuffEffects(CreatureObject *creatureObj, uint32 buffNameCrc, NetworkId caster)
{
	if(!creatureObj)
		return false;
	std::map<uint32, BuffRecord>::iterator it = ms_buffRecords.find(buffNameCrc);
	if(it == ms_buffRecords.end())
	{
		WARNING(true, ("Unknown buff crc %d\n", buffNameCrc));
		return false;
	}
	BuffRecord const &buffRecord = it->second;

	for(int effectNum = 0; effectNum < ms_numEffects; effectNum++)
	{
		uint32 effectCrc = buffRecord.effectParamStringCrc[effectNum];
		if(effectCrc)
		{
			std::map<uint32, EffectRecord>::iterator eit = ms_effectRecords.find(effectCrc);
			if(eit == ms_effectRecords.end())
			{
				WARNING(true, ("Unknown effect crc %d (%s)\n", effectCrc, buffRecord.effectParamString[effectNum].c_str()));
				return false;
			}
			EffectRecord const &effectRecord = eit->second;

			char tmp[512];
			snprintf(tmp, 512, "%s_%d", buffRecord.name.c_str(), effectNum);
			std::string effectName(tmp);

			//Attach the standard buff handler script
			if(!creatureObj->getScriptObject()->hasScript(BUFF_HANDLER_SCRIPT))
				creatureObj->getScriptObject()->attachScript(BUFF_HANDLER_SCRIPT, false);

			//Construct script trigger call using type
			std::string funcName = effectRecord.type;
			funcName.append(BUFF_HANDLER_REMOVE_TRIGGER);

			//Call through with the effect name, duration, and value
			if (creatureObj->getScriptObject()->hasFunction(funcName))
			{
				ScriptParams params;
				params.addParam(effectName.c_str());
				params.addParam(effectRecord.subType.c_str());
				params.addParam(buffRecord.duration);
				params.addParam(buffRecord.effectValue[effectNum]);
				params.addParam(buffRecord.name.c_str());
				params.addParam(caster);
				creatureObj->getScriptObject()->callScriptBuffHandler(funcName, params);
			}

			//Detach buff handler script
			//NOTE: we can't do this until a lower-level bug in obj_id.java involving pending script
			// attach and detach commands is fixed.
			//creatureObj->getScriptObject()->detachScript(BUFF_HANDLER_SCRIPT);
		}
	}
	return true;
}

//======================================================================

bool BuffManager::getIsBuffPersisted(uint32 buffNameCrc)
{
	std::map<uint32, BuffRecord>::iterator it = ms_buffRecords.find(buffNameCrc);
	if(it == ms_buffRecords.end())
	{
		WARNING(true, ("Unknown buff crc %d\n", buffNameCrc));
		return false;
	}
	BuffRecord const &buffRecord = it->second;
		
	// If a buff lasts longer than 5 minutes, persist it
	if(buffRecord.isPersistent && buffRecord.duration >= BUFF_PERSIST_THRESHOLD_IN_SECONDS)
		return true;
	return false;
}

//======================================================================

bool BuffManager::getIsBuffRemovedOnDeath(uint32 buffNameCrc)
{
	std::map<uint32, BuffRecord>::iterator it = ms_buffRecords.find(buffNameCrc);
	if(it == ms_buffRecords.end())
	{
		WARNING(true, ("Unknown buff crc %d\n", buffNameCrc));
		return true;
	}
	BuffRecord const &buffRecord = it->second;

	return buffRecord.removeOnDeath;
}

//======================================================================

int BuffManager::getBuffState(uint32 buffNameCrc)
{
	std::map<uint32, BuffRecord>::iterator it = ms_buffRecords.find(buffNameCrc);
	if(it == ms_buffRecords.end())
	{
		WARNING(true, ("Unknown buff crc %d\n", buffNameCrc));
		return 0;
	}
	BuffRecord const &buffRecord = it->second;
	return buffRecord.state;
}

//======================================================================

float BuffManager::getBuffDefaultDuration(uint32 buffNameCrc)
{
	std::map<uint32, BuffRecord>::iterator it = ms_buffRecords.find(buffNameCrc);
	if(it == ms_buffRecords.end())
	{
		WARNING(true, ("Unknown buff crc %d\n", buffNameCrc));
		return 0.0f;
	}
	BuffRecord const &buffRecord = it->second;
	return buffRecord.duration;
}

//======================================================================

std::string const & BuffManager::getBuffCallback(uint32 buffNameCrc)
{
	static std::string emptyString;
	std::map<uint32, BuffRecord>::iterator it = ms_buffRecords.find(buffNameCrc);
	if(it == ms_buffRecords.end())
	{
		WARNING(true, ("Unknown buff crc %d\n", buffNameCrc));
		return emptyString;
	}
	BuffRecord const &buffRecord = it->second;
	return buffRecord.callback;
}

//======================================================================

bool BuffManager::getIsBuffPlayerRemovable(uint32 buffNameCrc)
{
	std::map<uint32, BuffRecord>::iterator it = ms_buffRecords.find(buffNameCrc);
	if(it == ms_buffRecords.end())
	{
		WARNING(true, ("Unknown buff crc %d\n", buffNameCrc));
		return false;
	}
	BuffRecord const &buffRecord = it->second;

	return buffRecord.playerRemovable;	
}

//======================================================================


uint32 BuffManager::getBuffMaxStacks(uint32 buffNameCrc)
{
	std::map<uint32, BuffRecord>::iterator it = ms_buffRecords.find(buffNameCrc);
	if(it == ms_buffRecords.end())
	{
		WARNING(true, ("Unknown buff crc %d\n", buffNameCrc));
		return 0;
	}
	BuffRecord const &buffRecord = it->second;

	return buffRecord.maxStacks;	
}

//======================================================================


bool BuffManager::getBuffIsCelestial(uint32 buffNameCrc)
{
	std::map<uint32, BuffRecord>::iterator it = ms_buffRecords.find(buffNameCrc);
	if(it == ms_buffRecords.end())
	{
		WARNING(true, ("Unknown buff crc %d\n", buffNameCrc));
		return false;
	}
	BuffRecord const &buffRecord = it->second;

	return buffRecord.isCelestial;	
}

//======================================================================

bool BuffManager::getDoesBuffDecayOnPvPDeath(uint32 buffNameCrc)
{
	std::map<uint32, BuffRecord>::iterator it = ms_buffRecords.find(buffNameCrc);
	if(it == ms_buffRecords.end())
	{
		WARNING(true, ("Unknown buff crc %d\n", buffNameCrc));
		return true;
	}
	BuffRecord const &buffRecord = it->second;

	return buffRecord.decayOnPvPDeath;
}

//======================================================================
