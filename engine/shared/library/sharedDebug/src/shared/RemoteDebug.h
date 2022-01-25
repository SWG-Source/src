// ======================================================================
//
// RemoteDebug.h
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef REMOTE_DEBUG_H
#define REMOTE_DEBUG_H

#include <string>

// ======================================================================

/** This class is both a platform and network-protocol independent way of 
  * sending output and input to a remote application. It does not depend
  * on any specific network layer, and output may even be sent to a file or
  * other output.  Since it cannot rely on a network layer, it must build 
  * its own packets.  Below is the packet structure:    
  *
  * 1 byte for message type (current six types, so values 0-5.  
  * 4 bytes for the channel or variable number (since ints are registered 
  *    and used for the network.  
  * 4 bytes for the messagelength (if necessary, not all messages have a 
  *    payload to send).  
  * messageLength bytes for the payload (if necessary, could be a channel 
  *    message or a variable value).
  */
class RemoteDebug
{
protected:
	//forward declare inner classes (defined in RemoteDebug_inner.h)
	class Channel;
	class Variable;

public:
	///an enum of all the possible message types
	enum MESSAGE_TYPE
	{
		STREAM,
		NEW_STREAM,
		STREAM_SQUELCH,
		STREAM_UNSQUELCH,
		NEW_VARIABLE,
		VARIABLE_TYPE,
		VARIABLE_VALUE,
		NEW_STATIC,
		STATIC_SQUELCH,
		STATIC_UNSQUELCH,
		STATIC_BEGIN_FRAME,
		STATIC_END_FRAME,
		STATIC_LINE,
		STATIC_INPUT_TARGET,
		STATIC_UP,
		STATIC_DOWN,
		STATIC_LEFT,
		STATIC_RIGHT,
		STATIC_ENTER,
		REQUEST_ALL_CHANNELS
	};

	///an enum of all the possible variable types
	enum VARIABLE_TYPES
	{
		INT,      //32 bit signed int
		FLOAT,    //32 bit signed float
		CSTRING,  //nullptr terminated char[]
		BOOL      //0 or 1 (could use an int, but useful for tools)
	};

protected:
	//define typdefs of function types
	typedef void (*RemoveFunction)();
	typedef void (*OpenFunction)(const char *server, uint16 port);
	typedef void (*CloseFunction)();
	typedef void (*SendFunction)(void *buffer, uint32 bufferLen);
	typedef void (*IsReadyFunction)();

	typedef void (*UpFunction)();
	typedef UpFunction DownFunction;
	typedef UpFunction LeftFunction;
	typedef UpFunction RightFunction;
	typedef UpFunction EnterFunction;

//end inner classes, enums, and typedefs needed for public interface
// ----------------------------------------------------------------------
//begin public interface

public:
	virtual ~RemoteDebug () = 0;

public:
	///install the module, using the client defined functions
	static void install(RemoveFunction, OpenFunction, CloseFunction, SendFunction, IsReadyFunction);

	///open a session
	static void open(const char *server = nullptr, uint16 port = 0);

	///packs the data into a packet, and sends it using the client-defined SendFunction
	static void send(MESSAGE_TYPE type, const char* name = "");

	///emulate printf functionality specific to this class
	static void translateVarArgs(const char *format, ...);

	///register a variable with the system
	static uint32 registerVariable(const char* variableName, void *memLoc, VARIABLE_TYPES type, bool sendToClients);

	///send the variable value to the clients
	static void updateVariable(const char* variableName);

	///set the value of a variable (previously registered, so we know how to cast the void*)
	static void setVariableValue(const char* variableName, void *newValue, bool sendToClients);

	///register a function that the up key should call on the given static view
	static bool registerUpFunction(const char* staticViewName, UpFunction);

	///register a function that the down key should call on the given static view
	static bool registerDownFunction(const char* staticViewName, DownFunction);

	///register a function that the left key should call on the given static view
	static bool registerLeftFunction(const char* staticViewName, LeftFunction);

	///register a function that the right key should call on the given static view
	static bool registerRightFunction(const char* staticViewName, RightFunction);

