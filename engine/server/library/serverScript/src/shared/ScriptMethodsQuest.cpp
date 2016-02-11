// ======================================================================
//
// ScriptMethodsQuest.cpp
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/Chat.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"
#include "serverGame/StaticLootItemManager.h"
#include "sharedGame/Quest.h"
#include "sharedGame/QuestManager.h"
#include "sharedGame/QuestTask.h"
#include "sharedGame/SharedStringIds.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedNetworkMessages/MessageQueueQuestTaskCounterMessage.h"
#include "sharedNetworkMessages/MessageQueueQuestTaskLocationMessage.h"
#include "sharedNetworkMessages/MessageQueueQuestTaskTimerMessage.h"
#include "sharedObject/Controller.h"
#include "sharedNetworkMessages/MessageQueueCyberneticsOpen.h"

#include "UnicodeUtils.h"

using namespace JNIWrappersNamespace;


// ======================================================================

namespace ScriptMethodsQuestNamespace
{
	PlayerObject * getPlayerForCharacter(jlong playerCreatureId)
	{
		CreatureObject * playerCreature = nullptr;
		if (JavaLibrary::getObject(playerCreatureId, playerCreature))
		{
			return PlayerCreatureController::getPlayerObject(playerCreature);
		}
		else
		{
			NetworkId id(playerCreatureId);
			JAVA_THROW_SCRIPT_EXCEPTION(true, ("Requested player %s, who could not be found.", id.getValueString().c_str()));
			return nullptr;
		}
	}

	bool install();

	jint         JNICALL questGetQuestId(JNIEnv *env, jobject self, jstring questName);
	jstring      JNICALL questGetQuestName(JNIEnv *env, jobject self, jint questId);
	jboolean     JNICALL questIsQuestComplete(JNIEnv *env, jobject self, jint questId, jlong playerId);
	jint         JNICALL questCanActivateQuest(JNIEnv *env, jobject self, jint questId, jlong playerId);
	jboolean     JNICALL questIsQuestActive(JNIEnv *env, jobject self, jint questId, jlong playerId);
	jint         JNICALL questActivateQuest(JNIEnv *env, jobject self, jint questId, jlong playerId, jlong questGiver);
	jint         JNICALL questCompleteQuest(JNIEnv *env, jobject self, jint questId, jlong playerId);
	void         JNICALL questClearQuest(JNIEnv *env, jobject self, jint questId, jlong playerId);
	jboolean     JNICALL questIsTaskComplete(JNIEnv *env, jobject self, jint questId, jint taskId, jlong playerId);
	jint         JNICALL questCanActivateTask(JNIEnv *env, jobject self, jint questId, jint taskId, jlong playerId);
	jboolean     JNICALL questIsTaskActive(JNIEnv *env, jobject self, jint questId, jint taskId, jlong playerId);
	jint         JNICALL questActivateTask(JNIEnv *env, jobject self, jint questId, jint taskId, jlong playerId);
	jint         JNICALL questCompleteTask(JNIEnv *env, jobject self, jint questId, jint taskId, jlong playerId);
	jint         JNICALL questFailTask(JNIEnv *env, jobject self, jint questId, jint taskId, jlong playerId);
	void         JNICALL questClearQuestTask(JNIEnv *env, jobject self, jint questId, jint taskId, jlong playerId);
	void         JNICALL questSetDebugging(JNIEnv *env, jobject self, jboolean debugging);
	jboolean     JNICALL questGetDebugging(JNIEnv *env, jobject self);
	void         JNICALL questSetQuestTaskCounter(JNIEnv *env, jobject self, jlong playerId, jstring questName, jint taskId, jstring sourceName, jint counter, jint counterMax);
	void         JNICALL questSetQuestTaskLocation(JNIEnv *env, jobject self, jlong playerId, jstring questName, jint taskId, jobject taskLocation);
	void         JNICALL questSetQuestTaskTimer(JNIEnv *env, jobject self, jlong playerId, jstring questName, jint taskId, jstring sourceName, jint timerLength);
	jintArray    JNICALL questGetAllActiveQuestIds(JNIEnv *env, jobject self, jlong object);
	jintArray    JNICALL questGetAllCompletedQuestIds(JNIEnv *env, jobject self, jlong object);
	bool         JNICALL questIsQuestAbandonable(JNIEnv * env, jobject self, jint questCrc);
	bool         JNICALL questIsQuestForceAccept(JNIEnv * env, jobject self, jint questCrc);
	bool         JNICALL questDoesUseAcceptanceUI(JNIEnv * env, jobject self, jint questCrc);
	void         JNICALL requestActivateQuest(JNIEnv * env, jobject self, jint questCrc, jlong playerId, jlong questGiver);
	void         JNICALL requestCompleteQuest(JNIEnv * env, jobject self, jint questCrc, jlong playerId);
	void         JNICALL showCyberneticsPage(JNIEnv *env, jobject self, jlong playerId, jlong npcId, jint openType);
	void         JNICALL sendStaticItemDataToPlayer(JNIEnv *env, jobject self, jlong player, jobjectArray keys, jobjectArray values);
	void         JNICALL showLootBox(JNIEnv *env, jobject self, jlong player, jlongArray newObjects);
}

