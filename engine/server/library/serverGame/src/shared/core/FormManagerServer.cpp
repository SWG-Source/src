//======================================================================
//
// FormManagerServer.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "serverGame/FirstServerGame.h"
#include "serverGame/FormManagerServer.h"

#include "sharedFoundation/ConstCharCrcString.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedMath/Vector.h"
#include "sharedNetworkMessages/MessageQueueGenericValueType.h"
#include "sharedObject/Controller.h"
#include "sharedObject/NetworkIdManager.h"
#include "sharedObject/ObjectTemplateList.h"

#include "serverGame/CreatureObject.h"
#include "serverGame/ServerWorld.h"
#include "serverScript/GameScriptObject.h"
#include "serverScript/ScriptFunctionTable.h"
#include "serverScript/ScriptParameters.h"

#include "UnicodeUtils.h"

//======================================================================

namespace FormManagerServerNamespace
{
	bool s_installed = false;
}

using namespace FormManagerServerNamespace;

//----------------------------------------------------------------------

//======================================================================

void FormManagerServer::install ()
{
	FormManager::install();

	DEBUG_FATAL (s_installed, ("installed"));
	if(s_installed)
		return;

	s_installed = true;

	ExitChain::add(remove,"FormManagerServer::remove");
}

//----------------------------------------------------------------------

void FormManagerServer::remove ()
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	if(!s_installed)
		return;

	s_installed = false;
}

//----------------------------------------------------------------------

/**Given a set of data and an object, flag script to create this object and handle the data assignment as appropriate for that type of object
*/
void FormManagerServer::handleCreateObjectData(NetworkId const & actor, std::string const & templateName, Vector const & position, NetworkId const & cellId, FormManager::UnpackedFormData const & dataMap)
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	if(!s_installed)
		return;

	//get matching form
	Form const * const form = FormManagerServer::getFormForServerObjectTemplate(templateName);
	if(!form)
		return;

	//see if we should automatically create the object
	bool const autoCreateObject = getAutomaticallyCreateObject(templateName);

	if(autoCreateObject)
	{
		//create the object
		ConstCharCrcString crcName(ObjectTemplateList::lookUp(templateName.c_str()));
		uint32 const crc = crcName.getCrc();
		if (crc == 0)
		{
			return;
		}
		ServerObject *cell = 0;
		if (cellId.isValid())
		{
			cell = safe_cast<ServerObject *>(NetworkIdManager::getObjectById(cellId));
			if (!cell)
				return;
		}

		Transform tr;
		tr.setPosition_p(position);
		ServerObject * const newObject = ServerWorld::createNewObject(crc, tr, cell, false);
		if (newObject == nullptr)
			return;

		if (cellId == NetworkId::cms_invalid)
			newObject->addToWorld();

		//tell script to "edit" the object, since we've already created it
		handleEditObjectData(actor, newObject->getNetworkId(), dataMap);
	}
	else
	{
		//tell script to create the object in it's own special manner (since the datatable requests that)
		Object * const obj = NetworkIdManager::getObjectById(actor);
		ServerObject * const serverObj = obj ? obj->asServerObject() : nullptr;
		if(serverObj)
		{
			std::vector<char const *> keys;
			std::vector<char const *> values;

			for(FormManager::UnpackedFormData::const_iterator i = dataMap.begin(); i != dataMap.end(); ++i)
			{
				keys.push_back(i->first.c_str());
				values.push_back(i->second[0]/*TODO handle lists*/.c_str());
			}

			ScriptParams params;
			params.addParam(templateName.c_str());
			params.addParam(position.x);
			params.addParam(position.y);
			params.addParam(position.z);
			params.addParam(cellId);
			params.addParam(keys);
			params.addParam(values);

			IGNORE_RETURN(serverObj->getScriptObject()->trigAllScripts(Scripting::TRIG_FORM_CREATEOBJECT, params));
		}
	}
}

//----------------------------------------------------------------------

/**Given a set of data and an object, flag script to edit this object and handle the data assignment as appropriate for that type of object
*/
void FormManagerServer::handleEditObjectData(NetworkId const & actor, NetworkId const & objectToEdit, FormManager::UnpackedFormData const & dataMap)
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	if(!s_installed)
		return;

	Object * const actorObj = NetworkIdManager::getObjectById(actor);
	ServerObject * const actorServerObj = actorObj ? actorObj->asServerObject() : nullptr;
	if(actorServerObj)
	{
		Object * const objectToEditObj = NetworkIdManager::getObjectById(objectToEdit);
		ServerObject * const objectToEditServerObj = objectToEditObj ? objectToEditObj->asServerObject() : nullptr;
		if(objectToEditServerObj)
		{
			//get matching form
			Form const * const form = FormManagerServer::getFormForServerObjectTemplate(objectToEditServerObj->getTemplateName());
			if(!form)
				return;
			
			//set any objvar bindings before telling script
			std::vector<FormManager::Field const *> const & fields = form->getOrderedFieldList();
			for(std::vector<FormManager::Field const *>::const_iterator i = fields.begin(); i != fields.end(); ++i)
			{
				FormManager::Field const * const field = *i;
				if(!field)
					continue;

				std::string const & objvarBinding = field->getObjvarBinding();
				if(!objvarBinding.empty())
				{
					FormManager::UnpackedFormData::const_iterator i2 = dataMap.find(field->getName());
					if(i2 != dataMap.end())
					{
						//TODO handle non-strings
						std::string const & value = i2->second[0];/*TODO handle lists*/
						IGNORE_RETURN(objectToEditServerObj->setObjVarItem(objvarBinding, value));
					}
				}
			}
		
			std::vector<char const *> keys;
			std::vector<char const *> values;
			for(FormManager::UnpackedFormData::const_iterator i2 = dataMap.begin(); i2 != dataMap.end(); ++i2)
			{
				keys.push_back(i2->first.c_str());
				values.push_back(i2->second[0]/*TODO handle lists*/.c_str());
			}
			ScriptParams params;
			params.addParam(objectToEdit);
			params.addParam(keys);
			params.addParam(values);
			IGNORE_RETURN(actorServerObj->getScriptObject()->trigAllScripts(Scripting::TRIG_FORM_EDITOBJECT, params));
		}
	}
}

