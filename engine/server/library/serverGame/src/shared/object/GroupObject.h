//========================================================================
//
// GroupObject.h
//
// Copyright 2002 Sony Online Entertainment
//
//========================================================================

#ifndef INCLUDED_GroupObject_H
#define INCLUDED_GroupObject_H

// ======================================================================

#include "serverGame/UniverseObject.h"
#include "Archive/AutoDeltaVariableObserver.h"
#include "Archive/AutoDeltaVectorObserver.h"
#include "serverGame/GroupUpdateObserver.h"
#include "sharedMathArchive/VectorArchive.h"

// ======================================================================

class GroupMemberParam;
class ServerGroupObjectTemplate;

// ======================================================================

class GroupObject: public UniverseObject
{
public:
	typedef std::vector<GroupMemberParam> GroupMemberParamVector;

	GroupObject(ServerGroupObjectTemplate const *newTemplate);
	virtual ~GroupObject();

	virtual GroupObject *asGroupObject();
	virtual GroupObject const *asGroupObject() const;
	virtual Controller * createDefaultController ();
	virtual void         setupUniverse           ();
	virtual bool         isVisibleOnClient       (const Client &client) const;
	virtual void         getAttributes           (AttributeVector &data) const;

	static void          removeDefaultTemplate   ();
	static void          createGroup             (GroupMemberParam const & leader, GroupMemberParamVector const & members);
	static void          onChatRoomCreate        (std::string const &path);
	static void          onVoiceChatRoomCreate   (std::string const &channelName);
	static void          addToLeaderMap          (NetworkId const &leaderId, NetworkId const &groupId);
	static void          removeFromLeaderMap     (NetworkId const &leaderId, NetworkId const &groupId);
	static NetworkId     getGroupIdForLeader     (NetworkId const &leaderId);
	static void          createAllGroupChatRooms ();
	static int           maximumMembersInGroup();

	typedef std::pair<NetworkId, std::string> GroupMember;
	typedef std::vector<GroupMember> GroupMemberVector;
	typedef std::pair<NetworkId /*ship*/, int /*formation slot*/> GroupShipFormationMember;
	typedef std::pair<NetworkId /*POB ship*/, NetworkId /*owner of the POB*/> GroupPOBShipAndOwner;

	std::string const &       getGroupName       () const;
	void                      getGroupMemberIds  (std::vector<NetworkId> &members) const;
	NetworkId const &         getGroupLeaderId   () const;
	std::string const &       getGroupLeaderName () const;
	bool                      isGroupFull        () const;
	int                       getPCMemberCount   () const;
	bool isMemberPC(NetworkId const & memberId) const;
	bool doesGroupHaveRoomFor(int additionalMembers) const;
	GroupMemberVector const & getGroupMembers    () const;
	int                       getGroupLevel () const;
	uint32                    getFormationNameCrc() const;

	void                 setGroupName       (std::string const &groupName);
	void                 makeLeader         (NetworkId const &newLeaderId);
	void                 addGroupMember     (GroupMemberParam const & member);
	GroupMemberParam const createGroupMemberParamForMember(NetworkId const &memberId) const;
	void                 removeGroupMember  (NetworkId const &memberId);
	bool                 isGroupMember      (NetworkId const &memberId) const;
	void                 disbandGroup       ();
	void                 sendGroupChat      (NetworkId const &memberId, Unicode::String const &text) const;
	void                 setMemberLevel(NetworkId const &memberId, int level);
	void                 setMemberProfession(NetworkId const &memberId, uint8 profession);
	void                 onGroupMemberConnect(NetworkId const &memberId);
	void                 onGroupMemberRemoved(NetworkId const &memberId, bool disbanding);
	void                 setFormationNameCrc (uint32 formationNameCrc);

	void setShipForMember(NetworkId const & memberId, NetworkId const & shipId);
	void setShipFormationSlotForMember(NetworkId const & memberId, int const formationSlot);
	void addPOBShipAndOwner(NetworkId const & shipId, NetworkId const & ownerId);
	void removePOBShip(NetworkId const & shipId);

	NetworkId const & getPOBShipOwnedByMember(NetworkId const & memberId) const;
	NetworkId const & getPOBShipMemberIsContainedIn(NetworkId const & memberId) const;
	NetworkId const & getMemberShip(NetworkId const & memberId) const;
	bool getShipIsPOB(NetworkId const & shipId) const;

