//#ifndef BASE__API_H
//#define BASE__API_H

#include <vector>
#include <list>
#include <set>
#include <map>
#include "Base/profile.h"
#include "Base/stringutils.h"

#ifdef UDP_LIBRARY
#include "UdpLibrary/UdpLibrary.hpp"
#else
#include "TcpLibrary/TcpConnection.h"
#include "TcpLibrary/TcpManager.h"
#endif
#include "apiMessages.h"

#ifndef API_NAMESPACE
#pragma message ("api.h: API_NAMESPACE undefined")
//#else
//#pragma message ("api.h: API_NAMESPACE:")
//#pragma message API_NAMESPACE
#endif

#ifdef API_NAMESPACE
namespace API_NAMESPACE
{
#endif

	////////////////////////////////////////////////////////////////////////////////

	struct TrackedRequest
	{
		TrackedRequest();
		TrackedRequest(unsigned messageId, unsigned trackingNumber, unsigned result, const void * userData, unsigned timeout = 5);

		unsigned	mMsgId;
		unsigned	mTrackingNumber;
		unsigned	mResult;
		void *		mUserData;
		unsigned    mTimeout;
	};

	////////////////////////////////////////////////////////////////////////////////

#ifdef UDP_LIBRARY
	class CommonAPI : UdpConnectionHandler
#else
	class CommonAPI : TcpConnectionHandler
#endif
	{
#ifdef UDP_LIBRARY
		typedef UdpConnection ApiConnection;
#else
		typedef TcpConnection ApiConnection;
#endif
		typedef std::set<unsigned> TrackingSet_t;
		struct ApiConnectionInfo 
		{
			ApiConnectionInfo(ApiConnection * Connection = nullptr) : mConnection(Connection), mIsShuttingDown(false) { }

			ApiConnection * mConnection;
			TrackingSet_t mOutstandingRequests;
			bool mIsShuttingDown;
		};
		typedef std::map<ApiConnection *, std::string> HostMap_t;
		typedef std::set<ApiConnection *> ConnectionSet_t;
		typedef std::map<ApiConnection *, ApiConnectionInfo> ConnectionMap_t;
		typedef std::map<unsigned, TrackedRequest> RequestMap_t;
		typedef std::list<std::pair<unsigned,unsigned> > TimeoutList_t;
		typedef std::map<unsigned, ApiConnection *> ConnectionTrackMap_t;
		typedef std::map<unsigned, TimeoutList_t> TimeoutMap_t;
		typedef std::list<unsigned> TrackingList_t;
		typedef std::set<unsigned> MessagIdSet_t;
		public:
			CommonAPI(const char * hostList, const char * failoverHostList = 0, unsigned connectionLimit = 0, unsigned maxMsgSize = 1*1024, unsigned bufferSize = 64*1024);
			virtual ~CommonAPI();

			void						Process();
			void						DisconnectAll();
			void						AllowConnections();
			unsigned					GetConnectionCount() const;
			unsigned					GetOutstandingRequestCount() const;
			unsigned					GetLastRequestInputTime() const;
			unsigned					GetTrackingNumber();
			bool						Send(const unsigned char *data, unsigned dataLen);
			void						PrepareCallback(unsigned messageId, unsigned trackingNumber, unsigned result, const void * userData, unsigned timeout = 0);
			bool						SendAndPrepareCallback(const unsigned char *data, unsigned dataLen, unsigned messageId, unsigned trackingNumber, unsigned result, const void * userData, unsigned timeout);
			bool						GetCallbackUserData(unsigned trackingNumber, const void **pUserData);
        
		protected:
#ifdef UDP_LIBRARY
			virtual UdpManager::Params	GetConnectionParams();
			virtual void				OnConnectComplete(UdpConnection *con);
			virtual void				OnTerminated(UdpConnection *con);
			virtual void				OnRoutePacket(UdpConnection *con, const uchar *data, int dataLen);
			void						SendStatusMsg(UdpConnection *);
#else
			virtual TcpManager::TcpParams GetConnectionParams();
			virtual void				OnConnectRequest(TcpConnection *con);
			virtual void				OnTerminated(TcpConnection *con);
			virtual void				OnRoutePacket(TcpConnection *con, const unsigned char *data, int dataLen);
			void						SendStatusMsg(TcpConnection *);
#endif

			virtual unsigned			CheckMessageId(const unsigned char * data, unsigned dataLen) = 0;
			virtual unsigned			CheckTrackingNumber(const unsigned char * data, unsigned dataLen) = 0;
			virtual bool				CheckConnectReply(const unsigned char * data, unsigned dataLen) { return true; }

			bool						IsTracked(unsigned msgId);
			void						RegisterTrackedMessage(unsigned msgId);
			//	FormatStatusMsg provides a buffer/size for the derived class to write to and returns the size of the message written
			//		This function is called after connecting and periodically once connected.
			virtual unsigned			FormatStatusMsg(unsigned char * buffer, unsigned bufferLen) = 0;

			virtual void				OnConnectionOpened(const char * address) = 0;
			virtual void				OnConnectionFailed(const char * address) = 0;
			virtual void				OnConnectionClosed(const char * address, const char * reason) = 0;
			virtual void				OnConnectionShutdownNotified(const char * address, unsigned outstandingRequests) { }
			virtual void				Callback(const unsigned char * data, unsigned dataLen, unsigned messageId, void * userData) = 0;
			virtual void				Callback(unsigned messageId, unsigned trackingNumber, unsigned result, void * userData) = 0;

		private:
			ApiConnectionInfo *			GetNextUsableConnection();
			ApiConnectionInfo *			FindConnectionInfo(ApiConnection * connection);
			void						StopTrackingRequest(unsigned trackingNumber);
			void						SendShutdownNotification(ApiConnectionInfo *pInfo);
#ifdef UDP_LIBRARY
			UdpManager *				mManager;
#else
			TcpManager *				mManager;
#endif
			std::map<std::string, time_t>	mHostReconnectTimeout;
			std::list<std::string>		mIdleHosts[2];
			HostMap_t					mHostMap[2];
			ConnectionMap_t				mActiveHosts[2];
			ConnectionSet_t				mUsableHosts[2];
			ConnectionSet_t::iterator	mSendIterator[2];
			ConnectionTrackMap_t		mRequestConnections;
			TrackingList_t				mCallbackQueue;
			RequestMap_t				mRequestMap;
			TimeoutMap_t				mTimeoutMap;
			unsigned					mTrackingIndex;
			MessagIdSet_t				mTrackedMessages;
			unsigned					mTimeoutTimer;
			unsigned					mStatusTimer;
			unsigned					mLastRequestInputTime;
			unsigned					mConnectionLimit;
			unsigned					mMaxMsgSize;
			unsigned					mBufferSize;
			bool						mInCallback;
			bool						mRetriedConnection;
			bool						mShouldTryToConnect;
	};

