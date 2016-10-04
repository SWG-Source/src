// CSToolConnection.cpp
// copyright 2005 Sony Online Entertainment
// Author: Robert Hanz

#include "FirstLoginServer.h"

#include "Archive/ByteStream.h"
#include "ConfigLoginServer.h"
#include "CSToolConnection.h"
#include "serverNetworkMessages/CSToolRequest.h"
#include "serverUtility/AdminAccountManager.h"
#include "Session/LoginAPI/Client.h"
#include "SessionApiClient.h"
#include "sharedLog/Log.h"

#if defined(WIN32)
#include <winsock.h>
#else        // for non-windows platforms (linux)
#include <arpa/inet.h>
#endif

namespace CSToolConnectionNamespace
{
	std::map< uint32, CSToolConnection * > toolsByID;
}

uint32 CSToolConnection::m_curToolID = 0;

// statics

CSToolConnection * CSToolConnection::getCSToolConnectionByToolId(uint32 id)
{
	std::map< uint32, CSToolConnection * >::iterator it = CSToolConnectionNamespace::toolsByID.find(id);
	return it == CSToolConnectionNamespace::toolsByID.end() ? 0 : it->second;
}

void CSToolConnection::validateCSTool(uint32 toolId, apiResult result, const apiSession& session)
{
	//find the tool
	CSToolConnection * p_connection = getCSToolConnectionByToolId(toolId);
	if (!p_connection)
		return; // no connection, they must have bailed.

	// check the result
	if (result == RESULT_SUCCESS)
	{
		// if we're valid, set to logged in
		p_connection->m_bLoggedIn = true;
		if (session.GetIsSecure())
		{
			p_connection->m_bSecure = true;
		}

		// if we have a nullptr session type, then we aren't connected to the
		// Session Server and are in debug mode.
		//
		// if we *do* have a good session, then check the admin table for access level.
		int accessLevel = 100;

		bool sessionNull = (session.GetType() == SESSION_TYPE_NULL);
		bool isAdmin = AdminAccountManager::isAdminAccount(p_connection->m_sUserName, accessLevel);
		bool needSecure = ConfigLoginServer::getRequireSecureLoginForCsTool();
		bool isInternal = AdminAccountManager::isInternalIp(p_connection->getRemoteAddress());
		DEBUG_REPORT_LOG(true, ("CS Tool login for %s, admin=%s, access=%d, internal=%s\n",
			p_connection->m_sUserName.c_str(),
			isAdmin ? "true" : "false",
			accessLevel,
			isInternal ? "true" : "false"));

		bool canLogin = isInternal; // we always have to be internal.
		if (sessionNull)
		{
			// nullptr session means we can skip everything else.
			canLogin = canLogin && true;
			accessLevel = 100;
		}
		else
		{
			// if we don't have a session, we need to be on the admin table
			if (isAdmin)
			{
				// allow login if we either don't need to be secure, or if we are.
				if ((!needSecure) || p_connection->m_bSecure)
				{
					canLogin = canLogin && true;
				}
				else
				{
					canLogin = false;
				}
			}
			else
			{
				canLogin = false;
			}
		}

		if (canLogin)
		{
			// and inform
			std::string response = "Logged in successfully.";

			response += "\r\n";
			p_connection->sendToTool(response);

			p_connection->m_ui32PrivilegeLevel = (uint32)accessLevel;
		}
		else
		{
			p_connection->sendToTool("Invalid name/password.\r\n");
		}
	}
	// otherwise, give the appropriate response
	else if (result == RESULT_INVALID_NAME_OR_PASSWORD)
	{
		p_connection->sendToTool("Invalid name/password.\r\n");
	}
	else
	{
		DEBUG_REPORT_LOG(true, ("Could not log in CS Tool for user %s, reason %d\n", p_connection->m_sUserName.c_str(), result));
		p_connection->sendToTool("Invalid name/password.\r\n");
	}
}

// members

CSToolConnection::CSToolConnection(UdpConnectionMT * pUdpConn, TcpClient * pTcpConn) :
	Connection(pUdpConn, pTcpConn),
	m_ui32StationID(0),
	m_bSecure(false),
	m_bLoggedIn(false),
	m_ui32PrivilegeLevel(0),
	m_toolID(0)
{
	if (pTcpConn) // it'd better be!
		setRawTCP(true);
}

CSToolConnection::~CSToolConnection()
{
}

void CSToolConnection::onReceive(const Archive::ByteStream & message)
{
	// for testing purposes, just echo back to the user.

	// add to buffer
	std::string input;
	const unsigned char * uc = message.getBuffer();
	while (uc < message.getBuffer() + message.getSize())
	{
		char s = *uc;
		input += s;
		++uc;
	}
	m_sInputBuffer += input;

	// while we've got an EOL signifier
	std::string::size_type pos;
	while ((pos = m_sInputBuffer.find('\r')) != std::string::npos)
	{
		// rebuild message buffer properly.
		std::string command = m_sInputBuffer.substr(0, pos);
		// should dispatch here, for now we're echoing back for debug.

		//command = command + "\r\n";
		//sendToTool( command );
		this->parse(command);
		// then we have additional data.  Otherwise, we don't.
		if (m_sInputBuffer.size() >= pos + 2)
		{
			m_sInputBuffer = m_sInputBuffer.substr(pos + 2, m_sInputBuffer.length() - (pos + 2));
		}
		else
		{
			m_sInputBuffer.clear();
		}
	}
}

