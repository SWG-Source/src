// ============================================================================
// 
// ChatLogManager.cpp
// Copyright Sony Online Entertainment, Inc.
//
// ============================================================================

#include "serverUtility/FirstServerUtility.h"
#include "serverUtility/ChatLogManager.h"

#include "serverUtility/ConfigServerUtility.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/FormattedString.h"
#include "sharedFoundation/Os.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/ChatAvatarId.h"
#include "sharedNetworkMessages/MessageQueueSpatialChat.h"
#include "UnicodeUtils.h"

#include <list>
#include <map>

// ============================================================================
//
// ChatLogManagerNamespace
//
// ============================================================================

namespace ChatLogManagerNamespace
{
	class ChatLogEntry
	{
	public:

		ChatLogEntry(Unicode::String const &fromPlayer, Unicode::String const &toPlayer, Unicode::String const *text, Unicode::String const &channel, time_t const time)
		 : m_fromPlayer(Unicode::toLower(fromPlayer))
		 , m_toPlayer(Unicode::toLower(toPlayer))
		 , m_text(text)
		 , m_channel(channel)
		 , m_time(time)
		{
		}

		Unicode::String const &getFromPlayer() const
		{
			return m_fromPlayer;
		}

		Unicode::String const &getToPlayer() const
		{
			return m_toPlayer;
		}

		Unicode::String const &getText() const
		{
			return *m_text;
		}

		Unicode::String const &getChannel() const
		{
			return m_channel;
		}

		time_t getTime() const
		{
			return m_time;
		}

	private:

		Unicode::String m_fromPlayer;
		Unicode::String m_toPlayer;
		Unicode::String const *m_text;
		Unicode::String m_channel;
		time_t m_time;

		// Disabled

		ChatLogEntry();
	};

	typedef std::list<std::pair<time_t, int> > TimeLog;
	typedef std::map<int, ChatLogEntry> ChatLog;
	typedef std::map<Unicode::String, int> PlayerLog;
	typedef std::map<Unicode::String const *, int, LessPointerComparator> MessageList; // Second parameter is the reference count

	TimeLog s_timeLog;
	ChatLog s_chatLog;
	PlayerLog s_playerLog;
	MessageList s_chatMessageList;
	time_t s_purgeTime = 0;
	int s_currentIndex = 0;
	time_t s_chatLogMemoryTimer = 0;
	int s_cacheHits = 0;
	int s_cacheMisses = 0;
	
	void printPlayerLog();
	void addChatLogEntry(Unicode::String const &fromPlayer, Unicode::String const &toPlayer, Unicode::String const &message, Unicode::String const &channel, int const messageIndex, time_t const time);
	bool removeChatLogEntry(int const messageIndex);
	void purgeChatLog();
	std::string getTimeString(time_t const time);
	std::string getTimeStringShort(time_t const time);
};

using namespace ChatLogManagerNamespace;

//-----------------------------------------------------------------------------
void ChatLogManagerNamespace::printPlayerLog()
{
	LOGC(ConfigServerUtility::isChatLogManagerLoggingEnabled(), "ChatLogManager", ("-- ChatLogManagerNamespace::printPlayerLog() BEGIN -------------\n"));
	PlayerLog::const_iterator iterPlayerLog = s_playerLog.begin();

	for (; iterPlayerLog != s_playerLog.end(); ++iterPlayerLog)
	{
		LOGC(ConfigServerUtility::isChatLogManagerLoggingEnabled(), "ChatLogManager", ("player(%s) messages(%d)\n", Unicode::wideToNarrow(iterPlayerLog->first).c_str(), iterPlayerLog->second));
	}

	LOGC(ConfigServerUtility::isChatLogManagerLoggingEnabled(), "ChatLogManager", ("-- ChatLogManagerNamespace::printPlayerLog() END -------------\n"));
}

