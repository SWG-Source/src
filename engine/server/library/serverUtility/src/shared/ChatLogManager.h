// ============================================================================
// 
// ChatLogManager.h
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#ifndef INCLUDED_ChatLogManager_H
#define INCLUDED_ChatLogManager_H

class NetworkId;

//-----------------------------------------------------------------------------
class ChatLogManager
{
public:

	static void install();
	static void remove();

	static void logChat(Unicode::String const &fromPlayer, Unicode::String const &toPlayer, Unicode::String const &message, Unicode::String const &channel, int const messageIndex, time_t const time);
	static bool getChatMessage(int const messageIndex, Unicode::String &fromPlayer, Unicode::String &toPlayer, Unicode::String &text, Unicode::String &channel, time_t &time);
	static bool getChatMessage(int const messageIndex, Unicode::String &chatMessage);
	static int getNextMessageIndex();
	static void buildLogEntry(Unicode::String &result, Unicode::String const &fromPlayer, Unicode::String const &toPlayer, Unicode::String const &message, Unicode::String const &channel, time_t const time);
	static bool isPlayerInLogs(Unicode::String const &player);
	static void getReportHeader(Unicode::String & header, std::string const &reportingPlayer, NetworkId const &reportingPlayerNetworkId, std::string const &reportingPlayerStationName, uint32 reportingPlayerStationId, std::string const &harassingPlayer, NetworkId const &harassingPlayerNetworkId, std::string const &harassingPlayerStationName, uint32 harassingPlayerStationId);
	
private:

	// Disable

	ChatLogManager();
	~ChatLogManager();
	ChatLogManager(ChatLogManager const &);
	ChatLogManager &operator =(ChatLogManager const &);
};

// ============================================================================

#endif // INCLUDED_ChatLogManager_H
