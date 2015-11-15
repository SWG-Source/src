#ifndef TRACK_MESSAGE_FAILURES_H
#define TRACK_MESSAGE_FAILURES_H

/////////////////////////////////////////////////////////////////////////////
// Define the preprocessor directive below (TRACK_READ_WRITE_FAILURES)     //
// in *.vcproj/make file to enable tracking of message read/write failures //
/////////////////////////////////////////////////////////////////////////////

#ifdef TRACK_READ_WRITE_FAILURES

#include <string>
#include <vector>

namespace soe 
{
	std::string PrintToString(const char * format, ...);

	void PushMessageFailure(const std::string & failureDescription);
	
	void GetMessageFailureStack(std::vector<std::string> & failureDescriptions);
	
	void ClearMessageFailureStack();
	
	void SetSerializeMessageFailures(bool serialize);
};

#endif // TRACK_READ_WRITE_FAILURES

#endif // TRACK_MESSAGE_FAILURES_H
