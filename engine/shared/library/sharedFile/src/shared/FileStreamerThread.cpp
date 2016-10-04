// ======================================================================
//
// FileStreamerThread.cpp
// Portions copyright 1999 Bootprint Entertainment
// Portions copyright 2002 Bootprint Entertainment
// All Rights Reserved
//
// ======================================================================

#include "sharedFile/FirstSharedFile.h"
#include "sharedFile/FileStreamerThread.h"

#include "fileInterface/AbstractFile.h"
#include "sharedFile/FileStreamer.h"
#include "sharedFile/FileStreamerFile.h"
#include "sharedFile/OsFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/PerThreadData.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedSynchronization/Gate.h"
#include "sharedSynchronization/Semaphore.h"
#include "sharedSynchronization/Mutex.h"
#include "sharedThread/RunThread.h"
#include "sharedThread/ThreadHandle.h"

#include <string>

// ======================================================================

bool                                  FileStreamerThread::ms_installed;
ThreadHandle                          FileStreamerThread::ms_threadHandle;
Semaphore                             FileStreamerThread::ms_eventsPending;
Mutex                                 FileStreamerThread::ms_queueCriticalSection;
volatile FileStreamerThread::Request *FileStreamerThread::ms_firstAudioVideoRequest;
volatile FileStreamerThread::Request *FileStreamerThread::ms_lastAudioVideoRequest;
volatile FileStreamerThread::Request *FileStreamerThread::ms_firstDataRequest;
volatile FileStreamerThread::Request *FileStreamerThread::ms_lastDataRequest;
volatile FileStreamerThread::Request *FileStreamerThread::ms_firstLowRequest;
volatile FileStreamerThread::Request *FileStreamerThread::ms_lastLowRequest;

namespace FileStreamerThreadNamespace
{
	int const cms_maxReadSize = 128 * 1024;
};
using namespace FileStreamerThreadNamespace;

// ======================================================================

void FileStreamerThread::install()
{
	DEBUG_FATAL(ms_installed, ("FileStreamerThread::install already installed"));
	ms_installed = true;

	Request::install();

	// create the thread to handle the file access, will be triggered into action through the eventsPending semaphore
	ms_threadHandle = runNamedThread("File", threadRoutine);
	ms_threadHandle->setPriority(Thread::kHigh);

	ExitChain::add(&remove, "FileStreamerThread::remove");
}

// ----------------------------------------------------------------------

void FileStreamerThread::remove()
{
	DEBUG_FATAL(!ms_installed, ("FileStreamerThread::remove not installed"));

	//sumbit a quit request
	Request *newRequest = new Request;
	newRequest->type = Request::Quit;
	newRequest->priority  = AbstractFile::PriorityData;
	newRequest->gate = PerThreadData::getFileStreamerReadGate();
	submitRequest(newRequest);

	ms_threadHandle->wait();
	ms_installed = false;
}

// ----------------------------------------------------------------------
/**
 * Put a ruquest into the request queue(s).
 * 
 * This function puts a new request at the back of the appropriate queue.
 * All access to the queue is protected by critical sections.
 */

void FileStreamerThread::submitRequest(Request *request)
{
	NOT_NULL(request);

	if (request->priority == AbstractFile::PriorityAudioVideo)
	{
		ms_queueCriticalSection.enter();
			// add it to the linked list of requests
			if (ms_lastAudioVideoRequest)
				ms_lastAudioVideoRequest->next = request;
			else
				ms_firstAudioVideoRequest = request;
			ms_lastAudioVideoRequest = request;
		ms_queueCriticalSection.leave();
	}
	else
		if (request->priority == AbstractFile::PriorityData)
		{
			ms_queueCriticalSection.enter();
				// add it to the linked list of requests
				if (ms_lastDataRequest)
					ms_lastDataRequest->next = request;
				else
					ms_firstDataRequest = request;
				ms_lastDataRequest = request;
			ms_queueCriticalSection.leave();
		}
		else
			if (request->priority == AbstractFile::PriorityLow)
			{
				ms_queueCriticalSection.enter();
					// add it to the linked list of requests
					if (ms_lastLowRequest)
						ms_lastLowRequest->next = request;
					else
						ms_firstLowRequest = request;
					ms_lastLowRequest = request;
				ms_queueCriticalSection.leave();
			}
			else
			{
				DEBUG_FATAL(true,	("request has unknown priority type"));
			}

	// signal the thread that a new event is waiting
	ms_eventsPending.signal();
}