using namespace ScriptMethodsQuestNamespace;


//========================================================================
// install
//========================================================================

bool ScriptMethodsQuestNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsQuestNamespace::c)}
	JF("questGetQuestId","(Ljava/lang/String;)I",questGetQuestId),
	JF("questGetQuestName", "(I)Ljava/lang/String;",questGetQuestName),
	JF("_questIsQuestComplete","(IJ)Z",questIsQuestComplete),
	JF("_questCanActivateQuest","(IJ)I",questCanActivateQuest),
	JF("_questIsQuestActive","(IJ)Z",questIsQuestActive),
	JF("_questActivateQuest","(IJJ)I",questActivateQuest),
	JF("_questCompleteQuest","(IJ)I",questCompleteQuest),
	JF("_questClearQuest","(IJ)V",questClearQuest),
	JF("_questIsTaskComplete","(IIJ)Z",questIsTaskComplete),
	JF("_questCanActivateTask","(IIJ)I",questCanActivateTask),
	JF("_questIsTaskActive","(IIJ)Z",questIsTaskActive),
	JF("_questActivateTask","(IIJ)I",questActivateTask),
	JF("_questCompleteTask","(IIJ)I",questCompleteTask),
	JF("_questFailTask", "(IIJ)I", questFailTask),
	JF("_questClearQuestTask","(IIJ)V",questClearQuestTask),
	JF("questSetDebugging", "(Z)V", questSetDebugging),
	JF("questGetDebugging", "()Z", questGetDebugging),
	JF("_questSetQuestTaskCounter", "(JLjava/lang/String;ILjava/lang/String;II)V", questSetQuestTaskCounter),
	JF("_questSetQuestTaskLocation", "(JLjava/lang/String;ILscript/location;)V", questSetQuestTaskLocation),
	JF("_questSetQuestTaskTimer", "(JLjava/lang/String;ILjava/lang/String;I)V", questSetQuestTaskTimer),
	JF("_questGetAllActiveQuestIds", "(J)[I", questGetAllActiveQuestIds),
	JF("_questGetAllCompletedQuestIds", "(J)[I", questGetAllCompletedQuestIds),
	JF("_questIsQuestAbandonable", "(I)Z", questIsQuestAbandonable),
	JF("_questIsQuestForceAccept", "(I)Z", questIsQuestForceAccept),
	JF("_questDoesUseAcceptanceUI", "(I)Z", questDoesUseAcceptanceUI),
	JF("_requestActivateQuest", "(IJJ)V", requestActivateQuest),
	JF("_requestCompleteQuest", "(IJ)V", requestCompleteQuest),
	JF("_showCyberneticsPage", "(JJI)V", showCyberneticsPage),
	JF("_sendStaticItemDataToPlayer", "(J[Ljava/lang/String;[Ljava/lang/String;)V", sendStaticItemDataToPlayer),
	JF("_showLootBox", "(J[J)V", showLootBox),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}


// ======================================================================
// Quest functions

