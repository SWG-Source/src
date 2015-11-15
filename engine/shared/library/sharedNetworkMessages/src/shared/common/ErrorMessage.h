// ErrorMessage.h
// copyright 2001 Verant Interactive
// Author: Justin Randall

#ifndef _ErrorMessage_H
#define _ErrorMessage_H

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/GameNetworkMessage.h"

//-----------------------------------------------------------------------

class ErrorMessage : public GameNetworkMessage
{
public:
	ErrorMessage(const std::string & errorName, const std::string & description, const bool fatal=false);
	ErrorMessage(Archive::ReadIterator & source);
	virtual ~ErrorMessage();

	const std::string & getErrorName() const;
	const bool          getFatal() const;
	const std::string & getDescription() const;
private:
	Archive::AutoVariable<std::string> errorName;
	Archive::AutoVariable<std::string> description;
	Archive::AutoVariable<bool>        fatal;
};

//-----------------------------------------------------------------------

inline const std::string & ErrorMessage::getErrorName() const
{
	return errorName.get();
}

//-----------------------------------------------------------------------

inline const bool ErrorMessage::getFatal() const
{
	return fatal.get();
}

//-----------------------------------------------------------------------

inline const std::string & ErrorMessage::getDescription() const
{
	return description.get();
}

//-----------------------------------------------------------------------

#endif // _ErrorMessage_H
