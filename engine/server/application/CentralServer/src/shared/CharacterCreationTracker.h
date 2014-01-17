// ======================================================================
//
// CharacterCreationTracker.h
// copyright (c) 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_CharacterCreationTracker_H
#define INCLUDED_CharacterCreationTracker_H

// ======================================================================

class ConnectionCreateCharacter;
class LoginCreateCharacterMessage;

// ======================================================================

class CharacterCreationTracker
{
public:
	static CharacterCreationTracker &getInstance();
	static void install();
	static void remove();
	CharacterCreationTracker();
	~CharacterCreationTracker();

public:
	void handleCreateNewCharacter             (const ConnectionCreateCharacter &msg);
	void handleDatabaseCreateCharacterSuccess (StationId account, const Unicode::String &characterName, const NetworkId &characterObjectId, int templateId, bool jedi);
	void handleLoginCreateCharacterAck        (StationId account);
	void handleGameCreateCharacterFailed      (StationId account, Unicode::String const &characterName, StringId const &errorMessage, std::string const &optionalDetailedErrorMessage);
	void retryGameServerCreates               ();
	void retryLoginServerCreates              ();
	void onGameServerDisconnect               (uint32 serverId);
	void onLoginServerDisconnect              (uint32 loginServerId);
	void setFastCreationLock                  (StationId account);

private:

	struct CreationRecord
	{
		enum Stage {S_queuedForGameServer, S_sentToGameServer, S_queuedForLoginServer, S_sentToLoginServer};

		Stage                         m_stage;
		CentralCreateCharacter *      m_gameCreationRequest;
		LoginCreateCharacterMessage * m_loginCreationRequest;
		uint32                        m_creationTime;
		uint32                        m_gameServerId;
		uint32                        m_loginServerId;
		NetworkId                     m_characterId;

		CreationRecord();
		~CreationRecord();

		private:
		CreationRecord (const CreationRecord&); //disable
		CreationRecord &operator=(const CreationRecord&); //disable
	};

	typedef std::map<StationId, CreationRecord*> CreationsType;
	CreationsType m_creations;

	typedef std::map<StationId, uint32> FastCreationLockType;
	FastCreationLockType m_fastCreationLock;

private:
	void unlockAccount           (StationId account);
	const bool getStartLocation  (const std::string & name, std::string & planetName, Vector & coordinates, NetworkId & cellId) const;

private:
	static CharacterCreationTracker *ms_instance;
};

// ======================================================================

inline CharacterCreationTracker &CharacterCreationTracker::getInstance()
{
	NOT_NULL(ms_instance);
	return *ms_instance;
}

// ======================================================================

#endif
