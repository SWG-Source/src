// ScriptMethodsPlayerQuest.cpp
// Copyright 2009, Sony Online Entertainment Inc., all rights reserved.
// Author: Matt Boudreaux

//-----------------------------------------------------------------------

#include "serverScript/FirstServerScript.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureController.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/PlayerQuestObject.h"
#include "serverGame/PlanetObject.h"
#include "serverGame/ServerObject.h"
#include "serverScript/JavaLibrary.h"

#include "sharedGame/TextManager.h"

#include "sharedNetworkMessages/MessageQueueGenericValueType.h"

#include "UnicodeUtils.h"

using namespace JNIWrappersNamespace;


// ======================================================================
// ScriptMethodsPlayerQuest
// ======================================================================

namespace ScriptMethodsPlayerQuestNamespace
{
	bool install();

	//jlong	 createPlayerQuestObjectInDatapad(JNIEnv * env, jobject self, jlong creature);
	jlong    createPlayerQuestObjectInInventory(JNIEnv * env, jobject self, jlong creature);
	jboolean addPlayerQuestTask(JNIEnv * env, jobject self, jlong quest, jstring title, jstring desc, jint counter, jobject waypointLocation);
	jobjectArray getPlayerQuestTasks(JNIEnv * env, jobject self, jlong quest);
	jintArray getAllPlayerQuestTaskStatus(JNIEnv * env, jobject self, jlong quest);
	jint	  getPlayerQuestTaskStatus(JNIEnv * env, jobject self, jlong quest, jint taskIndex);
	void      setPlayerQuestTaskStatus(JNIEnv * env, jobject self, jlong quest, jint taskIndex, jint status);
	jboolean  setPlayerQuestTitle(JNIEnv * env, jobject self, jlong quest, jstring title);
	jboolean  setPlayerQuestDescription(JNIEnv * env, jobject self, jlong quest, jstring desc);
	void	  setPlayerQuestDifficulty(JNIEnv * env, jobject self, jlong quest, jint difficulty);
	jint      getPlayerQuestDifficulty(JNIEnv * env, jobject self, jlong quest);
	jint      getPlayerQuestTaskCounter(JNIEnv * env, jobject self, jlong quest, jint index);
	void      setPlayerQuestTaskCounter(JNIEnv * env, jobject self, jlong quest, jint index, jint value);
	jobject   getPlayerQuestWaypoint(JNIEnv * env, jobject self, jlong quest, jint index);
	jboolean  isPlayerQuestTaskComplete(JNIEnv * env, jobject self, jlong quest, jint index);
	jboolean  isPlayerQuestComplete(JNIEnv * env, jobject self, jlong quest);
	jobject   getPlayerQuestTitle(JNIEnv * env, jobject self, jlong quest);
	jobject   getPlayerQuestDescription(JNIEnv * env, jobject self, jlong quest);
	jobject   getPlayerQuestTaskTitle(JNIEnv * env, jobject self, jlong quest, jint index);
	jobject   getPlayerQuestTaskDescription(JNIEnv * env, jobject self, jlong quest, jint index);
	//void	  setPlayerQuestWaypoint(JNIEnv * env, jobject self, jlong quest, jint index, jobject loc);
	void      setPlayerQuestRecipe(JNIEnv * env, jobject self, jlong quest, jboolean recipe);
	jboolean  isPlayerQuestRecipe(JNIEnv * env, jobject self, jlong quest);
	void	  addPlayerQuestTaskRecipeData(JNIEnv * env, jobject self, jlong quest, jstring data);
	void      addPlayerQuestTaskRecipeDataWithIndex(JNIEnv * env, jobject self, jlong quest, jint index, jstring data);
	void	  setPlayerQuestRewardData(JNIEnv * env, jobject self, jlong quest, jstring data);
	void	  setPlayerQuestCreator(JNIEnv * env, jobject self, jlong quest, jlong creator);
	jstring	  filterText(JNIEnv * env, jobject self, jstring text);
	void	  openPlayerQuestRecipe(JNIEnv * env, jobject self, jlong player, jlong recipe);
	void	  resetAllPlayerQuestData(JNIEnv * env, jobject self, jlong quest);
}


//========================================================================
// install
//========================================================================

