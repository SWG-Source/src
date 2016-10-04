// ======================================================================
//
// RemoteDebug_inner.cpp
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedDebug/FirstSharedDebug.h"
#include "sharedDebug/RemoteDebug.h"
#include "sharedDebug/RemoteDebug_inner.h"

#include <string>
#include <list>
#include <map>

// ----------------------------------------------------------------------

RemoteDebugClient::NewStreamFunction       RemoteDebugClient::ms_newStreamFunction;
RemoteDebugClient::StreamMessageFunction   RemoteDebugClient::ms_streamMessageFunction;
RemoteDebugClient::NewVariableFunction     RemoteDebugClient::ms_newVariableFunction;
RemoteDebugClient::VariableValueFunction   RemoteDebugClient::ms_variableValueFunction;
RemoteDebugClient::VariableTypeFunction    RemoteDebugClient::ms_variableTypeFunction;
RemoteDebugClient::BeginFrameFunction      RemoteDebugClient::ms_beginFrameFunction;
RemoteDebugClient::EndFrameFunction        RemoteDebugClient::ms_endFrameFunction;
RemoteDebugClient::NewStaticViewFunction   RemoteDebugClient::ms_newStaticViewFunction;
RemoteDebugClient::LineFunction            RemoteDebugClient::ms_lineFunction;

uint32                                     RemoteDebugServer::ms_inputTarget;

// ======================================================================

RemoteDebug::Channel::Channel(const std::string& name, Channel *parent)
	: m_name(nullptr),
	m_parent(parent)
{
	m_name = new std::string(name);
	m_children = new NodeList;
}

// ----------------------------------------------------------------------

RemoteDebug::Channel::~Channel()
{
	m_parent = nullptr;
	if (m_name)
	{
		delete m_name;
		m_name = nullptr;
	}
	if (m_children)
	{
		delete m_children;
		m_children = nullptr;
	}
}

// ----------------------------------------------------------------------

void RemoteDebug::Channel::addChild(Channel *child)
{
	m_children->push_back(child);
}

// ----------------------------------------------------------------------

const std::string& RemoteDebug::Channel::name()
{
	return *m_name;
}

// ======================================================================

RemoteDebug::Variable::Variable(const std::string& name, void *memLoc, VARIABLE_TYPES type)
	: m_memLoc(memLoc),
	m_name(nullptr),
	m_type(type)
{
	m_name = new std::string(name);

	int32 i = 0;
	float f = 0.0;
	int32 b = 0;
	switch (m_type)
	{
	case INT:
		m_value.intValue = i;
		break;

	case FLOAT:
		m_value.floatValue = f;
		break;

	case CSTRING:
		m_value.stringValue = nullptr;
		break;

	case BOOL:
		m_value.boolValue = b;
		break;
	}
}

// ----------------------------------------------------------------------

RemoteDebug::Variable::~Variable()
{
	if (m_type == CSTRING)
		delete m_value.stringValue;

	if (m_name)
	{
		delete m_name;
		m_name = nullptr;
	}
}

// ----------------------------------------------------------------------

void RemoteDebug::Variable::setValue(VARIABLEVALUE v)
{
	switch (m_type)
	{
	case INT:
		m_value.intValue = v.intValue;
		break;

	case FLOAT:
		m_value.floatValue = v.floatValue;
		break;

	case CSTRING:
		m_value.stringValue = v.stringValue;
		break;

	case BOOL:
		m_value.boolValue = v.boolValue;
		break;
	}
}

// ----------------------------------------------------------------------

void RemoteDebug::Variable::setValue(void *memLoc)
{
	if (memLoc)
	{
		switch (m_type)
		{
		case INT:
			memcpy(&m_value.intValue, memLoc, sizeof(int32));
			break;

		case FLOAT:
			memcpy(&m_value.floatValue, memLoc, sizeof(float));
			break;

		case CSTRING:
			delete[] m_value.stringValue;
			m_value.stringValue = new char[strlen(static_cast<char *>(memLoc))];
			strcpy(m_value.stringValue, const_cast<const char *>(static_cast<char *>(memLoc)));
			break;

		case BOOL:
			memcpy(&m_value.boolValue, memLoc, sizeof(int32));
			break;
		}
	}
}

// ----------------------------------------------------------------------

RemoteDebug::Variable::VARIABLEVALUE RemoteDebug::Variable::value()
{
	return m_value;
}
// ----------------------------------------------------------------------

const std::string& RemoteDebug::Variable::name()
{
	return *m_name;
}

// ----------------------------------------------------------------------

void RemoteDebug::Variable::setType(VARIABLE_TYPES type)
{
	m_type = type;
}

// ----------------------------------------------------------------------

