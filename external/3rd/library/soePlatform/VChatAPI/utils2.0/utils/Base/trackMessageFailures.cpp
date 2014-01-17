#include "trackMessageFailures.h"

#ifdef TRACK_READ_WRITE_FAILURES

#	include <stdarg.h>
#	include <stdio.h>

#	if defined(_MT) || defined(_REENTRANT)
#		include "thread.h"
#	endif

#	ifdef WIN32
#		define vsnprintf _vsnprintf
#	endif

#	include <list>

namespace soe
{

////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////
	//  Variables for tracking read/write failure
	typedef std::list<std::string> stringList_t;

	stringList_t messageFailureList;

	bool serializeMessageFailures = false;

#if defined(_MT) || defined(_REENTRANT)
	class SwitchableScopeLock
	{
	public:
		SwitchableScopeLock()
		{
			if (serializeMessageFailures)
				{ msMutex.Lock(); }
		}
		~SwitchableScopeLock()
		{
			if (serializeMessageFailures)
				{ msMutex.Unlock(); }
		}
	private:
		static Mutex msMutex;
	};
	Mutex SwitchableScopeLock::msMutex;
#else
	class SwitchableScopeLock
	{
	public:
		SwitchableScopeLock() {}
		~SwitchableScopeLock() {}
	};
#endif

////////////////////////////////////////////////////////////////////////////////

	////////////////////////////////////////////////
	//  Functions for tracking read/write failure

	std::string PrintToString(const char * format, ...)
	{
		va_list arg;

		va_start(arg, format);

		char buffer[2048];

		vsnprintf((char *)buffer, sizeof(buffer), format, arg);

		va_end(arg);

		return buffer;
	}

	void SetSerializeMessageFailures(bool serialize)
	{
		serializeMessageFailures = serialize;
	}

	void PushMessageFailure(const std::string & failureDescription)
	{
		SwitchableScopeLock s;

		messageFailureList.push_front(failureDescription);
	}

	void GetMessageFailureStack(std::vector<std::string> & failureDescriptions)
	{
		SwitchableScopeLock s;
		size_t index = 0;

		failureDescriptions.resize(messageFailureList.size());
		for (stringList_t::const_iterator lit = messageFailureList.begin(); lit != messageFailureList.end(); lit++, index++)
		{
			failureDescriptions[index] = *lit;
		}
	}

	void ClearMessageFailureStack()
	{
		SwitchableScopeLock s;

		messageFailureList.clear();
	}

////////////////////////////////////////////////////////////////////////////////

}

#endif // TRACK_READ_WRITE_FAILURES

