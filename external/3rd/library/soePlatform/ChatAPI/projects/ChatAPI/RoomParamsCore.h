#if !defined (ROOMPARAMSCORE_H_)
#define ROOMPARAMSCORE_H_

#include <Unicode/Unicode.h>

#include "ChatEnum.h"

namespace ChatSystem {


struct RoomParamsCore
{
	Plat_Unicode::String m_name;
	ChatUnicodeString m_cName;
	Plat_Unicode::String m_topic;
	ChatUnicodeString m_cTopic;
	Plat_Unicode::String m_password;
	ChatUnicodeString m_cPassword;
	
	unsigned m_attributes;
	unsigned m_size;
};

};

#endif


