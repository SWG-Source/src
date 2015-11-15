// ============================================================================
// 
// ReportManager.h
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#ifndef INCLUDED_ReportManager_H
#define INCLUDED_ReportManager_H

struct ChatAvatarId;
class ChatOnRequestLog;
class NetworkId;

//-----------------------------------------------------------------------------
class ReportManager
{
public:

	static bool isThrottled(NetworkId const &reportingNetworkId);
	static int getThrottleTimeRemaining(NetworkId const &reportingNetworkId);

	static void addReport(Unicode::String const &reportingName, NetworkId const &reportingNetworkId, Unicode::String const &harassingName, NetworkId const &harassingNetworkId, std::string const &reportLogChannel, Unicode::String const & clientSentData = Unicode::String());
	static void handleMessage(ChatOnRequestLog const &chatOnRequestLog);

private:

	// Disable

	ReportManager();
	~ReportManager();
	ReportManager(ReportManager const &);
	ReportManager &operator =(ReportManager const &);
};

// ============================================================================

#endif // INCLUDED_ReportManager_H
