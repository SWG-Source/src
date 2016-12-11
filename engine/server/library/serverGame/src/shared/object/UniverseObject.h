// ======================================================================
//
// UniverseObject.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_UniverseObject_H
#define INCLUDED_UniverseObject_H

#include "serverGame/ServerObject.h"

class Region;
class ServerUniverseObjectTemplate;

// ======================================================================

/**
 * A UniverseObject is an object that is global to the entire server
 * cluster.  UniverseObjects represent global data (such as Resource
 * Classes) or objects with no definite location (such as Resource Pools).
 */
class UniverseObject : public ServerObject
{
public:
	         UniverseObject(const ServerUniverseObjectTemplate* newTemplate);
	virtual ~UniverseObject();

	static void         removeDefaultTemplate          (void);

	virtual Controller* createDefaultController (void); //???
	void                addMembersToPackages    ();

	virtual void setupUniverse();
	virtual void onServerUniverseGainedAuthority();

	virtual bool isVisibleOnClient (const Client & client) const;
	virtual void getAttributes     (std::vector<std::pair<std::string, Unicode::String> > &data) const;

	bool           isDynamicRegion() const;
	const Region * getDynamicRegion() const;

protected:
	virtual void initializeFirstTimeObject();
	virtual void endBaselines();

virtual void updatePlanetServerInternal(bool) const;
	virtual const SharedObjectTemplate *  getDefaultSharedTemplate(void) const;
	
private:
	  UniverseObject(const UniverseObject&);
	  UniverseObject & operator=(const UniverseObject&);

private:
	static const SharedObjectTemplate * m_defaultSharedTemplate;	// template to use if no shared template is given
};

// ======================================================================

#endif
