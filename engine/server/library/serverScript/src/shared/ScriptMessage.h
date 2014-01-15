// ScriptMessage.h
// copyright 2001 Sony Online Entertainment
// Author: Justin Randall

#ifndef	_INCLUDED_ScriptMessage_H
#define	_INCLUDED_ScriptMessage_H

//-----------------------------------------------------------------------

#include "sharedMessageDispatch/Message.h"
#include <string>

class ScriptParams;
class ScriptDictionary;


//-----------------------------------------------------------------------

class ScriptMessage : public MessageDispatch::MessageBase
{
public:
	 ScriptMessage         (const std::string & sourceScript, const std::string & scriptFunctionName, const ScriptDictionary & parameters);
	~ScriptMessage         ();

	const ScriptDictionary & getParameters          () const;
	const std::string &      getScriptFunctionName  () const;
	const std::string &      getSourceScriptName    () const;

private:
	ScriptMessage &  operator =     (const ScriptMessage & rhs);
	                 ScriptMessage  (const ScriptMessage & source);
	ScriptMessage();
	
private:
	const ScriptDictionary & m_parameters;
	const std::string        m_scriptFunctionName;
	const std::string        m_sourceScriptName;
};

//-----------------------------------------------------------------------

inline const ScriptDictionary & ScriptMessage::getParameters() const
{
	return m_parameters;
}

//-----------------------------------------------------------------------

inline const std::string & ScriptMessage::getScriptFunctionName() const
{
	return m_scriptFunctionName;
}

//-----------------------------------------------------------------------

inline const std::string & ScriptMessage::getSourceScriptName() const
{
	return m_sourceScriptName;
}

//-----------------------------------------------------------------------

#endif	// _INCLUDED_ScriptMessage_H