jint JNICALL ScriptMethodsQuestNamespace::questGetQuestId(JNIEnv *, jobject, jstring questName)
{
	JavaStringParam jquestName(questName);
	std::string questNameString;
	if (!JavaLibrary::convert(jquestName, questNameString))
		return 0;

	Quest const * const quest = QuestManager::getQuest(questNameString);
	return quest ? static_cast<int>(quest->getId()) : 0;
}

// ---------------------------------------------------------------------

jstring JNICALL ScriptMethodsQuestNamespace::questGetQuestName(JNIEnv *, jobject, jint questId)
{
	std::string const questName = QuestManager::getQuestName(static_cast<uint32>(questId));
	return JavaString(questName).getReturnValue();
}

// ---------------------------------------------------------------------

jboolean JNICALL ScriptMethodsQuestNamespace::questIsQuestComplete(JNIEnv *, jobject, jint questId, jlong playerId)
{
	PlayerObject const * const playerObject = getPlayerForCharacter(playerId);
	if (playerObject)
		return playerObject->questHasCompletedQuest(static_cast<uint32>(questId));
	else
		return JNI_FALSE;
}

// ---------------------------------------------------------------------

jint JNICALL ScriptMethodsQuestNamespace::questCanActivateQuest(JNIEnv *, jobject, jint questId, jlong playerId)
{
	PlayerObject const * const playerObject = getPlayerForCharacter(playerId);
	if (playerObject)
	{
		PlayerObject::QuestResult result = playerObject->questCanActivateQuest(static_cast<uint32>(questId));
		JAVA_THROW_SCRIPT_EXCEPTION(result==PlayerObject::QR_noSuchQuest,("Requested quest %i, which could not be found.", questId));
		return static_cast<int>(result);
	}
	else
		return static_cast<int>(PlayerObject::QR_noSuchPlayer);
}

// ---------------------------------------------------------------------

jboolean JNICALL ScriptMethodsQuestNamespace::questIsQuestActive(JNIEnv *, jobject, jint questId, jlong playerId)
{
	PlayerObject const * const playerObject = getPlayerForCharacter(playerId);
	if (playerObject)
		return playerObject->questHasActiveQuest(static_cast<uint32>(questId));
	else
		return false;
}

// ---------------------------------------------------------------------

jint JNICALL ScriptMethodsQuestNamespace::questActivateQuest(JNIEnv *, jobject, jint questId, jlong playerId, jlong questGiver)
{
	PlayerObject * const playerObject = getPlayerForCharacter(playerId);
	if (!playerObject)
		return static_cast<jint>(PlayerObject::QR_noSuchPlayer);

	//send down the loot info
	Quest const * const q = QuestManager::getQuest(questId);
	if(q)
	{
		std::vector<std::string> const & inclusiveStaticLootItems = q->getInclusiveStaticLootItemNames();
		for(std::vector<std::string>::const_iterator it = inclusiveStaticLootItems.begin(); it != inclusiveStaticLootItems.end(); ++it)
		{
			StaticLootItemManager::sendDataToClient(NetworkId(playerId), *it);
		}

		std::vector<std::string> const & exclusiveStaticLootItems = q->getExclusiveStaticLootItemNames();
		for(std::vector<std::string>::const_iterator it2 = exclusiveStaticLootItems.begin(); it2 != exclusiveStaticLootItems.end(); ++it2)
		{
			StaticLootItemManager::sendDataToClient(NetworkId(playerId), *it2);
		}
	}

	PlayerObject::QuestResult result = playerObject->questActivateQuest(static_cast<uint32>(questId), NetworkId(questGiver));
	JAVA_THROW_SCRIPT_EXCEPTION(result==PlayerObject::QR_noSuchQuest,("Requested quest %i, which could not be found.", questId));
	return static_cast<int>(result);
}

// ---------------------------------------------------------------------