	///register a function that the enter key should call on the given static view
	static bool registerEnterFunction(const char* staticViewName, EnterFunction);

//end public interface
// ----------------------------------------------------------------------
//begin protected functions and data members

protected:
	///can't use a const int because we want this value compiled into static arrays as their size
	#define MAX_BUFFER_SIZE 8192

	static void remove();

	///store the stream in a map, use an int for network communication
	static uint32 registerStream(const std::string& streamName);

	///store the static view in a map, use an int for network communication
	static uint32 registerStaticView(const std::string& staticViewName);

	///find and return the stream number for a given stream name, -1 if not found
	static int32 findStream(const std::string& name);

	///find and return the static view number for a given static view name, -1 if not found
	static int32 findStaticView(const std::string& name);

	///find and return the variable number for a given variable name, -1 if not found
	static int32 findVariable(const std::string& name);

	///emulate printf functionality specific to this class
	static void vprintf(const char *format, va_list va);

	//bound functions used  for the actual transfer of data (i.e. network calls)
	static RemoveFunction          ms_removeFunction;
	static OpenFunction            ms_openFunction;
	static CloseFunction           ms_closeFunction;
	static SendFunction            ms_sendFunction;
	static IsReadyFunction         ms_isReadyFunction;

	typedef std::map<uint32, Channel *> StreamMap;
	///stream names
	static  StreamMap                *ms_streams;

	typedef std::map<uint32, Channel *> VariableMap;
	///What variables are registered
	static  VariableMap              *ms_variables;

	typedef std::map<uint32, Channel *> StaticViewMap;
	///StaticView names
	static  StaticViewMap            *ms_staticViews;

	///a static packet buffer to prevent per-message new and delete
	static char                      ms_buffer[MAX_BUFFER_SIZE];

	///a static buffer used to build a string from the translateVarArgs data
	static char                      ms_varArgs_buffer[MAX_BUFFER_SIZE];

	///true if system has been installed, false otherwise
	static bool                      ms_installed;

	///Whether the connection is open or not 
	static bool                      ms_opened;

	///What streaming channels are squelched (i.e. don't send to client at all)
	static std::map<uint32, bool>      *ms_squelchedStream;

	///What static channels are squelched (i.e. don't send to client at all)
	static std::map<uint32, bool>      *ms_squelchedStatic;

	///the next stream number
	static uint32                    ms_nextStream;

	///the next variable number
	static uint32                    ms_nextVariable;

	///the next staticView number
	static uint32                    ms_nextStaticView;

	typedef std::map<uint32, Variable *> VariableValueMap;
	static VariableValueMap            *ms_variableValues;

	typedef std::map<uint32, UpFunction> MovementFunctionMap;
	static MovementFunctionMap         *ms_upFunctionMap;
	static MovementFunctionMap         *ms_downFunctionMap;
	static MovementFunctionMap         *ms_leftFunctionMap;
	static MovementFunctionMap         *ms_rightFunctionMap;
	static MovementFunctionMap         *ms_enterFunctionMap;
};

// ======================================================================

#define OUTPUT_CHANNEL(streamName, varArgs)                          (RemoteDebug::translateVarArgs varArgs, RemoteDebug::send(RemoteDebug::STREAM, streamName))
#define OUTPUT_REGISTER_VARIABLE(variableName, memLoc, type)          RemoteDebug::registerVariable(variableName, memLoc, type, true)
#define OUTPUT_UPDATE_VARIABLE(variableName)                          RemoteDebug::updateVariable(variableName)
#define OUTPUT_STATIC_VIEW(staticViewName, varArgs)                  (RemoteDebug::translateVarArgs varArgs, RemoteDebug::send(RemoteDebug::STATIC_LINE, staticViewName))
#define OUTPUT_STATIC_VIEW_BEGINFRAME(staticViewName)                 RemoteDebug::send(RemoteDebug::STATIC_BEGIN_FRAME, staticViewName)
#define OUTPUT_STATIC_VIEW_ENDFRAME(staticViewName)                   RemoteDebug::send(RemoteDebug::STATIC_END_FRAME, staticViewName)
#define OUTPUT_STATIC_VIEW_UP(staticViewName, upFunction)             RemoteDebug::registerUpFunction(staticViewName, upFunction)
#define OUTPUT_STATIC_VIEW_DOWN(staticViewName, downFunction)         RemoteDebug::registerDownFunction(staticViewName, downFunction)
#define OUTPUT_STATIC_VIEW_LEFT(staticViewName, leftFunction)         RemoteDebug::registerLeftFunction(staticViewName, leftFunction)
#define OUTPUT_STATIC_VIEW_RIGHT(staticViewName, rightFunction)       RemoteDebug::registerRightFunction(staticViewName, rightFunction)
#define OUTPUT_STATIC_VIEW_ENTER(staticViewName, enterFunction)       RemoteDebug::registerEnterFunction(staticViewName, enterFunction)