bool ScriptMethodsPlayerQuestNamespace::install()
{
	const JNINativeMethod NATIVES[] = {
#define JF(a,b,c) {a,b,(void*)(ScriptMethodsPlayerQuestNamespace::c)}
		//JF("_createPlayerQuestObjectInDatapad",						"(J)J", createPlayerQuestObjectInDatapad),
		JF("_createPlayerQuestObjectInInventory",					"(J)J", createPlayerQuestObjectInInventory),
		JF("_addPlayerQuestTask",									"(JLjava/lang/String;Ljava/lang/String;ILscript/location;)Z", addPlayerQuestTask),
		JF("_getPlayerQuestTasks",									"(J)[Ljava/lang/String;", getPlayerQuestTasks),
		JF("_getAllPlayerQuestTaskStatus",							"(J)[I", getAllPlayerQuestTaskStatus),
		JF("_getPlayerQuestTaskStatus",								"(JI)I", getPlayerQuestTaskStatus),
		JF("_setPlayerQuestTaskStatus",								"(JII)V", setPlayerQuestTaskStatus),
		JF("_setPlayerQuestTitle",									"(JLjava/lang/String;)Z", setPlayerQuestTitle),
		JF("_setPlayerQuestDescription",							"(JLjava/lang/String;)Z", setPlayerQuestDescription),
		JF("_setPlayerQuestDifficulty",								"(JI)V", setPlayerQuestDifficulty),
		JF("_getPlayerQuestDifficulty",								"(J)I", getPlayerQuestDifficulty),
		JF("_getPlayerQuestTaskCounter",							"(JI)I", getPlayerQuestTaskCounter),
		JF("_setPlayerQuestTaskCounter",							"(JII)V", setPlayerQuestTaskCounter),
		JF("_getPlayerQuestWaypoint",								"(JI)Ljava/lang/String;", getPlayerQuestWaypoint),
		JF("_isPlayerQuestTaskComplete",							"(JI)Z", isPlayerQuestTaskComplete),
		JF("_isPlayerQuestComplete",								"(J)Z", isPlayerQuestComplete),
		//JF("_setPlayerQuestWaypoint",								"(JILscript/location;)V", setPlayerQuestWaypoint)
		JF("_getPlayerQuestTitle",									"(J)Ljava/lang/String;", getPlayerQuestTitle),
		JF("_getPlayerQuestDescription",							"(J)Ljava/lang/String;", getPlayerQuestDescription),
		JF("_getPlayerQuestTaskTitle",								"(JI)Ljava/lang/String;", getPlayerQuestTaskTitle),
		JF("_getPlayerQuestTaskDescription",						"(JI)Ljava/lang/String;", getPlayerQuestTaskDescription),
		JF("_setPlayerQuestRecipe",                                 "(JZ)V", setPlayerQuestRecipe),
		JF("_isPlayerQuestRecipe",									"(J)Z", isPlayerQuestRecipe),
		JF("_addPlayerQuestTaskRecipeData",                         "(JLjava/lang/String;)V", addPlayerQuestTaskRecipeData),
		JF("_addPlayerQuestTaskRecipeDataWithIndex",                "(JILjava/lang/String;)V", addPlayerQuestTaskRecipeDataWithIndex),
		JF("_setPlayerQuestRewardData",								"(JLjava/lang/String;)V", setPlayerQuestRewardData),
		JF("_setPlayerQuestCreator",								"(JJ)V", setPlayerQuestCreator),
		JF("filterText",											"(Ljava/lang/String;)Ljava/lang/String;", filterText),
		JF("_openPlayerQuestRecipe",                                "(JJ)V", openPlayerQuestRecipe),
		JF("_resetAllPlayerQuestData",								"(J)V", resetAllPlayerQuestData),
	};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


//jlong ScriptMethodsPlayerQuestNamespace::createPlayerQuestObjectInDatapad(JNIEnv * env, jobject self, jlong creature)
//{
//	UNREF(env);
//	UNREF(self);
//	UNREF(creature);
//
//	CreatureObject * creatureObj = 0;
//	if(JavaLibrary::getObject(creature, creatureObj))
//	{
//		PlayerQuestObject * questObject = PlayerQuestObject::createPlayerQuestObjectInCreatureDatapad(creatureObj);
//		if(questObject)
//		{
//			return questObject->getNetworkId().getValue();
//		}
//	}
//	return 0;
//}

jlong ScriptMethodsPlayerQuestNamespace::createPlayerQuestObjectInInventory(JNIEnv * env, jobject self, jlong creature)
{
	UNREF(env);
	UNREF(self);
	UNREF(creature);

	CreatureObject * creatureObj = 0;
	if(JavaLibrary::getObject(creature, creatureObj))
	{
		PlayerQuestObject * questObject = PlayerQuestObject::createPlayerQuestObjectInCreatureInventory(creatureObj);
		if(questObject)
		{
			return questObject->getNetworkId().getValue();
		}
	}
	return 0;
}

jboolean ScriptMethodsPlayerQuestNamespace::addPlayerQuestTask(JNIEnv * env, jobject self, jlong quest, jstring title, jstring desc, jint counter, jobject waypointLocation)
{
	UNREF(env);
	UNREF(self);

	PlayerQuestObject * questObject;
	if(!JavaLibrary::getObject(quest, questObject))
	{
		WARNING(true, ("AddPlayerQuestTask: Could not get valid Player Quest Object from OID: %d", quest));
		return JNI_FALSE;
	}

	Vector waypointVec;
	std::string sceneId;
	if (!ScriptConversion::convertWorld(waypointLocation, waypointVec, sceneId))
	{
		// nullptr or Invalid Location passed in. That's fine, just means no waypoint.
		if(questObject)
		{
			std::string titleString;
			if(!JavaLibrary::convert(JavaString(title), titleString))
			{
				WARNING(true, ("AddPlayerQuestTask: Failed to localize title string."));
				return JNI_FALSE;
			}

			std::string descString;
			if(!JavaLibrary::convert(JavaString(desc), descString))
			{
				WARNING(true, ("AddPlayerQuestTask: Failed to localize description string."));
				return JNI_FALSE;
			}

			questObject->addNewTask(titleString, descString, counter, std::string());
			return JNI_TRUE;
		}
		return JNI_FALSE;
	}

	std::string formatedWaypoint;
	char buffer[256];
	memset(buffer, 0, 256);
	sprintf(buffer, "%f %f %f %s", waypointVec.x, waypointVec.y, waypointVec.z, sceneId.c_str());
	formatedWaypoint = buffer;

	if(questObject)
	{
		std::string titleString;
		if(!JavaLibrary::convert(JavaString(title), titleString))
		{
			WARNING(true, ("AddPlayerQuestTask: Failed to localize title string."));
			return JNI_FALSE;
		}

		std::string descString;
		if(!JavaLibrary::convert(JavaString(desc), descString))
		{
			WARNING(true, ("AddPlayerQuestTask: Failed to localize description string."));
			return JNI_FALSE;
		}

		questObject->addNewTask(titleString, descString, counter, formatedWaypoint);
		return JNI_TRUE;
	}

	return JNI_FALSE;
}

jobjectArray ScriptMethodsPlayerQuestNamespace::getPlayerQuestTasks(JNIEnv * env, jobject self, jlong quest)
{
	UNREF(env);
	UNREF(self);

	PlayerQuestObject * questObject;
	if(!JavaLibrary::getObject(quest, questObject))
	{
		WARNING(true, ("getPlayerQuestTasks: Could not get valid Player Quest Object from OID: %d", quest));
		return 0;
	}
	std::vector<Unicode::String> taskVector;
	questObject->getTasks(taskVector);

	LocalObjectArrayRefPtr valueArray = createNewObjectArray(taskVector.size(), JavaLibrary::getClsString());
	for (std::vector<Unicode::String>::size_type i = 0; i < taskVector.size(); ++i)
	{
		JavaString jval(Unicode::wideToNarrow(taskVector[i]));
		setObjectArrayElement(*valueArray, i, jval);
	}

	return valueArray->getReturnValue();

}

jintArray ScriptMethodsPlayerQuestNamespace::getAllPlayerQuestTaskStatus(JNIEnv * env, jobject self, jlong quest)
{
	UNREF(env);
	UNREF(self);

	PlayerQuestObject * questObject;
	if(!JavaLibrary::getObject(quest, questObject))
	{
		WARNING(true, ("getAllPlayerQuestTaskStatus: Could not get valid Player Quest Object from OID: %d", quest));
		return 0;
	}

	std::vector<int> taskStatus;
	questObject->getTaskStatus(taskStatus);

	LocalIntArrayRefPtr idArray = createNewIntArray(taskStatus.size());
	if (idArray == LocalIntArrayRef::cms_nullPtr)
	{
		if (env->ExceptionCheck())
			env->ExceptionDescribe();
		return 0;
	}

	std::vector<int>::iterator iter = taskStatus.begin();
	int count = 0;
	for (; iter != taskStatus.end(); ++iter)
	{
		jint jintTemp = (*iter);
		setIntArrayRegion(*idArray, count, 1, &jintTemp);
		++count;
	}

	return idArray->getReturnValue();
}

jint ScriptMethodsPlayerQuestNamespace::getPlayerQuestTaskStatus(JNIEnv * env, jobject self, jlong quest, jint taskIndex)
{
	UNREF(env);
	UNREF(self);

	PlayerQuestObject * questObject;
	if(!JavaLibrary::getObject(quest, questObject))
	{
		WARNING(true, ("getPlayerQuestTaskStatus: Could not get valid Player Quest Object from OID: %d", quest));
		return 0;
	}

	return questObject->getTaskStatus(taskIndex);
}

void ScriptMethodsPlayerQuestNamespace::setPlayerQuestTaskStatus(JNIEnv * env, jobject self, jlong quest, jint taskIndex, jint status)
{
	UNREF(env);
	UNREF(self);

	PlayerQuestObject * questObject;
	if(!JavaLibrary::getObject(quest, questObject))
	{
		WARNING(true, ("setPlayerQuestTaskStatus: Could not get valid Player Quest Object from OID: %d", quest));
		return;
	}

	questObject->setTaskStatus(taskIndex, status);
}

jboolean ScriptMethodsPlayerQuestNamespace::setPlayerQuestTitle(JNIEnv * env, jobject self, jlong quest, jstring title)
{
	UNREF(env);
	UNREF(self);

	PlayerQuestObject * questObject;
	if(!JavaLibrary::getObject(quest, questObject))
	{
		WARNING(true, ("setPlayerQuestTitle: Could not get valid Player Quest Object from OID: %d", quest));
		return JNI_FALSE;
	}

	if(questObject)
	{
		std::string titleString;
		if(!JavaLibrary::convert(JavaString(title), titleString))
		{
			WARNING(true, ("setPlayerQuestTitle: Failed to localize task string."));
			return JNI_FALSE;
		}

		questObject->setQuestTitle(titleString);
		return JNI_TRUE;
	}

	return JNI_FALSE;
}

jboolean ScriptMethodsPlayerQuestNamespace::setPlayerQuestDescription(JNIEnv * env, jobject self, jlong quest, jstring desc)
{
	UNREF(env);
	UNREF(self);

	PlayerQuestObject * questObject;
	if(!JavaLibrary::getObject(quest, questObject))
	{
		WARNING(true, ("setPlayerQuestDescription: Could not get valid Player Quest Object from OID: %d", quest));
		return JNI_FALSE;
	}

	if(questObject)
	{
		std::string descString;
		if(!JavaLibrary::convert(JavaString(desc), descString))
		{
			WARNING(true, ("setPlayerQuestDescription: Failed to localize task string."));
			return JNI_FALSE;
		}

		questObject->setQuestDescription(descString);
		return JNI_TRUE;
	}

	return JNI_FALSE;
}

void ScriptMethodsPlayerQuestNamespace::setPlayerQuestDifficulty(JNIEnv * env, jobject self, jlong quest, jint difficulty)
{
	UNREF(env);
	UNREF(self);

	PlayerQuestObject * questObject;
	if(!JavaLibrary::getObject(quest, questObject))
	{
		WARNING(true, ("setPlayerQuestDifficulty: Could not get valid Player Quest Object from OID: %d", quest));
		return;
	}

	if(questObject)
	{
		questObject->setQuestDifficulty(difficulty);
	}

	return;
}

jint ScriptMethodsPlayerQuestNamespace::getPlayerQuestDifficulty(JNIEnv * env, jobject self, jlong quest)
{
	UNREF(env);
	UNREF(self);

	PlayerQuestObject * questObject;
	if(!JavaLibrary::getObject(quest, questObject))
	{
		WARNING(true, ("setPlayerQuestDifficulty: Could not get valid Player Quest Object from OID: %d", quest));
		return -1;
	}

	if(questObject)
		return questObject->getQuestDifficulty();

	return -1;
}

jint ScriptMethodsPlayerQuestNamespace::getPlayerQuestTaskCounter(JNIEnv * env, jobject self, jlong quest, jint index)
{
	UNREF(env);
	UNREF(self);

	PlayerQuestObject * questObject;
	if(!JavaLibrary::getObject(quest, questObject))
	{
		WARNING(true, ("getPlayerQuestTaskCounter: Could not get valid Player Quest Object from OID: %d", quest));
		return -1;
	}

	if(questObject)
	{
		if(index < 0 || index > questObject->getTotalTasks())
		{
			WARNING(true, ("getPlayerQuestTaskCounter: index passed in was invalid. Index = %d Current number of Tasks = %d", index, questObject->getTotalTasks()));
			return -1;
		}

		return questObject->getTaskCounter(index);
	}

	return -1;
}

void ScriptMethodsPlayerQuestNamespace::setPlayerQuestTaskCounter(JNIEnv * env, jobject self, jlong quest, jint index, jint value)
{
	UNREF(env);
	UNREF(self);

	PlayerQuestObject * questObject;
	if(!JavaLibrary::getObject(quest, questObject))
	{
		WARNING(true, ("setPlayerQuestTaskCounter: Could not get valid Player Quest Object from OID: %d", quest));
		return;
	}

	if(questObject)
	{
		if(index < 0 || index > questObject->getTotalTasks())
		{
			WARNING(true, ("setPlayerQuestTaskCounter: index passed in was invalid. Index = %d Current number of Tasks = %d", index, questObject->getTotalTasks()));
			return;
		}

		questObject->setTaskCounter(index, value);
	}

	return;
}

jobject ScriptMethodsPlayerQuestNamespace::getPlayerQuestWaypoint(JNIEnv * env, jobject self, jlong quest, jint index)
{
	UNREF(env);
	UNREF(self);

	PlayerQuestObject * questObject;
	if(!JavaLibrary::getObject(quest, questObject))
	{
		WARNING(true, ("getPlayerQuestWaypoint: Could not get valid Player Quest Object from OID: %d", quest));
		return nullptr;
	}

	if(questObject)
	{
		JavaString jString(questObject->getWaypoint(index));
		return jString.getReturnValue();
	}

	return nullptr;
}


jboolean ScriptMethodsPlayerQuestNamespace::isPlayerQuestTaskComplete(JNIEnv * env, jobject self, jlong quest, jint index)
{
	UNREF(env);
	UNREF(self);

	PlayerQuestObject * questObject;
	if(!JavaLibrary::getObject(quest, questObject))
	{
		WARNING(true, ("isPlayerQuestTaskComplete: Could not get valid Player Quest Object from OID: %d", quest));
		return JNI_FALSE;
	}

	if(questObject)
	{
		if(questObject->getTaskStatus(index) == 2) // Not a magic number, it's defined in the Base_class.java file
			return JNI_TRUE;
	}

	return JNI_FALSE;
}

jboolean ScriptMethodsPlayerQuestNamespace::isPlayerQuestComplete(JNIEnv * env, jobject self, jlong quest)
{
	UNREF(env);
	UNREF(self);

	PlayerQuestObject * questObject;
	if(!JavaLibrary::getObject(quest, questObject))
	{
		WARNING(true, ("isPlayerQuestComplete: Could not get valid Player Quest Object from OID: %d", quest));
		return JNI_FALSE;
	}

	if(questObject)
	{
		if(questObject->isCompleted())
			return JNI_TRUE;
	}

	return JNI_FALSE;
}

jobject ScriptMethodsPlayerQuestNamespace::getPlayerQuestTitle(JNIEnv * env, jobject self, jlong quest)
{
	UNREF(env);
	UNREF(self);

	PlayerQuestObject * questObject;
	if(!JavaLibrary::getObject(quest, questObject))
	{
		WARNING(true, ("getPlayerQuestTitle: Could not get valid Player Quest Object from OID: %d", quest));
		return nullptr;
	}

	if(questObject)
	{
		JavaString returnVal(questObject->getQuestTitle().c_str());
		return returnVal.getReturnValue();
	}

	return nullptr;
}

jobject ScriptMethodsPlayerQuestNamespace::getPlayerQuestDescription(JNIEnv * env, jobject self, jlong quest)
{
	UNREF(env);
	UNREF(self);

	PlayerQuestObject * questObject;
	if(!JavaLibrary::getObject(quest, questObject))
	{
		WARNING(true, ("getPlayerQuestDescription: Could not get valid Player Quest Object from OID: %d", quest));
		return nullptr;
	}

	if(questObject)
	{
		JavaString returnVal(questObject->getQuestDescription().c_str());
		return returnVal.getReturnValue();
	}

	return nullptr;
}

jobject ScriptMethodsPlayerQuestNamespace::getPlayerQuestTaskTitle(JNIEnv * env, jobject self, jlong quest, jint index)
{
	UNREF(env);
	UNREF(self);

	PlayerQuestObject * questObject;
	if(!JavaLibrary::getObject(quest, questObject))
	{
		WARNING(true, ("getPlayerQuestTaskTitle: Could not get valid Player Quest Object from OID: %d", quest));
		return nullptr;
	}

	if(questObject)
	{
		JavaString returnVal(questObject->getTaskTitle(index).c_str());
		return returnVal.getReturnValue();
	}

	return nullptr;
}

jobject ScriptMethodsPlayerQuestNamespace::getPlayerQuestTaskDescription(JNIEnv * env, jobject self, jlong quest, jint index)
{
	UNREF(env);
	UNREF(self);

	PlayerQuestObject * questObject;
	if(!JavaLibrary::getObject(quest, questObject))
	{
		WARNING(true, ("getPlayerQuestTaskDescription: Could not get valid Player Quest Object from OID: %d", quest));
		return nullptr;
	}

	if(questObject)
	{
		JavaString returnVal(questObject->getTaskDescription(index).c_str());
		return returnVal.getReturnValue();
	}

	return nullptr;
}

void ScriptMethodsPlayerQuestNamespace::setPlayerQuestRecipe(JNIEnv * env, jobject self, jlong quest, jboolean recipe)
{
	UNREF(env);
	UNREF(self);

	PlayerQuestObject * questObject;
	if(!JavaLibrary::getObject(quest, questObject))
	{
		WARNING(true, ("setPlayerQuestRecipe: Could not get valid Player Quest Object from OID: %d", quest));
		return;
	}

	if(questObject)
	{
		questObject->setRecipe(recipe == JNI_TRUE ? true : false);
	}

}

jboolean ScriptMethodsPlayerQuestNamespace::isPlayerQuestRecipe(JNIEnv * env, jobject self, jlong quest)
{
	UNREF(env);
	UNREF(self);

	PlayerQuestObject * questObject;
	if(!JavaLibrary::getObject(quest, questObject))
	{
		WARNING(true, ("isPlayerQuestRecipe: Could not get valid Player Quest Object from OID: %d", quest));
		return JNI_FALSE;
	}

	if(questObject)
	{
		return questObject->isRecipe() ? JNI_TRUE : JNI_FALSE;
	}

	return JNI_FALSE;
}

void ScriptMethodsPlayerQuestNamespace::addPlayerQuestTaskRecipeData(JNIEnv * env, jobject self, jlong quest, jstring data)
{
	UNREF(env);
	UNREF(self);

	PlayerQuestObject * questObject;
	if(!JavaLibrary::getObject(quest, questObject))
	{
		WARNING(true, ("addPlayerQuestTaskRecipeData: Could not get valid Player Quest Object from OID: %d", quest));
		return;
	}

	std::string dataString;
	if(!JavaLibrary::convert(JavaString(data), dataString))
	{
		WARNING(true, ("addPlayerQuestTaskRecipeData: Failed to localize data string."));
		return;
	}

	if(questObject)
	{
		questObject->addRecipeTaskData(dataString);
	}

	return;
}

void ScriptMethodsPlayerQuestNamespace::addPlayerQuestTaskRecipeDataWithIndex(JNIEnv * env, jobject self, jlong quest, jint index, jstring data)
{
	UNREF(env);
	UNREF(self);

	PlayerQuestObject * questObject;
	if(!JavaLibrary::getObject(quest, questObject))
	{
		WARNING(true, ("addPlayerQuestTaskRecipeDataWithIndex: Could not get valid Player Quest Object from OID: %d", quest));
		return;
	}

	std::string dataString;
	if(!JavaLibrary::convert(JavaString(data), dataString))
	{
		WARNING(true, ("addPlayerQuestTaskRecipeDataWithIndex: Failed to localize data string."));
		return;
	}

	if(questObject)
	{
		questObject->setRecipeTaskData(index, dataString);
	}

	return;
}

void ScriptMethodsPlayerQuestNamespace::setPlayerQuestRewardData(JNIEnv * env, jobject self, jlong quest, jstring data)
{
	UNREF(env);
	UNREF(self);

	PlayerQuestObject * questObject;
	if(!JavaLibrary::getObject(quest, questObject))
	{
		WARNING(true, ("setPlayerQuestRewardData: Could not get valid Player Quest Object from OID: %d", quest));
		return;
	}

	std::string dataString;
	if(!JavaLibrary::convert(JavaString(data), dataString))
	{
		WARNING(true, ("setPlayerQuestRewardData: Failed to localize data string."));
		return;
	}

	if(questObject)
	{
		questObject->setRewards(dataString);
	}

	return;
}

void ScriptMethodsPlayerQuestNamespace::setPlayerQuestCreator(JNIEnv * env, jobject self, jlong quest, jlong creator)
{
	UNREF(env);
	UNREF(self);

	PlayerQuestObject * questObject;
	if(!JavaLibrary::getObject(quest, questObject))
	{
		WARNING(true, ("setPlayerQuestCreator: Could not get valid Player Quest Object from OID: %d", quest));
		return;
	}

	ServerObject * creatorObject;
	if(!JavaLibrary::getObject(creator, creatorObject))
	{
		WARNING(true, ("setPlayerQuestCreator: Could not get valid Player Object from OID: %d", creator));
		return;
	}

	if(questObject && creatorObject)
	{
		questObject->setCreator(creatorObject->getNetworkId());
	}
}

jstring ScriptMethodsPlayerQuestNamespace::filterText(JNIEnv * env, jobject self, jstring text)
{
	UNREF(env);
	UNREF(self);

	std::string textString;
	if(!JavaLibrary::convert(JavaString(text), textString))
	{
		WARNING(true, ("filterText: Failed to convert string."));
		std::string empty = "";
		JavaString jval(empty);
		return jval.getReturnValue();
	}

	Unicode::String filteredString = TextManager::filterText( Unicode::narrowToWide(textString) );

	std::string narrowFilter = Unicode::wideToNarrow(filteredString);

	JavaString jval(narrowFilter);

	return jval.getReturnValue();
}

void ScriptMethodsPlayerQuestNamespace::openPlayerQuestRecipe(JNIEnv * env, jobject self, jlong player, jlong recipe)
{
	UNREF(env);
	UNREF(self);

	CreatureObject * playerCreature = nullptr;
	if (!JavaLibrary::getObject(player, playerCreature))
	{
		DEBUG_WARNING(true, ("openPlayerQuestRecipe: Failed to get valid creature object with OID %d", player));
		return;
	}

	ServerObject * recipeObj = nullptr;
	if (!JavaLibrary::getObject(recipe, recipeObj))
	{
		DEBUG_WARNING(true, ("openPlayerQuestRecipe: Failed to get valid recipe object with OID %d", player));
		return;
	}

	if (playerCreature->isPlayerControlled() && playerCreature->getController() != nullptr)
	{
		playerCreature->getController()->appendMessage(
			static_cast<int>(CM_openRecipe),
			0,
			new MessageQueueGenericValueType< NetworkId >( recipeObj->getNetworkId() ),
			GameControllerMessageFlags::SEND |
			GameControllerMessageFlags::RELIABLE |
			GameControllerMessageFlags::DEST_AUTH_CLIENT);
	}

	return;
}

void ScriptMethodsPlayerQuestNamespace::resetAllPlayerQuestData(JNIEnv * env, jobject self, jlong quest)
{
	UNREF(env);
	UNREF(self);

	PlayerQuestObject * playerQuest = nullptr;
	if (!JavaLibrary::getObject(quest, playerQuest))
	{
		DEBUG_WARNING(true, ("resetAllPlayerQuestData: Failed to get valid recipe object with OID %d", quest));
		return;
	}

	if(playerQuest)
		playerQuest->resetAllData();

	return;
}
