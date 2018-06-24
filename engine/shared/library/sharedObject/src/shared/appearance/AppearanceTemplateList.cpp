// ======================================================================
//
// AppearanceTemplateList.cpp
// copyright 1999 Bootprint Entertainment
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/AppearanceTemplateList.h"

#include "sharedDebug/DataLint.h"
#include "sharedDebug/DebugFlags.h"
#include "sharedFoundation/CrcLowerString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedFile/AsynchronousLoader.h"
#include "sharedFile/Iff.h"
#include "sharedFile/TreeFile.h"
#include "sharedObject/AppearanceTemplate.h"
#include "sharedObject/ConfigSharedObject.h"
#include "sharedRandom/Random.h"

#include <map>
#include <set>
#include <algorithm>
#include <vector>

// ======================================================================
// AppearanceTemplateListNamespace
// ======================================================================

namespace AppearanceTemplateListNamespace
{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	typedef std::map<Tag, AppearanceTemplateList::CreateFunction>                    TagBindingMap;
	typedef std::map<const CrcString *, const CrcString *, LessPointerComparator>    RedirectorMap;
	typedef std::set<AppearanceTemplate *>                                           AnonymousTemplates;
	typedef std::map<const CrcString *, AppearanceTemplate *, LessPointerComparator> NamedTemplates;
	typedef std::pair<float, AppearanceTemplate *>                                   TimedTemplate;
	typedef std::map<const CrcString *, TimedTemplate, LessPointerComparator>        NamedTimedTemplates;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	const Tag TAG_APT  = TAG3(A,P,T);
	const Tag TAG_MESH = TAG(M,E,S,H);

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	const char* const   ms_defaultAppearanceTemplateName = "appearance/defaultappearance.apt";
	float               ms_keepTime;
	float               ms_keepEpsilon;
	bool                ms_installed;
	bool                ms_debugReport;
	bool                ms_debugReportVerbose;
	bool                ms_debugReportTimedTemplates;
	TagBindingMap       ms_tagBindingMap;
	RedirectorMap       ms_redirectorMap;
	AnonymousTemplates  ms_anonymousTemplates;
	NamedTemplates      ms_namedTemplates;
	NamedTimedTemplates ms_namedTimedTemplates;
	bool                ms_allowTimedTemplates;
	bool                ms_useTimedTemplates;
	bool ms_logCreate;
	bool ms_logFetch;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

	void                remove();
	AppearanceTemplate *create(const char *fileName);
	void                addAnonymousAppearanceTemplate(AppearanceTemplate *appearanceTemplate);
	void                addNamedAppearanceTemplate(AppearanceTemplate *appearanceTemplate);
	void                addNamedTimedAppearanceTemplate(AppearanceTemplate *appearanceTemplate);
	void                removeAppearanceTemplate(AppearanceTemplate *appearanceTemplate);
	void                debugReport();
	void                debugReportVerbose();
	void                debugReportTimedTemplates();

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}

using namespace AppearanceTemplateListNamespace;

// ======================================================================
// STATIC PUBLIC AppearanceTemplateList
// ======================================================================

/**
 * Install the AppearanceTemplateList.
 */
void AppearanceTemplateList::install(bool useTimedTemplates, bool ensureDefaultAppearanceExists)
{
	DEBUG_FATAL(ms_installed, ("double install"));
	ms_installed = true;

	DebugFlags::registerFlag(ms_debugReport, "SharedObject/AppearanceTemplateList", "debugReport", debugReport);
	DebugFlags::registerFlag(ms_debugReportTimedTemplates, "SharedObject/AppearanceTemplateList", "reportTimedTemplates", debugReportTimedTemplates);
	DebugFlags::registerFlag(ms_debugReportVerbose, "SharedObject/AppearanceTemplateList", "debugReportVerbose", debugReportVerbose);
	DebugFlags::registerFlag(ms_useTimedTemplates, "SharedObject/AppearanceTemplateList", "useTimedTemplates");
	DebugFlags::registerFlag(ms_logCreate, "SharedObject/AppearanceTemplateList", "logCreate");
	DebugFlags::registerFlag(ms_logFetch, "SharedObject/AppearanceTemplateList", "logFetch");

	ms_useTimedTemplates = useTimedTemplates;
	ms_keepTime          = ConfigSharedObject::getAppearanceTemplateListKeepTime();
	ms_keepEpsilon       = ConfigSharedObject::getAppearanceTemplateListKeepEpsilon();

	//-- verify the default appearance exists
	if (ensureDefaultAppearanceExists)
		FATAL(!TreeFile::exists(getDefaultAppearanceTemplateName()), ("%s could not be found. Are your paths set up correctly?", getDefaultAppearanceTemplateName()));

	if (TreeFile::isLoggingFiles())
		delete TreeFile::open(getDefaultAppearanceTemplateName(), AbstractFile::PriorityData, true);

	ExitChain::add(AppearanceTemplateListNamespace::remove, "AppearanceTemplateList::remove", 0, false);
}