void RemoteDebug::Variable::pushValue()
{
	if (m_memLoc)
		switch (m_type)
		{
		case BOOL:
		case INT:
			memcpy(m_memLoc, &m_value, sizeof(int32));
			break;

		case CSTRING:
			strcpy(static_cast<char *>(m_memLoc), m_value.stringValue);
			break;

		case FLOAT:
			memcpy(m_memLoc, &m_value, sizeof(float));
			break;
		}
}

// ----------------------------------------------------------------------

RemoteDebug::VARIABLE_TYPES RemoteDebug::Variable::type()
{
	return m_type;
}

// ======================================================================

void RemoteDebugClient::install(RemoveFunction rf, OpenFunction of,
	CloseFunction cf, SendFunction sf,
	IsReadyFunction irf, NewStreamFunction ncf,
	StreamMessageFunction cmf, NewVariableFunction nvf,
	VariableValueFunction vvf, VariableTypeFunction vtf,
	BeginFrameFunction bff, EndFrameFunction eff,
	NewStaticViewFunction nsvf, LineFunction lf)
{
	RemoteDebug::install(rf, of, cf, sf, irf);
	ms_newStreamFunction = ncf;
	ms_streamMessageFunction = cmf;
	ms_newVariableFunction = nvf;
	ms_variableValueFunction = vvf;
	ms_variableTypeFunction = vtf;
	ms_beginFrameFunction = bff;
	ms_endFrameFunction = eff;
	ms_newStaticViewFunction = nsvf;
	ms_lineFunction = lf;
}

// ----------------------------------------------------------------------

void RemoteDebugClient::remove()
{
	if (ms_opened)
		close();
	RemoteDebug::remove();
}

// ----------------------------------------------------------------------

void RemoteDebugClient::receive(const unsigned char * const message, const uint32 bufferLen)
{
	UNREF(bufferLen);
	//convert to a const char[] for conveinence
	const char * const charMessage = reinterpret_cast<const char * const>(message);
	//these values are known based on the packet protocol (see RemoteDebug.h for more info
	const int sizeOfMessageType = 1;
	const int sizeOfChannelNumber = 4;
	const int sizeOfSizeofPayload = 4;
	//this value is filled in from the packet
	uint32 channelNumber = 0;
	//this value if filled in from the packet
	uint32 sizeOfPayload = 0;

	//get the message type
	RemoteDebug::MESSAGE_TYPE type = static_cast<RemoteDebug::MESSAGE_TYPE>(charMessage[0]);

	//grab the channelNumber
	memcpy(&channelNumber, charMessage + sizeOfMessageType, sizeOfChannelNumber);

	//build the payload if needed
	if (type == RemoteDebug::STREAM ||
		type == RemoteDebug::STATIC_LINE ||
		type == RemoteDebug::NEW_STATIC ||
		type == RemoteDebug::NEW_STREAM ||
		type == RemoteDebug::VARIABLE_TYPE ||
		type == RemoteDebug::VARIABLE_VALUE ||
		type == RemoteDebug::NEW_VARIABLE)
	{
		memcpy(&sizeOfPayload, charMessage + sizeOfMessageType + sizeOfChannelNumber, sizeOfSizeofPayload);
		memcpy(ms_buffer, charMessage + sizeOfMessageType + sizeOfChannelNumber + sizeOfSizeofPayload, sizeOfPayload);
	}

	//can't have local variables defined within switch statements, so place them out here.
	int32 t;
	VARIABLE_TYPES varType;

	switch (type)
	{
	case RemoteDebug::STREAM:
		//see if we're redefining an already existing stream
		if (ms_streamMessageFunction)
			ms_streamMessageFunction(channelNumber, const_cast<const char*>(ms_buffer));
		break;

	case RemoteDebug::NEW_STREAM:
		if (channelNumber < ms_nextStream)
			return;
		registerStream(ms_buffer);
		if (ms_newStreamFunction)
			ms_newStreamFunction(channelNumber, const_cast<const char*>(ms_buffer));
		break;

	case RemoteDebug::NEW_VARIABLE:
		if (channelNumber < ms_nextVariable)
			return;
		//do not send value back to server (hence the "false")
		registerVariable(ms_buffer, nullptr, BOOL, false);
		if (ms_newVariableFunction)
			ms_newVariableFunction(channelNumber, const_cast<const char*>(ms_buffer));
		break;

	case RemoteDebug::VARIABLE_TYPE:
		t = atoi(ms_buffer);
		varType = static_cast<VARIABLE_TYPES>(t);
		(*ms_variableValues)[channelNumber]->setType(varType);
		if (ms_variableTypeFunction)
			ms_variableTypeFunction(channelNumber, varType);
		break;

	case RemoteDebug::VARIABLE_VALUE:
		(*ms_variableValues)[channelNumber]->setValue(ms_buffer);
		if (ms_variableValueFunction)
			ms_variableValueFunction(channelNumber, ms_buffer);
		break;

	case RemoteDebug::NEW_STATIC:
		if (channelNumber < ms_nextStaticView)
			return;
		registerStaticView(ms_buffer);
		if (ms_newStaticViewFunction)
			ms_newStaticViewFunction(channelNumber, const_cast<const char*>(ms_buffer));
		break;

	case RemoteDebug::STATIC_LINE:
		if (ms_lineFunction)
			ms_lineFunction(channelNumber, const_cast<const char*>(ms_buffer));
		break;

	case RemoteDebug::STATIC_BEGIN_FRAME:
		if (ms_beginFrameFunction)
			ms_beginFrameFunction(channelNumber);
		break;

	case RemoteDebug::STATIC_END_FRAME:
		if (ms_endFrameFunction)
			ms_endFrameFunction(channelNumber);
		break;

	case RemoteDebug::STREAM_SQUELCH:
	case RemoteDebug::STREAM_UNSQUELCH:
	case RemoteDebug::STATIC_SQUELCH:
	case RemoteDebug::STATIC_UNSQUELCH:
	case RemoteDebug::STATIC_INPUT_TARGET:
	case RemoteDebug::STATIC_UP:
	case RemoteDebug::STATIC_DOWN:
	case RemoteDebug::STATIC_LEFT:
	case RemoteDebug::STATIC_RIGHT:
	case RemoteDebug::STATIC_ENTER:
	case RemoteDebug::REQUEST_ALL_CHANNELS:
	default:
		break;
	}
}