//-----------------------------------------------------------------------------
void ChatLogManagerNamespace::addChatLogEntry(Unicode::String const &fromPlayer, Unicode::String const &toPlayer, Unicode::String const &message, Unicode::String const &channel, int const messageIndex, time_t const time)
{
	ChatLog::const_iterator iterChatLog = s_chatLog.find(messageIndex);

	if (iterChatLog != s_chatLog.end())
	{
		// This string has already been added

		++s_cacheHits;

		LOGC(ConfigServerUtility::isChatLogManagerLoggingEnabled(), "ChatLogManager", ("INSERT-HIT s_chatLog.size(%u) messageIndex(%d) time(%u) fromPlayer(%s) toPlayer(%s) channel(%s) text(%s)\n", s_chatLog.size(), messageIndex, iterChatLog->second.getTime(), Unicode::wideToNarrow(fromPlayer).c_str(), Unicode::wideToNarrow(toPlayer).c_str(), Unicode::wideToNarrow(channel).c_str(), Unicode::wideToNarrow(message).c_str()));
	}
	else
	{
		Unicode::String const *finalMessage = nullptr;

		// Add the new string to the master message list, or if it already exists, just increase the reference count

		MessageList::iterator iterChatMessageList = s_chatMessageList.find(&message);

		if (iterChatMessageList != s_chatMessageList.end())
		{
			// Increase the reference count

			++iterChatMessageList->second;

			finalMessage = iterChatMessageList->first;
		}
		else
		{
			// Add the new message to the list

			finalMessage = new Unicode::String(message);

			s_chatMessageList.insert(std::make_pair(finalMessage, 1));
		}

		++s_cacheMisses;

		s_timeLog.push_back(std::make_pair(time, messageIndex));
		s_chatLog.insert(std::make_pair(messageIndex, ChatLogEntry(fromPlayer, toPlayer, finalMessage, channel, time)));

		// Track the player's who are in the logs, and how many time they have messages in the logs

		Unicode::String const lowerFromPlayer(Unicode::toLower(fromPlayer));
		PlayerLog::iterator iterPlayerLog = s_playerLog.find(lowerFromPlayer);

		if (iterPlayerLog != s_playerLog.end())
		{
			++(iterPlayerLog->second);
		}
		else
		{
			s_playerLog.insert(std::make_pair(lowerFromPlayer, 1));
		}

		LOGC(ConfigServerUtility::isChatLogManagerLoggingEnabled(), "ChatLogManager", ("INSERT-MISS s_chatLog.size(%u) messageIndex(%d) time(%u) fromPlayer(%s) toPlayer(%s) channel(%s) text(%s)\n", s_chatLog.size(), messageIndex, time, Unicode::wideToNarrow(lowerFromPlayer).c_str(), Unicode::wideToNarrow(toPlayer).c_str(), Unicode::wideToNarrow(channel).c_str(), Unicode::wideToNarrow(message).c_str()));
	}
}

//-----------------------------------------------------------------------------
bool ChatLogManagerNamespace::removeChatLogEntry(int const messageIndex)
{
	bool result = false;
	ChatLog::iterator iterChatLog = s_chatLog.find(messageIndex);

	if (iterChatLog != s_chatLog.end())
	{
		result = true;

		// Remove the chat message

		Unicode::String const &message = iterChatLog->second.getText();
		MessageList::iterator iterChatMessageList = s_chatMessageList.find(&message);

		if (iterChatMessageList != s_chatMessageList.end())
		{
			if (--iterChatMessageList->second <= 0)
			{
				// No more chat logs are referencing this message, we can remove it

				delete iterChatMessageList->first;

				s_chatMessageList.erase(iterChatMessageList);
			}
		}
		else
		{
			DEBUG_WARNING(true, ("Master chat list does not contain message index(%d). This is a bug.", messageIndex));
		}

		// Remove the player log

		PlayerLog::iterator iterPlayerLog = s_playerLog.find(iterChatLog->second.getFromPlayer());

		if (iterPlayerLog != s_playerLog.end())
		{
			if (--iterPlayerLog->second <= 0)
			{
				s_playerLog.erase(iterPlayerLog);
			}
		}
		else
		{
			DEBUG_WARNING(true, ("A player log was unable to be removed for %s", Unicode::wideToNarrow(iterChatLog->second.getFromPlayer()).c_str()));
		}
	
		s_chatLog.erase(iterChatLog);
	}

	return result;
}

