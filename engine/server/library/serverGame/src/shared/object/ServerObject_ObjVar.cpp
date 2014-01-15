// ======================================================================
//
// ServerObject_ObjVar.cpp
//
// Copyright 2004 Sony Online Entertainment
//
// ======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/ServerObject.h"

#include "UnicodeUtils.h"
#include "sharedFoundation/DynamicVariableLocationData.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"

// ======================================================================

void ServerObject::removeObjVarItem(std::string const &name)
{
	if (isAuthoritative())
	{
		IGNORE_RETURN(m_objVars.removeItemByName(name));
	}
	else
	{
		sendControllerMessageToAuthServer(
			CM_removeObjVarItem,
			new MessageQueueGenericValueType<std::string>(name));
	}
}

// ----------------------------------------------------------------------

void ServerObject::eraseObjVarListEntries(std::string const &name)
{
	if (isAuthoritative())
	{
		IGNORE_RETURN(m_objVars.removeItemByName(name));
	}
	else
	{
		sendControllerMessageToAuthServer(
			CM_eraseObjVarListEntries,
			new MessageQueueGenericValueType<std::string>(name));
	}
}

// ----------------------------------------------------------------------

void ServerObject::copyObjVars(std::string const &name, ServerObject const &srcObject, std::string const &srcVarName)
{
	m_objVars.copy(name, srcObject.getObjVars(), srcVarName);
}

// ----------------------------------------------------------------------

bool ServerObject::setObjVarItem(std::string const &name, int value)
{
	if (isAuthoritative())
	{
		if (!m_objVars.setItem(name, value))
			return false;
	}
	else
	{
		sendControllerMessageToAuthServer(
			CM_setIntDynamicVariable,
			new MessageQueueGenericValueType<std::pair<std::string, int> >(
				std::make_pair(name, value)));
	}
	return true;
}

// ----------------------------------------------------------------------

bool ServerObject::setObjVarItem(std::string const &name, std::vector<int> const &value)
{
	if (isAuthoritative())
	{
		if (!m_objVars.setItem(name, value))
			return false;
	}
	else
	{
		if (DynamicVariable::getUTF8Length(value) > DynamicVariable::MAX_DYNAMIC_VARIABLE_UTF8_LENGTH)
			return false;
		sendControllerMessageToAuthServer(
			CM_setIntArrayDynamicVariable,
			new MessageQueueGenericValueType<std::pair<std::string, std::vector<int> > >(
				std::make_pair(name, value)));
	}
	return true;
}

// ----------------------------------------------------------------------

bool ServerObject::setObjVarItem(std::string const &name, float value)
{
	if (isAuthoritative())
	{
		if (!m_objVars.setItem(name, value))
			return false;
	}
	else
	{
		sendControllerMessageToAuthServer(
			CM_setFloatDynamicVariable,
			new MessageQueueGenericValueType<std::pair<std::string, float> >(
				std::make_pair(name, value)));
	}
	return true;
}

// ----------------------------------------------------------------------

bool ServerObject::setObjVarItem(std::string const &name, std::vector<float> const &value)
{
	if (isAuthoritative())
	{
		if (!m_objVars.setItem(name, value))
			return false;
	}
	else
	{
		if (DynamicVariable::getUTF8Length(value) > DynamicVariable::MAX_DYNAMIC_VARIABLE_UTF8_LENGTH)
			return false;
		sendControllerMessageToAuthServer(
			CM_setFloatArrayDynamicVariable,
			new MessageQueueGenericValueType<std::pair<std::string, std::vector<float> > >(
				std::make_pair(name, value)));
	}
	return true;
}

// ----------------------------------------------------------------------

bool ServerObject::setObjVarItem(std::string const &name, std::string const &value)
{
	if (isAuthoritative())
	{
		if (!m_objVars.setItem(name, value))
			return false;
	}
	else
	{
		if (DynamicVariable::getUTF8Length(value) > DynamicVariable::MAX_DYNAMIC_VARIABLE_UTF8_LENGTH)
			return false;
		sendControllerMessageToAuthServer(
			CM_setStringDynamicVariable,
			new MessageQueueGenericValueType<std::pair<std::string, Unicode::String> >(
				std::make_pair(name, Unicode::narrowToWide(value))));
	}
	return true;
}

