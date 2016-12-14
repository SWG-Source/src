//
// AsynchronousLoader.cpp
// Copyright 2002 Sony Online Entertainment
// All Rights Reserved
//
// ======================================================================

#include "sharedFile/FirstSharedFile.h"
#include "sharedFile/AsynchronousLoader.h"

#include "sharedDebug/DebugFlags.h"
#include "sharedFile/ConfigSharedFile.h"
#include "sharedFile/Iff.h"
#include "sharedFile/MemoryFile.h"
#include "sharedFile/TreeFile.h"
#include "sharedFoundation/Clock.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/Os.h"
#include "sharedFoundation/MemoryBlockManagerMacros.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedSynchronization/Mutex.h"
#include "sharedSynchronization/Semaphore.h"
#include "sharedThread/RunThread.h"
#include "sharedThread/ThreadHandle.h"

#include <deque>
#include <string>
#include <vector>
#include <map>

#ifdef _DEBUG
#include <algorithm>
#endif

// ======================================================================

const Tag TAG_ASYN = TAG(A,S,Y,N);
const Tag TAG_EXTN = TAG(E,X,T,N);
const Tag TAG_LOAD = TAG(L,O,A,D);

// ======================================================================

namespace AsynchronousLoaderNamespace
{
	struct ExtensionFunctions
	{
		char                                *extension;
		AsynchronousLoader::FetchFunction    fetchFunction;
		AsynchronousLoader::ReleaseFunction  releaseFunction;
	};
	typedef std::vector<ExtensionFunctions> ExtensionFunctionsList;

	struct FileRecord
	{
	public:
		int8          alreadyCached;
		const int8    extensionFunctionsIndex;
		const char    fileName[2];

	private:
		FileRecord();
		FileRecord &operator =(const FileRecord &);
	};
	typedef std::vector<FileRecord *> FileRecordList;

	class FileMapComparison
	{
	public:
		bool operator ()(const char *lhs, const char *rhs) const;
	};

	typedef std::map<const char *, FileRecordList *, FileMapComparison>  FileMap;

	struct CachedFile
	{
		FileRecord   *fileRecord;
		AbstractFile *file;
		const void   *resource;
	};
	typedef std::vector<CachedFile>   CachedFiles;
	typedef std::vector<CachedFiles*> CachedFilesPool;

	struct Request
	{
		FileRecordList                 *fileRecordList;
		AsynchronousLoader::Callback    callback;
		void                           *data;
		CachedFiles                    *cachedFiles;
	};
	typedef std::deque<Request *>  Requests;

	void remove();
	void submitRequest(Request *request);
	void threadRoutine();

#ifdef _DEBUG
	static void debugReport();
#endif

	bool                                        ms_installed;
#ifdef _DEBUG
	bool                                        ms_debugDisable;
	bool                                        ms_debugReport;
	bool                                        ms_suspendThread;
	bool                                        ms_threadIsSuspended;
	bool                                        ms_suspendCallbacks;
	int                                         ms_numberOfRequests;
	int                                         ms_numberOfPendingRequests;
	int                                         ms_numberOfCompletedRequests;
	int                                         ms_numberOfSubmittedRequests;
	int                                         ms_numberOfRetiredRequests;
	int                                         ms_numberOfFetchedResources;
	int                                         ms_numberOfAlreadyCachedFiles;
#endif
	int                                         ms_numberOfCachedBytes;
	int                                         ms_numberOfPostponedRequests;
	int const                                   cms_postponeThreshold = 8 * 1024 * 1024;
	int                                         ms_enabled;
	ThreadHandle                                ms_threadHandle;
	Semaphore                                   ms_eventsPending;
	Mutex                                       ms_mutex;
	Requests                                    ms_pendingRequests;
	Requests                                    ms_completedRequests;
	char                                       *ms_fileData;
	FileMap                                     ms_fileMap;
	ExtensionFunctionsList                      ms_extensionFunctionsList;
	CachedFilesPool                             ms_cachedFilesPool;
	MemoryBlockManager           *ms_requestMemoryBlockManager;
}
using namespace AsynchronousLoaderNamespace;

// ======================================================================

bool AsynchronousLoaderNamespace::FileMapComparison::operator()(const char *lhs, const char *rhs) const
{
	return strcmp(lhs, rhs) < 0;
}

// ======================================================================

