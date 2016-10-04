// ======================================================================
//
// RemoteDebug.cpp
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#include "sharedDebug/FirstSharedDebug.h"
#include "sharedDebug/RemoteDebug.h"
#include "sharedDebug/RemoteDebug_inner.h"

#include "sharedFoundation/ExitChain.h"

#include <string>
#include <list>
#include <map>
#include <cstdio>
#include <cstdarg>

// ----------------------------------------------------------------------

//static definitions
RemoteDebug::RemoveFunction        RemoteDebug::ms_removeFunction;
RemoteDebug::OpenFunction          RemoteDebug::ms_openFunction;
RemoteDebug::CloseFunction         RemoteDebug::ms_closeFunction;
RemoteDebug::SendFunction          RemoteDebug::ms_sendFunction;
RemoteDebug::IsReadyFunction       RemoteDebug::ms_isReadyFunction;

RemoteDebug::StreamMap            *RemoteDebug::ms_streams;
RemoteDebug::StaticViewMap        *RemoteDebug::ms_staticViews;
RemoteDebug::VariableMap          *RemoteDebug::ms_variables;
std::map<uint32, bool>            *RemoteDebug::ms_squelchedStream;
std::map<uint32, bool>            *RemoteDebug::ms_squelchedStatic;
RemoteDebug::VariableValueMap     *RemoteDebug::ms_variableValues;
RemoteDebug::MovementFunctionMap  *RemoteDebug::ms_upFunctionMap;
RemoteDebug::MovementFunctionMap  *RemoteDebug::ms_downFunctionMap;
RemoteDebug::MovementFunctionMap  *RemoteDebug::ms_leftFunctionMap;
RemoteDebug::MovementFunctionMap  *RemoteDebug::ms_rightFunctionMap;
RemoteDebug::MovementFunctionMap  *RemoteDebug::ms_enterFunctionMap;

bool                               RemoteDebug::ms_opened;
bool                               RemoteDebug::ms_installed;
char                               RemoteDebug::ms_buffer[MAX_BUFFER_SIZE];
char                               RemoteDebug::ms_varArgs_buffer[MAX_BUFFER_SIZE];
uint32                             RemoteDebug::ms_nextStream;
uint32                             RemoteDebug::ms_nextVariable;
uint32                             RemoteDebug::ms_nextStaticView;

//-----------------------------------------------------------------

RemoteDebug::~RemoteDebug ()
{
}

// ======================================================================

void RemoteDebug::install(RemoveFunction rm, OpenFunction o, CloseFunction c, SendFunction s, IsReadyFunction irf)//, ReceiveFunction rc)
{
	ms_streams         = new StreamMap;
	ms_staticViews     = new StaticViewMap;
	ms_variables       = new VariableMap;
	ms_squelchedStream = new std::map<uint32, bool>;
	ms_squelchedStatic = new std::map<uint32, bool>;
	ms_variableValues  = new VariableValueMap;

	ms_upFunctionMap    = new MovementFunctionMap;
	ms_downFunctionMap  = new MovementFunctionMap;
	ms_leftFunctionMap  = new MovementFunctionMap;
	ms_rightFunctionMap = new MovementFunctionMap;
	ms_enterFunctionMap = new MovementFunctionMap;

	ms_nextStream    = 0;
	ms_nextStaticView = 0;
	ms_nextVariable   = 0;

	ms_removeFunction = rm;
	ms_openFunction = o;
	ms_closeFunction = c;
	ms_sendFunction = s;
	ms_isReadyFunction = irf;
	ms_opened = false;
	ms_installed = true;
}

// ----------------------------------------------------------------------

