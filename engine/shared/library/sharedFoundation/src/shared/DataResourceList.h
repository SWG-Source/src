//========================================================================
//
// DataResourceList.h - factory method for creating classes derived from
// DataResource.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundefined-var-template"

#ifndef _INCLUDED_DataResourceList_H
#define _INCLUDED_DataResourceList_H

#include "sharedDebug/DataLint.h"
#include "sharedFile/Iff.h"
#include "sharedFoundation/ConfigSharedFoundation.h"
#include "sharedFoundation/CrcString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedFoundation/Tag.h"
#include "sharedFoundation/TemporaryCrcString.h"

#include <map>
#include <string>

//========================================================================

// T should be derived from class DataResource
template <typename T>
class DataResourceList
{
public:
	// DataResource registration function
	typedef T * (*CreateDataResourceFunc)(const std::string & filename);
	static void registerTemplate(Tag id, CreateDataResourceFunc createFunc);

	// functions to create a new DataResource
	static       T * fetch(Tag id);
	static const T * fetch(Iff & source);
	static const T * fetch(const CrcString & filename);
	static const T * fetch(const std::string & filename);
	static const T * fetch(const char * filename);

	static const bool  isLoaded(const std::string & fileName);

	// function to reload a data resource with new data
	static T * reload(Iff & source);

	// functions to reassign Tag->DataResource binding
	static CreateDataResourceFunc assignBinding(Tag id, CreateDataResourceFunc createFunc);
	static CreateDataResourceFunc removeBinding(Tag tag);

	static void install(void);
	static void remove(void);

	// function to delete a DataResource
	static void release(const T & dataResource);

	static void logLoadedResources(char const * classType);

	static void garbageCollect ();

private:

	typedef std::map<Tag, CreateDataResourceFunc> CreateDataResourceMap;
	static CreateDataResourceMap *ms_bindings;

	typedef std::map<const CrcString*, const T*, LessPointerComparator> LoadedDataResourceMap;
	static LoadedDataResourceMap *ms_loaded;

private:
	// no copying templates
	DataResourceList(void);
	DataResourceList(const DataResourceList &source);
	DataResourceList & operator =(const DataResourceList &source);
};

//----------------------------------------------------------------------

/**
 * Sets up the maps to keep track of resources.
 */
template <typename T>
inline void DataResourceList<T>::install()
{
	if (ms_bindings == nullptr)
	{
		ms_bindings = new CreateDataResourceMap();
		ms_loaded   = new LoadedDataResourceMap();

		ExitChain::add (remove, "DataResourceList::remove");
	}
}	// DataResourceList<T>::install

//----------------------------------------------------------------------

/**
 * Frees up memory associated with the list, and checks for resources still
 * allocated.
 */
template <typename T>
inline void DataResourceList<T>::remove(void)
{
	if (ms_loaded != nullptr)
	{
#ifdef _DEBUG
		if (!ms_loaded->empty())
		{
			DEBUG_REPORT_LOG (true, ("Data resources still allocated:\n"));
			for (typename LoadedDataResourceMap::const_iterator i = ms_loaded->begin (); i != ms_loaded->end (); ++i)
			{
				const T * const   dataResource = (*i).second;
				const int         users        = dataResource->getReferenceCount ();
				const char* const name         = dataResource->getName ();
				DEBUG_REPORT_LOG (true, (" %3d %s\n", users, name));
				UNREF (users);
				UNREF (name);
			}

			DEBUG_WARNING (true, ("Data resources still allocated"));
			return;
		}
#endif // _DEBUG

		delete ms_loaded;
		ms_loaded = nullptr;
	}

	if (ms_bindings != nullptr)
	{
		delete ms_bindings;
		ms_bindings = nullptr;
	}
}	// DataResourceList<T>::remove

//----------------------------------------------------------------------

/**
 * Maps a template Tag id to a function used to create a template.
 *
 * @param createFunc	template creation function
 * @param id			id of the template
 */
