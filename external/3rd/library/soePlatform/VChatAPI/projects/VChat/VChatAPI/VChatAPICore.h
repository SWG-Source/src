#ifndef _VCHAT_SERVER_API_CORE_H_
#define _VCHAT_SERVER_API_CORE_H_

#include "clientSerialize.h"
#include "VChatAPI.h"

#ifndef _VCHAT_API_API_HEADERS_
#define _VCHAT_API_API_HEADERS_
	#ifndef API_NAMESPACE
		#define API_NAMESPACE VChatSystem
		#include "Api/api.h"
		#undef API_NAMESPACE
	#else
		#include "Api/api.h"
	#endif
#endif

using namespace API_NAMESPACE;

namespace VChatSystem
{

    extern const char * encryptionKey;


    class VChatAPICore : public CommonAPI
    {
		private:
			VChatAPI & mClient;

        public:
            VChatAPICore(VChatAPI & client, const char * hostList, const char * failoverHostList = 0, unsigned connectionLimit = 0, unsigned maxMsgSize = 1*1024, unsigned bufferSize = 64*1024);
            virtual ~VChatAPICore();

			virtual const char *		GetVersion();
			virtual unsigned			CheckMessageId(const unsigned char * data, unsigned dataLen);
			virtual unsigned			CheckTrackingNumber(const unsigned char * data, unsigned dataLen);
			virtual unsigned 			FormatStatusMsg(unsigned char * buffer, unsigned size);
			virtual bool				CheckConnectReply(const unsigned char * data, unsigned dataLen);
			
			virtual void				OnConnectionOpened(const char * address);
			virtual void				OnConnectionFailed(const char * address);
			virtual void				OnConnectionClosed(const char * address, const char * reason);
			virtual void				OnConnectionShutdownNotified(const char * address, unsigned outstandingRequests);
			virtual void				Callback(const unsigned char * data, unsigned dataLen, unsigned messageId, void * userData);
			virtual void				Callback(unsigned messageId, unsigned trackingNumber, unsigned result, void * userData);

			HashValue_t					CreateHashValue(const std::string & name);

		private:
			VChatAPICore(const VChatAPICore &);
			VChatAPICore & operator=(const VChatAPICore &);
    };


}


#endif	//_VCHAT_SERVER_API_CORE_H_

