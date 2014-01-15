// ======================================================================
//
// ConsoleCommandParserObject.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ConsoleCommandParserObject_H
#define INCLUDED_ConsoleCommandParserObject_H

#include "sharedCommandParser/CommandParser.h"
#include "sharedMessageDispatch/Emitter.h"

class NetworkId;
class ServerObject;
class Transform;
class Vector;

// ======================================================================

/**
* Commands that are related to scripts
*/

class ConsoleCommandParserObject : 
public CommandParser, 
public MessageDispatch::Emitter
{
public:
    ConsoleCommandParserObject ();
    virtual bool performParsing (const NetworkId & userId, 
								 const StringVector_t & argv,
                                 const String_t & originalCommand,
                                 String_t & result,
                                 const CommandParser * node);
	
private:
    ConsoleCommandParserObject (const ConsoleCommandParserObject & rhs);
    ConsoleCommandParserObject &  operator= (const ConsoleCommandParserObject & rhs);

	bool performParsing2 (const NetworkId & userId, 
		const StringVector_t & argv,
		const String_t & originalCommand,
		String_t & result,
		const CommandParser * node);

    bool moveObject            (const NetworkId & oid, const Vector & position_w, const NetworkId & targetContainer, const Vector & position_p) const;
    bool rotateObject            (const NetworkId & oid, real yaw, real pitch, real roll) const;
    
};

// ======================================================================

#endif	// INCLUDED_ConsoleCommandParserObject_H












