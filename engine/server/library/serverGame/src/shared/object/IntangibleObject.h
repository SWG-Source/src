//========================================================================
//
// IntangibleObject.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef INCLUDED_IntangibleObject_H
#define INCLUDED_IntangibleObject_H

//----------------------------------------------------------------------

#include "serverGame/ServerObject.h"

class ServerIntangibleObjectTemplate;

//----------------------------------------------------------------------

/**
  * A IntangibleObject represents things that don't have a physical form in 
  * the game.
  */
class IntangibleObject : public ServerObject
{
public:

	// NOTE: If you change these values, change the corresponding ones in base_class.java!
	enum TheaterLocationType
	{
		TLT_none,
		TLT_getGoodLocation,
		TLT_flatten
	};

public:
	explicit IntangibleObject(const ServerIntangibleObjectTemplate* newTemplate);
	virtual ~IntangibleObject();

	virtual IntangibleObject *asIntangibleObject();
	virtual IntangibleObject const *asIntangibleObject() const;

	static void  removeDefaultTemplate (void);

	virtual float alter(float time);
	virtual void  onPermanentlyDestroyed();
	virtual void  persist();
	virtual bool  isVisibleOnClient (const Client & client) const;
	virtual void  getAttributes     (std::vector<std::pair<std::string, Unicode::String> > &data) const;
	virtual bool  onContainerAboutToTransfer(ServerObject * destination, ServerObject* transferer);
	
	int  getCount(void) const;
	void setCount(int value);
	void incrementCount(int delta);

	// Theater stuff
	static IntangibleObject * spawnTheater(const std::string & datatable, const Vector & position, const std::string & script, TheaterLocationType locationType);
	static int   getObjectsCreatedPerFrame();
	static int   getNumObjects(const std::string & datatable);
	static float getRadius(const std::string & datatable);
	       void  setTheater();
	       bool  isTheater() const;
		   void  setTheaterCreator(const NetworkId & creator);
	       bool  setTheaterName(const std::string & name);
	       void  setObjects(const std::vector<int32> & crcs, const std::vector<Vector> & positions, const std::vector<float> & headings, const std::vector<std::string> & scripts);
		   void  setPlayer(const CreatureObject & player);

protected:

	virtual void                          updatePlanetServerInternal(bool forceUpdate) const;
	virtual const SharedObjectTemplate *  getDefaultSharedTemplate(void) const;
	virtual void                          onLoadedFromDatabase();
	virtual void                          sendObjectSpecificBaselinesToClient(Client const &client) const;

private:
	IntangibleObject();
	IntangibleObject(const IntangibleObject& rhs);
	IntangibleObject&	operator=(const IntangibleObject& rhs);

private:
	void addMembersToPackages();

private:
	static const SharedObjectTemplate * m_defaultSharedTemplate;	// template to use if no shared template is given

	Archive::AutoDeltaVariable<int> m_count;         // generic counter

	// Theater stuff
	Archive::AutoDeltaVariable<bool>            m_isTheater;  // flag that this is a theater
	Archive::AutoDeltaVector<int32>             m_crcs;       // template crcs
	Archive::AutoDeltaVector<Vector>            m_positions;  // object positions (relative to us)
	Archive::AutoDeltaVector<float>             m_headings;   // object orientation
	Archive::AutoDeltaVector<std::string>       m_scripts;    // scripts to be attached to the created objects
	Archive::AutoDeltaVariable<CachedNetworkId> m_player;     // the player the theater is created for
	Archive::AutoDeltaVector<CachedNetworkId>   m_objects;    // objects created
	Archive::AutoDeltaVariable<Vector>          m_center;     // center of the theater (not neccessarily our position)
	Archive::AutoDeltaVariable<float>           m_radius;     // radius of the theater
	Archive::AutoDeltaVariable<NetworkId>       m_creator;    // id of who created the theater
	Archive::AutoDeltaVariable<std::string>     m_theaterName;// the name of the theater
	static uint32                               ms_lastFrame;
	static uint32                               ms_theaterTime;
#ifdef _DEBUG
	unsigned long m_theaterCreationTime;
#endif
};

//----------------------------------------------------------------------

inline int IntangibleObject::getCount(void) const
{
	return m_count.get();
}

inline void IntangibleObject::setCount(int value)
{
	m_count = value;
}

inline void IntangibleObject::incrementCount(int delta)
{
	m_count = m_count.get() + delta;
}

inline void IntangibleObject::setTheater()
{
	m_isTheater = true;
}

inline bool IntangibleObject::isTheater() const
{
	return m_isTheater.get();
}

inline void IntangibleObject::setTheaterCreator(const NetworkId & creator)
{
	m_creator = creator;
}

//----------------------------------------------------------------------


#endif	// INCLUDED_IntangibleObject_H
