//===================================================================
//
// SharedObjectTemplateClientData.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_SharedObjectTemplateClientData_H
#define INCLUDED_SharedObjectTemplateClientData_H

//===================================================================

#include "sharedFoundation/DataResource.h"

//===================================================================

class SharedObjectTemplateClientData : public DataResource
{
public:

	explicit SharedObjectTemplateClientData (const char* filename);
	virtual ~SharedObjectTemplateClientData () = 0;

private:

	SharedObjectTemplateClientData ();
	SharedObjectTemplateClientData (const SharedObjectTemplateClientData&);
	SharedObjectTemplateClientData& operator= (const SharedObjectTemplateClientData&);
};

//===================================================================

#endif
