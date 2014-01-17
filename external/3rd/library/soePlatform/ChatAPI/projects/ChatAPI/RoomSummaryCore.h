#if !defined (ROOMSUMMARYCORE_H_)
#define ROOMSUMMARYCORE_H_

#include <Unicode/Unicode.h>

#include "ChatEnum.h"

namespace ChatSystem {


struct RoomSummaryCore
{
	Plat_Unicode::String m_address;
	Plat_Unicode::String m_topic;


	unsigned m_attributes;
	unsigned m_curSize;
	unsigned m_maxSize;
};

};
#endif


