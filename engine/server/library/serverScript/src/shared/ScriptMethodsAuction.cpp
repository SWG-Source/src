//========================================================================
//
// ScriptMethodsAuction.cpp - implements script methods dealing with bank
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "sharedFoundation/DynamicVariable.h"
#include "sharedFoundation/DynamicVariableList.h"
#include "serverGame/CommoditiesMarket.h"
#include "serverGame/ContainerInterface.h"
#include "serverGame/CreatureObject.h"
#include "serverGame/GameServer.h"
#include "serverGame/ServerObject.h"
#include "serverGame/TangibleObject.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedObject/NetworkIdManager.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptParameters.h"

// ======================================================================
// ScriptMethodsAuctionNamespace
// ======================================================================

namespace ScriptMethodsAuctionNamespace
{
	bool install();

	jboolean JNICALL isCommoditiesServerAvailable(JNIEnv *env, jobject self);
	void JNICALL createVendorMarket(JNIEnv *env, jobject script, jlong player, jlong container, jint entranceCharge);
	void JNICALL updateVendorValue(JNIEnv *env, jobject script, jlong container);
	void JNICALL setSalesTax(JNIEnv *env, jobject script, jint salesTax, jlong bank, jlong container);
	void JNICALL requestVendorItemCount(JNIEnv *env, jobject script, jlong vendor);
	void JNICALL requestPlayerVendorCount(JNIEnv *env, jobject script, jlong player);
	void JNICALL updateVendorSearchOption(JNIEnv *env, jobject self, jlong vendor, jboolean enabled);
	void JNICALL setEntranceCharge(JNIEnv *env, jobject self, jlong vendor, jint entranceCharge);
	void JNICALL auctionCreatePermanent(JNIEnv *env, jobject script, jstring jownerName, jlong jitem, jlong jauctionContainer, jint cost, jstring userDescription);
	void JNICALL removeAllAuctions(JNIEnv *env, jobject script, jlong vendor);
	void JNICALL reinitializeVendor(JNIEnv *env, jobject script, jlong vendor, jlong player);
	void JNICALL updateVendorStatus(JNIEnv *env, jobject script, jlong vendor, jint status);
}

//========================================================================
// install
//========================================================================

