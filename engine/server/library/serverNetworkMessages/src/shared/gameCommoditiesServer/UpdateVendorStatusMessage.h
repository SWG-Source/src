#ifndef UpdateVendorStatusMessage_h
#define UpdateVendorStatusMessage_h

#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedFoundation/NetworkId.h"

class UpdateVendorStatusMessage : public GameNetworkMessage
{
public:
	UpdateVendorStatusMessage(
		const NetworkId &   vendorId,
		const std::string & location,
		int                 status);

	UpdateVendorStatusMessage(Archive::ReadIterator & source);
	~UpdateVendorStatusMessage();

	const NetworkId & GetVendorId() const {return m_vendorId.get();}
	const std::string & GetLocation() const {return m_location.get();}
	int GetStatus() const {return m_status.get();}

private:
	Archive::AutoVariable<NetworkId>        m_vendorId;
	Archive::AutoVariable<std::string>      m_location;
	Archive::AutoVariable<int>              m_status;
};

#endif