// ----------------------------------------------------------------------

/** Called when the clinet closes a session.  Since we don't want to maintain
  * data between sessions, clear out all data.
  */
void RemoteDebugClient::close()
{
	DEBUG_FATAL(!ms_installed, ("remoteDebug not installed"));

	//don't need to do anything if we're already closed
	if (!ms_opened)
		return;

	if (ms_closeFunction)
		ms_closeFunction();
	ms_opened = false;

	//empty out session-level data
	for (StreamMap::iterator it = ms_streams->begin(); it != ms_streams->end(); ++it)
		delete it->second;
	ms_streams->clear();

	for (StaticViewMap::iterator itr = ms_staticViews->begin(); itr != ms_staticViews->end(); ++itr)
		delete itr->second;
	ms_staticViews->clear();

	for (VariableMap::iterator iter = ms_variables->begin(); iter != ms_variables->end(); ++iter)
		delete iter->second;
	ms_variables->clear();

	for (VariableValueMap::iterator iterr = ms_variableValues->begin(); iterr != ms_variableValues->end(); ++iterr)
		delete iterr->second;
	ms_variableValues->clear();

	ms_squelchedStream->clear();
	ms_squelchedStatic->clear();

	ms_nextStream = 0;
	ms_nextStaticView = 0;
	ms_nextVariable = 0;
}

// ----------------------------------------------------------------------

void RemoteDebugClient::isReady()
{
	ms_opened = true;
	if (ms_isReadyFunction)
		ms_isReadyFunction();
	send(REQUEST_ALL_CHANNELS, "");
}

// ======================================================================

void RemoteDebugServer::install(RemoveFunction rmf, OpenFunction of,
	CloseFunction cf, SendFunction sf,
	IsReadyFunction irf)
{
	RemoteDebug::install(rmf, of, cf, sf, irf);
	ms_inputTarget = 0;
}

// ----------------------------------------------------------------------

void RemoteDebugServer::remove()
{
	if (ms_opened)
		close();
	RemoteDebug::remove();
}

// ----------------------------------------------------------------------

void RemoteDebugServer::isReady()
{
	ms_opened = true;
	if (ms_isReadyFunction)
		ms_isReadyFunction();
}
// ----------------------------------------------------------------------