// ----------------------------------------------------------------------
/**
 * Assign a binding between a Tag and a CreateFunction.
 * 
 * If a previous binding for the specified tag exists, it will be replaced
 * by the new binding.
 * 
 * @param tag  Tag for the binding
 * @param createFunction  Create function to call for data files with the tag
 */

void AppearanceTemplateList::assignBinding(const Tag tag, const CreateFunction createFunction)
{
	DEBUG_FATAL(!ms_installed, ("not installed"));
	NOT_NULL(createFunction);

#ifdef _DEBUG
	char tagString[5];
	ConvertTagToString(tag, tagString);
	DEBUG_REPORT_LOG_PRINT(ms_debugReport, ("Added AppearanceTemplateList binding %s\n", tagString));
#endif

	ms_tagBindingMap[tag] = createFunction;
}

// ----------------------------------------------------------------------
/**
 * Remove a binding from the AppearanceTemplateList.
 * 
 * If the specified Tag is not bound, the routine will return without
 * error.
 * 
 * @param tag  Tag for the binding to remove
 */

void AppearanceTemplateList::removeBinding(const Tag tag)
{
	DEBUG_FATAL(!ms_installed, ("not installed"));

	IGNORE_RETURN(ms_tagBindingMap.erase(tag));
}

// ----------------------------------------------------------------------

void AppearanceTemplateList::update(const float elapsedTime)
{
	DEBUG_FATAL(!ms_installed, ("not installed"));

	//-- walk the timed appearance template list and tick down
	NamedTimedTemplates::iterator iter = ms_namedTimedTemplates.begin();
	NamedTimedTemplates::iterator end  = ms_namedTimedTemplates.end();

	while (iter != end)
	{
		//-- see if the appearance template has expired
		iter->second.first -= elapsedTime;
		if (iter->second.first <= 0.f)
		{
			DEBUG_REPORT_LOG(ms_logCreate, ("Destroying timed appearance %s\n", iter->first->getString()));
			delete const_cast<AppearanceTemplate *>(iter->second.second);

			ms_namedTimedTemplates.erase(iter++);
			end = ms_namedTimedTemplates.end();
		}
		else
			++iter;
	}
}

// ----------------------------------------------------------------------

void AppearanceTemplateList::setAllowTimedTemplates(const bool allowTimedTemplates)
{
	ms_allowTimedTemplates = allowTimedTemplates;

	if (!ms_allowTimedTemplates)
		update(ms_keepTime + ms_keepEpsilon);
}

// ----------------------------------------------------------------------

const AppearanceTemplate *AppearanceTemplateList::fetch(const char *const fileName)
{
	bool found = false;
	return fetch(fileName, found);
}

//----------------------------------------------------------------------