// ----------------------------------------------------------------------

bool ServerObject::setObjVarItem(std::string const &name, Unicode::String const &value)
{
	if (isAuthoritative())
	{
		if (!m_objVars.setItem(name, value))
			return false;
	}
	else
	{
		if (DynamicVariable::getUTF8Length(value) > DynamicVariable::MAX_DYNAMIC_VARIABLE_UTF8_LENGTH)
			return false;
		sendControllerMessageToAuthServer(
			CM_setStringDynamicVariable,
			new MessageQueueGenericValueType<std::pair<std::string, Unicode::String> >(
				std::make_pair(name, value)));
	}
	return true;
}

// ----------------------------------------------------------------------

bool ServerObject::setObjVarItem(std::string const &name, std::vector<Unicode::String> const &value)
{
	if (isAuthoritative())
	{
		if (!m_objVars.setItem(name, value))
			return false;
	}
	else
	{
		if (DynamicVariable::getUTF8Length(value) > DynamicVariable::MAX_DYNAMIC_VARIABLE_UTF8_LENGTH)
			return false;
		sendControllerMessageToAuthServer(
			CM_setStringArrayDynamicVariable,
			new MessageQueueGenericValueType<std::pair<std::string, std::vector<Unicode::String> > >(
				std::make_pair(name, value)));
	}
	return true;
}

// ----------------------------------------------------------------------

bool ServerObject::setObjVarItem(std::string const &name, NetworkId const &value)
{
	if (isAuthoritative())
	{
		if (!m_objVars.setItem(name, value))
			return false;
	}
	else
	{
		sendControllerMessageToAuthServer(
			CM_setObjIdDynamicVariable,
			new MessageQueueGenericValueType<std::pair<std::string, NetworkId> >(
				std::make_pair(name, value)));
	}
	return true;
}

// ----------------------------------------------------------------------

bool ServerObject::setObjVarItem(std::string const &name, std::vector<NetworkId> const &value)
{
	if (isAuthoritative())
	{
		if (!m_objVars.setItem(name, value))
			return false;
	}
	else
	{
		if (DynamicVariable::getUTF8Length(value) > DynamicVariable::MAX_DYNAMIC_VARIABLE_UTF8_LENGTH)
			return false;
		sendControllerMessageToAuthServer(
			CM_setObjIdArrayDynamicVariable,
			new MessageQueueGenericValueType<std::pair<std::string, std::vector<NetworkId> > >(
				std::make_pair(name, value)));
	}
	return true;
}

// ----------------------------------------------------------------------

bool ServerObject::setObjVarItem(std::string const &name, DynamicVariableLocationData const &value)
{
	if (isAuthoritative())
	{
		if (!m_objVars.setItem(name, value))
			return false;
	}
	else
	{
		sendControllerMessageToAuthServer(
			CM_setLocationDynamicVariable,
			new MessageQueueGenericValueType<std::pair<std::string, DynamicVariableLocationData> >(
				std::make_pair(name, value)));
	}
	return true;
}

// ----------------------------------------------------------------------

bool ServerObject::setObjVarItem(std::string const &name, std::vector<DynamicVariableLocationData> const &value)
{
	if (isAuthoritative())
	{
		if (!m_objVars.setItem(name, value))
			return false;
	}
	else
	{
		if (DynamicVariable::getUTF8Length(value) > DynamicVariable::MAX_DYNAMIC_VARIABLE_UTF8_LENGTH)
			return false;
		sendControllerMessageToAuthServer(
			CM_setLocationArrayDynamicVariable,
			new MessageQueueGenericValueType<std::pair<std::string, std::vector<DynamicVariableLocationData> > >(
				std::make_pair(name, value)));
	}
	return true;
}

// ----------------------------------------------------------------------

