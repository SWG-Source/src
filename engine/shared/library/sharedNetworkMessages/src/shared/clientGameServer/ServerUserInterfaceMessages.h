
// ======================================================================
//
// ServerUserInferfaceMessages.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ServerUserInferfaceMessages_H
#define INCLUDED_ServerUserInferfaceMessages_H

//-----------------------------------------------------------------------

#include "Archive/Archive.h"
#include "Unicode.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "unicodeArchive/UnicodeArchive.h"

//-----------------------------------------------------------------------

class GameInByteStream;
class GameOutByteStream;

//-----------------------------------------------------------------------

/**
 * This is the base class for all ServerUserInterface messages.  It is mainly used
 * to store the indentifier that this client uses to refer to a given ServerUserInterface
 * dialog.
 */
class SUIMessage : public GameNetworkMessage
{
public:
	           SUIMessage (const char* message);

public:
	void                    setClientPageId(int pageId);
	int                     getClientPageId() const;

private:
	//the identifier for this client to use for the page
	Archive::AutoVariable<int> m_clientPageId;
};

//-----------------------------------------------------------------------

struct SuiCreatePage_Command
{
	enum Type
	{
		CLEAR_DATA_SOURCE,
		ADD_CHILD_WIDGET,
		SET_PROPERTY,
		ADD_DATA_ITEM,
		SUBSCRIBE_TO_PROPERTY
	} m_type;
	std::vector<Unicode::String> m_parameters;
};

//-----------------------------------------------------------------------

/**
 * This network message is used to communicate down to the client the information about
 * an new dialog to display.  It contains the base UI page to show, as well as a series
 * of commands to run that can add other widgets, and set the state of the dialog box.
 *  This message is meant to be sent from Server to Client.
 */
class SuiCreatePage : public SUIMessage
{
public:

	static const char * const MESSAGE_TYPE;

	/**
	 *  This struct describes an action to take while filling out the given dialog box.  It
	 *  could be a new widget description, an item for a data source, a property to set, etc.
	 */
	typedef SuiCreatePage_Command Command;
	typedef std::vector<Command> CommandVector;

	           SuiCreatePage ();
	explicit   SuiCreatePage (Archive::ReadIterator & source);
	virtual   ~SuiCreatePage ();

	void                    setPageName            (const std::string& name);
	const std::string&      getPageName            () const;
	int                     getNumberOfCommands    () const;
	const Command &         getCommand             (const unsigned int i) const;
	const CommandVector &   getCommands            () const;
	void                    addCommand             (const SuiCreatePage_Command::Type type, const std::vector<Unicode::String>& parameters);
	void                    setAssociatedObjectId  (const NetworkId & id);
	void                    setMaxRangeFromObject  (float range);

public:
	//the name of the UI page to use
	Archive::AutoVariable<std::string>  m_pageName;
	//the list of commands to apply to the page
	Archive::AutoArray<Command>         m_commands;
	Archive::AutoVariable<NetworkId>    m_associatedObjectId;
	Archive::AutoVariable<float>        m_maxRangeFromObject;

private:
	SuiCreatePage(const SuiCreatePage&);
	SuiCreatePage& operator= (const SuiCreatePage&);
};

//-----------------------------------------------------------------------

/**  This message is meant to be sent from Client to Server.
 */
class SuiClosedOk : public SUIMessage
{
public:

	static const char * const MESSAGE_TYPE;

	           SuiClosedOk ();
	explicit   SuiClosedOk (Archive::ReadIterator & source);
	virtual   ~SuiClosedOk ();

	void       addSubscribedProperty(const std::string& widgetName, const std::string& propertyName, const Unicode::String& propertyValue);
	std::vector<std::pair<std::pair<std::string, std::string>, Unicode::String> > getSubscribedProperties() const;

	void            setButtonPressed(const Unicode::String& button);
	Unicode::String getButtonPressed() const;

private:
	//data in the subscribed properties
	Archive::AutoArray<std::pair<std::pair<std::string, std::string>, Unicode::String> > m_subscribedProperties;
	Archive::AutoVariable<Unicode::String> m_buttonPressed;

private:
	SuiClosedOk(const SuiClosedOk&);
	SuiClosedOk& operator= (const SuiClosedOk&);
};

//-----------------------------------------------------------------------

/**  This message is meant to be sent from Server to Client.
 */
class SuiForceClosePage : public SUIMessage
{
public:

	static const char * const MESSAGE_TYPE;

	           SuiForceClosePage ();
	explicit   SuiForceClosePage (Archive::ReadIterator & source);
	virtual   ~SuiForceClosePage ();

private:
	SuiForceClosePage(const SuiForceClosePage&);
	SuiForceClosePage& operator= (const SuiForceClosePage&);
};

//-----------------------------------------------------------------------

/**  This message is meant to be sent from Server to Client.
 */
class SuiSetProperty : public SUIMessage
{
public:

	static const char * const MESSAGE_TYPE;

	           SuiSetProperty ();
	explicit   SuiSetProperty (Archive::ReadIterator & source);
	virtual   ~SuiSetProperty ();

	void       setWidgetName               (const std::string&);
	void       setPropertyName             (const std::string&);
	void       setPropertyValue            (const Unicode::String&);
	const std::string&     getWidgetName   ();
	const std::string&     getPropertyName ();
	const Unicode::String& getPropertyValue();

private:
	SuiSetProperty(const SuiSetProperty&);
	SuiSetProperty& operator= (const SuiSetProperty&);

	Archive::AutoVariable<std::string>     m_widgetName;
	Archive::AutoVariable<std::string>     m_propertyName;
	Archive::AutoVariable<Unicode::String> m_propertyValue;
};

//-----------------------------------------------------------------

//tell Archive how to pack/unpack my custom types
namespace Archive
{
	//a SuiCreatePage_Command::Type is an enum, so treat it like an int
	inline void put(ByteStream& target, const SuiCreatePage_Command::Type& source)
	{
		put(target, static_cast<int>(source));
	}

	inline void get(ReadIterator& source, SuiCreatePage_Command::Type& target)
	{
		signed int tmp;
		get(source, tmp);
		target = static_cast<SuiCreatePage_Command::Type>(tmp);
	}

	//SuiCreatePage_Command is a struct, so just pack/unpack each piece
	inline void put(ByteStream& target, const SuiCreatePage_Command& source)
	{
		put(target, source.m_type);
		put(target, source.m_parameters);
	}

	inline void get(ReadIterator& source, SuiCreatePage_Command& target)
	{
		get(source, target.m_type);
		get(source, target.m_parameters);
	}
}
//-----------------------------------------------------------------

#endif