// ----------------------------------------------------------------------
/**
 * Quit.
 * 
 * This request is queued and processed after the reads are completed
 */

void FileStreamerThread::processQuit(volatile Request *request)
{
	NOT_NULL(request);

	ExitChain::quit();
	Gate *gate = request->gate;
	delete request;
	gate->open();
}

// ----------------------------------------------------------------------
/**
 * Function to read data into a buffer.
 * 
 * This function breaks up reads over cms_maxReadSize into multiple reads by reading
 * cms_maxReadSize bytes, then updating the request data and putting the request back
 * on the head of the queue and resignaling the semaphore.  When the read is complete
 * it stores the number of bytes read into the game-held request->returnVal field and
 * triggers and event to tell the game that we're finished.
 */

void FileStreamerThread::processRead(volatile Request *request)
{
	NOT_NULL(request);

	// shortcut to the file
	OsFile *osFile = request->osFile;

	// seek to the requested offset
	osFile->seek(request->offset);

	// read the data
	if (request->bytesToBeRead > cms_maxReadSize)
	{
		// only read up to FileStreamerThread::cms_maxReadSize, then resubmit smaller request
		const int amountRead = osFile->read(request->buffer, cms_maxReadSize);

		request->offset += amountRead;
		request->bytesRead += amountRead;
		request->bytesToBeRead -= amountRead;
		request->buffer = reinterpret_cast<byte *>(request->buffer) + amountRead;

		// if we read less than we could have, we're done
		if (amountRead < cms_maxReadSize)
		{
			// store final number of bytes read in storage accessible to main thread
			*request->returnValue = static_cast<int>(request->bytesRead);

			Gate *gate = request->gate;
			delete request;

			//set event so other main thread continues
			gate->open();
		}
		else
		{
			// resubmit request (put it on the head so we get it back first)
			if (request->priority == AbstractFile::PriorityAudioVideo)
			{
				ms_queueCriticalSection.enter();
					if (ms_firstAudioVideoRequest)
					{
						request->next = ms_firstAudioVideoRequest;
						ms_firstAudioVideoRequest = request;
					}
					else
					{
						ms_firstAudioVideoRequest = request;
						ms_lastAudioVideoRequest = request;
					}
				ms_queueCriticalSection.leave();
			}
			else
				if (request->priority == AbstractFile::PriorityData)
				{
					ms_queueCriticalSection.enter();
						if (ms_firstDataRequest)
						{
							request->next = ms_firstDataRequest;
							ms_firstDataRequest = request;
						}
						else
						{
							ms_firstDataRequest = request;
							ms_lastDataRequest = request;
						}
					ms_queueCriticalSection.leave();
				}
				else
					if (request->priority == AbstractFile::PriorityLow)
					{
						ms_queueCriticalSection.enter();
							if (ms_firstLowRequest)
							{
								request->next = ms_firstLowRequest;
								ms_firstLowRequest = request;
							}
							else
							{
								ms_firstLowRequest = request;
								ms_lastLowRequest = request;
							}
						ms_queueCriticalSection.leave();
					}
					else
						DEBUG_FATAL(true, ("FileStreamerThread::processRead request has unknown priority type"));

			// signal the file thread that a new request is waiting
			ms_eventsPending.signal();
		}
	}
	else
	{
		// fulfill entire read request
		const int amountRead = osFile->read(request->buffer, request->bytesToBeRead);

		request->bytesRead += amountRead;
		request->bytesToBeRead -= amountRead;

		// store final number of bytes read in storage accessible to main thread
		*request->returnValue = static_cast<int>(request->bytesRead);

		Gate *gate = request->gate;
		delete request;
		gate->open();
	}
}