jint JNICALL ScriptMethodsQuestNamespace::questCompleteQuest(JNIEnv *, jobject, jint questId, jlong playerId)
{
	PlayerObject * const playerObject = getPlayerForCharacter(playerId);
	if (!playerObject)
		return static_cast<int>(PlayerObject::QR_noSuchPlayer);

	PlayerObject::QuestResult result = playerObject->questCompleteQuest(static_cast<uint32>(questId));
	JAVA_THROW_SCRIPT_EXCEPTION(result==PlayerObject::QR_noSuchQuest,("Requested quest %i, which could not be found.", questId));
	return static_cast<int>(result);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsQuestNamespace::questClearQuest(JNIEnv *, jobject, jint questId, jlong playerId)
{
	PlayerObject * const playerObject = getPlayerForCharacter(playerId);
	if (playerObject)
		playerObject->questClearQuest(static_cast<uint32>(questId));
}

// ======================================================================
// Task Functions

jboolean JNICALL ScriptMethodsQuestNamespace::questIsTaskComplete(JNIEnv *, jobject, jint questId, jint taskId, jlong playerId)
{
	if(!QuestManager::isValidTaskId(taskId))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("TaskId %i, outside legal range.", taskId));
		return JNI_FALSE;
	}

	PlayerObject const * const playerObject = getPlayerForCharacter(playerId);
	if (playerObject)
		return playerObject->questHasCompletedQuestTask(static_cast<uint32>(questId), taskId);
	else
		return JNI_FALSE;
}

// ---------------------------------------------------------------------

jint JNICALL ScriptMethodsQuestNamespace::questCanActivateTask(JNIEnv *, jobject, jint questId, jint taskId, jlong playerId)
{
	PlayerObject const * const playerObject = getPlayerForCharacter(playerId);
	if (playerObject)
	{
		if(!QuestManager::isValidTaskId(taskId))
		{
			JAVA_THROW_SCRIPT_EXCEPTION(true, ("TaskId %i, outside legal range.", taskId));
			return PlayerObject::QR_noSuchTask;
		}

		PlayerObject::QuestResult result = playerObject->questCanActivateQuestTask(static_cast<uint32>(questId), taskId);
		JAVA_THROW_SCRIPT_EXCEPTION(result==PlayerObject::QR_noSuchQuest,("Requested quest %i, which could not be found.", questId));
		JAVA_THROW_SCRIPT_EXCEPTION(result==PlayerObject::QR_noSuchTask,("Requested quest %i task %i, which could not be found.", questId, taskId));
		return static_cast<int>(result);
	}
	else
		return static_cast<int>(PlayerObject::QR_noSuchPlayer);
}

// ---------------------------------------------------------------------

jboolean JNICALL ScriptMethodsQuestNamespace::questIsTaskActive(JNIEnv *, jobject, jint questId, jint taskId, jlong playerId)
{
	PlayerObject const * const playerObject = getPlayerForCharacter(playerId);
	if(!QuestManager::isValidTaskId(taskId))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("TaskId %i, outside legal range.", taskId));
		return JNI_FALSE;
	}

	if (playerObject)
		return playerObject->questHasActiveQuestTask(static_cast<uint32>(questId), taskId);
	else
		return JNI_FALSE;
}

// ---------------------------------------------------------------------

jint JNICALL ScriptMethodsQuestNamespace::questActivateTask(JNIEnv *, jobject, jint questId, jint taskId, jlong playerId)
{
	PlayerObject * const playerObject = getPlayerForCharacter(playerId);
	if (!playerObject)
		return static_cast<int>(PlayerObject::QR_noSuchPlayer);

	if(!QuestManager::isValidTaskId(taskId))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("TaskId %i, outside legal range.", taskId));
		return PlayerObject::QR_noSuchTask;
	}

	PlayerObject::QuestResult result = playerObject->questActivateQuestTask(static_cast<uint32>(questId), taskId);
	JAVA_THROW_SCRIPT_EXCEPTION(result==PlayerObject::QR_noSuchQuest,("Requested quest %i, which could not be found.", questId));
	JAVA_THROW_SCRIPT_EXCEPTION(result==PlayerObject::QR_noSuchTask,("Requested quest %i task %i, which could not be found.", questId, taskId));
	return static_cast<int>(result);
}

// ---------------------------------------------------------------------

