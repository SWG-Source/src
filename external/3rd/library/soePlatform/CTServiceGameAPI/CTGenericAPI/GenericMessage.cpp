#include "GenericMessage.h"

//----------------------------------------
//
// WARNING: These files are NOT standard generic API files
// They have been modified for this project.
// Do NOT replace them with generic API files
//
//----------------------------------------

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif

using namespace Base;

//-------------------------------------------
GenericRequest::GenericRequest(short type, unsigned server_track)
: m_type(type), m_server_track(server_track)
//-------------------------------------------
{
}

//-------------------------------------------
GenericResponse::GenericResponse(short type, unsigned result, void *user)
: m_type(type), m_result(result), m_user(user)
//-------------------------------------------
{
}

//-----------------------------------------
void GenericResponse::unpack(ByteStream::ReadIterator &iter)
//-----------------------------------------
{
    get(iter, m_type);
    get(iter, m_track);
    get(iter, m_result);
}

#ifdef EXTERNAL_DISTRO
};
#endif
