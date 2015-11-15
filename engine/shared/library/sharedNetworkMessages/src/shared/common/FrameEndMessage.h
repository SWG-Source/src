// ======================================================================
//
// FrameEndMessage.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef _FrameEndMessage_H_
#define _FrameEndMessage_H_

// ======================================================================

#include "sharedNetworkMessages/GameNetworkMessage.h"

// ======================================================================

class FrameEndMessage: public GameNetworkMessage
{
public:
	FrameEndMessage(unsigned int processId, unsigned long frameTime, char const *profilerData);
	FrameEndMessage(Archive::ReadIterator &source);
	~FrameEndMessage();

	unsigned int getProcessId() const;
	unsigned long getFrameTime() const;
	std::string const &getProfilerData() const;

private:
	Archive::AutoVariable<unsigned int> m_processId;
	Archive::AutoVariable<unsigned long> m_frameTime;
	Archive::AutoVariable<std::string> m_profilerData;

	FrameEndMessage();
	FrameEndMessage(FrameEndMessage const &);
	FrameEndMessage &operator=(FrameEndMessage const &);
};

// ----------------------------------------------------------------------

inline unsigned int FrameEndMessage::getProcessId() const
{
	return m_processId.get();
}

// ----------------------------------------------------------------------

inline unsigned long FrameEndMessage::getFrameTime() const
{
	return m_frameTime.get();
}

// ----------------------------------------------------------------------

inline std::string const &FrameEndMessage::getProfilerData() const
{
	return m_profilerData.get();
}

// ======================================================================

#endif // _FrameEndMessage_H_

