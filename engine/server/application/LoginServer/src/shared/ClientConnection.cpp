// ClientConnection.cpp
// copyright 2000 Verant Interactive
// Author: Justin Randall


//-----------------------------------------------------------------------

#include "FirstLoginServer.h"
#include "ClientConnection.h"

#include "DatabaseConnection.h"
#include "ConfigLoginServer.h"
#include "SessionApiClient.h"
#include "sharedLog/Log.h"
#include "sharedNetworkMessages/ClientLoginMessages.h"
#include "sharedNetworkMessages/DeleteCharacterMessage.h"
#include "sharedNetworkMessages/DeleteCharacterReplyMessage.h"
#include "sharedNetworkMessages/ErrorMessage.h"
#include "sharedNetworkMessages/GenericValueTypeMessage.h"
#include "sharedNetworkMessages/LoginEnumCluster.h"

#include "sharedFoundation/CrcConstexpr.hpp"

#include "webAPI.h"

using namespace StellaBellum;

//-----------------------------------------------------------------------

ClientConnection::ClientConnection(UdpConnectionMT *u, TcpClient *t)
        : ServerConnection(u, t), m_clientId(0), m_isValidated(false), m_isSecure(false), m_adminLevel(-1),
          m_stationId(0), m_requestedAdminSuid(0), m_gameBits(0), m_subscriptionBits(0),
          m_waitingForCharacterLoginDeletion(false), m_waitingForCharacterClusterDeletion(false) {
}

//-----------------------------------------------------------------------

ClientConnection::~ClientConnection() {
}

//-----------------------------------------------------------------------

void ClientConnection::onConnectionClosed() {
    // client has disconnected
    if (m_stationId) {
        DEBUG_REPORT_LOG(true, ("Client %lu disconnected\n", m_stationId));
        LOG("LoginClientConnection", ("onConnectionClosed() for stationId (%lu) at IP (%s)", m_stationId, getRemoteAddress().c_str()));
    }

    LoginServer::getInstance().removeClient(m_clientId);

    /* if ((ConfigLoginServer::getValidateStationKey() || ConfigLoginServer::getDoSessionLogin()) && !m_isValidated) {
         SessionApiClient *session = LoginServer::getInstance().getSessionApiClient();
         if (session) {
             session->dropClient(this);
         }
     }*/

}

//-----------------------------------------------------------------------

void ClientConnection::onConnectionOpened() {
    m_clientId = LoginServer::getInstance().addClient(*this);
    setOverflowLimit(ConfigLoginServer::getClientOverflowLimit());

    LOG("LoginClientConnection", ("onConnectionOpened() for stationId (%lu) at IP (%s)", m_stationId, getRemoteAddress().c_str()));
}

//-----------------------------------------------------------------------

