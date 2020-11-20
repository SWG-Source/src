//========================================================================
//
// ScriptMethodsCollection.cpp - implements script methods dealing with collection
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#include "serverScript/FirstServerScript.h"
#include "serverScript/JavaLibrary.h"

#include "serverGame/CreatureObject.h"
#include "serverGame/PlayerCreatureController.h"
#include "serverGame/PlayerObject.h"

using namespace JNIWrappersNamespace;

// ======================================================================
// ScriptMethodsCollectionNamespace
// ======================================================================

namespace ScriptMethodsCollectionNamespace
{
	bool install();
	jboolean     JNICALL modifyCollectionSlotValue(JNIEnv *env, jobject self, jlong player, jstring slotName, jlong delta);
	jlong        JNICALL getCollectionSlotValue(JNIEnv *env, jobject self, jlong player, jstring slotName);
	jboolean     JNICALL hasCompletedCollectionSlotPrereq(JNIEnv *env, jobject self, jlong player, jstring slotName);
	jboolean     JNICALL hasCompletedCollectionSlot(JNIEnv *env, jobject self, jlong player, jstring slotName);
	jboolean     JNICALL hasCompletedCollection(JNIEnv *env, jobject self, jlong player, jstring collectionName);
	jboolean     JNICALL hasCompletedCollectionPage(JNIEnv *env, jobject self, jlong player, jstring pageName);
	jboolean     JNICALL hasCompletedCollectionBook(JNIEnv *env, jobject self, jlong player, jstring bookName);
	jobjectArray JNICALL getCompletedCollectionSlotsInCollection(JNIEnv *env, jobject self, jlong player, jstring collectionName);
	jobjectArray JNICALL getCompletedCollectionSlotsInPage(JNIEnv *env, jobject self, jlong player, jstring pageName);
	jobjectArray JNICALL getCompletedCollectionsInPage(JNIEnv *env, jobject self, jlong player, jstring pageName);
	jobjectArray JNICALL getCompletedCollectionSlotsInBook(JNIEnv *env, jobject self, jlong player, jstring bookName);
	jobjectArray JNICALL getCompletedCollectionsInBook(JNIEnv *env, jobject self, jlong player, jstring bookName);
	jobjectArray JNICALL getCompletedCollectionPagesInBook(JNIEnv *env, jobject self, jlong player, jstring bookName);
	jobjectArray JNICALL getCompletedCollectionBooks(JNIEnv *env, jobject self, jlong player);
	jobjectArray JNICALL getCollectionSlotInfo(JNIEnv *env, jobject self, jstring slotName);
	jboolean     JNICALL isCollectionSlotATitle(JNIEnv *env, jobject self, jstring slotName);
	jboolean     JNICALL isCollectionATitle(JNIEnv *env, jobject self, jstring collectionName);
	jboolean     JNICALL isCollectionPageATitle(JNIEnv *env, jobject self, jstring pageName);
	jobjectArray JNICALL getCollectionSlotCategoryInfo(JNIEnv *env, jobject self, jstring slotName);
	jobjectArray JNICALL getCollectionSlotPrereqInfo(JNIEnv *env, jobject self, jstring slotName);
	jstring      JNICALL getCollectionSlotName(JNIEnv *env, jobject self, jint collectionSlotId);
	jlong		 JNICALL getCollectionSlotMaxValue(JNIEnv *env, jobject self, jstring slotName);
	jobjectArray JNICALL getAllCollectionSlotsInCollection(JNIEnv *env, jobject self, jstring collectionName);
	jobjectArray JNICALL getAllCollectionSlotsInPage(JNIEnv *env, jobject self, jstring pageName);
	jobjectArray JNICALL getAllCollectionsInPage(JNIEnv *env, jobject self, jstring pageName);
	jobjectArray JNICALL getAllCollectionSlotsInBook(JNIEnv *env, jobject self, jstring bookName);
	jobjectArray JNICALL getAllCollectionsInBook(JNIEnv *env, jobject self, jstring bookName);
	jobjectArray JNICALL getAllCollectionPagesInBook(JNIEnv *env, jobject self, jstring bookName);
	jobjectArray JNICALL getAllCollectionBooks(JNIEnv *env, jobject self);
	jobjectArray JNICALL getAllCollectionSlotsInCategory(JNIEnv *env, jobject self, jstring categoryName);
	jobjectArray JNICALL getAllCollectionSlotsInCategoryInCollection(JNIEnv *env, jobject self, jstring collectionName, jstring categoryName);
	jobjectArray JNICALL getAllCollectionSlotsInCategoryInPage(JNIEnv *env, jobject self, jstring pageName, jstring categoryName);
	jobjectArray JNICALL getAllCollectionSlotsInCategoryInBook(JNIEnv *env, jobject self, jstring bookName, jstring categoryName);
	jobjectArray JNICALL getAllCollectionSlotCategoriesInCollection(JNIEnv *env, jobject self, jstring collectionName);
	jobjectArray JNICALL getAllCollectionSlotCategoriesInPage(JNIEnv *env, jobject self, jstring pageName);
	jobjectArray JNICALL getAllCollectionSlotCategoriesInBook(JNIEnv *env, jobject self, jstring bookName);
	jobjectArray JNICALL getAllCollectionSlotCategories(JNIEnv *env, jobject self);
}