	////////////////////////////////////////////////////////////////////////////////

	class DECLSPEC VersionMap
	{
	public:
		class InsertionObject
		{
		public:
			InsertionObject(const std::string &versionString, unsigned enumeratedValue);
			~InsertionObject() { }
		};

		static const std::string &getEarliestVersionString();
		static unsigned getEarliestVersionEnumeration();
		static const std::string &getLatestVersionString();
		static unsigned getLatestVersionEnumeration();

		static const std::string &getVersionString(unsigned enumeratedValue);
		static unsigned getEnumeratedValue(const std::string &versionString);
		static void addVersionPair(const std::string &versionString, unsigned enumeratedValue);

	private:
		VersionMap() { }
		~VersionMap() { }

		static void initializeMaps();

		static std::map<std::string, unsigned> *mspVersionStringToEnumerationMap;
		static std::map<unsigned, std::string> *mspEnumerationToVersionStringMap;
	};

	inline bool IsAtLeastVersion(unsigned version, unsigned test)
	{
		return ((version != 0) && (version >= test));
	}

	inline bool IsBeforeVersion(unsigned version, unsigned test)
	{
		return ((version != 0) && (version < test));
	}

#define ADD_VERSION_SUPPORT(__name__, __number__)	VersionMap::InsertionObject supportsVersion ## __number__(__name__, __number__)

	class DECLSPEC ConfigurationMap
	{
	public:
		class InsertionObject
		{
		public:
			InsertionObject(const std::string &label, std::string &valueHolder);
			InsertionObject(const std::string &label, soe::uint32 &valueHolder);
			InsertionObject(const std::string &label, soe::int32 &valueHolder);
			InsertionObject(const std::string &label, soe::uint16 &valueHolder);
			InsertionObject(const std::string &label, soe::int16 &valueHolder);
			InsertionObject(const std::string &label, double &valueHolder);
			InsertionObject(const std::string &label, float &valueHolder);
			~InsertionObject() { }
		};

		enum EValueType
		{
			eValueTypeUnknown,
			eValueTypeString,
			eValueTypeUInt32,
			eValueTypeInt32,
			eValueTypeUInt16,
			eValueTypeInt16,
			eValueTypeDouble,
			eValueTypeFloat
		};

		struct ConfigurationEntry
		{
			ConfigurationEntry() : valueType(eValueTypeUnknown) {}
			ConfigurationEntry(const std::string &labelIn, void * valueHolderIn, EValueType valueTypeIn);

			std::string label;
			void * valueHolder;
			EValueType valueType;
		};

		static void addConfigurationEntry(const ConfigurationEntry &entry);
		static void getConfigurationEntries(soe::NameValuePairs_t &labelsAndValues);
		static void setConfigurationEntries(const soe::NameValuePairs_t &labelsAndValues);

	private:
		ConfigurationMap() { }
		~ConfigurationMap() { }

		static void initializeMap();

		typedef std::map<soe::upperCaseString, ConfigurationEntry> LabelToEntryMap_t;
		static LabelToEntryMap_t *mspLabelToEntryMap;
	};

#define ADD_CONFIG_VAR(Type, Name, Default)												\
	DECLSPEC Type Name = Default;														\
	ConfigurationMap::InsertionObject configVariable ## Name(#Name, Name);

	/************************************************************************/
	/* Call this function after changing any configurable variables			*/
	/*	that affect class serialization										*/
	/************************************************************************/
	void ForceReinitializationOfAllClassScribes();

#ifdef API_NAMESPACE
}
#endif

//#endif
