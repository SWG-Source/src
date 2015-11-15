// ======================================================================
//
// NetworkController.cpp
// copyright 2000 Verant Interactive
//
// ======================================================================

#include "sharedObject/FirstSharedObject.h"
#include "sharedObject/NetworkController.h"

#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedNetworkMessages/MessageQueueDataTransform.h"
#include "sharedNetworkMessages/MessageQueueDataTransformWithParent.h"
#include "sharedNetworkMessages/ObjectChannelMessages.h"
#include "sharedFoundation/GameControllerMessage.h"
#include "sharedObject/Object.h"

//-----------------------------------------------------------------------

NetworkController::NetworkController(Object *const owner) :
	Controller(owner)
{
}

//-----------------------------------------------------------------------

NetworkController::~NetworkController()
{
}

//-----------------------------------------------------------------------

void NetworkController::conclude()
{
	//-- if the object is valid
    const Object *const object = getOwner ();
    if (object)
	{
		//-- if the network id is valid
		const NetworkId networkId = object->getNetworkId ();
		if (networkId != NetworkId::cms_invalid)
		{
			int const messageCount = getNumberOfMessages ();
			for (int i = 0; i < messageCount; ++i)
			{
				int    message;
				float  value;
				uint32 flags;
				MessageQueue::Data* data;
				getMessage (i, &message, &value, &data, &flags);

				if (message == 0)
				{
//					WARNING(true, ("Pulled message with id 0 from queue"));
				}
				//-- if the message is flagged for sending
				else if (flags & GameControllerMessageFlags::SEND)
				{
					DEBUG_FATAL ((flags & GameControllerMessageFlags::DESTINATIONS) == 0, ("SEND controller message %d with no destination!", message));
					
					const ObjControllerMessage c (networkId, static_cast<uint32>(message), value, flags, data);
					sendControllerMessage (c);
				}
				
				//-- delete the message queue data if it exists
				if (data)
				{
					if ((flags & GameControllerMessageFlags::SEND) != 0 ||
						(flags & GameControllerMessageFlags::SOURCE_REMOTE_SERVER) != 0 ||
						(flags & GameControllerMessageFlags::SOURCE_REMOTE_CLIENT) != 0)
					{
						clearMessageAndData (i);						
						
						delete data;  
						data = 0;
					}
				}
			}
		}
	}

	//-- chain up to the base class
	Controller::conclude ();
}

//-----------------------------------------------------------------------

bool NetworkController::getAuthoritative() const
{
	return getOwner()->isAuthoritative();
}

//-----------------------------------------------------------------------
/**
 * Processes a message.
 *
 * @param message		message id
 * @param data			message data
 * @param value			message value
 * @param time			time since last update
 * @param flags     delivery flags
 */
void NetworkController::handleMessage (const int message, const float /*value*/, const MessageQueue::Data* const data, const uint32 /*flags*/)
{
	switch (message)
	{
		case CM_netUpdateTransformWithParent:
			{
				const MessageQueueDataTransformWithParent *const messageData = dynamic_cast<const MessageQueueDataTransformWithParent *>(data);
				if (messageData)
					handleNetUpdateTransformWithParent(*messageData);
			}
			break;

		case CM_netUpdateTransform:
			{
				const MessageQueueDataTransform *const messageData = dynamic_cast<const MessageQueueDataTransform *>(data);
				if (messageData)
					handleNetUpdateTransform(*messageData);
			}
			break;
		default:
			break;
	}
}

//-----------------------------------------------------------------------

/**
 * Sets this controller's object to be authoritative.
 *
 * @param authoritative		authoritative flag
 */
void NetworkController::setAuthoritative(bool authoritative)
{
	getOwner()->setAuthoritative(authoritative);
}

//-----------------------------------------------------------------------