void RemoteDebug::remove(void)
{
	if(!ms_installed)
	{
		DEBUG_FATAL(!ms_installed, ("not installed"));
		return;
	}

	ms_removeFunction = nullptr;
	ms_openFunction = nullptr;
	ms_closeFunction = nullptr;
	ms_sendFunction = nullptr;

	//empty out session-level data
	for (StreamMap::iterator it = ms_streams->begin(); it != ms_streams->end(); ++it)
		delete it->second;
	delete ms_streams;

	for (StaticViewMap::iterator itr = ms_staticViews->begin(); itr != ms_staticViews->end(); ++itr)
		delete itr->second;
	delete ms_staticViews;

	for (VariableMap::iterator iter = ms_variables->begin(); iter != ms_variables->end(); ++iter)
		delete iter->second;
	delete ms_variables;

	for (VariableValueMap::iterator iterr = ms_variableValues->begin(); iterr != ms_variableValues->end(); ++iterr)
		delete iterr->second;
	delete ms_variableValues;

	ms_nextStream     = 0;
	ms_nextStaticView = 0;
	ms_nextVariable   = 0;

	delete ms_squelchedStream;
	delete ms_squelchedStatic;
	delete ms_upFunctionMap;
	delete ms_downFunctionMap;
	delete ms_leftFunctionMap;
	delete ms_rightFunctionMap;
	delete ms_enterFunctionMap;

//	delete ms_sessions;
	ms_installed = false;
}

// ----------------------------------------------------------------------

void RemoteDebug::open(const char* server, uint16 port)
{
	DEBUG_FATAL(!ms_installed, ("remoteDebug not installed"));
	DEBUG_FATAL(ms_opened, ("remoteDebug session already open"));

	ms_openFunction(server, port);
}

// ----------------------------------------------------------------------

uint32 RemoteDebug::registerStream(const std::string& streamName)
{
	DEBUG_FATAL(!ms_installed, ("remoteDebug not installed"));

	uint32 streamNum = 0;
	Channel *parent = nullptr;
	std::string base;
	std::string rest = streamName;
	//look for subchannels with a double backslash
	uint32 baseIndex = 0;
	uint32 restIndex = streamName.find('\\');
	while(restIndex != std::string::npos)
	{
		//pass the '\'
		++restIndex;
		//break channel into the base and the rest
		base = streamName.substr(baseIndex, restIndex-baseIndex-1);
		rest = streamName.substr(restIndex, streamName.size());
		//build the channel
		std::string newBase = streamName.substr(0, restIndex-1);
		int32 streamNumber = findStream(newBase);
		//only add the channel if we don't have it yet
		if (streamNumber == -1)
		{
			Channel *child = new Channel(newBase, parent);
			if (parent)
				parent->addChild(child);
			parent = child;
			(*ms_streams)[ms_nextStream] = child;
			streamNum = ms_nextStream++;
			//every new channel is not squelched by default
			(*ms_squelchedStream)[streamNum] = false;
			//make sure we send the new base channel to the clients
			send(NEW_STREAM, newBase.c_str());
		}
		//look for any other subChannel
		uint32 oldRestIndex = restIndex;
		baseIndex = restIndex;
		restIndex = rest.find('\\');
		if (restIndex != std::string::npos)
			restIndex += oldRestIndex;
	}
	//no more sub channels, add this channel
	Channel *child = new Channel(streamName, parent);
	if (parent)
		parent->addChild(child);
	(*ms_streams)[ms_nextStream] = child;
	streamNum = ms_nextStream++;
	//every new channel is not squelched by default
	(*ms_squelchedStream)[streamNum] = false;

	//return the new channelID
	return streamNum;
}

// ----------------------------------------------------------------------