// ----------------------------------------------------------------------
/**
 * Routine where the file thread runs.
 * 
 * The file thread waits at the WaitForSingleObject until the game thread
 * triggers it by calling a submit-request function that involves the queues.
 * It then services the first AudioVideo request, and if there are no AudioVideo requests to
 * be serviced, a Data request instead.  All access to the queue is protected by
 * critical sections.
 */

void FileStreamerThread::threadRoutine()
{
 	bool quit = false;

	//loop until a quit request is processed
	while (!quit)
	{
		volatile Request* request;

		//wait until there is data to processs
		ms_eventsPending.wait();

		//get the request to service
		ms_queueCriticalSection.enter();
			request = nullptr;

			//if there is an AudioVideo request, service it first (interrupting any current data request)
			if (ms_firstAudioVideoRequest)
			{
				request = ms_firstAudioVideoRequest;
				ms_firstAudioVideoRequest = ms_firstAudioVideoRequest->next;
				if (ms_firstAudioVideoRequest == nullptr)
					ms_lastAudioVideoRequest = nullptr;
				request->next = nullptr;
			}
			else
				if (ms_firstDataRequest)
				{
					request = ms_firstDataRequest;
					ms_firstDataRequest = ms_firstDataRequest->next;
					if (ms_firstDataRequest == nullptr)
						ms_lastDataRequest = nullptr;
					request->next = nullptr;
				}
				else
					if (ms_firstLowRequest)
					{
						request = ms_firstLowRequest;
						ms_firstLowRequest = ms_firstLowRequest->next;
						if (ms_firstLowRequest == nullptr)
							ms_lastLowRequest = nullptr;
						request->next = nullptr;
					}
					else
					{
						DEBUG_FATAL(true, ("no request waiting"));
					}

		ms_queueCriticalSection.leave();

		if (request)
		{
			switch (request->type)
			{
				case Request::Quit:
					processQuit(request);
					quit = true;
					break;

				case Request::Read:
					processRead(request);
					break;

				case Request::Unknown:
				default:
					DEBUG_FATAL(true, ("FileStreamerThread::threadRoutine unknown request %d", static_cast<int>(request->type)));
					break;
			}
		}
	}
}

// ======================================================================

MemoryBlockManager *FileStreamerThread::Request::ms_memoryBlockManager;

// ======================================================================

void FileStreamerThread::Request::install()
{
	ms_memoryBlockManager = new MemoryBlockManager("FileStreamerThread::Request::memoryBlockManager", true, sizeof(Request), 0, 0, 0);
	ExitChain::add(&remove, "FileStreamerThread::Request");
}

// ----------------------------------------------------------------------

void FileStreamerThread::Request::remove()
{
	delete ms_memoryBlockManager;
	ms_memoryBlockManager = nullptr;
}

// ----------------------------------------------------------------------

void *FileStreamerThread::Request::operator new(size_t size)
{
	UNREF(size);
	NOT_NULL(ms_memoryBlockManager);
	DEBUG_FATAL(size != sizeof(Request), ("incorrect request size"));
	DEBUG_FATAL(size != static_cast<size_t>(ms_memoryBlockManager->getElementSize()), ("installed with bad size"));

	return ms_memoryBlockManager->allocate();
}

// ----------------------------------------------------------------------

void FileStreamerThread::Request::operator delete(void *pointer)
{
	NOT_NULL(ms_memoryBlockManager);
	ms_memoryBlockManager->free(pointer);
}

// ----------------------------------------------------------------------

FileStreamerThread::Request::Request()
: next(nullptr),
  type(Request::Unknown),
	osFile(nullptr),
	buffer(0),
  bytesToBeRead(0),
  bytesRead(0),
	gate(nullptr),
  priority(AbstractFile::PriorityData),
	returnValue(nullptr)
{
}

// ----------------------------------------------------------------------

FileStreamerThread::Request::~Request()
{
#ifdef _DEBUG
	next = nullptr;
	buffer = nullptr;
	gate = nullptr;
	returnValue = nullptr;
#endif
}

// ======================================================================