//========================================================================
// install
//========================================================================

bool ScriptMethodsCollectionNamespace::install()
{
	const JNINativeMethod NATIVES[] = {
#define JF(a,b,c) {a,b,(void*)(ScriptMethodsCollectionNamespace::c)}
		JF("_modifyCollectionSlotValue", "(JLjava/lang/String;J)Z", modifyCollectionSlotValue),
		JF("_getCollectionSlotValue", "(JLjava/lang/String;)J", getCollectionSlotValue),
		JF("_hasCompletedCollectionSlotPrereq", "(JLjava/lang/String;)Z", hasCompletedCollectionSlotPrereq),
		JF("_hasCompletedCollectionSlot", "(JLjava/lang/String;)Z", hasCompletedCollectionSlot),
		JF("_hasCompletedCollection", "(JLjava/lang/String;)Z", hasCompletedCollection),
		JF("_hasCompletedCollectionPage", "(JLjava/lang/String;)Z", hasCompletedCollectionPage),
		JF("_hasCompletedCollectionBook", "(JLjava/lang/String;)Z", hasCompletedCollectionBook),
		JF("_getCompletedCollectionSlotsInCollection", "(JLjava/lang/String;)[Ljava/lang/String;", getCompletedCollectionSlotsInCollection),
		JF("_getCompletedCollectionSlotsInPage", "(JLjava/lang/String;)[Ljava/lang/String;", getCompletedCollectionSlotsInPage),
		JF("_getCompletedCollectionsInPage", "(JLjava/lang/String;)[Ljava/lang/String;", getCompletedCollectionsInPage),
		JF("_getCompletedCollectionSlotsInBook", "(JLjava/lang/String;)[Ljava/lang/String;", getCompletedCollectionSlotsInBook),
		JF("_getCompletedCollectionsInBook", "(JLjava/lang/String;)[Ljava/lang/String;", getCompletedCollectionsInBook),
		JF("_getCompletedCollectionPagesInBook", "(JLjava/lang/String;)[Ljava/lang/String;", getCompletedCollectionPagesInBook),
		JF("_getCompletedCollectionBooks", "(J)[Ljava/lang/String;", getCompletedCollectionBooks),
		JF("getCollectionSlotInfo", "(Ljava/lang/String;)[Ljava/lang/String;", getCollectionSlotInfo),
		JF("isCollectionSlotATitle", "(Ljava/lang/String;)Z", isCollectionSlotATitle),
		JF("isCollectionATitle", "(Ljava/lang/String;)Z", isCollectionATitle),
		JF("isCollectionPageATitle", "(Ljava/lang/String;)Z", isCollectionPageATitle),
		JF("getCollectionSlotCategoryInfo", "(Ljava/lang/String;)[Ljava/lang/String;", getCollectionSlotCategoryInfo),
		JF("getCollectionSlotPrereqInfo", "(Ljava/lang/String;)[Ljava/lang/String;", getCollectionSlotPrereqInfo),
		JF("getCollectionSlotName", "(I)Ljava/lang/String;", getCollectionSlotName),
		JF("getCollectionSlotMaxValue", "(Ljava/lang/String;)J", getCollectionSlotMaxValue),
		JF("getAllCollectionSlotsInCollection", "(Ljava/lang/String;)[Ljava/lang/String;", getAllCollectionSlotsInCollection),
		JF("getAllCollectionSlotsInPage", "(Ljava/lang/String;)[Ljava/lang/String;", getAllCollectionSlotsInPage),
		JF("getAllCollectionsInPage", "(Ljava/lang/String;)[Ljava/lang/String;", getAllCollectionsInPage),
		JF("getAllCollectionSlotsInBook", "(Ljava/lang/String;)[Ljava/lang/String;", getAllCollectionSlotsInBook),
		JF("getAllCollectionsInBook", "(Ljava/lang/String;)[Ljava/lang/String;", getAllCollectionsInBook),
		JF("getAllCollectionPagesInBook", "(Ljava/lang/String;)[Ljava/lang/String;", getAllCollectionPagesInBook),
		JF("getAllCollectionBooks", "()[Ljava/lang/String;", getAllCollectionBooks),
		JF("getAllCollectionSlotsInCategory", "(Ljava/lang/String;)[Ljava/lang/String;", getAllCollectionSlotsInCategory),
		JF("getAllCollectionSlotsInCategoryInCollection", "(Ljava/lang/String;Ljava/lang/String;)[Ljava/lang/String;", getAllCollectionSlotsInCategoryInCollection),
		JF("getAllCollectionSlotsInCategoryInPage", "(Ljava/lang/String;Ljava/lang/String;)[Ljava/lang/String;", getAllCollectionSlotsInCategoryInPage),
		JF("getAllCollectionSlotsInCategoryInBook", "(Ljava/lang/String;Ljava/lang/String;)[Ljava/lang/String;", getAllCollectionSlotsInCategoryInBook),
		JF("getAllCollectionSlotCategoriesInCollection", "(Ljava/lang/String;)[Ljava/lang/String;", getAllCollectionSlotCategoriesInCollection),
		JF("getAllCollectionSlotCategoriesInPage", "(Ljava/lang/String;)[Ljava/lang/String;", getAllCollectionSlotCategoriesInPage),
		JF("getAllCollectionSlotCategoriesInBook", "(Ljava/lang/String;)[Ljava/lang/String;", getAllCollectionSlotCategoriesInBook),
		JF("getAllCollectionSlotCategories", "()[Ljava/lang/String;", getAllCollectionSlotCategories),
	};

	return JavaLibrary::registerNatives(NATIVES, sizeof(NATIVES)/sizeof(NATIVES[0]));
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsCollectionNamespace::modifyCollectionSlotValue(JNIEnv *env, jobject self, jlong player, jstring slotName, jlong delta)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(player, creature))
		return JNI_FALSE;

	PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(creature);
	if (!playerObject)
		return JNI_FALSE;

	JavaStringParam localSlotName(slotName);
	std::string slotNameString;
	JavaLibrary::convert(localSlotName, slotNameString);

	return playerObject->modifyCollectionSlotValue(slotNameString, static_cast<int64>(delta));
}