void ClientConnection::onReceive(const Archive::ByteStream &message) {
    try {
        //Handle all client messages here.  Do not forward out.
        Archive::ReadIterator ri = message.begin();
        GameNetworkMessage m(ri);
        ri = message.begin();

        const uint32 messageType = m.getType();

        //Validation check
        if (!getIsValidated() && messageType != constcrc("LoginClientId")) {
            //Receiving message from unvalidated client.  Pitch it.
            DEBUG_WARNING(true, ("Received %s message from unknown, unvalidated client", m.getCmdName().c_str()));
            return;
        }

        switch (messageType) {
            case constcrc("LoginClientId") : {
                // send the client the server "now" Epoch time so that the
                // client has an idea of how much difference there is between
                // the client's Epoch time and the server Epoch time
                GenericValueTypeMessage <int32> const serverNowEpochTime("ServerNowEpochTime", static_cast<int32>(::time(nullptr)));
                send(serverNowEpochTime, true);

                LoginClientId id(ri);

                // verify version
#if PRODUCTION == 1
                if(!ConfigLoginServer::getValidateClientVersion() || id.getVersion() == GameNetworkMessage::NetworkVersionId)
                {
                    validateClient(id.getId(), id.getKey());
                }
                else
                {
                    LOG("CustomerService", ("Login:LoginServer dropping client (stationId=[%lu], ip=[%s], id=[%s], key=[%s], version=[%s]) because of network version mismatch (required version=[%s])", m_stationId, getRemoteAddress().c_str(), id.getId().c_str(), id.getKey().c_str(), id.getVersion().c_str(), GameNetworkMessage::NetworkVersionId.c_str()));
                    // disconnect is handled on the client side, as soon as it recieves this message
#if _DEBUG
                    LoginIncorrectClientId incorrectId(GameNetworkMessage::NetworkVersionId, ApplicationVersion::getInternalVersion());
#else
                    LoginIncorrectClientId incorrectId("", "");
#endif // _DEBUG
                    send(incorrectId, true);
                }
#else
                validateClient(id.getId(), id.getKey());
#endif // PRODUCTION == 1

                break;
            }
            case constcrc("RequestExtendedClusterInfo") : {
                LoginServer::getInstance().sendExtendedClusterInfo(*this);
                break;
            }
            case constcrc("DeleteCharacterMessage") : {
                DeleteCharacterMessage msg(ri);
                std::vector<NetworkId>::const_iterator f = std::find(m_charactersPendingDeletion.begin(), m_charactersPendingDeletion.end(), msg.getCharacterId());
                if ((m_waitingForCharacterLoginDeletion || m_waitingForCharacterClusterDeletion) &&
                    f != m_charactersPendingDeletion.end()) {
                    DeleteCharacterReplyMessage reply(DeleteCharacterReplyMessage::rc_ALREADY_IN_PROGRESS);
                    send(reply, true);
                } else {
                    if (LoginServer::getInstance().deleteCharacter(msg.getClusterId(), msg.getCharacterId(), getStationId())) {
                        m_waitingForCharacterLoginDeletion = true;
                        m_waitingForCharacterClusterDeletion = true;
                        m_charactersPendingDeletion.push_back(msg.getCharacterId());
                    } else {
                        DeleteCharacterReplyMessage reply(DeleteCharacterReplyMessage::rc_CLUSTER_DOWN);
                        send(reply, true);
                    }
                }
                break;
            }
        }
    } catch (const Archive::ReadException &readException) {
        WARNING(true, ("Archive read error (%s) on message from client. Disconnecting client.", readException.what()));
        disconnect();
    }
}

//-----------------------------------------------------------------------
// originally was used to validate station API credentials, now uses our custom api
void ClientConnection::validateClient(const std::string &id, const std::string &key) {
    bool authOK = false;
    bool testMode = false;
    static const std::string authURL(ConfigLoginServer::getExternalAuthUrl());

    std::string uname;
    std::string parentAccount;
    std::string sessionID;

    StationId user_id;
    StationId parent_id;
    std::unordered_map<int, std::string> childAccounts;

    if (!authURL.empty()) {
        // create the object
        webAPI api(authURL);

        // add our data
        api.addJsonData<std::string>("user_name", id);
        api.addJsonData<std::string>("user_password", key);
        api.addJsonData<std::string>("ip", getRemoteAddress());

        if (api.submit()) {
            bool status = api.getNullableValue<bool>("status");
            uname = api.getString("username");
            sessionID = api.getString("session_key");

            if (status && !sessionID.empty() && !uname.empty()) {
                authOK = true;

                parentAccount = api.getString("mainAccount");
                childAccounts = api.getStringMap("subAccounts");

                if (!ConfigLoginServer::getUseOldSuidGenerator()) {
                    user_id = static_cast<StationId>(api.getNullableValue<int>("user_id"));
                    parent_id = static_cast<StationId>(api.getNullableValue<int>("parent_id"));
                } else {
                    if (parentAccount.length() > MAX_ACCOUNT_NAME_LENGTH) {
                        parentAccount.resize(MAX_ACCOUNT_NAME_LENGTH);
                    }

                    if (uname.length() > MAX_ACCOUNT_NAME_LENGTH) {
                        uname.resize(MAX_ACCOUNT_NAME_LENGTH);
                    }

                    parent_id = std::hash < std::string > {}(parentAccount.c_str());
                    user_id = std::hash < std::string > {}(uname.c_str());
                }
            } else {
                std::string msg(api.getString("message"));
                if (msg.empty()) {
                    msg = "Invalid username or password.";
                }

                ErrorMessage err("Login Failed", msg);
                this->send(err, true);
            }
        } else {
            ErrorMessage err("Login Failed", "Could not connect to remote.");
            this->send(err, true);
        }
    } else {
        // test mode
        authOK = true;
        testMode = true;
        uname = id;

        if (uname.length() > MAX_ACCOUNT_NAME_LENGTH) {
            uname.resize(MAX_ACCOUNT_NAME_LENGTH);
        }

        user_id = std::hash < std::string > {}(uname.c_str());
    }

    if (authOK) {
        m_stationId = user_id;

        if (!testMode) {
            REPORT_LOG(true, ("Client connected. Username: %s (%i) \n", uname.c_str(), user_id));

            if (!parentAccount.empty()) {
                if (parentAccount != uname) {
                    REPORT_LOG(true, ("\t%s's parent is %s (%i) \n", uname.c_str(), parentAccount.c_str(), parent_id));
                }
            } else {
                parentAccount = "(Empty Parent!) " + uname;
            }

            for (auto i : childAccounts) {
                StationId child_id = static_cast<StationId>(i.first);
                std::string child(i.second);

                if (!child.empty() && i.first > 0) {
                    if (ConfigLoginServer::getUseOldSuidGenerator()) {
                        if (child.length() > MAX_ACCOUNT_NAME_LENGTH) {
                            child.resize(MAX_ACCOUNT_NAME_LENGTH);
                        }

                        child_id = std::hash < std::string > {}(child.c_str());
                    }

                    REPORT_LOG((parent_id !=
                                child_id), ("\tchild of %s (%i) is %s (%i) \n", parentAccount.c_str(), parent_id, child.c_str(), child_id));

                    // insert all related accounts, if not already there, into the db
                    if (parent_id != child_id) {
                        DatabaseConnection::getInstance().upsertAccountRelationship(parent_id, child_id);
                    }
                } else {
                    WARNING(true, ("Login API returned empty child account(s)."));
                }
            }

            LoginServer::getInstance().onValidateClient(m_stationId, uname, this, true, sessionID.c_str(), 0xFFFFFFFF, 0xFFFFFFFF);
        } else {
            LoginServer::getInstance().onValidateClient(m_stationId, uname, this, true, nullptr, 0xFFFFFFFF, 0xFFFFFFFF);
        }
        LOG("LoginClientConnection", ("validateClient() for stationId (%i) at IP (%s), id (%s)", user_id, getRemoteAddress().c_str(), uname.c_str()));
    }
}