uint32 RemoteDebug::registerStaticView(const std::string& channelName)
{
	DEBUG_FATAL(!ms_installed, ("remoteDebug not installed"));
	uint32 staticViewNum = 0;
	Channel *parent = nullptr;
	std::string base;
	std::string rest = channelName;
	//look for subchannels with a double backslash
	uint32 baseIndex = 0;
	uint32 restIndex = channelName.find('\\');
	while(restIndex != std::string::npos)
	{
		//pass the '\'
		++restIndex;
		//break channel into the base and the rest
		base = channelName.substr(baseIndex, restIndex-baseIndex-1);
		rest = channelName.substr(restIndex, channelName.size());
		//build the channel
		std::string newBase = channelName.substr(0, restIndex-1);
		int32 channelNumber = findStaticView(newBase);
		//only add the channel if we don't have it yet
		if (channelNumber == -1)
		{
			Channel *child = new Channel(newBase, parent);
			if (parent)
				parent->addChild(child);
			parent = child;
			(*ms_staticViews)[ms_nextStaticView] = child;
			staticViewNum = ms_nextStaticView++;
			//every new channel is not squelched by default
			(*ms_squelchedStatic)[staticViewNum] = false;
			//make sure we send the new base channel to the clients
			send(NEW_STATIC, newBase.c_str());
		}
		//look for any other subChannel
		uint32 oldRestIndex = restIndex;
		baseIndex = restIndex;
		restIndex = rest.find('\\');
		if (restIndex != std::string::npos)
			restIndex += oldRestIndex;
	}
		//no more sub channels, add this channel
		Channel *child = new Channel(channelName, parent);
		if (parent)
			parent->addChild(child);
		(*ms_staticViews)[ms_nextStaticView] = child;
		staticViewNum = ms_nextStaticView++;
		//every new static view is not by default
		(*ms_squelchedStatic)[staticViewNum] = true;

		//return the new channelID
	return staticViewNum;
}

// ----------------------------------------------------------------------

/** This function is called via a globally-accessible macro, so we can't assume that
  * the system is installed.
  */
bool RemoteDebug::registerUpFunction(const char* staticViewName, UpFunction func)
{
	if (!ms_installed)
		return false;

	int32 chan = findStaticView(staticViewName);
	if (chan == -1)
		return false;

	(*ms_upFunctionMap)[chan] = func;
	return true;
}

// ----------------------------------------------------------------------

/** This function is called via a globally-accessible macro, so we can't assume that
  * the system is installed.
  */
bool RemoteDebug::registerDownFunction(const char* staticViewName, UpFunction func)
{
	if (!ms_installed)
		return false;

	int32 chan = findStaticView(staticViewName);
	if (chan == -1)
		return false;

	(*ms_downFunctionMap)[chan] = func;
	return true;
}

// ----------------------------------------------------------------------

/** This function is called via a globally-accessible macro, so we can't assume that
  * the system is installed.
  */
bool RemoteDebug::registerLeftFunction(const char* staticViewName, UpFunction func)
{
	if (!ms_installed)
		return false;

	int32 chan = findStaticView(staticViewName);
	if (chan == -1)
		return false;

	(*ms_leftFunctionMap)[chan] = func;
	return true;
}

// ----------------------------------------------------------------------

/** This function is called via a globally-accessible macro, so we can't assume that
  * the system is installed.
  */
bool RemoteDebug::registerRightFunction(const char* staticViewName, UpFunction func)
{
	if (!ms_installed)
		return false;

	int32 chan = findStaticView(staticViewName);
	if (chan == -1)
		return false;

	(*ms_rightFunctionMap)[chan] = func;
	return true;
}

// ----------------------------------------------------------------------

/** This function is called via a globally-accessible macro, so we can't assume that
  * the system is installed.
  */
bool RemoteDebug::registerEnterFunction(const char* staticViewName, UpFunction func)
{
	if (!ms_installed)
		return false;

	int32 chan = findStaticView(staticViewName);
	if (chan == -1)
		return false;

	(*ms_enterFunctionMap)[static_cast<uint32>(chan)] = func;
	return true;
}

// ----------------------------------------------------------------------

/** This function is called via a globally-accessible macro, so we can't assume that
  * the system is installed.
  */