void AsynchronousLoader::install(const char *fileName)
{
	DEBUG_FATAL(ms_installed, ("Already installed"));

	if (!ConfigSharedFile::getEnableAsynchronousLoader())
		return;

	Iff iff;
	if (!iff.open(fileName, true))
	{
		DEBUG_WARNING(true, ("Could not load asynchronous data file %s\n", fileName));
		ms_installed = false;
		return;
	}

	// add to exit chain
	ExitChain::add(&AsynchronousLoaderNamespace::remove, "AsynchronousLoader::remove");

	// load all the data
	iff.enterForm(TAG_ASYN);
		iff.enterForm(TAG_0001);

			// read all the names
			iff.enterChunk(TAG_NAME);
				ms_fileData = iff.read_char(iff.getChunkLengthTotal());
			iff.exitChunk(TAG_NAME);

			iff.enterChunk(TAG_EXTN);
				const int numberOfExtensions = iff.getChunkLengthTotal(sizeof(int32));
				ms_extensionFunctionsList.reserve(numberOfExtensions);
				ExtensionFunctions extensionFunctions;
				extensionFunctions.extension = nullptr;
				extensionFunctions.fetchFunction = nullptr;
				extensionFunctions.releaseFunction = nullptr;
				for (int i = 0; i < numberOfExtensions; ++i)
				{
					extensionFunctions.extension = ms_fileData + iff.read_int32();

					// hack to deal with bad data where the extension pointed at the entire file name instead of just the exception
					char * const dot = strrchr(extensionFunctions.extension, '.');
					if (dot)
						extensionFunctions.extension = dot + 1;

					ms_extensionFunctionsList.push_back(extensionFunctions);
				}
			iff.exitChunk(TAG_EXTN);

			// read all the asynchronous loading records
			iff.enterChunk(TAG_LOAD);
				while (iff.getChunkLengthLeft())
				{
					const int32 count = iff.read_int32();
					FileRecordList *fileRecordList = new FileRecordList;
					fileRecordList->reserve(count);

					for (int i = 0; i < count; ++i)
						fileRecordList->push_back(reinterpret_cast<FileRecord *>(ms_fileData + iff.read_int32()));

					const bool result = ms_fileMap.insert(FileMap::value_type(fileRecordList->front()->fileName, fileRecordList)).second;
					UNREF(result);
					DEBUG_FATAL(!result, ("item was already present"));
				}
			iff.exitChunk(TAG_LOAD);

		iff.exitForm(TAG_0001);
	iff.exitForm(TAG_ASYN);

	ms_requestMemoryBlockManager = new MemoryBlockManager("AsynchronousLoader::Request", true, sizeof(Request), 0, 0, 0);

	ms_threadHandle = runNamedThread("AsynchronousLoader", threadRoutine);
	switch (ConfigSharedFile::getAsynchronousLoaderPriority())
	{
		case -2: ms_threadHandle->setPriority(Thread::kIdle);     break;
		case -1: ms_threadHandle->setPriority(Thread::kLow);      break;
		case  0: ms_threadHandle->setPriority(Thread::kNormal);   break;
		case  1: ms_threadHandle->setPriority(Thread::kHigh);     break;
		case  2: ms_threadHandle->setPriority(Thread::kCritical); break;
		default: ms_threadHandle->setPriority(Thread::kNormal);   break;
	};

#ifdef _DEBUG
	DebugFlags::registerFlag(ms_debugDisable,     "SharedFile", "runtimeDisableAsynchronousLoader");
	DebugFlags::registerFlag(ms_debugReport,      "SharedFile", "reportAsynchronousLoader", &debugReport);
	DebugFlags::registerFlag(ms_suspendCallbacks, "SharedFile", "suspendAsynchronousLoaderCallbacks");
	DebugFlags::registerFlag(ms_suspendThread,    "SharedFile", "suspendAsynchronousLoaderThread");
#endif

	ms_installed = true;
	ms_enabled = 0;
}

// ----------------------------------------------------------------------

