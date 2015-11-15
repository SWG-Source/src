// ======================================================================
//
// SuiForceClosePage.cpp
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

//-----------------------------------------------------------------------

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/ServerUserInterfaceMessages.h"

#include "UnicodeUtils.h"

//-----------------------------------------------------------------------

const char * const SuiCreatePage::MESSAGE_TYPE     = "SuiCreatePage";
const char * const SuiClosedOk::MESSAGE_TYPE       = "SuiClosedOk";
const char * const SuiForceClosePage::MESSAGE_TYPE = "SuiForceClosePage";
const char * const SuiSetProperty::MESSAGE_TYPE    = "SuiSetProperty";

//======================================================================

SUIMessage::SUIMessage(const char * message)
: GameNetworkMessage(message),
  m_clientPageId(-1)
{
	addVariable(m_clientPageId);
}

//-----------------------------------------------------------------------

void SUIMessage::setClientPageId(int p)
{
	m_clientPageId.set(p);
}

//-----------------------------------------------------------------------

int SUIMessage::getClientPageId() const
{
	return m_clientPageId.get();
}

//======================================================================

SuiCreatePage::SuiCreatePage()
: SUIMessage (MESSAGE_TYPE),
  m_pageName (),
  m_commands (),
  m_associatedObjectId (),
  m_maxRangeFromObject (32.0f)
{
	addVariable (m_pageName);
	addVariable (m_commands);
	addVariable (m_associatedObjectId);
	addVariable (m_maxRangeFromObject);
}

//----------------------------------------------------------------------

SuiCreatePage::SuiCreatePage(Archive::ReadIterator & source)
: SUIMessage (MESSAGE_TYPE),
  m_pageName (),
  m_commands (),
  m_associatedObjectId (),
  m_maxRangeFromObject (32.0f)
{
	addVariable (m_pageName);
	addVariable (m_commands);
	addVariable (m_associatedObjectId);
	addVariable (m_maxRangeFromObject);
	unpack (source);
}

//----------------------------------------------------------------------

void SuiCreatePage::setAssociatedObjectId  (const NetworkId & id)
{
	m_associatedObjectId.set (id);
}

//----------------------------------------------------------------------

void SuiCreatePage::setMaxRangeFromObject (float range)
{
	m_maxRangeFromObject.set (range);
}

//----------------------------------------------------------------------

SuiCreatePage::~SuiCreatePage()
{
}

//----------------------------------------------------------------------

void SuiCreatePage::setPageName(const std::string& name)
{
	m_pageName.set(name);
}

//----------------------------------------------------------------------

const std::string& SuiCreatePage::getPageName() const
{
	return m_pageName.get();
}
	
//----------------------------------------------------------------------

int SuiCreatePage::getNumberOfCommands() const
{
	return m_commands.get().size();
}

//----------------------------------------------------------------------

const SuiCreatePage::Command & SuiCreatePage::getCommand(const unsigned int i) const
{
	DEBUG_FATAL(i < m_commands.get().size(), ("bad index into SuiCreatePage::Command"));
	return m_commands.get()[i];
}

//----------------------------------------------------------------------

const SuiCreatePage::CommandVector & SuiCreatePage::getCommands() const
{
	return m_commands.get();
}

//----------------------------------------------------------------------

void SuiCreatePage::addCommand(const SuiCreatePage_Command::Type type, const std::vector<Unicode::String>& parameters) 
{
	Command c;
	c.m_type = type;
	c.m_parameters = parameters;
	m_commands.get().push_back(c);
}

//======================================================================

SuiClosedOk::SuiClosedOk()
: SUIMessage(MESSAGE_TYPE),
  m_subscribedProperties(),
  m_buttonPressed()
{
	addVariable (m_buttonPressed);
	addVariable (m_subscribedProperties);
}

//----------------------------------------------------------------------

SuiClosedOk::SuiClosedOk(Archive::ReadIterator & source)
: SUIMessage(MESSAGE_TYPE),
  m_subscribedProperties(),
  m_buttonPressed()
{
	addVariable (m_buttonPressed);
	addVariable (m_subscribedProperties);
	unpack (source);
}

//----------------------------------------------------------------------

SuiClosedOk::~SuiClosedOk()
{
}

//----------------------------------------------------------------------

void SuiClosedOk::addSubscribedProperty(const std::string& widgetName, const std::string& propertyName, const Unicode::String& propertyValue)
{
	m_subscribedProperties.get().push_back(std::make_pair(std::make_pair(widgetName, propertyName), propertyValue));
}

//----------------------------------------------------------------------

std::vector<std::pair<std::pair<std::string, std::string>, Unicode::String> > SuiClosedOk::getSubscribedProperties() const
{
	return m_subscribedProperties.get();
}

//----------------------------------------------------------------------

void SuiClosedOk::setButtonPressed(const Unicode::String& button)
{
	m_buttonPressed.set(button);
}

//----------------------------------------------------------------------

Unicode::String SuiClosedOk::getButtonPressed() const
{
	return m_buttonPressed.get();
}

//======================================================================

SuiForceClosePage::SuiForceClosePage()
: SUIMessage(MESSAGE_TYPE)
{
}

//----------------------------------------------------------------------

SuiForceClosePage::SuiForceClosePage(Archive::ReadIterator & source)
: SUIMessage(MESSAGE_TYPE)
{
	unpack (source);
}

//----------------------------------------------------------------------

SuiForceClosePage::~SuiForceClosePage()
{
}

//======================================================================

SuiSetProperty::SuiSetProperty()
: SUIMessage     (MESSAGE_TYPE),
	m_widgetName   (),
	m_propertyName (),
	m_propertyValue()
{
	addVariable (m_widgetName);
	addVariable (m_propertyName);
	addVariable (m_propertyValue);
}

//----------------------------------------------------------------------

SuiSetProperty::SuiSetProperty(Archive::ReadIterator & source)
: SUIMessage     (MESSAGE_TYPE),
	m_widgetName   (),
	m_propertyName (),
	m_propertyValue()
{
	addVariable (m_widgetName);
	addVariable (m_propertyName);
	addVariable (m_propertyValue);
	unpack (source);
}

//----------------------------------------------------------------------

SuiSetProperty::~SuiSetProperty()
{
}

//----------------------------------------------------------------------

void SuiSetProperty::setWidgetName(const std::string& widgetName)
{
	m_widgetName.set(widgetName);
}

//----------------------------------------------------------------------

void SuiSetProperty::setPropertyName(const std::string& propertyName)
{
	m_propertyName.set(propertyName);
}

//----------------------------------------------------------------------

void SuiSetProperty::setPropertyValue(const Unicode::String& propertyValue)
{
	m_propertyValue.set(propertyValue);
}

//-----------------------------------------------------------------------

const std::string& SuiSetProperty::getWidgetName()
{
	return m_widgetName.get();
}

//-----------------------------------------------------------------------

const std::string& SuiSetProperty::getPropertyName()
{
	return m_propertyName.get();
}

//-----------------------------------------------------------------------

const Unicode::String& SuiSetProperty::getPropertyValue()
{
	return m_propertyValue.get();
}

//-----------------------------------------------------------------------