const AppearanceTemplate *AppearanceTemplateList::fetch(const char *const fileName, bool & found)
{
	DEBUG_FATAL(!ms_installed, ("not installed"));
	NOT_NULL(fileName);

	DEBUG_REPORT_LOG(ms_logFetch, ("[fetch] AppearanceTemplateList::fetch: %s\n", fileName));
	
	found = false;
	char const * actualFileName = fileName;
	if (!fileName)
	{
		DEBUG_WARNING(true, ("AppearanceTemplateList::fetch passed nullptr fileName, using default"));
		actualFileName = getDefaultAppearanceTemplateName();
	}
	else if (!*fileName)
	{
		DEBUG_WARNING(true, ("AppearanceTemplateList::fetch passed empty fileName, using default"));
		actualFileName = getDefaultAppearanceTemplateName();
	}
	else if (!TreeFile::exists(fileName))
	{
		DEBUG_WARNING(true, ("AppearanceTemplateList::fetch could not open '%s', using default", fileName));
		actualFileName = getDefaultAppearanceTemplateName();
	}
	
	if (actualFileName) 
	{
		found = true;

		//-- search for the appearance
		AppearanceTemplate *const appearanceTemplate = create(actualFileName);

		//-- up the reference count
		return fetch(appearanceTemplate);
	}

	DEBUG_WARNING(true, ("AppearanceTemplateList::fetch actualFileName fetch for %s failed.", actualFileName));

	return nullptr;
}

// ----------------------------------------------------------------------
/**
 * Add a reference to the specified Appearance.
 * 
 * This routine will do nothing if passed in nullptr.  Otherwise, it will
 * increase the reference count of the specified AppearanceTemplate
 * by one.
 * 
 * @param appearanceTemplate  AppearanceTemplate to remove the reference count from
 * @return The AppearanceTemplate that was passed into the routine.
 * @see AppearanceTemplateList::release()
 */

const AppearanceTemplate *AppearanceTemplateList::fetch(const AppearanceTemplate *const appearanceTemplate)
{
	DEBUG_FATAL(!ms_installed, ("not installed"));
	if (appearanceTemplate)
		IGNORE_RETURN(appearanceTemplate->incrementReference());

	return appearanceTemplate;
}

// ----------------------------------------------------------------------
/**
 * Fetch an AppearanceTemplate from an Iff.
 * 
 * This routine will increment the reference count of the AppearanceTemplate
 * by one.
 * 
 * @param iff  File from which to load the AppearanceTemplate
 */

const AppearanceTemplate *AppearanceTemplateList::fetch(Iff *const iff)
{
	DEBUG_FATAL(!ms_installed, ("not installed"));
	NOT_NULL(iff);

	const Tag tag = iff->getCurrentName();

	TagBindingMap::iterator iter = ms_tagBindingMap.find(tag);
	if (iter == ms_tagBindingMap.end())
	{
		char tagString[5];
		ConvertTagToString(tag, tagString);
		WARNING(true, ("AppearanceTemplate binding %s not found", tagString));
		return 0; //lint !e527 // unreachable
	}

	AppearanceTemplate *const appearanceTemplate = (*iter).second(nullptr, iff);
	NOT_NULL(appearanceTemplate);

	addAnonymousAppearanceTemplate(appearanceTemplate);

	//-- up the reference count
	return fetch(appearanceTemplate);
}

// ----------------------------------------------------------------------
/**
 * Fetch an AppearanceTemplate from an existing object and add the object
 * to the AppearanceTemplateList.
 *
 * The AppearanceTemplate object is assumed to be created outside the
 * AppearanceTemplateList fetch-from-file or fetch-from-iff mechanism.
 * This may happen within an editor that creates an AppearanceTemplate
 * object from scratch.  In this case, the object is created outside
 * the AppearanceTemplateList and must be added to the list.  Use this
 * function to do so.  Failure to add the object to this
 * list likely will cause failures when an Appearance making use of the
 * AppearanceTemplate object gets released.
 *
 * @param appearanceTemplate  the AppearanceTemplate object to add to the AppearanceTemplateList
 *
 * @return                    the AppearanceTemplate object, returned just like all the other fetch routines
 */

const AppearanceTemplate *AppearanceTemplateList::fetchNew(AppearanceTemplate *const appearanceTemplate)
{
	DEBUG_FATAL(!ms_installed, ("not installed"));
	NOT_NULL(appearanceTemplate);

	const CrcLowerString &crcName  = appearanceTemplate->getCrcName();
	const char *const    crcString = crcName.getString();
	if (crcString && *crcString)
	{
		//-- we've got a named appearance template list
		addNamedAppearanceTemplate(appearanceTemplate);
	}
	else
	{
		//-- we've got an anonymous appearance template list
		addAnonymousAppearanceTemplate(appearanceTemplate);
	}

	//-- up the reference count
	return fetch(appearanceTemplate);
}