// ----------------------------------------------------------------------

jlong JNICALL ScriptMethodsCollectionNamespace::getCollectionSlotValue(JNIEnv *env, jobject self, jlong player, jstring slotName)
{
	UNREF(self);

	CreatureObject * creature = 0;
	if (!JavaLibrary::getObject(player, creature))
		return static_cast<int64>(-1);

	PlayerObject * const playerObject = PlayerCreatureController::getPlayerObject(creature);
	if (!playerObject)
		return static_cast<int64>(-1);

	JavaStringParam localSlotName(slotName);
	std::string slotNameString;
	JavaLibrary::convert(localSlotName, slotNameString);

	unsigned long value;
	if (!playerObject->getCollectionSlotValue(slotNameString, value))
		return static_cast<int64>(-1);

	return static_cast<int64>(value);
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsCollectionNamespace::hasCompletedCollectionSlotPrereq(JNIEnv *env, jobject self, jlong player, jstring slotName)
{
	UNREF(self);

	CreatureObject const * creature = 0;
	if (!JavaLibrary::getObject(player, creature))
		return JNI_FALSE;

	PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(creature);
	if (!playerObject)
		return JNI_FALSE;

	JavaStringParam localSlotName(slotName);
	std::string slotNameString;
	JavaLibrary::convert(localSlotName, slotNameString);

	return playerObject->hasCompletedCollectionSlotPrereq(slotNameString);
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsCollectionNamespace::hasCompletedCollectionSlot(JNIEnv *env, jobject self, jlong player, jstring slotName)
{
	UNREF(self);

	CreatureObject const * creature = 0;
	if (!JavaLibrary::getObject(player, creature))
		return JNI_FALSE;

	PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(creature);
	if (!playerObject)
		return JNI_FALSE;

	JavaStringParam localSlotName(slotName);
	std::string slotNameString;
	JavaLibrary::convert(localSlotName, slotNameString);

	return playerObject->hasCompletedCollectionSlot(slotNameString);
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsCollectionNamespace::hasCompletedCollection(JNIEnv *env, jobject self, jlong player, jstring collectionName)
{
	UNREF(self);

	CreatureObject const * creature = 0;
	if (!JavaLibrary::getObject(player, creature))
		return JNI_FALSE;

	PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(creature);
	if (!playerObject)
		return JNI_FALSE;

	JavaStringParam localCollectionName(collectionName);
	std::string collectionNameString;
	JavaLibrary::convert(localCollectionName, collectionNameString);

	return playerObject->hasCompletedCollection(collectionNameString);
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsCollectionNamespace::hasCompletedCollectionPage(JNIEnv *env, jobject self, jlong player, jstring pageName)
{
	UNREF(self);

	CreatureObject const * creature = 0;
	if (!JavaLibrary::getObject(player, creature))
		return JNI_FALSE;

	PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(creature);
	if (!playerObject)
		return JNI_FALSE;

	JavaStringParam localPageName(pageName);
	std::string pageNameString;
	JavaLibrary::convert(localPageName, pageNameString);

	return playerObject->hasCompletedCollectionPage(pageNameString);
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsCollectionNamespace::hasCompletedCollectionBook(JNIEnv *env, jobject self, jlong player, jstring bookName)
{
	UNREF(self);

	CreatureObject const * creature = 0;
	if (!JavaLibrary::getObject(player, creature))
		return JNI_FALSE;

	PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(creature);
	if (!playerObject)
		return JNI_FALSE;

	JavaStringParam localBookName(bookName);
	std::string bookNameString;
	JavaLibrary::convert(localBookName, bookNameString);

	return playerObject->hasCompletedCollectionBook(bookNameString);
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsCollectionNamespace::getCompletedCollectionSlotsInCollection(JNIEnv *env, jobject self, jlong player, jstring collectionName)
{
	UNREF(self);

	CreatureObject const * creature = 0;
	if (!JavaLibrary::getObject(player, creature))
		return 0;

	PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(creature);
	if (!playerObject)
		return 0;

	JavaStringParam localCollectionName(collectionName);
	std::string collectionNameString;
	JavaLibrary::convert(localCollectionName, collectionNameString);

	std::vector<CollectionsDataTable::CollectionInfoSlot const *> collectionInfo;
	IGNORE_RETURN(playerObject->getCompletedCollectionSlotCountInCollection(collectionNameString, &collectionInfo));
	if (collectionInfo.empty())
		return 0;

	int i = 0;
	LocalObjectArrayRefPtr valueArray = createNewObjectArray(collectionInfo.size(), JavaLibrary::getClsString());
	for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iter = collectionInfo.begin(); iter != collectionInfo.end(); ++iter)
	{
		JavaString jval((*iter)->name);
		setObjectArrayElement(*valueArray, i++, jval);
	}

	return valueArray->getReturnValue();
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsCollectionNamespace::getCompletedCollectionSlotsInPage(JNIEnv *env, jobject self, jlong player, jstring pageName)
{
	UNREF(self);

	CreatureObject const * creature = 0;
	if (!JavaLibrary::getObject(player, creature))
		return 0;

	PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(creature);
	if (!playerObject)
		return 0;

	JavaStringParam localPageName(pageName);
	std::string pageNameString;
	JavaLibrary::convert(localPageName, pageNameString);

	std::vector<CollectionsDataTable::CollectionInfoSlot const *> collectionInfo;
	IGNORE_RETURN(playerObject->getCompletedCollectionSlotCountInPage(pageNameString, &collectionInfo));
	if (collectionInfo.empty())
		return 0;

	int i = 0;
	LocalObjectArrayRefPtr valueArray = createNewObjectArray(collectionInfo.size(), JavaLibrary::getClsString());
	for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iter = collectionInfo.begin(); iter != collectionInfo.end(); ++iter)
	{
		JavaString jval((*iter)->name);
		setObjectArrayElement(*valueArray, i++, jval);
	}

	return valueArray->getReturnValue();
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsCollectionNamespace::getCompletedCollectionsInPage(JNIEnv *env, jobject self, jlong player, jstring pageName)
{
	UNREF(self);

	CreatureObject const * creature = 0;
	if (!JavaLibrary::getObject(player, creature))
		return 0;

	PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(creature);
	if (!playerObject)
		return 0;

	JavaStringParam localPageName(pageName);
	std::string pageNameString;
	JavaLibrary::convert(localPageName, pageNameString);

	std::vector<CollectionsDataTable::CollectionInfoCollection const *> collectionInfo;
	IGNORE_RETURN(playerObject->getCompletedCollectionCountInPage(pageNameString, &collectionInfo));
	if (collectionInfo.empty())
		return 0;

	int i = 0;
	LocalObjectArrayRefPtr valueArray = createNewObjectArray(collectionInfo.size(), JavaLibrary::getClsString());
	for (std::vector<CollectionsDataTable::CollectionInfoCollection const *>::const_iterator iter = collectionInfo.begin(); iter != collectionInfo.end(); ++iter)
	{
		JavaString jval((*iter)->name);
		setObjectArrayElement(*valueArray, i++, jval);
	}

	return valueArray->getReturnValue();
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsCollectionNamespace::getCompletedCollectionSlotsInBook(JNIEnv *env, jobject self, jlong player, jstring bookName)
{
	UNREF(self);

	CreatureObject const * creature = 0;
	if (!JavaLibrary::getObject(player, creature))
		return 0;

	PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(creature);
	if (!playerObject)
		return 0;

	JavaStringParam localBookName(bookName);
	std::string bookNameString;
	JavaLibrary::convert(localBookName, bookNameString);

	std::vector<CollectionsDataTable::CollectionInfoSlot const *> collectionInfo;
	IGNORE_RETURN(playerObject->getCompletedCollectionSlotCountInBook(bookNameString, &collectionInfo));
	if (collectionInfo.empty())
		return 0;

	int i = 0;
	LocalObjectArrayRefPtr valueArray = createNewObjectArray(collectionInfo.size(), JavaLibrary::getClsString());
	for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iter = collectionInfo.begin(); iter != collectionInfo.end(); ++iter)
	{
		JavaString jval((*iter)->name);
		setObjectArrayElement(*valueArray, i++, jval);
	}

	return valueArray->getReturnValue();
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsCollectionNamespace::getCompletedCollectionsInBook(JNIEnv *env, jobject self, jlong player, jstring bookName)
{
	UNREF(self);

	CreatureObject const * creature = 0;
	if (!JavaLibrary::getObject(player, creature))
		return 0;

	PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(creature);
	if (!playerObject)
		return 0;

	JavaStringParam localBookName(bookName);
	std::string bookNameString;
	JavaLibrary::convert(localBookName, bookNameString);

	std::vector<CollectionsDataTable::CollectionInfoCollection const *> collectionInfo;
	IGNORE_RETURN(playerObject->getCompletedCollectionCountInBook(bookNameString, &collectionInfo));
	if (collectionInfo.empty())
		return 0;

	int i = 0;
	LocalObjectArrayRefPtr valueArray = createNewObjectArray(collectionInfo.size(), JavaLibrary::getClsString());
	for (std::vector<CollectionsDataTable::CollectionInfoCollection const *>::const_iterator iter = collectionInfo.begin(); iter != collectionInfo.end(); ++iter)
	{
		JavaString jval((*iter)->name);
		setObjectArrayElement(*valueArray, i++, jval);
	}

	return valueArray->getReturnValue();
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsCollectionNamespace::getCompletedCollectionPagesInBook(JNIEnv *env, jobject self, jlong player, jstring bookName)
{
	UNREF(self);

	CreatureObject const * creature = 0;
	if (!JavaLibrary::getObject(player, creature))
		return 0;

	PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(creature);
	if (!playerObject)
		return 0;

	JavaStringParam localBookName(bookName);
	std::string bookNameString;
	JavaLibrary::convert(localBookName, bookNameString);

	std::vector<CollectionsDataTable::CollectionInfoPage const *> collectionInfo;
	IGNORE_RETURN(playerObject->getCompletedCollectionPageCountInBook(bookNameString, &collectionInfo));
	if (collectionInfo.empty())
		return 0;

	int i = 0;
	LocalObjectArrayRefPtr valueArray = createNewObjectArray(collectionInfo.size(), JavaLibrary::getClsString());
	for (std::vector<CollectionsDataTable::CollectionInfoPage const *>::const_iterator iter = collectionInfo.begin(); iter != collectionInfo.end(); ++iter)
	{
		JavaString jval((*iter)->name);
		setObjectArrayElement(*valueArray, i++, jval);
	}

	return valueArray->getReturnValue();
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsCollectionNamespace::getCompletedCollectionBooks(JNIEnv *env, jobject self, jlong player)
{
	UNREF(self);

	CreatureObject const * creature = 0;
	if (!JavaLibrary::getObject(player, creature))
		return 0;

	PlayerObject const * const playerObject = PlayerCreatureController::getPlayerObject(creature);
	if (!playerObject)
		return 0;

	std::vector<CollectionsDataTable::CollectionInfoBook const *> collectionInfo;
	IGNORE_RETURN(playerObject->getCompletedCollectionBookCount(&collectionInfo));
	if (collectionInfo.empty())
		return 0;

	int i = 0;
	LocalObjectArrayRefPtr valueArray = createNewObjectArray(collectionInfo.size(), JavaLibrary::getClsString());
	for (std::vector<CollectionsDataTable::CollectionInfoBook const *>::const_iterator iter = collectionInfo.begin(); iter != collectionInfo.end(); ++iter)
	{
		JavaString jval((*iter)->name);
		setObjectArrayElement(*valueArray, i++, jval);
	}

	return valueArray->getReturnValue();
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsCollectionNamespace::getCollectionSlotInfo(JNIEnv *env, jobject self, jstring slotName)
{
	UNREF(self);

	JavaStringParam localSlotName(slotName);
	std::string slotNameString;
	JavaLibrary::convert(localSlotName, slotNameString);

	CollectionsDataTable::CollectionInfoSlot const * slot = CollectionsDataTable::getSlotByName(slotNameString);
	if (!slot)
		return 0;

	LocalObjectArrayRefPtr valueArray = createNewObjectArray(4, JavaLibrary::getClsString());
	setObjectArrayElement(*valueArray, 0, JavaString(slot->collection.page.book.name));
	setObjectArrayElement(*valueArray, 1, JavaString(slot->collection.page.name));
	setObjectArrayElement(*valueArray, 2, JavaString(slot->collection.name));
	setObjectArrayElement(*valueArray, 3, JavaString(slot->music));

	return valueArray->getReturnValue();
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsCollectionNamespace::isCollectionSlotATitle(JNIEnv *env, jobject self, jstring slotName)
{
	UNREF(self);

	JavaStringParam localSlotName(slotName);
	std::string slotNameString;
	JavaLibrary::convert(localSlotName, slotNameString);

	CollectionsDataTable::CollectionInfoSlot const * const slot = CollectionsDataTable::getSlotByName(slotNameString);
	return (slot && !slot->titles.empty());
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsCollectionNamespace::isCollectionATitle(JNIEnv *env, jobject self, jstring collectionName)
{
	UNREF(self);

	JavaStringParam localCollectionName(collectionName);
	std::string collectionNameString;
	JavaLibrary::convert(localCollectionName, collectionNameString);

	CollectionsDataTable::CollectionInfoCollection const * const collection = CollectionsDataTable::getCollectionByName(collectionNameString);

	return (collection && !collection->titles.empty());
}

// ----------------------------------------------------------------------

jboolean JNICALL ScriptMethodsCollectionNamespace::isCollectionPageATitle(JNIEnv *env, jobject self, jstring pageName)
{
	UNREF(self);

	JavaStringParam localPageName(pageName);
	std::string pageNameString;
	JavaLibrary::convert(localPageName, pageNameString);

	CollectionsDataTable::CollectionInfoPage const * const page = CollectionsDataTable::getPageByName(pageNameString);

	return (page && !page->titles.empty());
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsCollectionNamespace::getCollectionSlotCategoryInfo(JNIEnv *env, jobject self, jstring slotName)
{
	UNREF(self);

	JavaStringParam localSlotName(slotName);
	std::string slotNameString;
	JavaLibrary::convert(localSlotName, slotNameString);

	CollectionsDataTable::CollectionInfoSlot const * slot = CollectionsDataTable::getSlotByName(slotNameString);
	if (!slot)
		return 0;

	if (slot->categories.empty())
		return 0;

	int i = 0;
	LocalObjectArrayRefPtr valueArray = createNewObjectArray(slot->categories.size(), JavaLibrary::getClsString());
	for (std::vector<std::string>::const_iterator iter = slot->categories.begin(); iter != slot->categories.end(); ++iter)
	{
		JavaString jval(*iter);
		setObjectArrayElement(*valueArray, i++, jval);
	}

	return valueArray->getReturnValue();
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsCollectionNamespace::getCollectionSlotPrereqInfo(JNIEnv *env, jobject self, jstring slotName)
{
	UNREF(self);

	JavaStringParam localSlotName(slotName);
	std::string slotNameString;
	JavaLibrary::convert(localSlotName, slotNameString);

	CollectionsDataTable::CollectionInfoSlot const * slot = CollectionsDataTable::getSlotByName(slotNameString);
	if (!slot)
		return 0;

	if (slot->prereqsPtr.empty())
		return 0;

	int i = 0;
	LocalObjectArrayRefPtr valueArray = createNewObjectArray(slot->prereqsPtr.size(), JavaLibrary::getClsString());
	for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iter = slot->prereqsPtr.begin(); iter != slot->prereqsPtr.end(); ++iter)
	{
		JavaString jval((*iter)->name);
		setObjectArrayElement(*valueArray, i++, jval);
	}

	return valueArray->getReturnValue();
}

// ----------------------------------------------------------------------

jstring JNICALL ScriptMethodsCollectionNamespace::getCollectionSlotName(JNIEnv *env, jobject self, jint collectionSlotId)
{
	UNREF(self);

	CollectionsDataTable::CollectionInfoSlot const * slot = CollectionsDataTable::getSlotByBeginSlotId(collectionSlotId);
	if (!slot)
		return 0;

	return JavaString(slot->name).getReturnValue();
}

// ----------------------------------------------------------------------

jlong JNICALL ScriptMethodsCollectionNamespace::getCollectionSlotMaxValue(JNIEnv *env, jobject self, jstring slotName)
{
	UNREF(self);
	
	JavaStringParam localSlotName(slotName);
	std::string slotNameString;
	JavaLibrary::convert(localSlotName, slotNameString);
	
	CollectionsDataTable::CollectionInfoSlot const * slot = CollectionsDataTable::getSlotByName(slotNameString);
	if (!slot)
		return -1;

	return static_cast<int64>(slot->maxSlotValue);
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsCollectionNamespace::getAllCollectionSlotsInCollection(JNIEnv *env, jobject self, jstring collectionName)
{
	UNREF(self);

	JavaStringParam localCollectionName(collectionName);
	std::string collectionNameString;
	JavaLibrary::convert(localCollectionName, collectionNameString);

	std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & slots = CollectionsDataTable::getSlotsInCollection(collectionNameString);
	if (slots.empty())
		return 0;

	int i = 0;
	LocalObjectArrayRefPtr valueArray = createNewObjectArray(slots.size(), JavaLibrary::getClsString());
	for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iter = slots.begin(); iter != slots.end(); ++iter)
	{
		JavaString jval((*iter)->name);
		setObjectArrayElement(*valueArray, i++, jval);
	}

	return valueArray->getReturnValue();
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsCollectionNamespace::getAllCollectionSlotsInPage(JNIEnv *env, jobject self, jstring pageName)
{
	UNREF(self);

	JavaStringParam localPageName(pageName);
	std::string pageNameString;
	JavaLibrary::convert(localPageName, pageNameString);

	std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & slots = CollectionsDataTable::getSlotsInPage(pageNameString);
	if (slots.empty())
		return 0;

	int i = 0;
	LocalObjectArrayRefPtr valueArray = createNewObjectArray(slots.size(), JavaLibrary::getClsString());
	for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iter = slots.begin(); iter != slots.end(); ++iter)
	{
		JavaString jval((*iter)->name);
		setObjectArrayElement(*valueArray, i++, jval);
	}

	return valueArray->getReturnValue();
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsCollectionNamespace::getAllCollectionsInPage(JNIEnv *env, jobject self, jstring pageName)
{
	UNREF(self);

	JavaStringParam localPageName(pageName);
	std::string pageNameString;
	JavaLibrary::convert(localPageName, pageNameString);

	std::vector<CollectionsDataTable::CollectionInfoCollection const *> const & collections = CollectionsDataTable::getCollectionsInPage(pageNameString);
	if (collections.empty())
		return 0;

	int i = 0;
	LocalObjectArrayRefPtr valueArray = createNewObjectArray(collections.size(), JavaLibrary::getClsString());
	for (std::vector<CollectionsDataTable::CollectionInfoCollection const *>::const_iterator iter = collections.begin(); iter != collections.end(); ++iter)
	{
		JavaString jval((*iter)->name);
		setObjectArrayElement(*valueArray, i++, jval);
	}

	return valueArray->getReturnValue();
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsCollectionNamespace::getAllCollectionSlotsInBook(JNIEnv *env, jobject self, jstring bookName)
{
	UNREF(self);

	JavaStringParam localBookName(bookName);
	std::string bookNameString;
	JavaLibrary::convert(localBookName, bookNameString);

	std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & slots = CollectionsDataTable::getSlotsInBook(bookNameString);
	if (slots.empty())
		return 0;

	int i = 0;
	LocalObjectArrayRefPtr valueArray = createNewObjectArray(slots.size(), JavaLibrary::getClsString());
	for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iter = slots.begin(); iter != slots.end(); ++iter)
	{
		JavaString jval((*iter)->name);
		setObjectArrayElement(*valueArray, i++, jval);
	}

	return valueArray->getReturnValue();
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsCollectionNamespace::getAllCollectionsInBook(JNIEnv *env, jobject self, jstring bookName)
{
	UNREF(self);

	JavaStringParam localBookName(bookName);
	std::string bookNameString;
	JavaLibrary::convert(localBookName, bookNameString);

	std::vector<CollectionsDataTable::CollectionInfoCollection const *> const & collections = CollectionsDataTable::getCollectionsInBook(bookNameString);
	if (collections.empty())
		return 0;

	int i = 0;
	LocalObjectArrayRefPtr valueArray = createNewObjectArray(collections.size(), JavaLibrary::getClsString());
	for (std::vector<CollectionsDataTable::CollectionInfoCollection const *>::const_iterator iter = collections.begin(); iter != collections.end(); ++iter)
	{
		JavaString jval((*iter)->name);
		setObjectArrayElement(*valueArray, i++, jval);
	}

	return valueArray->getReturnValue();
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsCollectionNamespace::getAllCollectionPagesInBook(JNIEnv *env, jobject self, jstring bookName)
{
	UNREF(self);

	JavaStringParam localBookName(bookName);
	std::string bookNameString;
	JavaLibrary::convert(localBookName, bookNameString);

	std::vector<CollectionsDataTable::CollectionInfoPage const *> const & pages = CollectionsDataTable::getPagesInBook(bookNameString);
	if (pages.empty())
		return 0;

	int i = 0;
	LocalObjectArrayRefPtr valueArray = createNewObjectArray(pages.size(), JavaLibrary::getClsString());
	for (std::vector<CollectionsDataTable::CollectionInfoPage const *>::const_iterator iter = pages.begin(); iter != pages.end(); ++iter)
	{
		JavaString jval((*iter)->name);
		setObjectArrayElement(*valueArray, i++, jval);
	}

	return valueArray->getReturnValue();
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsCollectionNamespace::getAllCollectionBooks(JNIEnv *env, jobject self)
{
	UNREF(self);

	std::vector<CollectionsDataTable::CollectionInfoBook const *> const & books = CollectionsDataTable::getAllBooks();
	if (books.empty())
		return 0;

	int i = 0;
	LocalObjectArrayRefPtr valueArray = createNewObjectArray(books.size(), JavaLibrary::getClsString());
	for (std::vector<CollectionsDataTable::CollectionInfoBook const *>::const_iterator iter = books.begin(); iter != books.end(); ++iter)
	{
		JavaString jval((*iter)->name);
		setObjectArrayElement(*valueArray, i++, jval);
	}

	return valueArray->getReturnValue();
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsCollectionNamespace::getAllCollectionSlotsInCategory(JNIEnv *env, jobject self, jstring categoryName)
{
	UNREF(self);

	JavaStringParam localCategoryName(categoryName);
	std::string categoryNameString;
	JavaLibrary::convert(localCategoryName, categoryNameString);

	std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & slots = CollectionsDataTable::getSlotsInCategory(categoryNameString);
	if (slots.empty())
		return 0;

	int i = 0;
	LocalObjectArrayRefPtr valueArray = createNewObjectArray(slots.size(), JavaLibrary::getClsString());
	for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iter = slots.begin(); iter != slots.end(); ++iter)
	{
		JavaString jval((*iter)->name);
		setObjectArrayElement(*valueArray, i++, jval);
	}

	return valueArray->getReturnValue();
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsCollectionNamespace::getAllCollectionSlotsInCategoryInCollection(JNIEnv *env, jobject self, jstring collectionName, jstring categoryName)
{
	UNREF(self);

	JavaStringParam localCollectionName(collectionName);
	std::string collectionNameString;
	JavaLibrary::convert(localCollectionName, collectionNameString);

	JavaStringParam localCategoryName(categoryName);
	std::string categoryNameString;
	JavaLibrary::convert(localCategoryName, categoryNameString);

	std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & slots = CollectionsDataTable::getSlotsInCategoryByCollection(collectionNameString, categoryNameString);
	if (slots.empty())
		return 0;

	int i = 0;
	LocalObjectArrayRefPtr valueArray = createNewObjectArray(slots.size(), JavaLibrary::getClsString());
	for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iter = slots.begin(); iter != slots.end(); ++iter)
	{
		JavaString jval((*iter)->name);
		setObjectArrayElement(*valueArray, i++, jval);
	}

	return valueArray->getReturnValue();
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsCollectionNamespace::getAllCollectionSlotsInCategoryInPage(JNIEnv *env, jobject self, jstring pageName, jstring categoryName)
{
	UNREF(self);

	JavaStringParam localPageName(pageName);
	std::string pageNameString;
	JavaLibrary::convert(localPageName, pageNameString);

	JavaStringParam localCategoryName(categoryName);
	std::string categoryNameString;
	JavaLibrary::convert(localCategoryName, categoryNameString);

	std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & slots = CollectionsDataTable::getSlotsInCategoryByPage(pageNameString, categoryNameString);
	if (slots.empty())
		return 0;

	int i = 0;
	LocalObjectArrayRefPtr valueArray = createNewObjectArray(slots.size(), JavaLibrary::getClsString());
	for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iter = slots.begin(); iter != slots.end(); ++iter)
	{
		JavaString jval((*iter)->name);
		setObjectArrayElement(*valueArray, i++, jval);
	}

	return valueArray->getReturnValue();
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsCollectionNamespace::getAllCollectionSlotsInCategoryInBook(JNIEnv *env, jobject self, jstring bookName, jstring categoryName)
{
	UNREF(self);

	JavaStringParam localBookName(bookName);
	std::string bookNameString;
	JavaLibrary::convert(localBookName, bookNameString);

	JavaStringParam localCategoryName(categoryName);
	std::string categoryNameString;
	JavaLibrary::convert(localCategoryName, categoryNameString);

	std::vector<CollectionsDataTable::CollectionInfoSlot const *> const & slots = CollectionsDataTable::getSlotsInCategoryByBook(bookNameString, categoryNameString);
	if (slots.empty())
		return 0;

	int i = 0;
	LocalObjectArrayRefPtr valueArray = createNewObjectArray(slots.size(), JavaLibrary::getClsString());
	for (std::vector<CollectionsDataTable::CollectionInfoSlot const *>::const_iterator iter = slots.begin(); iter != slots.end(); ++iter)
	{
		JavaString jval((*iter)->name);
		setObjectArrayElement(*valueArray, i++, jval);
	}

	return valueArray->getReturnValue();
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsCollectionNamespace::getAllCollectionSlotCategoriesInCollection(JNIEnv *env, jobject self, jstring collectionName)
{
	UNREF(self);

	JavaStringParam localCollectionName(collectionName);
	std::string collectionNameString;
	JavaLibrary::convert(localCollectionName, collectionNameString);

	std::set<std::string> const & categories = CollectionsDataTable::getAllSlotCategoriesInCollection(collectionNameString);
	if (categories.empty())
		return 0;

	int i = 0;
	LocalObjectArrayRefPtr valueArray = createNewObjectArray(categories.size(), JavaLibrary::getClsString());
	for (std::set<std::string>::const_iterator iter = categories.begin(); iter != categories.end(); ++iter)
	{
		JavaString jval(*iter);
		setObjectArrayElement(*valueArray, i++, jval);
	}

	return valueArray->getReturnValue();
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsCollectionNamespace::getAllCollectionSlotCategoriesInPage(JNIEnv *env, jobject self, jstring pageName)
{
	UNREF(self);

	JavaStringParam localPageName(pageName);
	std::string pageNameString;
	JavaLibrary::convert(localPageName, pageNameString);

	std::set<std::string> const & categories = CollectionsDataTable::getAllSlotCategoriesInPage(pageNameString);
	if (categories.empty())
		return 0;

	int i = 0;
	LocalObjectArrayRefPtr valueArray = createNewObjectArray(categories.size(), JavaLibrary::getClsString());
	for (std::set<std::string>::const_iterator iter = categories.begin(); iter != categories.end(); ++iter)
	{
		JavaString jval(*iter);
		setObjectArrayElement(*valueArray, i++, jval);
	}

	return valueArray->getReturnValue();
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsCollectionNamespace::getAllCollectionSlotCategoriesInBook(JNIEnv *env, jobject self, jstring bookName)
{
	UNREF(self);

	JavaStringParam localBookName(bookName);
	std::string bookNameString;
	JavaLibrary::convert(localBookName, bookNameString);

	std::set<std::string> const & categories = CollectionsDataTable::getAllSlotCategoriesInBook(bookNameString);
	if (categories.empty())
		return 0;

	int i = 0;
	LocalObjectArrayRefPtr valueArray = createNewObjectArray(categories.size(), JavaLibrary::getClsString());
	for (std::set<std::string>::const_iterator iter = categories.begin(); iter != categories.end(); ++iter)
	{
		JavaString jval(*iter);
		setObjectArrayElement(*valueArray, i++, jval);
	}

	return valueArray->getReturnValue();
}

// ----------------------------------------------------------------------

jobjectArray JNICALL ScriptMethodsCollectionNamespace::getAllCollectionSlotCategories(JNIEnv *env, jobject self)
{
	UNREF(self);

	std::set<std::string> const & categories = CollectionsDataTable::getAllSlotCategories();
	if (categories.empty())
		return 0;

	int i = 0;
	LocalObjectArrayRefPtr valueArray = createNewObjectArray(categories.size(), JavaLibrary::getClsString());
	for (std::set<std::string>::const_iterator iter = categories.begin(); iter != categories.end(); ++iter)
	{
		JavaString jval(*iter);
		setObjectArrayElement(*valueArray, i++, jval);
	}

	return valueArray->getReturnValue();
}

//========================================================================