uint32 RemoteDebug::registerVariable(const char* variableName, void *memLoc, VARIABLE_TYPES type, bool sendToClients)
{
	if (!ms_installed)
		return 0;

	std::string variable = variableName;

	uint32 variableNum = 0;
	Channel *parent = nullptr;
	std::string base;
	std::string rest = variable;
	//look for subchannels with a double backslash
	uint32 baseIndex = 0;
	uint32 restIndex = variable.find('\\');
	while(restIndex != std::string::npos)
	{
		//pass the '\'
		++restIndex;
		//break channel into the base and the rest
		base = variable.substr(baseIndex, restIndex-baseIndex-1);
		rest = variable.substr(restIndex, variable.size());
		//build the channel
		std::string newBase = variable.substr(0, restIndex-1);
		int32 variableNumber = findVariable(newBase);
		//only add the variable if we don't have it yet
		if (variableNumber == -1)
		{
			registerVariable(newBase.c_str(), nullptr, BOOL, sendToClients);
		}
		//look for any other subChannel
		uint32 oldRestIndex = restIndex;
		baseIndex = restIndex;
		restIndex = rest.find('\\');
		if (restIndex != std::string::npos)
			restIndex += oldRestIndex;
	}
	//don't register duplicates
	if(findVariable(variable) != -1)
		return 0;

	//no more sub channels, add this channel
	Channel *child = new Channel(variable, parent);
	if (parent)
		parent->addChild(child);
	(*ms_variables)[ms_nextVariable] = child;
	variableNum = ms_nextVariable++;

	Variable *v = new Variable(variable, memLoc, type);
	(*ms_variableValues)[variableNum] = v;

	if(sendToClients)
	{
		send(NEW_VARIABLE, variable.c_str());
		send(VARIABLE_TYPE, variable.c_str());
		send(VARIABLE_VALUE, variable.c_str());
	}

	setVariableValue(variable.c_str(), memLoc, sendToClients);

	//return the new channelID
	return variableNum;
}

// ----------------------------------------------------------------------

void RemoteDebug::updateVariable(const char* variableName)
{
	send(VARIABLE_VALUE, variableName);
}

// ----------------------------------------------------------------------

/** This function is called via a globally-accessible macro, so we can't assume that
  * the system is installed.
  */
void RemoteDebug::setVariableValue(const char* variableName, void *newValue, bool sendToClients)
{
	if(!ms_installed)
		return;

	int32 variableNum = findVariable(variableName);
	Variable *v = (*ms_variableValues)[static_cast<uint32>(variableNum)];

	if(!v)
	{
		DEBUG_FATAL(true, ("message on undefined variable"));
	}
	v->setValue(newValue);

	if(sendToClients)
		send(VARIABLE_VALUE, variableName);
}

// ----------------------------------------------------------------------

/** This function builds a remoteDebug packet and sends it to the registered sendFunction.
  * For some of the messages, it assumes that the varArgs static buffer has been filled
  * in with the appropriate data (done with translateVarArgs, and taken care of in the
  * macros defined in the header file).
  */
