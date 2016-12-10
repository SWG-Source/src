//========================================================================
//
// VehicleObject.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef INCLUDED_VehicleObject_H
#define INCLUDED_VehicleObject_H

#include "serverGame/TangibleObject.h"
class ServerVehicleObjectTemplate;

/**
  *  A VehicleObject represents a vehicle.  Duh.
  */
class VehicleObject : public TangibleObject
{
	friend class VehicleController;

public:
	VehicleObject(const ServerVehicleObjectTemplate* newTemplate);
	virtual ~VehicleObject();

	virtual Controller* createDefaultController (void);
	void                addMembersToPackages    ();
	void                getAttributes           (std::vector<std::pair<std::string, Unicode::String> > &data) const;
	
public:
	int getBogus() const;
//	void setBogus(int newBogus);
	
protected:
    explicit  VehicleObject(const Tag objectType);

	// setter functions that should be called by the object's controller
	void setBogus(int bogus);

public:

private:
	VehicleObject();
	VehicleObject(const VehicleObject& rhs);
	VehicleObject&	operator=(const VehicleObject& rhs);

private:

// BPM VehicleObject : TangibleObject // Begin persisted members.
	Archive::AutoDeltaVariable<int> m_bogus;
// EPM
};

//-----------------------------------------------------------------------

inline int VehicleObject::getBogus() const
{
	return m_bogus.get();
}

//-----------------------------------------------------------------------

inline void VehicleObject::setBogus(int bogus)
{
	m_bogus = bogus;
}

//-----------------------------------------------------------------------

#endif	// INCLUDED_VehicleObject_H