bool ServerObject::setObjVarItem(std::string const &name, StringId const &value)
{
	if (isAuthoritative())
	{
		if (!m_objVars.setItem(name, value))
			return false;
	}
	else
	{
		if (DynamicVariable::getUTF8Length(value) > DynamicVariable::MAX_DYNAMIC_VARIABLE_UTF8_LENGTH)
			return false;
		sendControllerMessageToAuthServer(
			CM_setStringIdDynamicVariable,
			new MessageQueueGenericValueType<std::pair<std::string, StringId> >(
				std::make_pair(name, value)));
	}
	return true;
}

// ----------------------------------------------------------------------

bool ServerObject::setObjVarItem(std::string const &name, std::vector<StringId> const &value)
{
	if (isAuthoritative())
	{
		if (!m_objVars.setItem(name, value))
			return false;
	}
	else
	{
		if (DynamicVariable::getUTF8Length(value) > DynamicVariable::MAX_DYNAMIC_VARIABLE_UTF8_LENGTH)
			return false;
		sendControllerMessageToAuthServer(
			CM_setStringIdArrayDynamicVariable,
			new MessageQueueGenericValueType<std::pair<std::string, std::vector<StringId> > >(
				std::make_pair(name, value)));
	}
	return true;
}

// ----------------------------------------------------------------------

bool ServerObject::setObjVarItem(std::string const &name, Transform const &value)
{
	if (isAuthoritative())
	{
		if (!m_objVars.setItem(name, value))
			return false;
	}
	else
	{
		if (DynamicVariable::getUTF8Length(value) > DynamicVariable::MAX_DYNAMIC_VARIABLE_UTF8_LENGTH)
			return false;
		sendControllerMessageToAuthServer(
			CM_setTransformDynamicVariable,
			new MessageQueueGenericValueType<std::pair<std::string, Transform> >(
				std::make_pair(name, value)));
	}
	return true;
}

// ----------------------------------------------------------------------

bool ServerObject::setObjVarItem(std::string const &name, std::vector<Transform> const &value)
{
	if (isAuthoritative())
	{
		if (!m_objVars.setItem(name, value))
			return false;
	}
	else
	{
		if (DynamicVariable::getUTF8Length(value) > DynamicVariable::MAX_DYNAMIC_VARIABLE_UTF8_LENGTH)
			return false;
		sendControllerMessageToAuthServer(
			CM_setTransformArrayDynamicVariable,
			new MessageQueueGenericValueType<std::pair<std::string, std::vector<Transform> > >(
				std::make_pair(name, value)));
	}
	return true;
}

// ----------------------------------------------------------------------

bool ServerObject::setObjVarItem(std::string const &name, Vector const &value)
{
	if (isAuthoritative())
	{
		if (!m_objVars.setItem(name, value))
			return false;
	}
	else
	{
		if (DynamicVariable::getUTF8Length(value) > DynamicVariable::MAX_DYNAMIC_VARIABLE_UTF8_LENGTH)
			return false;
		sendControllerMessageToAuthServer(
			CM_setVectorDynamicVariable,
			new MessageQueueGenericValueType<std::pair<std::string, Vector> >(
				std::make_pair(name, value)));
	}
	return true;
}

// ----------------------------------------------------------------------

bool ServerObject::setObjVarItem(std::string const &name, std::vector<Vector> const &value)
{
	if (isAuthoritative())
	{
		if (!m_objVars.setItem(name, value))
			return false;
	}
	else
	{
		if (DynamicVariable::getUTF8Length(value) > DynamicVariable::MAX_DYNAMIC_VARIABLE_UTF8_LENGTH)
			return false;
		sendControllerMessageToAuthServer(
			CM_setVectorArrayDynamicVariable,
			new MessageQueueGenericValueType<std::pair<std::string, std::vector<Vector> > >(
				std::make_pair(name, value)));
	}
	return true;
}

// ----------------------------------------------------------------------

bool ServerObject::setObjVarItem(std::string const &name, DynamicVariable const &value)
{
	FATAL(!isAuthoritative(), ("setObjVarItem using a DynamicVariable value can only be used on auth objects"));
	if (!m_objVars.setItem(name, value))
		return false;
	return true;
}

// ======================================================================