// ----------------------------------------------------------------------
/**
 * Remove a reference to the specified AppearanceTemplate.
 * 
 * This routine will do nothing if passed in nullptr.
 * 
 * If the reference count drops to 0, the AppearanceTemplate will be deleted.
 * 
 * @param appearanceTemplate  AppearanceTemplate to remove the reference count from
 * @see AppearanceTemplateList::release()
 */

void AppearanceTemplateList::release(const AppearanceTemplate *const appearanceTemplate)
{
	DEBUG_FATAL(!ms_installed, ("not installed"));
	if (appearanceTemplate && appearanceTemplate->decrementReference() <= 0)
	{
		DEBUG_FATAL(appearanceTemplate->getReferenceCount() < 0, ("referenceCount is negative"));

		AppearanceTemplate *const nonConstAppearanceTemplate = const_cast<AppearanceTemplate *>(appearanceTemplate);

		//-- remove from list
		removeAppearanceTemplate(nonConstAppearanceTemplate);
		CrcLowerString const & crcName  = appearanceTemplate->getCrcName();
		char const * const crcString = crcName.getString();

		//-- add it to the timed template list
		if (ms_useTimedTemplates && ms_allowTimedTemplates && crcString && *crcString)
		{
			DEBUG_REPORT_LOG(ms_logCreate, ("Adding timed appearance %s\n", appearanceTemplate->getName()));
			addNamedTimedAppearanceTemplate(nonConstAppearanceTemplate);
		}
		else
			delete nonConstAppearanceTemplate;
	}
}

// ----------------------------------------------------------------------
/**
 * Create an Appearance for the specified AppearanceTemplate.
 * 
 * @param fileName  Name of the AppearanceTemplate
 * @return An appearance for the specified AppearanceTemplate
 */

Appearance *AppearanceTemplateList::createAppearance(const char *const fileName)
{
	DEBUG_FATAL(!ms_installed, ("not installed"));

	//-- get the appearance template
	const AppearanceTemplate *const appearanceTemplate = fetch(fileName);

#ifdef _DEBUG 
        DataLint::pushAsset(fileName);
#endif	
	
	//probably should modify the macro sometime to just be quiet if this isn't defined
	if (appearanceTemplate == nullptr){
	    DEBUG_WARNING(true, ("FIX ME: Appearance template for %s could not be fetched - is it missing?", fileName));
	    return nullptr;  // Cekis: TODO: Figure out why the template can't be fetched. DarthArgus: always is due to a missing file or one of the redirectors having a bad path
	}

	//-- creating the appearance will increment the reference count
	Appearance *const appearance = appearanceTemplate->createAppearance();

#ifdef _DEBUG
	DataLint::popAsset();
#endif // _DEBUG

	//-- release the appearance template, since the appearance now holds a ref count for it
	release(appearanceTemplate);

	return appearance;
}

// ----------------------------------------------------------------------

const char* AppearanceTemplateList::getDefaultAppearanceTemplateName()
{
	return ms_defaultAppearanceTemplateName;
}

// ----------------------------------------------------------------------

void AppearanceTemplateList::garbageCollect ()
{
	//-- release the timed (cached) appearance templates
	if (ms_allowTimedTemplates)
	{
		setAllowTimedTemplates (false);
		setAllowTimedTemplates (true);
	}

	//-- call garbage collect on all appearance templates
	{
		NamedTemplates::iterator end = ms_namedTemplates.end ();
		NamedTemplates::iterator iter = ms_namedTemplates.begin ();
		for (; iter != end; ++iter)
			const_cast<AppearanceTemplate*> (iter->second)->garbageCollect ();
	}
}

// ======================================================================
// STATIC PRIVATE AppearanceTemplateList
// ======================================================================

/**
 * Remove a AppearanceTemplateList.
 */