//-----------------------------------------------------------------------------
void ChatLogManagerNamespace::purgeChatLog()
{
	// This purges the list of old data once a second

	time_t const systemTime = Os::getRealSystemTime();

	if (s_purgeTime != systemTime)
	{
		s_purgeTime = systemTime;

		time_t const chatLogPurgeTime = static_cast<time_t>(ConfigServerUtility::getChatLogMinutes() * 60);
		TimeLog::iterator iterTimeLog = s_timeLog.begin();
		int chatLogCount = static_cast<int>(s_timeLog.size());
		int purgedMessages = 0;

		for (; iterTimeLog != s_timeLog.end(); ++iterTimeLog)
		{
			time_t const time = iterTimeLog->first;

			if (   (time < (systemTime - chatLogPurgeTime))
			    || (static_cast<int>(s_chatMessageList.size()) > ConfigServerUtility::getServerMaxChatLogLines()))
			{
				int const messageIndex = iterTimeLog->second;

				if (removeChatLogEntry(messageIndex))
				{
					--chatLogCount;
					++purgedMessages;

					LOGC(ConfigServerUtility::isChatLogManagerLoggingEnabled(), "ChatLogManager", ("REMOVE index(%d) s_chatLog.size(%u) s_chatMessageList.size(%u)\n", messageIndex, s_chatLog.size(), s_chatMessageList.size()));
				}
				else
				{
					LOGC(ConfigServerUtility::isChatLogManagerLoggingEnabled(), "ChatLogManager", ("ERROR: REMOVE index(%d) Unable to resolve the message index in the chat log.\n", messageIndex));
				}
			}
			else
			{
				break;
			}
		}

		if (purgedMessages > 0)
		{
			// Remove the items from the time log

			s_timeLog.erase(s_timeLog.begin(), iterTimeLog);
		}

		// Make sure the time log and chat log are equal in size, if not, then something is seriously wrong

		DEBUG_WARNING((s_timeLog.size() != s_chatLog.size()), ("ERROR: Time log and chat log are not the same size. This is a bug."));
	}
}

//-----------------------------------------------------------------------------
std::string ChatLogManagerNamespace::getTimeString(time_t const time)
{
	std::string result(ctime(&time));

	if (!result.empty())
	{
		result.erase(result.size() - 1);
	}

	return result;
}

//-----------------------------------------------------------------------------
std::string ChatLogManagerNamespace::getTimeStringShort(time_t const time)
{
	std::string result(ctime(&time));

	if (!result.empty())
	{
		result = result.substr(11, 8);
	}

	return result;
}

// ============================================================================
//
// ChatLogManager
//
// ============================================================================

//-----------------------------------------------------------------------------
void ChatLogManager::install()
{
	LOG("ChatLogManager", ("chatLogMinutes(%d)", ConfigServerUtility::getChatLogMinutes()));
	LOG("ChatLogManager", ("serverMaxChatLogLines(%d)", ConfigServerUtility::getServerMaxChatLogLines()));
	LOG("ChatLogManager", ("playerMaxChatLogLines(%d)", ConfigServerUtility::getPlayerMaxChatLogLines()));
	LOG("ChatLogManager", ("chatLogManagerLoggingEnabled(%s)", ConfigServerUtility::isChatLogManagerLoggingEnabled() ? "yes" : "no"));

	ExitChain::add(&remove, "ChatLogManager::remove");
}

//-----------------------------------------------------------------------------
void ChatLogManager::remove()
{
	while (!s_chatMessageList.empty())
	{
		MessageList::iterator iterMessageList = s_chatMessageList.begin();

		delete iterMessageList->first;

		s_chatMessageList.erase(iterMessageList);
	}
}

//-----------------------------------------------------------------------------
void ChatLogManager::logChat(Unicode::String const & fromPlayer, Unicode::String const & toPlayer, Unicode::String const & message, Unicode::String const & channel, int const messageIndex, time_t const time)
{
	addChatLogEntry(fromPlayer, toPlayer, message, channel, messageIndex, time);
	purgeChatLog();
	
	if (ConfigServerUtility::isChatLogManagerLoggingEnabled())
	{
		printPlayerLog();
	}
}

//-----------------------------------------------------------------------------
int ChatLogManager::getNextMessageIndex()
{
	return s_currentIndex++;
}

//-----------------------------------------------------------------------------
bool ChatLogManager::getChatMessage(int const messageIndex, Unicode::String &chatMessage)
{
	bool result = false;
	ChatLog::const_iterator iterChatLog = s_chatLog.find(messageIndex);

	if (iterChatLog != s_chatLog.end())
	{
		result = true;
		chatMessage.clear();
		ChatLogEntry const &chatLogEntry = iterChatLog->second;

		buildLogEntry(chatMessage, chatLogEntry.getFromPlayer(), chatLogEntry.getToPlayer(), chatLogEntry.getText(), chatLogEntry.getChannel(), chatLogEntry.getTime());
	}
	else
	{
		LOGC(ConfigServerUtility::isChatLogManagerLoggingEnabled(), "ChatLogManager", ("Unable to find chat log message(%d)\n", messageIndex));
	}

	return result;
}