void RemoteDebugServer::receive(const unsigned char * const message, const uint32 bufferLen)
{
	UNREF(bufferLen);
	//convert to a const char[] for conveinence
	const char * const charMessage = reinterpret_cast<const char * const>(message);
	//these values are known based on the packet protocol (see RemoteDebug.h for more info
	const int sizeOfMessageType = 1;
	const int sizeOfChannelNumber = 4;
	const int sizeOfSizeofPayload = 4;
	//this value is filled in from the packet
	uint32 channelNumber = 0;
	//this value if filled in from the packet
	uint32 sizeOfPayload = 0;

	//get the message type
	RemoteDebug::MESSAGE_TYPE type = static_cast<RemoteDebug::MESSAGE_TYPE>(charMessage[0]);

	//grab the channelNumber
	memcpy(&channelNumber, charMessage + sizeOfMessageType, sizeOfChannelNumber);

	//build the payload if needed
	if (type == RemoteDebug::STREAM ||
		type == RemoteDebug::NEW_STREAM ||
		type == RemoteDebug::VARIABLE_TYPE ||
		type == RemoteDebug::VARIABLE_VALUE ||
		type == RemoteDebug::NEW_VARIABLE)
	{
		memcpy(&sizeOfPayload, charMessage + sizeOfMessageType + sizeOfChannelNumber, sizeOfSizeofPayload);
		memcpy(ms_buffer, charMessage + sizeOfMessageType + sizeOfChannelNumber + sizeOfSizeofPayload, sizeOfPayload);
	}

	Variable* v = nullptr;
	switch (type)
	{
	case STREAM:
	case NEW_STREAM:
	case VARIABLE_TYPE:
	case NEW_VARIABLE:
		break;

	case STREAM_SQUELCH:
		(*ms_squelchedStream)[channelNumber] = true;
		break;

	case STREAM_UNSQUELCH:
		(*ms_squelchedStream)[channelNumber] = false;
		break;

	case VARIABLE_VALUE:
		//set the new value into the variable on the server side
		v = (*ms_variableValues)[channelNumber];
		if (!v)
		{
			DEBUG_FATAL(true, ("message on undefined variable"));
			return;  //lint !e527 unreachable code
		}
		v->setValue(ms_buffer);
		//put that new value back into the game object
		v->pushValue();
		break;

	case STATIC_SQUELCH:
		(*ms_squelchedStatic)[channelNumber] = true;
		break;

	case STATIC_UNSQUELCH:
		(*ms_squelchedStatic)[channelNumber] = false;
		break;

	case REQUEST_ALL_CHANNELS:
		sendAllChannels();
		break;

	case STATIC_INPUT_TARGET:
		ms_inputTarget = channelNumber;
		break;

	case STATIC_UP:
		if ((*ms_upFunctionMap)[ms_inputTarget] != nullptr)
			(*ms_upFunctionMap)[ms_inputTarget](); //lint !e10, !e522 ("expecting a function" and "expected void assignment)
		break;

	case STATIC_DOWN:
		if ((*ms_downFunctionMap)[ms_inputTarget] != nullptr)
			(*ms_downFunctionMap)[ms_inputTarget](); //lint !e10, !e522 ("expecting a function" and "expected void assignment)
		break;

	case STATIC_LEFT:
		if ((*ms_leftFunctionMap)[ms_inputTarget] != nullptr)
			(*ms_leftFunctionMap)[ms_inputTarget](); //lint !e10, !e522 ("expecting a function" and "expected void assignment)
		break;

	case STATIC_RIGHT:
		if ((*ms_rightFunctionMap)[ms_inputTarget] != nullptr)
			(*ms_rightFunctionMap)[ms_inputTarget](); //lint !e10, !e522 ("expecting a function" and "expected void assignment)
		break;

	case STATIC_ENTER:
		if ((*ms_enterFunctionMap)[ms_inputTarget] != nullptr)
			(*ms_enterFunctionMap)[ms_inputTarget](); //lint !e10, !e522 ("expecting a function" and "expected void assignment)
		break;

	case NEW_STATIC:
	case STATIC_BEGIN_FRAME:
	case STATIC_END_FRAME:
	case STATIC_LINE:
	default:
		break;
	}
}

// ----------------------------------------------------------------------

/** Called when a session is closed.  Since this is the server side, and must
  * maintain data between sessions, data is NOT cleared out.
  */
void RemoteDebugServer::close()
{
	DEBUG_FATAL(!ms_installed, ("remoteDebug not installed"));

	if (ms_closeFunction)
		ms_closeFunction();
	ms_opened = false;
}

// ----------------------------------------------------------------------

void RemoteDebugServer::sendAllChannels()
{
	for (StreamMap::iterator it = ms_streams->begin(); it != ms_streams->end(); ++it)
		send(NEW_STREAM, it->second->name().c_str());
	for (VariableMap::iterator itr = ms_variables->begin(); itr != ms_variables->end(); ++itr)
	{
		send(NEW_VARIABLE, itr->second->name().c_str());
		send(VARIABLE_TYPE, itr->second->name().c_str());
		send(VARIABLE_VALUE, itr->second->name().c_str());
	}
	for (StaticViewMap::iterator iter = ms_staticViews->begin(); iter != ms_staticViews->end(); ++iter)
		send(NEW_STATIC, iter->second->name().c_str());
}

// ======================================================================