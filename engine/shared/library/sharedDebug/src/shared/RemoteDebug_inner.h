// ======================================================================
//
// RemoteDebug_inner.h
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef REMOTE_DEBUG_INNER_H
#define REMOTE_DEBUG_INNER_H

#include "sharedDebug/RemoteDebug.h"

// ======================================================================


	/** This inner class is used to maintain relationships between channels.  It stores both its parent
	  * and child nodes, as well as it's fully qualified name (i.e. "Foundation\\MemoryManager\\MemoryMap")
	  */
class RemoteDebug::Channel
{
	public:
		Channel(const std::string& name, Channel *parent);
		~Channel();
		void addChild(Channel *child);
		const std::string& name();
	private:
		typedef std::list<Channel *> NodeList;
		///its children
		NodeList* m_children;
		///the fully qualified name of the channel
		std::string* m_name;
		///its parent (which is nullptr for top-level nodes
		Channel *m_parent;
};

/** This class stores data about a variable channel, both on the server (where the variable actually
  * resides), and the client (which just displays it).  The client simply creates Variable's with 
  * nullptr'd out memLoc's, since it doesn't have direct access to the memory.
  */
class RemoteDebug::Variable
{
	public:
		union VARIABLEVALUE
		{
			int32 intValue;
			float floatValue;
			int32 boolValue;
			char* stringValue;
		};
		
		Variable(const std::string& name, void *memLoc, VARIABLE_TYPES type);
		~Variable();
		void setValue(VARIABLEVALUE v);
		void setValue(void *memLoc);
		VARIABLEVALUE value();
		const std::string& name();
		void setType(VARIABLE_TYPES type);
		VARIABLE_TYPES type();
		void pushValue();
	private:
		///a data structure used to hold the current variable data
		VARIABLEVALUE m_value;
		void *m_memLoc;
		std::string* m_name;
		VARIABLE_TYPES m_type;
};

// ======================================================================


/** This derived class represents a client application that uses the RemoteDebug module.
  * It receives messages and can do things like send messages to the app to clear the screen.
  * It can also send squelch and unsquelch messages.  This module would be used by a text-based,
  * MFC, or Qt app for instance.
  */
class RemoteDebugClient : public RemoteDebug
{
protected:
	typedef void (*NewStreamFunction)(uint32 streamNumber, const char *streamName);
	typedef void (*StreamMessageFunction)(uint32 streamNumber, const char *message);
	typedef void (*NewVariableFunction)(uint32 variableNumber, const char *variableName);
	typedef void (*VariableTypeFunction)(uint32 variableNumber, VARIABLE_TYPES type);
	typedef void (*VariableValueFunction)(uint32 variableNumber, const char *message);
	typedef void (*BeginFrameFunction)(uint32 staticViewNumber);
	typedef void (*EndFrameFunction)(uint32 staticViewNumber);
	typedef void (*NewStaticViewFunction)(uint32 variableNumber, const char *variableName);
	typedef void (*LineFunction)(uint32 staticViewNumber, const char *message);

	static NewStreamFunction       ms_newStreamFunction;
	static StreamMessageFunction   ms_streamMessageFunction;
	static NewVariableFunction     ms_newVariableFunction;
	static VariableValueFunction   ms_variableValueFunction;
	static VariableTypeFunction    ms_variableTypeFunction;
	static BeginFrameFunction      ms_beginFrameFunction;
	static EndFrameFunction        ms_endFrameFunction;
	static NewStaticViewFunction   ms_newStaticViewFunction;
	static LineFunction            ms_lineFunction;

public:
	///used to initialize the client application
	static void install(RemoveFunction rf, OpenFunction of, CloseFunction cf, SendFunction sf, 
	                    IsReadyFunction irf, NewStreamFunction ncf, StreamMessageFunction cmf, 
	                    NewVariableFunction nvf , VariableValueFunction vvf, VariableTypeFunction vtf, 
	                    BeginFrameFunction bff, EndFrameFunction eff, NewStaticViewFunction nsvf,
	                    LineFunction lf);

	///uninstall the module
	static void remove();

	static void close();

	///receives and translates a packet, takes appropriate action
	static void receive(const unsigned char * const buffer, const uint32 bufferLen);

	///called when actually ready to transfer data (commonly called from an open ack)
	static void isReady();
};

// ======================================================================

/** This derived class represents a server application that uses the RemoteDebug module.
  * It sends messages and can do things like receive squelch and unsquelch messages.
  * This module would be used by applications like gameserver, loginservers, and the game client.
  */
class RemoteDebugServer : public RemoteDebug
{
public:

	///install the module, using the client defined functions
	static void install(RemoveFunction rmf, OpenFunction of, 
	                    CloseFunction cf, SendFunction sf, 
	                    IsReadyFunction irf);

	///uninstall the module
	static void remove();

	///receives and translates a packet, takes appropriate action
	static void receive(const unsigned char * const message, const uint32 bufferLen);

	static void close();

	///called when actually ready to transfer data (commonly called from an open ack)
	static void isReady();

protected:
	///send all the channels to the client
	static void sendAllChannels();

	///stores the channel number of the static view that currently receives keyboard input
	static uint32 ms_inputTarget;
};

// ======================================================================

#endif // REMOTE_DEBUG_INNER_H