void AsynchronousLoaderNamespace::remove()
{
	DEBUG_FATAL(!ms_installed, ("not installed"));
	Request *request = reinterpret_cast<Request*>(ms_requestMemoryBlockManager->allocate());
	request->fileRecordList = nullptr;
	request->callback = nullptr;
	request->data = nullptr;
	request->cachedFiles = nullptr;

	submitRequest(request);

	// wait for all the requests to be serviced
	bool empty = false;
	while (!empty)
	{
		Os::sleep(250);
		AsynchronousLoader::processCallbacks();

		ms_mutex.enter();
			empty = ms_pendingRequests.empty() && ms_completedRequests.empty();
		ms_mutex.leave();
	}

	// make sure the thread is dead
	ms_threadHandle->wait();

	delete [] ms_fileData;

	const FileMap::iterator iEnd = ms_fileMap.end();
	for (FileMap::iterator i = ms_fileMap.begin(); i != iEnd; ++i)
		delete i->second;
	ms_fileMap.clear();

	const CachedFilesPool::iterator jEnd = ms_cachedFilesPool.end();
	for (CachedFilesPool::iterator j = ms_cachedFilesPool.begin(); j != jEnd; ++j)
		delete *j;
	ms_cachedFilesPool.clear();

	delete ms_requestMemoryBlockManager;
	ms_requestMemoryBlockManager = nullptr;
}

// ----------------------------------------------------------------------

bool AsynchronousLoader::isInstalled()
{
	return ms_installed;
}

// ----------------------------------------------------------------------

bool AsynchronousLoader::isEnabled()
{
#ifdef _DEBUG
	if (ms_debugDisable)
		return false;
#endif

	return ms_installed && ms_enabled == 0;
}

// ----------------------------------------------------------------------

bool AsynchronousLoader::isIdle()
{
	ms_mutex.enter();
		bool const idle = ms_pendingRequests.empty() && ms_completedRequests.empty();
	ms_mutex.leave();

	return idle;
}

// ----------------------------------------------------------------------

void AsynchronousLoader::disable()
{
	--ms_enabled;
}

// ----------------------------------------------------------------------

void AsynchronousLoader::enable()
{
	++ms_enabled;
	DEBUG_FATAL(ms_enabled > 0, ("AsynchronousLoader::enable enabled too many times"));
}

// ----------------------------------------------------------------------

#ifdef _DEBUG

void AsynchronousLoaderNamespace::debugReport()
{
	DEBUG_REPORT_LOG_PRINT(true, ("%5.2f=fps %3d=sub %3d=pend %3d=comp %3d=ret %3d=pst %5d=kb\n", Clock::framesPerSecond(), ms_numberOfSubmittedRequests, ms_numberOfPendingRequests, ms_numberOfCompletedRequests, ms_numberOfRetiredRequests, ms_numberOfPostponedRequests, ms_numberOfCachedBytes / 1024));
	ms_numberOfSubmittedRequests = 0;
	ms_numberOfRetiredRequests = 0;
	ms_numberOfFetchedResources = 0;
}

#endif

// ----------------------------------------------------------------------

void AsynchronousLoader::bindFetchReleaseFunctions(const char *extension, FetchFunction fetchFunction, ReleaseFunction releaseFunction)
{
	DEBUG_FATAL((fetchFunction != 0) ^ (releaseFunction != 0), ("Must supply both a fetch & release function, or neither"));

	ExtensionFunctionsList::iterator iEnd = ms_extensionFunctionsList.end();
	for (ExtensionFunctionsList::iterator i = ms_extensionFunctionsList.begin(); i != iEnd; ++i)
	{
		ExtensionFunctions &extensionFunctions = *i;

		if (strcmp(extensionFunctions.extension, extension) == 0)
		{
			extensionFunctions.fetchFunction = fetchFunction;
			extensionFunctions.releaseFunction = releaseFunction;
			return;
		}
	}
}

// ----------------------------------------------------------------------

void AsynchronousLoader::add(const char *fileName, Callback callback, void *data)
{
	DEBUG_FATAL(!isEnabled(), ("AsynchronousLoader not enabled"));
	NOT_NULL(callback);

	char buffer[Os::MAX_PATH_LENGTH];
	TreeFile::fixUpFileName(fileName, buffer);

	const FileMap::const_iterator i = ms_fileMap.find(buffer);
	if (i != ms_fileMap.end())
	{
		Request *request = reinterpret_cast<Request*>(ms_requestMemoryBlockManager->allocate());
		request->fileRecordList = i->second;
		request->callback = callback;
		request->data = data;
		request->cachedFiles = nullptr;
		submitRequest(request);
	}
	else
	{
		// couldn't find the file, can't asynchronously load it.
		(*callback)(data);
	}
}