template <typename T>
inline void DataResourceList<T>::registerTemplate(Tag id,
	CreateDataResourceFunc createFunc)
{
	if (ms_bindings == nullptr)
		install();

#ifdef _DEBUG
	// Added to help debug
	char text[256];
	ConvertTagToString(id, text);
#endif // _DEBUG

	// check if that id was already registered and print a warning if it was
	if (ms_bindings->find(id) != ms_bindings->end())
	{
		char buffer[5];
		ConvertTagToString(id, buffer);
		DEBUG_WARNING(true, ("DataResourceList::registerTemplate: re-installing tag %s!", buffer));
	}
	(*ms_bindings)[id] = createFunc;
}	// DataResourceList<T>::registerTemplate

//----------------------------------------------------------------------

/**
 * Changes the default tag->create function binding.
 *
 * @param id			the tag to change
 * @param createFunc	the creation function to associate with the id
 *
 * @return the old creation function associated with the tag
 */
template <typename T>
inline typename DataResourceList<T>::CreateDataResourceFunc DataResourceList<T>::assignBinding(
	Tag id, CreateDataResourceFunc createFunc)
{
	NOT_NULL(ms_bindings);

	CreateDataResourceFunc oldFunc = (*ms_bindings)[id];
	(*ms_bindings)[id] = createFunc;
	return oldFunc;
}	// DataResourceList<T>::assignBinding

//----------------------------------------------------------------------

/**
 * Removes the default tag->create function binding.
 *
 * @param id			the tag to remove
 *
 * @return the old creation function associated with the tag
 */
template <typename T>
inline typename DataResourceList<T>::CreateDataResourceFunc DataResourceList<T>::removeBinding(
	Tag id)
{
	NOT_NULL(ms_bindings);

	CreateDataResourceFunc oldFunc = (*ms_bindings)[id];
	ms_bindings->erase(id);
	return oldFunc;
}	// DataResourceList<T>::removeBinding

//----------------------------------------------------------------------

template <typename T>
inline const T * DataResourceList<T>::fetch(const std::string & filename)
{
	return fetch(TemporaryCrcString(filename.c_str(), true));
}	// DataResourceList<T>::fetch(const std::string &)

//----------------------------------------------------------------------

template <typename T>
inline const T * DataResourceList<T>::fetch(const char * filename)
{
	return fetch(TemporaryCrcString(filename, true));
}	// DataResourceList<T>::fetch(const char *)

//----------------------------------------------------------------------

/**
 * Creates a data resource from a registered data resource id. The caller is
 * responsible for deleting the data resource.
 *
 * @param id		the iff tag for the data resource
 *
 * @return the blank data resource
 */
template <class T>
inline T * DataResourceList<T>::fetch(Tag id)
{
	NOT_NULL(ms_bindings);

	typename CreateDataResourceMap::iterator iter = ms_bindings->find(id);
	if (iter == ms_bindings->end())
		return nullptr;

	return (*(*iter).second)("");
}	// DataResourceList<T>::fetch(Tag)

//----------------------------------------------------------------------

/**
 * Loads a data resource from an iff file/buffer.
 *
 * @param source		iff source to read from
 *
 * @return the data resource
 */
template <typename T>
inline const T * DataResourceList<T>::fetch(Iff &source)
{
	NOT_NULL(ms_bindings);

#ifdef _DEBUG
	DataLint::pushAsset(source.getFileName());
#endif // _DEBUG

	// read the 1st form; based on the form id, create the template
	Tag id = source.getCurrentName();
	typename CreateDataResourceMap::iterator createIter = ms_bindings->find(id);
	if (createIter == ms_bindings->end())
	{
		char buffer[5];
		ConvertTagToString(id, buffer);
		DEBUG_WARNING(true, ("DataResourceList::fetch Iff: trying to fetch resource for unknown tag %s!", buffer));
		return nullptr;
	}

	T *newDataResource = (*(*createIter).second)(source.getFileName());
	if (newDataResource != nullptr)
	{
		// initialize the data resource
		newDataResource->loadFromIff(source);
	}

#ifdef _DEBUG
	DataLint::popAsset();
#endif // _DEBUG

	return newDataResource;
}	// DataResourceList<T>::fetch(Iff &)

//----------------------------------------------------------------------

/**
 * Loads a data resource from an iff file.
 *
 * @param filename		the file to load from
 *
 * @return the data resource
 */
