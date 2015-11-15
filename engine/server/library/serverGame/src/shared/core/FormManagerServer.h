//======================================================================
//
// FormManagerServer.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_FormManagerServer_H
#define INCLUDED_FormManagerServer_H

//======================================================================

#include "sharedGame/FormManager.h"

//======================================================================

class CellProperty;
class DataTable;
class NetworkId;
class Vector;

//----------------------------------------------------------------------

class FormManagerServer : public FormManager
{
public:
	static void install ();
	static void remove  ();
	static void handleCreateObjectData(NetworkId const & actor, std::string const & templateName, Vector const & position, NetworkId const & cell, FormManager::UnpackedFormData const & dataMap);
	static void handleEditObjectData(NetworkId const & actor, NetworkId const & objectToEdit, FormManager::UnpackedFormData const & dataMap);
	static void requestEditObjectDataForClient(NetworkId const & actor, NetworkId const & objectToEdit);
	static void sendEditObjectDataToClient(NetworkId const & actor, NetworkId const & objectToEdit,FormManager::UnpackedFormData const & dataMap);
};

//======================================================================

#endif
