//========================================================================
//
// InstallationObject.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_InstallationObject_H
#define _INCLUDED_InstallationObject_H

#include "serverGame/TangibleObject.h"
#include "sharedFoundation/NetworkIdArchive.h"

class MessageQueueResourceEmptyHopper;
class ServerInstallationObjectTemplate;

/** 
  *  An InstallationObject typically represents a large un-enterable structure that performs the duties of an installation
  *  as described by the installation documentation.
  */

class InstallationObject : public TangibleObject
{
	friend class InstallationController;
	
public:

	InstallationObject(const ServerInstallationObjectTemplate* newTemplate);
	virtual ~InstallationObject();

	static void         removeDefaultTemplate(void);

	virtual Controller* createDefaultController(void);
	void                addMembersToPackages();
	virtual float       alter(float time);
	virtual void        getAttributes(std::vector<std::pair<std::string, Unicode::String> > &data) const;

	virtual void getByteStreamFromAutoVariable(const std::string & name, Archive::ByteStream & target) const;
	virtual void setAutoVariableFromByteStream(const std::string & name, const Archive::ByteStream & source);

	//simulation functions:
	virtual void activate         (const NetworkId &actorId);
	virtual void finishActivate   (float installedEfficiency, float tickCount);
	virtual void deactivate       ();
	virtual void harvest          ();
	virtual void handleCMessageTo (const MessageToPayload &message);

	//getters
	bool         isActive() const;
	float        getTickCount() const;

	// power functions
	float        getPower(void) const;
	void         setPower(float power);
	void         changePower(float delta);
	float        getPowerRate(void) const;
	void         setPowerRate(float rate);
	float        getTimePowered(void) const;
	float        getOutOfPowerTime(void) const;

	struct InstallationObjectPointerHash
	{
		size_t operator()(const InstallationObject * const ptr) const
		{
			return (reinterpret_cast<const size_t>(ptr) >> 4);
		};
	};

	typedef std::unordered_set<const InstallationObject *, InstallationObjectPointerHash> AllInstallationsSet;
	static const AllInstallationsSet & getAllInstallations();

protected:
	
	void         setTickCount(float count);
	void         setActivateStartTime(float time);

	virtual const SharedObjectTemplate *  getDefaultSharedTemplate(void) const;

private:
  	InstallationObject();
	InstallationObject(const InstallationObject& rhs);
	InstallationObject&	operator=(const InstallationObject& rhs);

private:

	static const SharedObjectTemplate * m_defaultSharedTemplate;	// template to use if no shared template is given

//BPM InstallationObject : TangibleObject // Begin persisted members.
	Archive::AutoDeltaVariable<int32>  m_installationType;	///< The type of installation (accessible by game scripts).
	Archive::AutoDeltaVariable<float>  m_tickCount;         ///< For active installations, the tick count the last time they harvested resources
	Archive::AutoDeltaVariable<bool>   m_activated;
	Archive::AutoDeltaVariable<float>  m_power;             // Amount of power the installation has
	Archive::AutoDeltaVariable<float>  m_powerRate;         // Power used, in units/hour
	Archive::AutoDeltaVariable<float>  m_activateStartTime; // Time when the installation was turned on, from ServerClock::getGameTimeSeconds()
//EPM
	
/// Members yet to be implemented.
//  permissionsList
};


inline bool InstallationObject::isActive() const
{
	return m_activated.get();
}

inline float InstallationObject::getTickCount() const
{
	return m_tickCount.get();
}

inline void InstallationObject::setTickCount(float count)
{
	if (isAuthoritative())
		m_tickCount = count;
}

inline void InstallationObject::setPower(float power)
{
	changePower(power - getPower());
}

inline float InstallationObject::getPowerRate(void) const
{
	return m_powerRate.get();
}


#endif