bool ScriptMethodsAuctionNamespace::install()
{
const JNINativeMethod NATIVES[] = {
	#define JF(a,b,c) {a,b,(void*)(ScriptMethodsAuctionNamespace::c)}
	JF("isCommoditiesServerAvailable", "()Z", isCommoditiesServerAvailable),
	JF("_createVendorMarket", "(JJI)V",createVendorMarket),
	JF("_updateVendorValue", "(J)V",updateVendorValue),
	JF("_setSalesTax", "(IJJ)V",setSalesTax),
	JF("_requestVendorItemCount", "(J)V", requestVendorItemCount),
	JF("_requestPlayerVendorCount", "(J)V", requestPlayerVendorCount),
	JF("_updateVendorSearchOption", "(JZ)V", updateVendorSearchOption),
	JF("_setEntranceCharge", "(JI)V", setEntranceCharge),
	JF("_auctionCreatePermanent", "(Ljava/lang/String;JJILjava/lang/String;)V",auctionCreatePermanent),
	JF("_removeAllAuctions", "(J)V",removeAllAuctions),
	JF("_reinitializeVendor", "(JJ)V",reinitializeVendor),
	JF("_updateVendorStatus", "(JI)V", updateVendorStatus),
};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsAuctionNamespace::isCommoditiesServerAvailable(JNIEnv *env, jobject script)
{
	return CommoditiesMarket::isCommoditiesServerAvailable();
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsAuctionNamespace::createVendorMarket(JNIEnv *env, jobject script, jlong player, jlong container, jint entranceCharge)
{
	ServerObject * sourceObj = 0;
	if (!JavaLibrary::getObject(player, sourceObj))
	{
		DEBUG_WARNING(true, ("[designer bug] createVendorMarket() player parameter (1) is invalid"));
		return;
	}

	ServerObject *containerObj = 0;
	if (!JavaLibrary::getObject(container, containerObj))
	{
		DEBUG_WARNING(true, ("[designer bug] createVendorMarket() container parameter (2) is invalid"));
		return;
	}
	containerObj->makeVendor();

	CreatureObject *playerObj = dynamic_cast<CreatureObject *>(sourceObj);
	if (playerObj)
	{
		CommoditiesMarket::createVendorMarket(*playerObj, *containerObj, entranceCharge);
	}
	else
	{
		DEBUG_WARNING(true, ("[designer bug] createVendorMarket() player parameter (1) is not a creature"));
	}
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsAuctionNamespace::auctionCreatePermanent(JNIEnv *env, jobject script, jstring jownerName, jlong jitem, jlong jauctionContainer, jint jprice, jstring juserDescription)
{
	TangibleObject *itemObject = nullptr;
	ServerObject *containerObj = nullptr;

	if (!JavaLibrary::getObject(jitem, itemObject))
	{
		DEBUG_WARNING(true, ("[designer bug] auctionCreatePermanent() item parameter (2) is invalid"));
		return;
	}

	if (!JavaLibrary::getObject(jauctionContainer, containerObj))
	{
		DEBUG_WARNING(true, ("[designer bug] auctionCreatePermanent() container parameter (3) is invalid"));
		return;
	}

	JavaStringParam ownerNameParam(jownerName);
	JavaStringParam descriptionParam(juserDescription);

	std::string ownerName;
	Unicode::String description;
	if(!JavaLibrary::convert(ownerNameParam, ownerName))
	{
		DEBUG_WARNING(true, ("[script bug] invalid ownerName passed to auctionCreatePermanent"));
		return;
	}
	if(!JavaLibrary::convert(descriptionParam, description))
	{
		DEBUG_WARNING(true, ("[script bug] invalid description passed to auctionCreatePermanent"));
		return;
	}

	CommoditiesMarket::auctionCreatePermanent(ownerName, *itemObject,
		*containerObj, jprice, description);
}

// ----------------------------------------------------------------------

void JNICALL ScriptMethodsAuctionNamespace::updateVendorValue(JNIEnv *env, jobject script, jlong container)
{
	ServerObject *containerObj = 0;
	if (!JavaLibrary::getObject(container, containerObj))
	{
		DEBUG_WARNING(true, ("[designer bug] updateVendorValue() container parameter (1) is invalid"));
		return;
	}

	CommoditiesMarket::getVendorValue(*containerObj);
}

// salesTax is an integer that will be divided by 10000.  So a 10% sales tax would be an integer=1000
void JNICALL ScriptMethodsAuctionNamespace::setSalesTax(JNIEnv *env, jobject script, jint salesTax, jlong bank, jlong container)
{
	ServerObject *containerObj = 0;
	ServerObject *bankObj = 0;
	if( !JavaLibrary::getObject(container, containerObj) )
	{
		DEBUG_WARNING(true, ("[designer bug] setSalesTax() auction container is invalid."));
		return;
	}

	// are we setting a sales tax?
	if( salesTax!=0 )
	{
		// do we have a bank object?
		if( !JavaLibrary::getObject(bank, bankObj) )
		{
			DEBUG_WARNING(true, ("[designer bug]  setSalesTax() bank object is invalid."));
			return;
		}
	}

	if( salesTax!=0 )
	{
		CommoditiesMarket::setSalesTax(salesTax, bankObj->getNetworkId(), *containerObj);
	}
	else
	{
		// turn off sales tax for this vendor
		CommoditiesMarket::setSalesTax(0, NetworkId(), *containerObj);
	}
}

void JNICALL ScriptMethodsAuctionNamespace::requestVendorItemCount(JNIEnv *env, jobject script, jlong vendor)
{
	ServerObject* vendorObject = nullptr;
	if( !JavaLibrary::getObject(vendor, vendorObject) )
	{
		WARNING(true, ("[designer bug] requestVendorItemLimit() vendor is invalid."));
		return;
	}

	CommoditiesMarket::requestVendorItemCount(vendorObject->getNetworkId());
}

void JNICALL ScriptMethodsAuctionNamespace::requestPlayerVendorCount(JNIEnv *env, jobject script, jlong player)
{
	ServerObject* playerObject = nullptr;
	if( !JavaLibrary::getObject(player, playerObject) )
	{
		WARNING(true, ("[designer bug] requestPlayerVendorCount() player is invalid."));
		return;
	}

	CommoditiesMarket::requestPlayerVendorCount(playerObject->getNetworkId());
}

void JNICALL ScriptMethodsAuctionNamespace::updateVendorSearchOption(JNIEnv *env, jobject script, jlong vendor, jboolean enable)
{
	ServerObject* vendorObject = nullptr;
	bool enabled;
	if( !JavaLibrary::getObject(vendor, vendorObject) )
	{
		WARNING(true, ("[designer bug] updateVendorSearchOption() vendor is invalid."));
		return;
	}

	if (enable == JNI_TRUE)
		enabled = true;
	else
		enabled = false;
	CommoditiesMarket::updateVendorSearchOption(vendorObject->getNetworkId(), enabled);
}

void JNICALL ScriptMethodsAuctionNamespace::setEntranceCharge(JNIEnv *env, jobject script, jlong vendor, jint entranceCharge)
{
	ServerObject* vendorObject = nullptr;
	if( !JavaLibrary::getObject(vendor, vendorObject) )
	{
		WARNING(true, ("[designer bug] setEntranceCharge() vendor is invalid."));
		return;
	}

	CommoditiesMarket::setEntranceCharge(vendorObject->getNetworkId(), entranceCharge);
}

void JNICALL ScriptMethodsAuctionNamespace::removeAllAuctions(JNIEnv *env, jobject script, jlong jvendor)
{
	ServerObject* vendorObject = nullptr;
	if( !JavaLibrary::getObject(jvendor, vendorObject) )
	{
		WARNING(true, ("[designer bug] removeAllAuctions() vendor is invalid."));
		return;
	}
	
	vendorObject->removeAllAuctions();
}

void JNICALL ScriptMethodsAuctionNamespace::reinitializeVendor(JNIEnv *env, jobject script, jlong vendor, jlong player)
{
	ServerObject* vendorObject = nullptr;
	if( !JavaLibrary::getObject(vendor, vendorObject) )
	{
		WARNING(true, ("[designer bug] reinitializeVendor() vendor is invalid.")); 
		return;
	}
	ServerObject* auctionContainer = vendorObject->getBazaarContainer();
	if ( auctionContainer == nullptr )
	{
		WARNING(true, ("[designer bug] reinitializeVendor() vendor doesn't have an auction container.")); 
		return;
	}
	

	ServerObject* playerObject = nullptr;
	if ( !JavaLibrary::getObject(player, playerObject) )
	{
		WARNING(true, ("[designer bug] reinitializeVendor() player is invalid.")); 
		return;
	}

	NetworkId playerId = playerObject->getNetworkId();
	if( playerId == NetworkId::cms_invalid )
	{
		WARNING(true, ("[designer bug] reinitializeVendor() player has an invalid NetworkId.")); 
		return;
	}
	
	CommoditiesMarket::reinitializeVendor( *auctionContainer, playerId );
}

void JNICALL ScriptMethodsAuctionNamespace::updateVendorStatus(JNIEnv *env, jobject script, jlong vendor, jint status)
{
	ServerObject const * vendorObject = nullptr;

	if (!JavaLibrary::getObject(vendor, vendorObject))
	{
		WARNING(true, ("[designer bug] updateVendorStatus() vendor is invalid.")); 
		return;
	}

	CommoditiesMarket::updateVendorStatus(vendorObject->getNetworkId(), status);
}
