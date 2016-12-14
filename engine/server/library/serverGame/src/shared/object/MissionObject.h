// MissionObject.h
// Copyright 2000-01, Sony Online Entertainment Inc., all rights reserved. 
// Author: Justin Randall

#ifndef	_INCLUDED_MissionObject_H
#define	_INCLUDED_MissionObject_H

//-----------------------------------------------------------------------

#include "serverGame/CreatureObject.h"
#include "serverGame/IntangibleObject.h"
#include "sharedUtility/Location.h"
#include "sharedGame/Waypoint.h"

class Location;
class ServerMissionObjectTemplate;
class Token;
class MessageQueueMissionCreateRequestData;
class MessageQueueMissionDetailsResponse;

namespace MessageDispatch
{
	class Callback;
}

//-----------------------------------------------------------------------

class MissionObject : public IntangibleObject
{
public:
    virtual ~MissionObject  ();

	static MissionObject *         createMissionObjectInCreatureMissionBag(CreatureObject * creatureWithMissionBag);
	virtual MissionObject *        asMissionObject();
	virtual MissionObject const *  asMissionObject() const;

	static void             removeDefaultTemplate(void);

	void                    abortMission        ();
	virtual bool            canTrade() const;
	virtual bool            onContainerAboutToLoseItem(ServerObject * destination, ServerObject& item, ServerObject* transferer);
	virtual bool            onContainerAboutToTransfer(ServerObject * destination, ServerObject* transferer);
	virtual int             onContainerAboutToGainItem(ServerObject& item, ServerObject* transferer);

	void                    endMission          (const bool succeeded);
	const NetworkId &       getMissionHolderId  () const;
	virtual void            getAttributes       (std::vector<std::pair<std::string, Unicode::String> > &data) const;


/*mission data*/
	const int                getDifficulty             () const;
	const StringId &         getDescription            () const;
	const Location &         getEndLocation            () const;
	const Unicode::String &  getMissionCreator         () const;
	const unsigned int       getMissionType            () const;
	const std::string &      getMissionTypeString      () const;
	const int                getReward                 () const;
	const std::string &      getRootScriptName         () const;
	const Location &         getStartLocation          () const;
	const unsigned int       getTargetAppearance       () const;
	const std::string &      getTargetName             () const;
	const StringId &         getTitle                  () const;
	const int                getStatus                 () const;
	const Waypoint &         getWaypoint               () const;

	void                     grantMissionTo            (const NetworkId & target);
	virtual void             onAddedToWorld            ();

	void  setDescription          (const StringId & description);
	void  setDifficulty           (const int difficulty);
	void  setEndLocation          (const Location & endLocation);
	void  setMissionCreator       (const Unicode::String & creator);
	void  setMissionType          (const unsigned char);
	void  setMissionType          (const std::string &);
	void  setReward               (const int reward);
	void  setRootScriptName       (const std::string & rootScriptName);
	void  setStatus               (int newStatus);
	void  setStartLocation        (const Location & startLocation);
	void  setTargetAppearance     (const std::string & appearance);
	void  setTargetAppearance     (unsigned int appearanceNameCrc);
	void  setTargetName           (const std::string & targetName);
	void  setTitle                (const StringId & title);
	void  setMissionHolderId      (const NetworkId & h);
	void  setWaypoint             (const Waypoint & w);
/* end mission data*/

	virtual void getByteStreamFromAutoVariable(const std::string & name, Archive::ByteStream & target) const;
	virtual void setAutoVariableFromByteStream(const std::string & name, const Archive::ByteStream & source);

protected:
	virtual void            endBaselines        ();
	virtual void            initializeFirstTimeObject();

	virtual const SharedObjectTemplate *  getDefaultSharedTemplate(void) const;
	friend class ServerMissionObjectTemplate;
	MissionObject  (const ServerMissionObjectTemplate * missionObjectTemplate);

private:
	MissionObject & operator = (const MissionObject & rhs);
	MissionObject(const MissionObject & source);
	static const ServerMissionObjectTemplate * getBogusMissionObjectTemplate();
	void notifyOwnerMissionAccepted();
	void notifyOwnerMissionAborted();
	void notifyOwnerMissionFailed();
	void notifyOwnerMissionSucceeded();
	void onWaypointLoad(Waypoint::ChangeNotification &);

private:

	static const SharedObjectTemplate * m_defaultSharedTemplate;	// template to use if no shared template is given
	MessageDispatch::Callback *                  m_callback;
	Archive::AutoDeltaVariable<StringId>         m_description;
	Archive::AutoDeltaVariable<int>              m_difficulty;
	Archive::AutoDeltaVariable<Location>         m_endLocation;
	Archive::AutoDeltaVariable<Unicode::String>  m_missionCreator;
	Archive::AutoDeltaVariable<unsigned int>     m_missionType;
	Archive::AutoDeltaVariable<int>              m_reward;
	Archive::AutoDeltaVariable<std::string>      m_rootScriptName;
	Archive::AutoDeltaVariable<Location>         m_startLocation;
	Archive::AutoDeltaVariable<unsigned int>     m_targetAppearance;
	Archive::AutoDeltaVariable<StringId>         m_title;
	Archive::AutoDeltaVariable<NetworkId>        m_missionHolderId;
	Archive::AutoDeltaVariable<int>              m_status;
	Archive::AutoDeltaVariable<std::string>      m_targetName;
	Archive::AutoDeltaVariable<Waypoint>         m_waypoint;

private:
	void addMembersToPackages();
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_MissionObject_H