// ----------------------------------------------------------------------

void AsynchronousLoader::remove(Callback callback, void *data)
{
	ms_mutex.enter();

		{
			Requests::iterator iEnd = ms_pendingRequests.end();
			for (Requests::iterator i = ms_pendingRequests.begin(); i != iEnd; ++i)
				if ((*i)->callback == callback && (*i)->data == data)
				{
					(*i)->callback = nullptr;
					(*i)->data = nullptr;
				}
		}

		{
			Requests::iterator iEnd = ms_completedRequests.end();
			for (Requests::iterator i = ms_completedRequests.begin(); i != iEnd; ++i)
				if ((*i)->callback == callback && (*i)->data == data)
				{
					(*i)->callback = nullptr;
					(*i)->data = nullptr;
				}
		}

	ms_mutex.leave();
}

// ----------------------------------------------------------------------

void AsynchronousLoaderNamespace::submitRequest(Request *request)
{
	ms_mutex.enter();
		ms_pendingRequests.push_back(request);
#ifdef _DEBUG
		++ms_numberOfRequests;
		++ms_numberOfPendingRequests;
		++ms_numberOfSubmittedRequests;
#endif
	ms_mutex.leave();

	ms_eventsPending.signal();
}

// ----------------------------------------------------------------------

void AsynchronousLoaderNamespace::threadRoutine()
{
	// loop until a quit request is processed
	for (;;)
	{
		// wait until there is a request to processs
		ms_eventsPending.wait();

		bool postpone = false;

		// get the request to service
		ms_mutex.enter();

			Request *request = ms_pendingRequests.front();
			const FileRecordList *fileRecordList = request->fileRecordList;

			if (fileRecordList && ms_numberOfCachedBytes > cms_postponeThreshold)
			{
				postpone = true;
				ms_numberOfPostponedRequests += 1;
			}

		ms_mutex.leave();

		if (postpone)
			continue;

		int bytes = 0;

		// make sure the request is still pending
		if (fileRecordList && request->callback)
		{
			// load all the resource
			const FileRecordList::const_iterator jEnd = fileRecordList->end();
			for (FileRecordList::const_iterator j = fileRecordList->begin(); j != jEnd; ++j)
			{
				FileRecord *fileRecord = *j;

				CachedFile cachedFile;
				cachedFile.fileRecord = fileRecord;
				cachedFile.file = nullptr;
				cachedFile.resource = nullptr;

				// check if the resource is already loaded
				if (!fileRecord->alreadyCached)
				{
					// try to increase the reference count on the resource
					const ExtensionFunctions &extensionFunctions = ms_extensionFunctionsList[fileRecord->extensionFunctionsIndex];
					if (extensionFunctions.fetchFunction)
					{
						cachedFile.resource = extensionFunctions.fetchFunction(fileRecord->fileName);
#ifdef _DEBUG
						++ms_numberOfFetchedResources;
#endif
					}

					// othersize, try to open the file
					if (!cachedFile.resource)
					{
						AbstractFile *file = TreeFile::open(fileRecord->fileName, AbstractFile::PriorityLow, true);
						if (file)
						{
							if (file->isZlibCompressed())
							{
								bytes += file->getZlibCompressedLength();
								cachedFile.file = file;
							}
							else
							{
								bytes += file->length();
								MemoryFile *memoryFile = new MemoryFile(file);
								cachedFile.file = memoryFile;
								delete file;
							}

							// mark the file as already loaded
							if (extensionFunctions.fetchFunction)
							{
								ms_mutex.enter();
									fileRecord->alreadyCached = true;
								ms_mutex.leave();
							}
						}
					}

					// add the file to the loaded list
					if (cachedFile.file || cachedFile.resource)
					{
						if (!request->cachedFiles)
						{
							// first try to get one from the pool
							ms_mutex.enter();

								if (!ms_cachedFilesPool.empty())
								{
									request->cachedFiles = ms_cachedFilesPool.back();
									ms_cachedFilesPool.pop_back();
								}

							ms_mutex.leave();
						}

						if (!request->cachedFiles)
						{
							// if that didn't work, we have to allocate a new one
							request->cachedFiles = new CachedFiles;
						}

						request->cachedFiles->push_back(cachedFile);
					}
				}
#ifdef _DEBUG
				else
					++ms_numberOfAlreadyCachedFiles;
#endif
			}

			// DEBUG_REPORT_LOG_PRINT(true, ("Async loading done %s (%d)\n", request->fileName, request->cachedFiles.size()));
		}

		// move the request to the completed queue
		ms_mutex.enter();

			ms_pendingRequests.pop_front();
			ms_completedRequests.push_back(request);

#ifdef _DEBUG
			--ms_numberOfPendingRequests;
			++ms_numberOfCompletedRequests;
#endif
			ms_numberOfCachedBytes += bytes;

		ms_mutex.leave();

		// a request of no files signals thread termination
		if (!fileRecordList)
 			break;
	}
}