void CSToolConnection::parse(const std::string command)
{
	// grab the first word of the command.
	std::string first;
	std::string args;
	std::string::size_type pos;
	if ((pos = command.find(' ')) == std::string::npos)
	{
		first = command;
	}
	else
	{
		first = command.substr(0, pos);
		args = command.substr(pos + 1, command.size() - (pos + 1));
	}
	// test for empty commands.
	if (first.length() == 0 || first[0] == ' ')
	{
		return;
	}

	std::string response = "ls:";

	if (first == "clusters")
	{
		response += "Active clusters:\r\n";
		std::map< std::string, uint32 > data;
		LoginServer::getInstance().getAllClusterNamesAndIDs(data);
		for (std::map< std::string, uint32 >::iterator it = data.begin();
			it != data.end();
			++it)
		{
			response += it->first + "\r\n";
		}
		sendToTool(response);
	}
	else if (first == "select")
	{
		// see if we're adding, removing, or exclusive-adding.
		if (args[0] == '+')
		{
			if (args.size() > 1)
			{
				if (args[1] == '+')
				{
					m_selectedClusters.clear();
					sendToTool("ls:deselected all servers\r\n");
				}
				pos = args.find_first_not_of("+");
				if (pos == std::string::npos)
				{
				}
				else
				{
					std::string cluster = args.substr(pos, args.size() - pos);
					response += selectCluster(cluster);
					sendToTool(response);
				}
			}
		}
		else if (args[0] == '-')
		{
			if (args.size() > 1)
			{
				std::string cluster = args.substr(1, args.size() - 1);
				if (m_selectedClusters.find(cluster) != m_selectedClusters.end())
				{
					response += deselectCluster(cluster);
					sendToTool(response);
				}
			}
		}
		else
		{
			sendToTool("Error!  Bad select format!\r\n");
		}
	}
	else if (first == "showselected")
	{
		response += "Currently selected clusters:\r\n";
		for (std::set< std::string >::iterator it = m_selectedClusters.begin();
			it != m_selectedClusters.end();
			++it)
		{
			response += *it + "\r\n";
		}
		sendToTool(response);
	}
	else if (first == "login")
	{
		// assume name/pw do not have spaces.
		std::string name;
		std::string pw;

		// split apart
		unsigned pos = args.find(" ");
		if (pos != std::string::npos)
		{
			name = args.substr(0, pos);
			pw = args.substr(pos + 1, args.length() - pos - 1);
			m_sUserName = name;
			if (!LoginServer::getInstance().getSessionApiClient())
			{
				apiSession session;
				validateCSTool(m_toolID, RESULT_SUCCESS, session);
				return;
			}
			LoginServer::getInstance().getSessionApiClient()->
				SessionLogin(name.c_str(),
					pw.c_str(),
					SESSION_TYPE_STARWARS,
					inet_addr(getRemoteAddress().c_str()),
					0,
					_defaultNamespace,
					(void *)m_toolID);
		}

		// pass to session.
	}
	// default command handler.
	else
	{
		if (m_bLoggedIn)
		{
			sendToClusters(first, command);
		}
		else
		{
			sendToTool("Not logged in.\r\n");
		}
	}
}

void CSToolConnection::sendToClusters(const std::string & sCommand, const std::string & sCommandLine)
{
	// build the message
	CSToolRequest msg(m_ui32StationID, m_ui32PrivilegeLevel, sCommandLine, sCommand, m_toolID, m_sUserName);

	// send to all currently selected clusters.
	for (std::set< std::string >::iterator it = m_selectedClusters.begin();
		it != m_selectedClusters.end();
		++it)
	{
		LoginServer::getInstance().sendToCluster(LoginServer::getInstance().getClusterIDByName(*it), msg);
	}
}

std::string CSToolConnection::selectCluster(const std::string & cluster)
{
	std::string response;
	// see if we have a valid cluster.
	uint32 cluster_id = LoginServer::getInstance().getClusterIDByName(cluster);
	if (cluster_id == 0)
	{
		response += "Cluster " + cluster + " does not exist.\r\n";
	}
	else
	{
		m_selectedClusters.insert(cluster);
		response += "Cluster " + cluster + " added to active list\r\n";
	}
	return response;
}

std::string CSToolConnection::deselectCluster(const std::string & cluster)
{
	std::string response;

	// see if we are currently talking to that cluster.
	std::set< std::string >::iterator it = m_selectedClusters.find(cluster);
	if (it == m_selectedClusters.end())
	{
		response += "Cluster " + cluster + " is not currently selected.\r\n";
	}
	else
	{
		m_selectedClusters.erase(it);
		response += "Cluster " + cluster + " removed from active list.\r\n";
	}
	return response;
}

void CSToolConnection::sendToTool(const std::string & message)
{
	// convert to ByteStream
	std::string temp = message;
	temp += "\r\n*\r\n";
	Archive::ByteStream bs;
	bs.put(temp.c_str(), temp.size());
	// send.
	this->send(bs, true);
}

void CSToolConnection::onConnectionOpened()
{
	LOG("CSTool", ("Connection opened"));
	// assign an ID, and stick it in our map.
	m_curToolID++;
	m_toolID = m_curToolID;
	CSToolConnectionNamespace::toolsByID[m_toolID] = this;
}

void CSToolConnection::onConnectionClosed()
{
	LOG("CSTool", ("Connection closed"));
	CSToolConnectionNamespace::toolsByID.erase(m_toolID);
}