void AppearanceTemplateListNamespace::remove()
{
	DEBUG_FATAL(!ms_installed, ("not installed"));
	ms_installed = false;

	DebugFlags::unregisterFlag(ms_debugReport);
	DebugFlags::unregisterFlag(ms_debugReportTimedTemplates);
	DebugFlags::unregisterFlag(ms_debugReportVerbose);
	DebugFlags::unregisterFlag(ms_useTimedTemplates);
	DebugFlags::unregisterFlag(ms_logCreate);
	DebugFlags::unregisterFlag(ms_logFetch);

	{
		//-- report if we leaked anonymous appearance templates
		const size_t entryCount = ms_anonymousTemplates.size();
		DEBUG_WARNING(entryCount, ("leaked %u anonymous appearance templates", entryCount));
		UNREF(entryCount);
		// shouldn't delete because item may depend on items that are memory block managed and already deleted, which could cause a crash
	}

	{
		//-- report if we leaked named appearance templates
#ifdef _DEBUG
		const size_t entryCount = ms_namedTemplates.size();
		DEBUG_WARNING(entryCount, ("leaked %u named appearance templates", entryCount));

		NamedTemplates::iterator end = ms_namedTemplates.end();
		for (NamedTemplates::iterator it = ms_namedTemplates.begin(); it != end; ++it)
		{
			DEBUG_REPORT_LOG(true, ("--leaked [%s]\n", it->second->getCrcName().getString()));
			// shouldn't delete because item may depend on items that are memory block managed and already deleted, which could cause a crash
		}
#endif
	}

	{
#ifdef _DEBUG
		const size_t entryCount = ms_namedTimedTemplates.size();
		if (entryCount)
		{
			DEBUG_REPORT_LOG(true, ("%u timed appearance templates remain\n", entryCount));
		}
#endif
	}

	{
		DEBUG_REPORT_LOG(ms_debugReport, ("AppearanceTemplateList redirector map:\n"));

		//-- delete the redirector map
		for (RedirectorMap::iterator iter = ms_redirectorMap.begin(); iter != ms_redirectorMap.end(); ++iter)
		{
			DEBUG_REPORT_LOG(ms_debugReport, ("  %4i  %s -> %s\n", std::distance(ms_redirectorMap.begin(), iter), iter->first->getString(), iter->second->getString()));
			delete const_cast<CrcString *>(iter->first);
			delete const_cast<CrcString *>(iter->second);
		}
	}
}

// ----------------------------------------------------------------------

