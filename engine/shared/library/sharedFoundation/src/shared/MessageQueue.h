// ======================================================================
//
// MessageQueue.h
// Portions copyright 1998 Bootprint Entertainment.
// Portions copyright 2000-2001 Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_MessageQueue_H
#define INCLUDED_MessageQueue_H

// ======================================================================

class MessageQueue
{
public:

	class Data
	{
	public:

		Data();
		virtual ~Data()=0;
	};

	class Notification
	{
	public:
		
		Notification();
		virtual ~Notification()=0;

		virtual void onChanged() const=0;
	};

public:

	explicit MessageQueue(int initialSize=0);
	~MessageQueue();

	int  getNumberOfMessages() const;
	void getMessage(int index, int *message, float *value, uint32 *flags=0) const;
	void getMessage(int index, int *message, float *value, Data **data, uint32 *flags=0) const;

	void setMessageFlags(int index, uint32 flags);

	void clearMessage(int index);
	void clearMessageData(int index);

	void appendMessage(int message, float value, uint32 flags=0);
	void appendMessage(int message, float value, Data *data, uint32 flags=0);

	void beginFrame();

	void setNotification(Notification* notification);

private:

	struct Message
	{
	public:

		Message();
		~Message();

	public:

		int     m_message;
		float   m_value;
		Data   *m_data;
		uint32  m_flags;
	};

private:

	typedef std::vector<Message> MessageList;

private:

	MessageQueue(const MessageQueue &);
	MessageQueue &operator =(const MessageQueue &);

	void clearDataFromMessageList(MessageList &messageList, bool destructor);

private:

	MessageList  *const m_messageQueue1;
	MessageList  *const m_messageQueue2;
	MessageList  *m_messageQueueRead;
	MessageList  *m_messageQueueWrite;
	Notification *m_notification;
};

// ======================================================================

#endif