//-----------------------------------------------------------------------------
bool ChatLogManager::getChatMessage(int const messageIndex, Unicode::String &fromPlayer, Unicode::String &toPlayer, Unicode::String &text, Unicode::String &channel, time_t &time)
{
	bool result = false;

	ChatLog::const_iterator iterChatLog = s_chatLog.find(messageIndex);

	if (iterChatLog != s_chatLog.end())
	{
		result = true;
		ChatLogEntry const &chatLogEntry = iterChatLog->second;

		fromPlayer = chatLogEntry.getFromPlayer();
		toPlayer = chatLogEntry.getToPlayer();
		text = chatLogEntry.getText();
		channel = chatLogEntry.getChannel();
		time = chatLogEntry.getTime();
	}
	else
	{
		DEBUG_WARNING(true, ("Unable to find chat log message(%d)\n", messageIndex));
	}

	return result;
}

//-----------------------------------------------------------------------------
void ChatLogManager::buildLogEntry(Unicode::String &result, Unicode::String const &fromPlayer, Unicode::String const &toPlayer, Unicode::String const &message, Unicode::String const &channel, time_t const time)
{
	result.clear();
	result.append(Unicode::narrowToWide(getTimeStringShort(time)));
	result.append(Unicode::narrowToWide(" ["));
	result.append(channel);
	result.append(Unicode::narrowToWide("] "));
	result.append(fromPlayer);

	if (!toPlayer.empty())
	{
		result.append(Unicode::narrowToWide(" -> "));
		result.append(toPlayer);
	}

	result.append(Unicode::narrowToWide(": "));
	result.append(message);
	result.append(Unicode::narrowToWide("\n"));
}

//-----------------------------------------------------------------------------
bool ChatLogManager::isPlayerInLogs(Unicode::String const &player)
{
	return (s_playerLog.find(Unicode::toLower(player)) != s_playerLog.end());
}

//-----------------------------------------------------------------------------
void ChatLogManager::getReportHeader(Unicode::String & header, std::string const &reportingPlayer, NetworkId const &reportingPlayerNetworkId, std::string const &reportingPlayerStationName, uint32 reportingPlayerStationId, std::string const &harassingPlayer, NetworkId const &harassingPlayerNetworkId, std::string const &harassingPlayerStationName, uint32 harassingPlayerStationId)
{
	header.clear();
	header.append(Unicode::narrowToWide("\n\n//--------------------------------------------------------\n"));

	FormattedString<512> fs;

	// Reporting

	header.append(Unicode::narrowToWide(fs.sprintf("Reporting: %s (%s)", reportingPlayer.c_str(), reportingPlayerNetworkId.getValueString().c_str())));

	if (!reportingPlayerStationName.empty() || (reportingPlayerStationId > 0))
	{
		header.append(Unicode::narrowToWide(" Station ID:"));
		if (!reportingPlayerStationName.empty())
			header.append(Unicode::narrowToWide(fs.sprintf(" %s", reportingPlayerStationName.c_str())));
		if (reportingPlayerStationId > 0)
			header.append(Unicode::narrowToWide(fs.sprintf(" (%lu)", reportingPlayerStationId)));
	}

	header.append(Unicode::narrowToWide("\n"));

	// Harassing

	header.append(Unicode::narrowToWide(fs.sprintf("Harassing: %s (%s)", harassingPlayer.c_str(), harassingPlayerNetworkId.getValueString().c_str())));

	if (!harassingPlayerStationName.empty() || (harassingPlayerStationId > 0))
	{
		header.append(Unicode::narrowToWide(" Station ID:"));
		if (!harassingPlayerStationName.empty())
			header.append(Unicode::narrowToWide(fs.sprintf(" %s", harassingPlayerStationName.c_str())));
		if (harassingPlayerStationId > 0)
			header.append(Unicode::narrowToWide(fs.sprintf(" (%lu)", harassingPlayerStationId)));
	}

	header.append(Unicode::narrowToWide("\n"));

	// Creation time

	header.append(Unicode::narrowToWide(fs.sprintf("Log Creation Time: %s\n", getTimeString(Os::getRealSystemTime()).c_str())));

	// Time length

	header.append(Unicode::narrowToWide(fs.sprintf("Log Time Length: %d minutes\n", ConfigServerUtility::getChatLogMinutes())));
}

// ============================================================================