AppearanceTemplate *AppearanceTemplateListNamespace::create(const char *const fileName)
{
	NOT_NULL(fileName);

	TemporaryCrcString actualFileName(fileName, true);

	//-- does the filename end in apt?
	if (strstr(fileName, ".apt") != 0)
	{
		//-- is the filename in the redirector map?
		RedirectorMap::iterator iter = ms_redirectorMap.find((const CrcString*)&actualFileName);
		if (iter == ms_redirectorMap.end())
		{
			//-- extract the real name from the apt
			Iff iff(fileName);
			iff.enterForm(TAG_APT);
				iff.enterForm(TAG_0000);
					iff.enterChunk(TAG_NAME);
				
						char redirectedFileName[Os::MAX_PATH_LENGTH];
						iff.read_string(redirectedFileName, Os::MAX_PATH_LENGTH);

						//-- make sure redirector name is not an apt file
						FATAL(strstr(redirectedFileName, ".apt") != 0, ("multiple levels of indirection found for appearance template redirector files %s(%s)", fileName, redirectedFileName));

						//-- add it to the redirector map
						iter = ms_redirectorMap.insert(RedirectorMap::value_type(new PersistentCrcString(actualFileName), new PersistentCrcString(redirectedFileName, true))).first;

					iff.exitChunk(TAG_NAME);
				iff.exitForm(TAG_0000);
			iff.exitForm(TAG_APT);
		}

		//-- create the target name instead
		return create(iter->second->getString());
	}

	//-- search for the appearance in the named list
	{
		NamedTemplates::iterator iter = ms_namedTemplates.find((const CrcString*)&actualFileName);
		if (iter != ms_namedTemplates.end())
			return iter->second;
	}

	AppearanceTemplate *appearanceTemplate = 0;

	//-- search for the appearance in the named timed list
	if (ms_useTimedTemplates)
	{
		NamedTimedTemplates::iterator iter = ms_namedTimedTemplates.find((const CrcString*)&actualFileName);
		if (iter != ms_namedTimedTemplates.end())
		{
			appearanceTemplate = iter->second.second;
			ms_namedTimedTemplates.erase(iter);
		}
	}

	DEBUG_REPORT_LOG(ms_logCreate, ("Creating appearance %s\n", actualFileName.getString()));

	//-- hack to asynchronously load .msh files
	if (!appearanceTemplate && AsynchronousLoader::isEnabled() && (strstr(actualFileName.getString(), ".msh") != 0 || strstr(actualFileName.getString(), ".MSH") != 0))
	{
		// DEBUG_REPORT_LOG_PRINT(true, ("Loading mesh %s\n", actualFileName.getString()));
		TagBindingMap::iterator iter = ms_tagBindingMap.find(TAG_MESH);
		if (iter != ms_tagBindingMap.end())
			appearanceTemplate = iter->second(actualFileName.getString(), nullptr);
	}

	//-- we now need to create the appearance from disk
	if (!appearanceTemplate)
	{
		Iff iff;
		if (!iff.open(actualFileName.getString(), true))
			FATAL(true, ("Could not open appearance file %s", actualFileName.getString()));

		const Tag tag = iff.getCurrentName();
		TagBindingMap::iterator iter = ms_tagBindingMap.find(tag);
		if (iter != ms_tagBindingMap.end()) {
			appearanceTemplate = iter->second(actualFileName.getString(), &iff);
		}
		else
		{
			char tagString[5];
			ConvertTagToString(tag, tagString);
			DEBUG_WARNING(true, ("AppearanceTemplate binding %s not found for file %s", tagString, actualFileName.getString()));
		}
	}

	//-- add the appearance template to the named list
	if (appearanceTemplate)
	{
		addNamedAppearanceTemplate(appearanceTemplate);
	}
	
	return appearanceTemplate;
}

// ----------------------------------------------------------------------
/**
 * Add an anonymous appearance template to the AppearanceTemplateList.
 * 
 * @param appearanceTemplate  AppearanceTemplate to add
 */

void AppearanceTemplateListNamespace::addAnonymousAppearanceTemplate(AppearanceTemplate *const appearanceTemplate)
{
	NOT_NULL(appearanceTemplate);

	//-- add to anonymous list
	std::pair<AnonymousTemplates::iterator, bool> result = ms_anonymousTemplates.insert(appearanceTemplate);

	//-- make sure it's not already there
	DEBUG_FATAL(!result.second, ("tried to add an existing anonymous appearanceTemplate"));
	UNREF(result);
}

// ----------------------------------------------------------------------
/**
 * Add a named appearance template to the AppearanceTemplateList.
 * 
 * @param appearanceTemplate  AppearanceTemplate to add
 */

void AppearanceTemplateListNamespace::addNamedAppearanceTemplate(AppearanceTemplate *const appearanceTemplate)
{
	NOT_NULL(appearanceTemplate);

	//-- add to named list
	std::pair<NamedTemplates::iterator, bool> result = ms_namedTemplates.insert(std::make_pair((const CrcString*)&appearanceTemplate->getCrcName(), appearanceTemplate));

	//-- make sure it's not already there
	DEBUG_FATAL(!result.second, ("tried to add existing named appearanceTemplate %s", appearanceTemplate->getName()));
	UNREF(result);
}

// ----------------------------------------------------------------------
/**
 * Add a named timed appearance template to the AppearanceTemplateList.
 * 
 * @param appearanceTemplate  AppearanceTemplate to add
 */