//----------------------------------------------------------------------

/**Ask script to build and reply with the set of data for the given object.  Script should eventually call JavaLibrary::editFormData.
*/
void FormManagerServer::requestEditObjectDataForClient(NetworkId const & actor, NetworkId const & objectToEdit)
{
	Object * const actorObj = NetworkIdManager::getObjectById(actor);
	ServerObject * const actorServerObj = actorObj ? actorObj->asServerObject() : nullptr;
	if(actorServerObj)
	{
		Object * const objectToEditObj = NetworkIdManager::getObjectById(objectToEdit);
		ServerObject * const objectToEditServerObj = objectToEditObj ? objectToEditObj->asServerObject() : nullptr;
		if(objectToEditServerObj)
		{
			//get matching form
			Form const * const form = FormManagerServer::getFormForServerObjectTemplate(objectToEditServerObj->getTemplateName());
			if(!form)
				return;
			
			std::vector<char const *> keysWeDontHave;
			FormManager::UnpackedFormData dataMap;
			DynamicVariableList const & objvars = objectToEditServerObj->getObjVars();
			std::string value;
			std::vector<std::string> values;
			//build a list of all the fields that DON'T have objvar bindings (since script will need to tell us about them in JavaLibrary::editFormData
			std::vector<FormManager::Field const *> const & fields = form->getOrderedFieldList();
			for(std::vector<FormManager::Field const *>::const_iterator i = fields.begin(); i != fields.end(); ++i)
			{
				FormManager::Field const * const field = *i;
				if(!field)
					continue;

				std::string const & objvarBinding = field->getObjvarBinding();
				if(objvarBinding.empty())
				{
					keysWeDontHave.push_back(field->getName().c_str());
				}
				else
				{
					IGNORE_RETURN(objvars.getItem(objvarBinding, value));
					values.clear();
					if(value.empty())
						value = getEmptyStringPlaceholder();
					values.push_back(value);
					/*TODO handle lists*/
					dataMap[field->getName()] = values;
				}
			}

			//if we can grab all the data, don't ask script for it
			if(keysWeDontHave.empty())
			{
				sendEditObjectDataToClient(actor, objectToEdit, dataMap);	
			}
			else
			{
				//script needs to provide at least some of the data, ask for it
				ScriptParams params;
				params.addParam(objectToEdit);
				params.addParam(keysWeDontHave);
				IGNORE_RETURN(actorServerObj->getScriptObject()->trigAllScripts(Scripting::TRIG_FORM_REQUESTDATA, params));
			}
		}
	}
}

//----------------------------------------------------------------------

/**Given an object and a set of form data from script, send it to the client so they can view/edit it
*/
void FormManagerServer::sendEditObjectDataToClient(NetworkId const & client, NetworkId const & objectToEdit, FormManager::UnpackedFormData const & dataMap)
{
	DEBUG_FATAL (!s_installed, ("not installed"));
	if(!s_installed)
		return;

	Object * const playerObject = NetworkIdManager::getObjectById(client);
	ServerObject * const playerServer = playerObject ? playerObject->asServerObject() : nullptr;
	CreatureObject * const playerCreature = playerServer ? playerServer->asCreatureObject() : nullptr;

	if(playerCreature)
	{
		FormManager::PackedFormData const packedData = FormManager::packFormData(dataMap);

		typedef std::pair<NetworkId, Unicode::String> Payload;
		MessageQueueGenericValueType<Payload> * const msg = new MessageQueueGenericValueType<Payload>(Payload(objectToEdit, packedData));

		Controller * const controller = playerCreature->getController();
		if(controller)
		{
			controller->appendMessage(static_cast<int>(CM_formDataForEdit), 0.0f, msg, GameControllerMessageFlags::SEND | GameControllerMessageFlags::RELIABLE | GameControllerMessageFlags::DEST_AUTH_CLIENT);
		}
		else
		{
			DEBUG_FATAL(true, ("No controller for object %s in FormManagerServer::sendEditObjectData", client.getValueString().c_str()));
		}
	}
	else
	{
		DEBUG_FATAL(true, ("Could not turn object %s into a CreatureObject", client.getValueString().c_str()));
	}
}

//======================================================================
