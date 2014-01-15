//========================================================================
//
// ScriptMethodsBank.cpp - implements script methods dealing with bank
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "sharedFoundation/DynamicVariable.h"
#include "sharedFoundation/DynamicVariableList.h"

#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/ServerObject.h"
#include "sharedNetworkMessages/ClientOpenContainerMessage.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedObject/NetworkIdManager.h"


// ======================================================================
// ScriptMethodsBankNamespace
// ======================================================================

namespace ScriptMethodsBankNamespace
{
	bool install();

	void JNICALL openBankContainer(JNIEnv *env, jobject script, jlong self, jlong player);
	void JNICALL quitBank(JNIEnv *env, jobject self, jlong player);
	void JNICALL joinBank(JNIEnv *env, jobject self, jlong player, jstring bank);
}

//========================================================================
// install
//========================================================================

bool ScriptMethodsBankNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsBankNamespace::c)}
	JF("_openBankContainer", "(JJ)V",openBankContainer),
	JF("_quitBank", "(J)V",quitBank),
	JF("_joinBank", "(JLjava/lang/String;)V",joinBank),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsBankNamespace::openBankContainer(JNIEnv *env, jobject script, jlong self, jlong player)
{
	CreatureObject *playerObj = 0;
	ServerObject *bankObj = 0;
	if (!JavaLibrary::getObject(player, playerObj) || !JavaLibrary::getObject(self, bankObj))
		return;

	Client *client = playerObj->getClient();
	if (client)
	{
		ServerObject *bankContainer = playerObj->getBankContainer();
		if (bankContainer)
		{
			if (!bankContainer->areContentsLoaded())
			{
				bankContainer->loadAllContentsFromDB();
			}
			else
				client->openContainer(*bankContainer, 0, std::string());

			static const std::string bankLocation("open_bank_location");
			NetworkId bankId = bankObj->getNetworkId();
			playerObj->setObjVarItem(bankLocation, bankId);
		}
	}
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsBankNamespace::joinBank(JNIEnv *env, jobject self, jlong player, jstring bank)
{
	ServerObject * sourceObj = 0;
	if (!JavaLibrary::getObject(player, sourceObj))
		return;

	CreatureObject *playerObj = dynamic_cast<CreatureObject *>(sourceObj);
	if (playerObj)
	{
		Unicode::String myBankName;

		JavaString temp(bank);
		if (!JavaLibrary::convert(temp,myBankName))
			return;
		playerObj->joinBank(myBankName);
	}
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsBankNamespace::quitBank(JNIEnv *env, jobject self, jlong player)
{
	ServerObject * sourceObj = 0;
	if (!JavaLibrary::getObject(player, sourceObj))
		return;

	CreatureObject *playerObj = dynamic_cast<CreatureObject *>(sourceObj);
	if (playerObj)
	{
		playerObj->quitBank();
	}
}

//========================================================================