jint JNICALL ScriptMethodsQuestNamespace::questCompleteTask(JNIEnv *, jobject, jint questId, jint taskId, jlong playerId)
{
	PlayerObject * const playerObject = getPlayerForCharacter(playerId);
	if (!playerObject)
		return static_cast<int>(PlayerObject::QR_noSuchPlayer);

	if(!QuestManager::isValidTaskId(taskId))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("TaskId %i, outside legal range.", taskId));
		return PlayerObject::QR_noSuchTask;
	}

	PlayerObject::QuestResult result = playerObject->questCompleteQuestTask(static_cast<uint32>(questId), taskId);
	JAVA_THROW_SCRIPT_EXCEPTION(result==PlayerObject::QR_noSuchQuest,("Requested quest %i, which could not be found.", questId));
	JAVA_THROW_SCRIPT_EXCEPTION(result==PlayerObject::QR_noSuchTask,("Requested quest %i task %i, which could not be found.", questId, taskId));
	return static_cast<int>(result);
}

// ---------------------------------------------------------------------

jint JNICALL ScriptMethodsQuestNamespace::questFailTask(JNIEnv *, jobject, jint questId, jint taskId, jlong playerId)
{
	PlayerObject * const playerObject = getPlayerForCharacter(playerId);
	if (!playerObject)
		return static_cast<int>(PlayerObject::QR_noSuchPlayer);

	if(!QuestManager::isValidTaskId(taskId))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("TaskId %i, outside legal range.", taskId));
		return PlayerObject::QR_noSuchTask;
	}

	PlayerObject::QuestResult result = playerObject->questFailQuestTask(static_cast<uint32>(questId), taskId);
	JAVA_THROW_SCRIPT_EXCEPTION(result==PlayerObject::QR_noSuchQuest,("Requested quest %i, which could not be found.", questId));
	JAVA_THROW_SCRIPT_EXCEPTION(result==PlayerObject::QR_noSuchTask,("Requested quest %i task %i, which could not be found.", questId, taskId));
	return static_cast<int>(result);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsQuestNamespace::questClearQuestTask(JNIEnv *, jobject, jint questId, jint taskId, jlong playerId)
{
	if(!QuestManager::isValidTaskId(taskId))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("TaskId %i, outside legal range.", taskId));
		return;
	}

	PlayerObject * const playerObject = getPlayerForCharacter(playerId);
	if (playerObject)
		IGNORE_RETURN(playerObject->questClearQuestTask(static_cast<uint32>(questId), taskId));
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsQuestNamespace::questSetDebugging(JNIEnv *env, jobject self, jboolean debugging)
{
	QuestManager::setDebugging(debugging);
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsQuestNamespace::questGetDebugging(JNIEnv *env, jobject self)
{
	return QuestManager::getDebugging();
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsQuestNamespace::questSetQuestTaskCounter(JNIEnv *env, jobject self, jlong playerId, jstring questName, jint taskId, jstring sourceName, jint counter, jint counterMax)
{
	if(!QuestManager::isValidTaskId(taskId))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("TaskId %i, outside legal range.", taskId));
		return;
	}

	PlayerObject * const playerObject = getPlayerForCharacter(playerId);
	CreatureObject * const creatureObject = playerObject ? playerObject->getCreatureObject() : nullptr;
	Controller * const controller = creatureObject ? creatureObject->getController() : nullptr;
	if(!controller)
		return;

	JavaStringParam jquestNameName(questName);
	std::string questNameString;
	if (!JavaLibrary::convert(jquestNameName, questNameString))
		return;

	JavaStringParam jsourceNameName(sourceName);
	std::string sourceNameString;
	if (!JavaLibrary::convert(jsourceNameName, sourceNameString))
		return;

	MessageQueueQuestTaskCounterMessage * const msg = new MessageQueueQuestTaskCounterMessage(questNameString, taskId, Unicode::narrowToWide(sourceNameString), counter, counterMax);
	controller->appendMessage(static_cast<int>(CM_questTaskCounterData), 0.0f, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsQuestNamespace::questSetQuestTaskLocation(JNIEnv *env, jobject self, jlong playerId, jstring questName, jint taskId, jobject taskLocation)
{
	if(!QuestManager::isValidTaskId(taskId))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("TaskId %i, outside legal range.", taskId));
		return;
	}

	PlayerObject * const playerObject = getPlayerForCharacter(playerId);
	CreatureObject * const creatureObject = playerObject ? playerObject->getCreatureObject() : nullptr;
	Controller * const controller = creatureObject ? creatureObject->getController() : nullptr;
	if(!controller)
		return;

	JavaStringParam jquestNameName(questName);
	std::string questNameString;
	if (!JavaLibrary::convert(jquestNameName, questNameString))
		return;

	Location location;
	if(!ScriptConversion::convert(LocalRefParam(taskLocation), location))
		return;

	MessageQueueQuestTaskLocationMessage * const msg = new MessageQueueQuestTaskLocationMessage(questNameString, taskId, location);
	controller->appendMessage(static_cast<int>(CM_questTaskLocationData), 0.0f, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsQuestNamespace::questSetQuestTaskTimer(JNIEnv *env, jobject self, jlong playerId, jstring questName, jint taskId, jstring sourceName, jint finalPlayedTime)
{
	if(!QuestManager::isValidTaskId(taskId))
	{
		JAVA_THROW_SCRIPT_EXCEPTION(true, ("TaskId %i, outside legal range.", taskId));
		return;
	}

	PlayerObject * const playerObject = getPlayerForCharacter(playerId);
	CreatureObject * const creatureObject = playerObject ? playerObject->getCreatureObject() : nullptr;
	Controller * const controller = creatureObject ? creatureObject->getController() : nullptr;
	if(!controller)
		return;

	JavaStringParam jquestNameName(questName);
	std::string questNameString;
	if (!JavaLibrary::convert(jquestNameName, questNameString))
		return;

	JavaStringParam jsourceNameName(sourceName);
	std::string sourceNameString;
	if (!JavaLibrary::convert(jsourceNameName, sourceNameString))
		return;

	MessageQueueQuestTaskTimerMessage * const msg = new MessageQueueQuestTaskTimerMessage(questNameString, taskId, Unicode::narrowToWide(sourceNameString), finalPlayedTime);
	controller->appendMessage(static_cast<int>(CM_questTaskTimerData), 0.0f, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsQuestNamespace::requestActivateQuest(JNIEnv * env, jobject self, jint questCrc, jlong playerId, jlong questGiver)
{
	PlayerObject * const playerObject = getPlayerForCharacter(playerId);
	if (playerObject)
	{
		if(playerObject->questHasActiveQuest(static_cast<uint32>(questCrc)))
		{
			return;
		}

		CreatureObject * playerCreature = nullptr;
		if (JavaLibrary::getObject(playerId, playerCreature) && playerCreature)
		{
			Quest const * const q = QuestManager::getQuest(questCrc);
			if(q)
			{
				std::vector<std::string> const & inclusiveStaticLootItems = q->getInclusiveStaticLootItemNames();
				for(std::vector<std::string>::const_iterator it = inclusiveStaticLootItems.begin(); it != inclusiveStaticLootItems.end(); ++it)
				{
					StaticLootItemManager::sendDataToClient(NetworkId(playerId), *it);
				}

				std::vector<std::string> const & exclusiveStaticLootItems = q->getExclusiveStaticLootItemNames();
				for(std::vector<std::string>::const_iterator it2 = exclusiveStaticLootItems.begin(); it2 != exclusiveStaticLootItems.end(); ++it2)
				{
					StaticLootItemManager::sendDataToClient(NetworkId(playerId), *it2);
				}

				if(QuestManager::isQuestForceAccepted(q->getName().getString()))
				{
					PlayerObject::QuestResult result = playerObject->questActivateQuest(static_cast<uint32>(questCrc), NetworkId(questGiver));
					JAVA_THROW_SCRIPT_EXCEPTION(result==PlayerObject::QR_noSuchQuest,("Requested quest %i, which could not be found.", questCrc));
					MessageQueueGenericValueType<int> * const msg = new MessageQueueGenericValueType<int>(static_cast<int>(questCrc));
					Controller * const controller = playerCreature->getController();
					if(controller)
					{
						controller->appendMessage(static_cast<int>(CM_forceActivateQuest), 0.0f, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
					}
				}
				//see if this quest needs to go through our acceptance/completion UI flow
				else if(QuestManager::isQuestAcceptable(q->getName().getString()))
				{
					Controller * const controller = playerCreature->getController();
					if(controller)
					{
						if(QuestManager::getMaximumNumberOfQuestsPerPlayer() > -1)
						{
							if(playerObject->getNumberOfActiveQuests() >= QuestManager::getMaximumNumberOfQuestsPerPlayer())
							{
								Chat::sendSystemMessage(*playerCreature, SharedStringIds::quest_journal_is_full, Unicode::emptyString);
								return;
							}
						}
						playerObject->setPendingRequestQuestInformation(questCrc, NetworkId(questGiver));
						MessageQueueGenericValueType<int> * const msg = new MessageQueueGenericValueType<int>(static_cast<int>(questCrc));
						controller->appendMessage(static_cast<int>(CM_requestActivateQuest), 0.0f, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
					}
				}
				else
				{
					PlayerObject::QuestResult result = playerObject->questActivateQuest(static_cast<uint32>(questCrc), NetworkId(questGiver));
					JAVA_THROW_SCRIPT_EXCEPTION(result==PlayerObject::QR_noSuchQuest,("Requested quest %i, which could not be found.", questCrc));
				}
			}
		}
	}
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsQuestNamespace::requestCompleteQuest(JNIEnv * env, jobject self, jint questCrc, jlong playerId)
{
	PlayerObject * const playerObject = getPlayerForCharacter(playerId);
	if (playerObject)
	{
		if(!playerObject->questHasActiveQuest(static_cast<uint32>(questCrc)))
		{
			return;
		}

		CreatureObject * playerCreature = nullptr;
		if (JavaLibrary::getObject(playerId, playerCreature) && playerCreature)
		{
			Quest const * const q = QuestManager::getQuest(questCrc);
			if(q)
			{
				PlayerObject::QuestResult result = playerObject->questCompleteQuest(static_cast<uint32>(questCrc));
				JAVA_THROW_SCRIPT_EXCEPTION(result==PlayerObject::QR_noSuchQuest,("Requested quest %i, which could not be found.", questCrc));
			}
		}
	}
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsQuestNamespace::showCyberneticsPage(JNIEnv *env, jobject self, jlong playerId, jlong npcId, jint openType)
{
	MessageQueueCyberneticsOpen::OpenType const type = static_cast<MessageQueueCyberneticsOpen::OpenType>(openType);

	CreatureObject * npc = nullptr;
	if (!JavaLibrary::getObject(npcId, npc))
		return;

	if(!npc)
		return;

	CreatureObject * playerCreature = nullptr;
	if (JavaLibrary::getObject(playerId, playerCreature))
	{
		Controller * const playerController = playerCreature->getController();
		if(playerController)
		{
			MessageQueueCyberneticsOpen * msg = new MessageQueueCyberneticsOpen(type, npc->getNetworkId());
			playerController->appendMessage(CM_cyberneticsOpen, 0.0f, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
		}
	}
}

// ----------------------------------------------------------------------

jintArray JNICALL ScriptMethodsQuestNamespace::questGetAllActiveQuestIds(JNIEnv *env, jobject self, jlong playerId)
{
	PlayerObject const * const playerObject = getPlayerForCharacter(playerId);
	if (!playerObject)
	{
		LocalIntArrayRefPtr results = createNewIntArray(0);
		return results->getReturnValue();
	}
	std::vector<int> questIds = playerObject->questGetAllActiveQuestIds();
	LocalIntArrayRefPtr results = createNewIntArray(static_cast<int>(questIds.size()));
	setIntArrayRegion(*results, 0 , static_cast<int>(questIds.size()), reinterpret_cast<jint*>(&(questIds[0])));
	return results->getReturnValue();
}

// ----------------------------------------------------------------------

jintArray JNICALL ScriptMethodsQuestNamespace::questGetAllCompletedQuestIds(JNIEnv *env, jobject self, jlong playerId)
{
	PlayerObject const * const playerObject = getPlayerForCharacter(playerId);
	if (!playerObject)
	{
		LocalIntArrayRefPtr results = createNewIntArray(0);
		return results->getReturnValue();
	}
	std::vector<int> questIds = playerObject->questGetAllCompletedQuestIds();
	LocalIntArrayRefPtr results = createNewIntArray(static_cast<int>(questIds.size()));
	setIntArrayRegion(*results, 0 , static_cast<int>(questIds.size()), reinterpret_cast<jint*>(&(questIds[0])));
	return results->getReturnValue();
}

// ----------------------------------------------------------------------

/**
 * Given a dictionary representing the static item data for loot item, pack it all into a single vector and send to the client.
 */
void JNICALL ScriptMethodsQuestNamespace::sendStaticItemDataToPlayer(JNIEnv *env, jobject self, jlong playerId, jobjectArray keys, jobjectArray values) 
{
	//turn the java objects into parallel vectors
	std::vector<const Unicode::String *> keyStrings;
	getStringArray(LocalObjectArrayRefParam(keys), keyStrings);
	std::vector<const Unicode::String *> valueStrings;
	getStringArray(LocalObjectArrayRefParam(values), valueStrings);

	//build a single dictionary to hold ALL the data. ALL keys, then ALL data.  gcc can't handle it as a pair of vector,
	// it caused an deep in archive where it blows out the template instanctiation depth
	std::vector<Unicode::String> flattenedDictionary;
	flattenedDictionary.reserve(keyStrings.size() * 2);

	//put everything in it
	Unicode::String s;
	for(std::vector<const Unicode::String *>::const_iterator i = keyStrings.begin(); i != keyStrings.end(); ++i)
	{
		s = **i;
		flattenedDictionary.push_back(s);
	}
	for(std::vector<const Unicode::String *>::const_iterator i2 = valueStrings.begin(); i2 != valueStrings.end(); ++i2)
	{
		s = **i2;
		flattenedDictionary.push_back(s);
	}

	//send to player
	CreatureObject * playerCreature = nullptr;
	if (JavaLibrary::getObject(playerId, playerCreature))
	{
		Controller * const playerController = playerCreature->getController();
		if(playerController)
		{
			MessageQueueGenericValueType<std::vector<Unicode::String> > * const msg = new MessageQueueGenericValueType<std::vector<Unicode::String> >(flattenedDictionary);
			playerController->appendMessage(static_cast<int>(CM_staticLootItemData), 0.0f, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
		}
	}
}

// ----------------------------------------------------------------------

/**
 * Given a player and a list of networkids, tell the client to show a UI listing these objects as newly earned
 */
void JNICALL ScriptMethodsQuestNamespace::showLootBox(JNIEnv *env, jobject self, jlong playerId, jlongArray newObjects)
{
	std::vector<NetworkId> objectIds;
	if (!ScriptConversion::convert(newObjects, objectIds))
		return;

	//send to player
	CreatureObject * playerCreature = nullptr;
	if (JavaLibrary::getObject(playerId, playerCreature))
	{
		Controller * const playerController = playerCreature->getController();
		if(playerController)
		{
			MessageQueueGenericValueType<std::vector<NetworkId> > * const msg = new MessageQueueGenericValueType<std::vector<NetworkId> >(objectIds);
			playerController->appendMessage(static_cast<int>(CM_showLootBox), 0.0f, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
		}
	}
}

// ----------------------------------------------------------------------

bool JNICALL ScriptMethodsQuestNamespace::questIsQuestAbandonable(JNIEnv * , jobject , jint questCrc)
{
	Quest const * const q = QuestManager::getQuest(questCrc);
	if(!q)
		return JNI_FALSE;

	return(QuestManager::isQuestAbandonable(q->getName().getString())) ? JNI_TRUE : JNI_FALSE;
}

// ----------------------------------------------------------------------

bool JNICALL ScriptMethodsQuestNamespace::questIsQuestForceAccept(JNIEnv * , jobject , jint questCrc)
{
	Quest const * const q = QuestManager::getQuest(questCrc);
	if(!q)
		return JNI_FALSE;

	return(QuestManager::isQuestForceAccepted(q->getName().getString())) ? JNI_TRUE : JNI_FALSE;
}

// ----------------------------------------------------------------------

bool JNICALL ScriptMethodsQuestNamespace::questDoesUseAcceptanceUI(JNIEnv * , jobject , jint questCrc)
{
	Quest const * const q = QuestManager::getQuest(questCrc);
	if(!q)
		return JNI_FALSE;

	return(QuestManager::isQuestAcceptable(q->getName().getString())) ? JNI_TRUE : JNI_FALSE;
}

// ======================================================================
