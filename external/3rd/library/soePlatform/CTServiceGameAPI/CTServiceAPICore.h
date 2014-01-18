#ifndef APICORE_H
#define APICORE_H

#pragma warning (disable : 4786)

#include "CTGenericAPI/GenericApiCore.h"
#include "CTGenericAPI/GenericConnection.h"
#include <Base/Archive.h>

namespace CTService 
{

class CTServiceAPI;

//---------------------------------------------------
class CTServiceAPICore : public GenericAPICore
//---------------------------------------------------
{
public:
	CTServiceAPICore(const char *hostName[], const short port[], int count, CTServiceAPI *api, const char *game);
	virtual ~CTServiceAPICore();

	void OnConnect(GenericConnection *con);
	void OnDisconnect(GenericConnection *con);
    void responseCallback(GenericResponse *res);
    void responseCallback(short type, Base::ByteStream::ReadIterator &iter, GenericConnection *con);


private:
	CTServiceAPI	*m_api;
	unsigned		m_mappedServerTrack;
};

}; // namespace

#endif	//APICORE_H