// ----------------------------------------------------------------------

void AsynchronousLoader::processCallbacks()
{
#if defined(_DEBUG) && defined(_WIN32)
	if (ms_suspendThread != ms_threadIsSuspended)
	{
		if (ms_suspendThread)
			ms_threadHandle->suspend();
		else
			ms_threadHandle->resume();

		ms_threadIsSuspended = ms_suspendThread;
	}
#endif

#ifdef _DEBUG
	if (!ms_suspendCallbacks)
#endif
	{
		int callbacksAllowed = ConfigSharedFile::getAsynchronousLoaderCallbacksPerFrame();

		// look for completed requests to service
		for (;;)
		{
			ms_mutex.enter();

				if (ms_completedRequests.empty())
				{
					ms_mutex.leave();
					break;
				}

				Request *request = ms_completedRequests.front();
				ms_completedRequests.pop_front();

			ms_mutex.leave();

			int bytes = 0;

			if (request->callback)
			{
				// add all the preloaded files into the TreeFile cache
				if (request->cachedFiles)
				{
					const CachedFiles::iterator iEnd = request->cachedFiles->end();
					for (CachedFiles::iterator i = request->cachedFiles->begin(); i != iEnd; ++i)
					{
						CachedFile &cachedFile = *i;
						if (cachedFile.file)
						{
							if (cachedFile.file->isZlibCompressed())
								bytes += cachedFile.file->getZlibCompressedLength();
							else
								bytes += cachedFile.file->length();
							TreeFile::addCachedFile(cachedFile.fileRecord->fileName, cachedFile.file);
							cachedFile.file = nullptr;
						}
					}
				}

				// allow the object to load
				(*request->callback)(request->data);

				// pitch the loaded files
				TreeFile::clearCachedFiles();
			}

			// free any resources that this asynchronous load still owns
			if (request->cachedFiles)
			{
				const CachedFiles::iterator iEnd = request->cachedFiles->end();
				for (CachedFiles::iterator i = request->cachedFiles->begin(); i != iEnd; ++i)
				{
					CachedFile &cachedFile = *i;

					ms_mutex.enter();
						cachedFile.fileRecord->alreadyCached = false;
					ms_mutex.leave();

					if (cachedFile.file)
					{
						if (cachedFile.file->isZlibCompressed())
							bytes += cachedFile.file->getZlibCompressedLength();
						else
							bytes += cachedFile.file->length();
						delete cachedFile.file;
						cachedFile.file = nullptr;
					}

					if (cachedFile.resource)
					{
						ms_extensionFunctionsList[cachedFile.fileRecord->extensionFunctionsIndex].releaseFunction(cachedFile.resource);
						cachedFile.resource = nullptr;
					}
				}
				request->cachedFiles->clear();

				ms_mutex.enter();

					ms_cachedFilesPool.push_back(request->cachedFiles);
					request->cachedFiles = nullptr;

				ms_mutex.leave();
			}

			ms_mutex.enter();
	#ifdef _DEBUG
				--ms_numberOfCompletedRequests;
				++ms_numberOfRetiredRequests;
	#endif
				ms_numberOfCachedBytes -= bytes;
			ms_mutex.leave();

			// free the request
			ms_requestMemoryBlockManager->free(request);

			if (callbacksAllowed && --callbacksAllowed == 0)
				break;
		}
	}

	{
		ms_mutex.enter();
			int postponed = ms_numberOfPostponedRequests;
			ms_numberOfPostponedRequests = 0;
		ms_mutex.leave();

		while (postponed)
		{
			ms_eventsPending.signal();
			postponed--;
		}
	}
}

// ======================================================================