	std::string const & getMemberName(NetworkId const & memberId) const;
	int getMemberLevel(NetworkId const & memberId) const;
	int getMemberLevelByMemberIndex(unsigned int memberIndex) const;
	uint8 getMemberProfession(NetworkId const & memberId) const;
	uint8 getMemberProfessionByMemberIndex(unsigned int memberIndex) const;

	NetworkId const & getLootMasterId() const;
	void makeLootMaster(NetworkId const & newLootMasterId);
	int getLootRule() const;
	void setLootRule(int const rule);

	std::pair<int32, int32> const & getGroupPickupTimer() const;
	unsigned int getSecondsLeftOnGroupPickup() const;
	void setGroupPickupTimer(time_t startTime, time_t endTime);

	std::pair<std::string, Vector> const & getGroupPickupLocation() const;
	void setGroupPickupLocation(std::string const & planetName, Vector const & location);

	void reenterGroupChatRoom(CreatureObject const &who) const;

protected:
	virtual SharedObjectTemplate const *getDefaultSharedTemplate() const;

private:
	GroupObject();
	GroupObject(GroupObject const &rhs);
	GroupObject& operator=(GroupObject const &rhs);

	void addMembersToPackages();
	std::string getChatRoomPath() const;
	std::string getVoiceChatDisplayName() const;
	std::string getChatName(std::string const &memberName) const;
	void createGroupChatRoom() const;
	void destroyGroupChatRoom() const;
	void addToGroupChatRoom(std::string const &memberName) const;
	void addToGroupVoiceChatRoom(NetworkId const & memberId, std::string const &memberName) const;
	void removeFromGroupChatRoom(std::string const &memberName) const;
	void removeFromGroupVoiceChatRoom(NetworkId const & memberId, std::string const &memberName) const;
	void onGroupChatRoomCreated();
	void onGroupVoiceChatRoomCreated();
	bool chatRoomCreated() const;
	void setChatRoomCreated();
	void calcGroupLevel();

private:

	static const SharedObjectTemplate * m_defaultSharedTemplate;	// template to use if no shared template is given

	Archive::AutoDeltaVariable<std::string> m_groupName;
	Archive::AutoDeltaVectorObserver<GroupMember, GroupUpdateObserver, GroupObject> m_groupMembers;
	Archive::AutoDeltaVectorObserver<GroupShipFormationMember, GroupUpdateObserver, GroupObject> m_groupShipFormationMembers;
	Archive::AutoDeltaVectorObserver<GroupPOBShipAndOwner, GroupUpdateObserver, GroupObject> m_groupPOBShipAndOwners;
	Archive::AutoDeltaVariable<int16> m_groupLevel;
	Archive::AutoDeltaVector<int16> m_groupMemberLevels;
	Archive::AutoDeltaVector<uint8> m_groupMemberProfessions;
	Archive::AutoDeltaVariable<uint32> m_formationNameCrc;
	Archive::AutoDeltaSet<NetworkId> m_allMembers;
	Archive::AutoDeltaSet<NetworkId> m_nonPCMembers;
	Archive::AutoDeltaVariableObserver<NetworkId, GroupUpdateObserver, GroupObject> m_lootMaster;
	Archive::AutoDeltaVariable<uint32> m_lootRule;
	Archive::AutoDeltaVariable<std::pair<int32, int32> > m_groupPickupTimer; // <start time_t, end time_t> (may not be 0, but if the end time is < the current time, it means no group pickup is currently active)>
	Archive::AutoDeltaVariable<std::pair<std::string, Vector> > m_groupPickupLocation; // planet/zone name, and (x,y,z) world location
};

// ======================================================================

inline std::string const &GroupObject::getGroupName() const
{
	return m_groupName.get();
}

// ----------------------------------------------------------------------

inline GroupObject::GroupMemberVector const &GroupObject::getGroupMembers() const
{
	return m_groupMembers.get();
}

//----------------------------------------------------------------------

inline int GroupObject::getGroupLevel () const
{
	return m_groupLevel.get ();
}

//----------------------------------------------------------------------

inline uint32 GroupObject::getFormationNameCrc () const
{
	return m_formationNameCrc.get();
}

// ----------------------------------------------------------------------

inline bool GroupObject::isGroupMember(NetworkId const &memberId) const
{
	return m_allMembers.contains(memberId);
}

// ----------------------------------------------------------------------

inline std::pair<int32, int32> const & GroupObject::getGroupPickupTimer() const
{
	return m_groupPickupTimer.get();
}

// ----------------------------------------------------------------------

inline std::pair<std::string, Vector> const & GroupObject::getGroupPickupLocation() const
{
	return m_groupPickupLocation.get();
}

// ======================================================================

#endif
