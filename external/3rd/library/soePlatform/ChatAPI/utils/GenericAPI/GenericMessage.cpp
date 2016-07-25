#include "GenericMessage.h"

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE
{
#endif
	namespace GenericAPI
	{
		GenericMessage::GenericMessage(short type)
			: m_type(type)
		{
		}

		GenericRequest::GenericRequest(short type)
			: GenericMessage(type)
		{
		}

		GenericResponse::GenericResponse(short type, unsigned result, void *user)
			: GenericMessage(type),
			m_result(result),
			m_user(user),
			m_track(0),
			m_timeout(100)
		{
		}
	};

#ifdef EXTERNAL_DISTRO
};
#endif