void RemoteDebug::send(MESSAGE_TYPE type, const char* theName)
{
	if (!ms_installed)
	{
		return;
	}
	if (!ms_opened)
	{
		return;
	}

	if(ExitChain::isFataling())
		return;

	const std::string name = theName;

	//build a payload based on messagetype
	char messageType = 0;
	uint32 channelNumber = 0;
	//use signed value to allow a negative value for failure in the search routines
	int32 chan = -1;
//	StreamMap::iterator it = ms_streams->begin();
	//pack in the message type
	messageType = static_cast<char>(type);
	char scratchString[10];
	Variable *v;
	//this variable is used when we want to send 4 bytes of 0's etc, without doing a strlen on the buffer
	int32 explicitMessageLength = 0;
	Variable::VARIABLEVALUE val;
	switch(type)
	{
		case STREAM:
			chan = findStream(name);
			if (chan == -1)
			{
				//add the channel, sending that packet first
				channelNumber = registerStream(name);
				send(NEW_STREAM, name.c_str());
			}
			else
			{
				channelNumber = static_cast<uint32>(chan);
			}
			//bail out if this channel is squelched
			if ((*ms_squelchedStream)[static_cast<uint32>(chan)] == true)
				return;
			break;

		case NEW_STREAM:
			//get the registered int for the new channel
			chan = findStream(name);
			if (chan != -1)
				channelNumber = static_cast<uint32>(chan);
			else
			{
				channelNumber = registerStream(name);
			}
			strcpy(ms_varArgs_buffer, name.c_str());
			break;

		case STREAM_SQUELCH:
		case STREAM_UNSQUELCH:
			chan = findStream(name);
			if (chan != -1)
				channelNumber =  static_cast<uint32>(chan);
			else
			{
				DEBUG_FATAL(true, ("message on unregistered channel"));
			}
			break;

			//end stream channel cases
			/////
			//begin variable cases

		case NEW_VARIABLE:
			//get the registered int for the new variable
			chan = findVariable(name);
			if (chan != -1)
				channelNumber =  static_cast<uint32>(chan);
			else
			{
				DEBUG_FATAL(true, ("message on unregistered channel"));
				channelNumber = 0; //lint !e527 unreachable code
			}
			strcpy(ms_varArgs_buffer, name.c_str());
			break;


		case VARIABLE_TYPE:
			chan = findVariable(name);
			if (chan != -1)
				channelNumber =  static_cast<uint32>(chan);
			v = (*ms_variableValues)[channelNumber];
			_itoa(v->type(), scratchString, 10);
			strcpy(ms_varArgs_buffer, scratchString);
			break;

		case VARIABLE_VALUE:
			chan = findVariable(name);
			if (chan != -1)
				channelNumber =  static_cast<uint32>(chan);
			else
			{
				DEBUG_FATAL(true, ("message on unregistered channel"));
				channelNumber = 0; //lint !e527 unreachable code
			}
			v = (*ms_variableValues)[channelNumber];
			if(!v)
			{
				DEBUG_FATAL(true, ("message on undefined variable"));
				return;  //lint !e527 unreachable code
			}
			val = v->value();
			switch(v->type())
			{
				case INT:
					memcpy(ms_varArgs_buffer, &val.intValue, sizeof(int32));
					explicitMessageLength = sizeof(int32);
					break;

				case FLOAT:
					memcpy(ms_varArgs_buffer, &val.floatValue, sizeof(float));
					explicitMessageLength = sizeof(float);
					break;

				case CSTRING:
					strcpy(ms_varArgs_buffer, val.stringValue);
					explicitMessageLength = strlen(val.stringValue)+1;
					break;

				case BOOL:
					memcpy(ms_varArgs_buffer, &val.boolValue, sizeof(int32));
					explicitMessageLength = sizeof(int32);
					break;
			}
			break;

			//end variable cases
			/////
			//begin static view cases

		case NEW_STATIC:
			//get the registered int for the new channel
			chan = findStaticView(name);
			if (chan != -1)
				channelNumber = static_cast<uint32>(chan);
			else
			{
				DEBUG_FATAL(true, ("message on unregistered channel"));
				channelNumber = 0; //lint !e527 unreachable code
			}
			strcpy(ms_varArgs_buffer, name.c_str());
			break;

		case STATIC_LINE:
			chan = findStaticView(name);
			if (chan == -1)
			{
				//add the channel, sending that packet first
				channelNumber = registerStaticView(name);
				send(NEW_STATIC, name.c_str());
				send(STATIC_SQUELCH, name.c_str());
			}
			else
			{
				channelNumber = static_cast<uint32>(chan);
			}
			//bail out if this channel is squelched
			if ((*ms_squelchedStatic)[static_cast<uint32>(chan)] == true)
				return;
			break;

		case STATIC_SQUELCH:
		case STATIC_UNSQUELCH:
			chan = findStaticView(name);
			if (chan != -1)
				channelNumber = static_cast<uint32>(chan);
			else
			{
				channelNumber = registerStaticView(name);
				send(NEW_STATIC, name.c_str());
			}
			break;

		case STATIC_INPUT_TARGET:
			chan = findStaticView(name);
			if (chan == -1)
				return;
			else
				channelNumber = static_cast<uint32>(chan);
			break;

		case STATIC_BEGIN_FRAME:
		case STATIC_END_FRAME:
			chan = findStaticView(name);
			if (chan != -1)
				channelNumber = static_cast<uint32>(chan);
			else
			{
				channelNumber = registerStaticView(name);
				send(NEW_STATIC, name.c_str());
				send(STATIC_SQUELCH, name.c_str());
			}
			//bail our if this channel is squelched
			if ((*ms_squelchedStatic)[static_cast<uint32>(chan)] == true)
				return;
			break;

		case RemoteDebug::STATIC_UP:
		case RemoteDebug::STATIC_DOWN:
		case RemoteDebug::STATIC_LEFT:
		case RemoteDebug::STATIC_RIGHT:
		case RemoteDebug::STATIC_ENTER:
		case RemoteDebug::REQUEST_ALL_CHANNELS:
		default:
			break;
	}
	//get sizes for building the packet
	int messageTypeLength   = sizeof(messageType);
	int channelNumberLength = sizeof(channelNumber);
	uint32 messageLength    = 0;
	int messageLengthLength = 0;

	if (explicitMessageLength != 0)
		messageLength         = explicitMessageLength;
	else if (ms_varArgs_buffer[0] == '\0')
	{
		//only grab buffer sizes if needed
		messageLength         = strlen(ms_varArgs_buffer)+1;
	}
	else
	{
		messageLength         = strlen(ms_buffer)+1;
	}
	messageLengthLength   = sizeof(messageLength);
	//allocate the mem for the packet

	uint32 packetLength = static_cast<uint32>(messageTypeLength + channelNumberLength + messageLengthLength + static_cast<int>(messageLength));

	if (ms_varArgs_buffer[0] == '\0')
	{
		//copy data into the packet
		memcpy(ms_buffer, &messageType, static_cast<uint32>(messageTypeLength));
		memcpy(ms_buffer + messageTypeLength, &channelNumber, static_cast<uint32>(channelNumberLength));
		//only copy the buffer data if needed
		memcpy(ms_buffer + messageTypeLength + channelNumberLength, &messageLength, static_cast<uint32>(messageLengthLength));
		memcpy(ms_buffer + messageTypeLength + channelNumberLength + messageLengthLength, ms_varArgs_buffer, messageLength);
	}

	//send and clean up
	ms_sendFunction(ms_buffer, packetLength);
	//clear out varArgs buffer
	ms_varArgs_buffer[0] = '\0';
}

