#include "serverNetworkMessages/FirstServerNetworkMessages.h"
#include "SetSalesTaxMessage.h"

SetSalesTaxMessage::SetSalesTaxMessage(Archive::ReadIterator & source) :
GameNetworkMessage("SetSalesTaxMessage"),
m_responseId(),
m_trackId(),
m_salesTax(),
m_bankId(),
m_location()
{
  AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_salesTax);	
	AutoByteStream::addVariable(m_bankId);
	AutoByteStream::addVariable(m_location);
	unpack(source);
}

SetSalesTaxMessage::SetSalesTaxMessage(
		int requestId,
		int salesTax,
		const NetworkId & bankId,
		const std::string & location
) :
GameNetworkMessage("SetSalesTaxMessage"),
m_responseId(requestId),
m_trackId(0),
m_salesTax(salesTax),
m_bankId(bankId),
m_location(location)
{
  AutoByteStream::addVariable(m_responseId);
	AutoByteStream::addVariable(m_trackId);
	AutoByteStream::addVariable(m_salesTax);	
	AutoByteStream::addVariable(m_bankId);
	AutoByteStream::addVariable(m_location);
}

SetSalesTaxMessage::~SetSalesTaxMessage()
{
}