template <typename T>
inline const T * DataResourceList<T>::fetch(const CrcString &filename)
{
	NOT_NULL(ms_loaded);

	// see if we already have loaded the template
	typename LoadedDataResourceMap::iterator iter = ms_loaded->find(&filename);
	if (iter != ms_loaded->end())
	{
		(*iter).second->addReference();
		return (*iter).second;
	}

	// load the template
	Iff iff;
	if (!iff.open(filename.getString(), true))
		return nullptr;

	// put the template in the loaded list
	const T * const newDataResource = fetch(iff);
	if (newDataResource != nullptr)
	{
		newDataResource->addReference();
		ms_loaded->insert(std::make_pair(&newDataResource->getCrcName (), newDataResource));
	}

	// clean up
	iff.close();
	return newDataResource;
}

//----------------------------------------------------------------------

/**
 * Checks the reference count of a resource, if it is 0, deletes it.
 *
 * @param dataResource		the data resource to release
 */
template <typename T>
inline void DataResourceList<T>::release(const T & dataResource)
{
	NOT_NULL(ms_loaded);

	if (ms_loaded != nullptr && dataResource.getReferenceCount() == 0)
	{
		typename LoadedDataResourceMap::iterator iter = ms_loaded->find(&dataResource.getCrcName());
		if (iter != ms_loaded->end())
		{
			const T * const temp = (*iter).second;
			(*iter).second = nullptr;
			ms_loaded->erase(iter);
			delete temp;
		}
	}
}	// DataResourceList<T>::release

//----------------------------------------------------------------------

/**
 * Reloads a data resource from an iff file/buffer.
 *
 * @param source		iff source to read from
 *
 * @return the data resource
 */
template <typename T>
inline T * DataResourceList<T>::reload(Iff &source)
{
	NOT_NULL(ms_loaded);

	const TemporaryCrcString sourceCrcString (source.getFileName(), true);
	typename LoadedDataResourceMap::iterator iter = ms_loaded->find((const CrcString*)&sourceCrcString);
	if (iter == ms_loaded->end())
	{
		DEBUG_WARNING(true, ("DataResourceList::reload: trying to reload unloaded resource %s!", source.getFileName()));
		return nullptr;
	}

	T * const dataResource = const_cast<T *>((*iter).second);
	if (dataResource != nullptr)
	{
		// initialize the data resource
		dataResource->loadFromIff(source);
	}
	return dataResource;
}	// DataResourceList<T>::reload(Iff &)

//-----------------------------------------------------------------------

template<typename T>
inline const bool DataResourceList<T>::isLoaded(const std::string & source)
{
	NOT_NULL(ms_loaded);
	const TemporaryCrcString sourceCrcString (source.c_str (), true);
	typename LoadedDataResourceMap::iterator iter = ms_loaded->find((const CrcString*)&sourceCrcString);
	return (iter != ms_loaded->end());
}

//-----------------------------------------------------------------------

template<typename T>
void DataResourceList<T>::logLoadedResources(char const * classType)
{
	UNREF(classType); //necessary for release build
	typename LoadedDataResourceMap::iterator const iterEnd = ms_loaded->end();
	DEBUG_REPORT_LOG_PRINT(true, ("Begin log of loaded %s\n", classType));

	for (typename LoadedDataResourceMap::iterator iter = ms_loaded->begin(); iter != iterEnd; ++iter)
		DEBUG_REPORT_LOG_PRINT(true, (" %d %s\n", iter->second->getReferenceCount(), iter->second->getName()));

	DEBUG_REPORT_LOG_PRINT(true, ("End log of loaded %s\n", classType));
}

//----------------------------------------------------------------------

template<typename T>
void DataResourceList<T>::garbageCollect ()
{
	typename LoadedDataResourceMap::iterator end = ms_loaded->end ();
	typename LoadedDataResourceMap::iterator iter = ms_loaded->begin ();
	for (; iter != end; ++iter)
		const_cast<T*> (iter->second)->garbageCollect ();
}

//----------------------------------------------------------------------
#pragma clang diagnostic pop
#endif	// _INCLUDED_DataResourceList_H