void AppearanceTemplateListNamespace::addNamedTimedAppearanceTemplate(AppearanceTemplate *const appearanceTemplate)
{
	NOT_NULL(appearanceTemplate);

	//-- add to named list
	std::pair<NamedTimedTemplates::iterator, bool> result = ms_namedTimedTemplates.insert(std::make_pair((const CrcString*)&appearanceTemplate->getCrcName(), std::make_pair(ms_keepTime + Random::randomReal(ms_keepEpsilon), appearanceTemplate)));

	//-- make sure it's not already there
	DEBUG_FATAL(!result.second, ("tried to add existing named timed appearanceTemplate %s", appearanceTemplate->getName()));
	UNREF(result);
}

// ----------------------------------------------------------------------
/**
 * Remove an appearance template from the AppearanceTemplateList.
 * 
 * @param appearanceTemplate  AppearanceTemplate to remove
 */

void AppearanceTemplateListNamespace::removeAppearanceTemplate(AppearanceTemplate *const appearanceTemplate)
{
	NOT_NULL(appearanceTemplate);
	CrcLowerString const & crcName  = appearanceTemplate->getCrcName();
	char const * const crcString = crcName.getString();

	if (crcString && *crcString)
	{
		//-- remove appearance template from the named list
		NamedTemplates::iterator iter = ms_namedTemplates.find((const CrcString*)&appearanceTemplate->getCrcName());
		if (iter != ms_namedTemplates.end())
			ms_namedTemplates.erase(iter);
		else
			DEBUG_FATAL(true, ("AppearanceTemplateListNamespace::removeAppearanceTemplate: could not find appearance template %s in named list", crcString));
	}
	else
	{
		//-- remove appearance template from the anonymous list
		AnonymousTemplates::iterator iter = ms_anonymousTemplates.find(appearanceTemplate);
		if (iter != ms_anonymousTemplates.end())
			ms_anonymousTemplates.erase(iter);
		else
			DEBUG_FATAL(true, ("AppearanceTemplateListNamespace::removeAppearanceTemplate: could not find anonymous appearance template in anonymous list"));
	}
}

// ----------------------------------------------------------------------

void AppearanceTemplateListNamespace::debugReport()
{
#ifdef _DEBUG
	DEBUG_REPORT_PRINT(true, ("Tags installed        = %i\n", ms_tagBindingMap.size()));
	DEBUG_REPORT_PRINT(true, ("Named templates       = %i\n", ms_namedTemplates.size()));
	DEBUG_REPORT_PRINT(true, ("Named timed templates = %i\n", ms_namedTimedTemplates.size()));
	DEBUG_REPORT_PRINT(true, ("Anonymous templates   = %i\n", ms_anonymousTemplates.size()));
#endif
}

// ----------------------------------------------------------------------

void AppearanceTemplateListNamespace::debugReportVerbose()
{
#ifdef _DEBUG
	DEBUG_REPORT_PRINT(true, ("AppearanceTemplates loaded:\n"));

	DEBUG_OUTPUT_STATIC_VIEW_BEGINFRAME("Engine\\AppearanceTemplateList");
	DEBUG_OUTPUT_STATIC_VIEW("Engine\\AppearanceTemplateList", ("AppearanceTemplates loaded:\n"));

	for (NamedTemplates::iterator iter = ms_namedTemplates.begin(); iter != ms_namedTemplates.end(); ++iter)
	{
		const AppearanceTemplate *const t = (*iter).second;
		DEBUG_REPORT_PRINT(true, ("  %s %d\n", t->getCrcName().getString(), t->getReferenceCount()));
		DEBUG_OUTPUT_STATIC_VIEW("Engine\\AppearanceTemplateList", ("  %s %d\n", t->getCrcName().getString(), t->getReferenceCount()));
		UNREF(t);
	}

	DEBUG_OUTPUT_STATIC_VIEW_ENDFRAME("Engine\\AppearanceTemplateList");
#endif
}

// ----------------------------------------------------------------------

void AppearanceTemplateListNamespace::debugReportTimedTemplates()
{
#ifdef _DEBUG
	for (NamedTimedTemplates::iterator iter = ms_namedTimedTemplates.begin(); iter != ms_namedTimedTemplates.end(); ++iter)
		DEBUG_REPORT_PRINT(true, ("% 2.1f  %s\n", iter->second.first, iter->first->getString()));
#endif
}

// ======================================================================