#ifdef _DEBUG
#define DEBUG_OUTPUT_CHANNEL(streamName, varArgs)                     OUTPUT_CHANNEL(streamName, varArgs)
#define DEBUG_OUTPUT_REGISTER_VARIABLE(variableName, memLoc, type)    OUTPUT_REGISTER_VARIABLE(variableName, memLoc, type)
#define DEBUG_OUTPUT_UPDATE_VARIABLE(variableName)                    OUTPUT_UPDATE_VARIABLE(variableName)
#define DEBUG_OUTPUT_STATIC_VIEW(staticViewName, varArgs)             OUTPUT_STATIC_VIEW(staticViewName, varArgs)
#define DEBUG_OUTPUT_STATIC_VIEW_BEGINFRAME(staticViewName)           OUTPUT_STATIC_VIEW_BEGINFRAME(staticViewName)
#define DEBUG_OUTPUT_STATIC_VIEW_ENDFRAME(staticViewName)             OUTPUT_STATIC_VIEW_ENDFRAME(staticViewName)
#define DEBUG_OUTPUT_STATIC_VIEW_UP(staticViewName, upFunction)       OUTPUT_STATIC_VIEW_UP(staticViewName, upFunction)
#define DEBUG_OUTPUT_STATIC_VIEW_DOWN(staticViewName, downFunction)   OUTPUT_STATIC_VIEW_DOWN(staticViewName, downFunction)
#define DEBUG_OUTPUT_STATIC_VIEW_LEFT(staticViewName, leftFunction)   OUTPUT_STATIC_VIEW_LEFT(staticViewName, leftFunction)
#define DEBUG_OUTPUT_STATIC_VIEW_RIGHT(staticViewName, rightFunction) OUTPUT_STATIC_VIEW_RIGHT(staticViewName, rightFunction)
#define DEBUG_OUTPUT_STATIC_VIEW_ENTER(staticViewName, enterFunction) OUTPUT_STATIC_VIEW_ENTER(staticViewName, enterFunction)

#else  //!_DEBUG
#define DEBUG_OUTPUT_CHANNEL(streamName, varArgs)                     NOP
#define DEBUG_OUTPUT_REGISTER_VARIABLE(variableName, memLoc, type)    NOP
#define DEBUG_OUTPUT_VARIABLE(variableName, newValue)                 NOP
#define DEBUG_OUTPUT_STATIC_VIEW(staticViewName, varArgs)             NOP
#define DEBUG_OUTPUT_STATIC_VIEW_BEGINFRAME(staticViewName)           NOP
#define DEBUG_OUTPUT_STATIC_VIEW_ENDFRAME(staticViewName)             NOP
#define DEBUG_OUTPUT_STATIC_VIEW_UP(staticViewName, upFunction)       NOP
#define DEBUG_OUTPUT_STATIC_VIEW_DOWN(staticViewName, downFunction)   NOP
#define DEBUG_OUTPUT_STATIC_VIEW_LEFT(staticViewName, leftFunction)   NOP
#define DEBUG_OUTPUT_STATIC_VIEW_RIGHT(staticViewName, rightFunction) NOP
#define DEBUG_OUTPUT_STATIC_VIEW_ENTER(staticViewName, enterFunction) NOP
#endif // _DEBUG


#endif // REMOTE_DEBUG_H