// ----------------------------------------------------------------------

int32 RemoteDebug::findStream(const std::string& name)
{
	for(StreamMap::iterator it = ms_streams->begin(); it != ms_streams->end(); ++it)
	{
		if (it->second->name().compare(name) == 0)
		{
			return static_cast<int32>(it->first);
		}
	}
	return -1;
}

// ----------------------------------------------------------------------

int32 RemoteDebug::findStaticView(const std::string& name)
{
	for(StaticViewMap::iterator it = ms_staticViews->begin(); it != ms_staticViews->end(); ++it)
	{
		if (it->second->name().compare(name) == 0)
		{
			return static_cast<int32>(it->first);
		}
	}
	return -1;
}

// ----------------------------------------------------------------------

int32 RemoteDebug::findVariable(const std::string& name)
{
	for(VariableMap::iterator it = ms_variables->begin(); it != ms_variables->end(); ++it)
	{
		if (it->second->name().compare(name) == 0)
		{
			return static_cast<int32>(it->first);
		}
	}
	return -1;
}

// ----------------------------------------------------------------------

/** Format and print a debugging message.
  *
  * This function copies the va_formatted into plain text and stores it into a static buffer
  */
void RemoteDebug::vprintf(const char *format, va_list va)
{
	// format the string
	if (_vsnprintf(ms_varArgs_buffer, sizeof(ms_varArgs_buffer), format, va) < 0)
	{
		// handle overflow reasonably nicely
		ms_varArgs_buffer[sizeof(ms_varArgs_buffer)-2] = '+';
		ms_varArgs_buffer[sizeof(ms_varArgs_buffer)-1] = '\0';
	}
}

// ----------------------------------------------------------------------

/**
  * Format and print a debugging message.
  *
  * This function ends up filling a static buffer with the formatted message
  */
void RemoteDebug::translateVarArgs(const char *format, ...)
{
	va_list va;
	va_start(va, format);
	//copy the data into a static buffer
	vprintf(format, va);
	va_end(va);
}

// ======================================================================

