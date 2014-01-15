// ScriptMessage.cpp
// copyright 2001 Sony Online Entertainment
// Author: Justin Randall

//-----------------------------------------------------------------------
#include "serverScript/FirstServerScript.h"
#include "serverScript/ScriptMessage.h"
#include "serverScript/ScriptDictionary.h"


//-----------------------------------------------------------------------

ScriptMessage::ScriptMessage(const std::string & sourceScriptName, const std::string & scriptFunctionName, const ScriptDictionary & parameters) :
MessageBase(scriptFunctionName.c_str()),
m_parameters(parameters),
m_scriptFunctionName(scriptFunctionName),
m_sourceScriptName(sourceScriptName)
{
}

//-----------------------------------------------------------------------

ScriptMessage::ScriptMessage(const ScriptMessage & source) :
MessageBase(source.getType()),
m_parameters(source.getParameters()),
	m_scriptFunctionName(source.getScriptFunctionName())
{
}

//-----------------------------------------------------------------------

ScriptMessage::~ScriptMessage()
{
}

//-----------------------------------------------------------------------

ScriptMessage & ScriptMessage::operator = (const ScriptMessage & rhs)
{
	if(this != &rhs)
	{
		// make assignments if right hand side is not this instance
	}
	return *this;
}

//-----------------------------------------------------------------------