// ----------------------------------------------------------------------------

/**
 * The character has been deleted from the login database.  1/2 of what is
 * required for character deletion.  If the character has already been deleted
 * from the cluster, send the reply message to the client.
 */
void ClientConnection::onCharacterDeletedFromLoginDatabase(const NetworkId &characterId) {
    m_waitingForCharacterLoginDeletion = false;
    if (!m_waitingForCharacterClusterDeletion) {
        std::vector<NetworkId>::iterator f = std::find(m_charactersPendingDeletion.begin(), m_charactersPendingDeletion.end(), characterId);
        if (f != m_charactersPendingDeletion.end()) {
            m_charactersPendingDeletion.erase(f);
        }

        DeleteCharacterReplyMessage reply(DeleteCharacterReplyMessage::rc_OK);
        send(reply, true);
        LOG("CustomerService", ("Player:deleted character %s for stationId %u at IP: %s", characterId.getValueString().c_str(), m_stationId, getRemoteAddress().c_str()));
    }
}

// ----------------------------------------------------------------------

void ClientConnection::onCharacterDeletedFromCluster(const NetworkId &characterId) {
    m_waitingForCharacterClusterDeletion = false;
    if (!m_waitingForCharacterLoginDeletion) {
        std::vector<NetworkId>::iterator f = std::find(m_charactersPendingDeletion.begin(), m_charactersPendingDeletion.end(), characterId);
        if (f != m_charactersPendingDeletion.end()) {
            m_charactersPendingDeletion.erase(f);

            // TODO: send api request and decrement # characters on this account/subaccount
        }

        DeleteCharacterReplyMessage reply(DeleteCharacterReplyMessage::rc_OK);
        send(reply, true);
        LOG("CustomerService", ("Player:deleted character %s for stationId %u at IP: %s", characterId.getValueString().c_str(), m_stationId, getRemoteAddress().c_str()));
    }
}

// ----------------------------------------------------------------------

StationId ClientConnection::getRequestedAdminSuid() const {
    return m_requestedAdminSuid;
}

// ======================================================================

