#ifndef UDPLIBRARY_UDPLOGICALPACKET_H
#define UDPLIBRARY_UDPLOGICALPACKET_H

// Copyright 2004 Sony Online Entertainment, all rights reserved.
// Author: Jeff Petersen

#include "UdpHandler.h"
#include "UdpLinkedList.h"

namespace UdpLibrary
{

class SimpleLogicalPacket;
class GroupLogicalPacket;


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // The purpose of the LogicalPacket class is to provide means whereby multiple connections can share
    // a queued packet to save having each connection make its own copy.  It is highly recommended that
    // logical packets be used for optimal performance when sending reliable data as well, since internally
    // it will just end up creating one for you anyhow (LogicalPackets are used by the reliable layer to
    // hold onto the data until it is acknowledged)
    //
    // Logical packets passed in are never modified by the udp library.
    // After calling Send, application should immediately Release the logical packet if they don't need it for
    // something else (like sending to another connection).  The udp library will addref it if it decided it
    // wants to keep it around past the Send call (for reliable data, it will always hang onto it, for unreliable
    // data, it will only hang onto it if it gets promoted to reliable due to size.)
    //
    // Application is encouraged to derive their own application-specific packet classes from the LogicalPacket
    // The object is required to be able to provide a pointer to the raw packet data that will remain valid
    // for as long as the LogicalPacket object exists.
    //
    // for example:
    //   class PlayerLoginPacket : public LogicalPacket
    //   {
    //        public:
    //          PlayerLoginPacket(char *userName, char *password) { mData.packetType = cPacketTypePlayerLogin; strcpy(mData.userName, userName); strcpy(mData.password, password); }
    //           virtual const void *GetDataPtr() const { return(&mData); }
    //           virtual int GetDataLen() const { return(sizeof(mData)); }
    //       protected:
    //           virtual ~PlayerLoginPacket() {};
    //           struct
    //           {
    //               udp_uchar packetType;
    //               char userName[32];
    //               char password[32];
    //           } mData;
    //   };
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
class LogicalPacket : public UdpRefCount
{
    public:
        LogicalPacket();

        virtual void *GetDataPtr() = 0;
        virtual const void *GetDataPtr() const = 0;
        virtual int GetDataLen() const = 0;
        virtual void SetDataLen(int len) = 0;

            // returns true if this logical packet is an internal packet
            // by default this returns false, and applications should not override this function to do
            // otherwise.  Basically, if this returns true, it tells the udp library that the packet has
            // an internal-packet header (starts with a zero byte) on it that should not be escaped when sent.
            // The packet will ultimately be processed by the internal udp library code on the other side.
            // The purpose of this is to support features such as GroupLogicalPacket, such that the application
            // can prep grouped packets and then send them via traditional means (it's how the udp library
            // knows that the packet is a group packet instead of just a application-packet that happens to start with 0)
        virtual bool IsInternalPacket() const;
    protected:
        virtual ~LogicalPacket();            // protected since the class should only be deleted by Releasing it

    private:
        friend class UdpReliableChannel;
        UdpLinkedListMember<LogicalPacket> mReliableLink;  // used by reliable channel object to hold a list of these packets
};

class SimpleLogicalPacket : public LogicalPacket
{
        // this class is a simply dynamically allocated packet.  Applications are welcome to use it, but
        // it was originally created to allow the internal code to handle reliable data that was sent
        // via the Send(char *, int) api call.
    public:
        SimpleLogicalPacket(const void *data, int dataLen);        // data can be nullptr if you want to populate it after it is allocated (get the pointer and write to it)
        virtual void *GetDataPtr();
        virtual const void *GetDataPtr() const;
        virtual int GetDataLen() const;
        virtual void SetDataLen(int len);
    protected:
        virtual ~SimpleLogicalPacket();
        udp_uchar *mData;
        int mDataLen;
};

class WrappedLogicalPacket : public LogicalPacket
{
        // this class is used internally by the library such that it can get a free and clear mNext pointer
        // in cases where a logical packet is being shared by multiple connections (such that the packet
        // can reside in the linked-list for the reliable channel queue)
    public:
        WrappedLogicalPacket(UdpManager *manager);    // lives in a pool

        virtual void AddRef() const;
        virtual void Release() const;
        virtual void *GetDataPtr();
        virtual const void *GetDataPtr() const;
        virtual int GetDataLen() const;
        virtual void SetDataLen(int len);
    protected:
        virtual ~WrappedLogicalPacket();
        const LogicalPacket *mPacket;

    protected:
        friend class UdpManager;
        friend class UdpReliableChannel;
        void TrueRelease() const;
        void SetLogicalPacket(const LogicalPacket *packet);
        UdpManager *mUdpManager;
        UdpLinkedListMember<WrappedLogicalPacket> mAvailableLink;  // for available linked list in manager (used by reliable channel object as well while it is borrowed)
        UdpLinkedListMember<WrappedLogicalPacket> mCreatedLink;    // for created linked list in manager
};

template<int t_quickSize> class FixedLogicalPacket : public LogicalPacket
{
        // this class is similar to the SimpleLogicalPacket in that it is designed to just store raw
        // data of various sizes.  The difference here is that this class may be created at compile
        // time to be any size desired (via template parameters), and is more efficient than the
        // SimpleLogicalPacket when used (it avoid the internal alloc as the data is inline).  The
        // UdpMisc::CreateQuickLogicalPacket function automatically attempts to use this for small
        // packets and will fall back and use SimpleLogicalPackets for larger packets.
    public:
        FixedLogicalPacket(const void *data, int dataLen);
        virtual void *GetDataPtr();
        virtual const void *GetDataPtr() const;
        virtual int GetDataLen() const;
        virtual void SetDataLen(int len);
    protected:
        udp_uchar mData[t_quickSize];
        int mDataLen;
};

template<typename T> class StructLogicalPacket : public LogicalPacket
{
        // this class is designed to turn any raw struct into a logical packet.  The application
        // can then access the struct through thisLogicalPacket->mStruct.structMember.  The template
        // will take care of all the wrappings necessary to turn the struct into a fully functional
        // logical packet that can be sent to a connection.  As a word of caution, sending struct's
        // across the wire is not considered very portable.  There are byte-ordering and structure-packing
        // issues that are platform dependent.  At a minimum, you will probably want to make sure that
        // struct's you use in this way are packed to 1-byte boundaries, such that you are not sending
        // wasteful information.  Do not attempt to send the member-content of classes (particularly classes 
        // with virtual functions) via this method as they may contain hidden data-members (such as pointers
        // to vtables).
    public:
        StructLogicalPacket(T *initData = nullptr);
        virtual void *GetDataPtr();
        virtual const void *GetDataPtr() const;
        virtual int GetDataLen() const;
        virtual void SetDataLen(int len);
    protected:
        T mStruct;
};

class GroupLogicalPacket : public LogicalPacket
{
        // this class is a helper object intended for use by the application (it is not used internally)
        // It allows you to add multiple application-packets and them send them all as an autonomous unit.
        // The receiving end will automatically split the packet up and send them to the application callback function
        // as if the individual packets had all been sent one at at time.
        //
        // This facility is primarily intended for reliable data, though it can be used to group unreliable data as well.
        // Grouping unreliable data together would effectively give you an all-or-nothing type of delivery system.  Be
        // mindful that if the group of packets gets larger than max-raw-packet-size, it will end up getting
        // promoted to being a reliable-packet with the associated overhead involved in that.
        //
        // You might be wondering what the advantage would be of grouping packets together at the application level as opposed to
        // just letting the internal multi-buffer take care of the problem for you.  For starters, the internal multi-buffer
        // is incapable of combining partial-packets, so you end up sending less than maximum-size packets if you let the
        // internal layer take care of it.  Additionally, there is additional overhead in that if you send 100 tiny logical
        // packets, each one will need to be ack'ed by the receiving end (even though they are getting combined down at the physical-packet
        // level in order to reduce UDP overhead).  Finally, grouping them together at the higher level will allow the
        // logical-packet compression helper routines to operate on larger chunks of data at a time, which tends to improve
        // compression efficiency.  The downside to combining is that none of the application packets get delivered until the
        // entire group arrives, then all are delivered in one fell swoop...the net effect being that in order to gain these efficeincies,
        // the first-packet takes longer to be delivered to the application than it otherwise would have (while the last packet will in theory
        // get there is less time as there will be less overhead).
        //
        // Internal packets can be added to the group (though that is somewhat unlikely), which means you can add a group to a group
    public:
        GroupLogicalPacket();
        void AddPacket(const LogicalPacket *packet);        // cannot add internal logical packets to the group
        void AddPacket(const void *data, int dataLen);
        virtual void *GetDataPtr();
        virtual const void *GetDataPtr() const;
        virtual int GetDataLen() const;
        virtual void SetDataLen(int len);
        virtual bool IsInternalPacket() const;        // returns true if this is considering an internal logical packet type

    protected:
        virtual ~GroupLogicalPacket();
        void AddPacketInternal(const void *data, int dataLen, bool isInternalPacket);

        udp_uchar *mData;
        int mDataLen;
};

class PooledLogicalPacket : public LogicalPacket
{
        // a pooled logical packet is like other logical packets, only when it's refCount gets down to 1, it notifies
        // its manager to add it back to the pool.  (The manager keeps the last ref count on the packet)
    public:
        PooledLogicalPacket(UdpManager *manager, int len);

        virtual void AddRef() const;
        virtual void Release() const;
        virtual void *GetDataPtr();
        virtual const void *GetDataPtr() const;
        virtual int GetDataLen() const;
        virtual void SetDataLen(int len);
    protected:
        virtual ~PooledLogicalPacket();

        udp_uchar *mData;
        int mDataLen;
        int mMaxDataLen;
    protected:
        friend class UdpManager;
        void TrueRelease() const;
        void SetData(const void *data, int dataLen, const void *data2 = nullptr, int dataLen2 = 0);
        UdpManager *mUdpManager;
        UdpLinkedListMember<PooledLogicalPacket> mAvailableLink;  // for available linked list in manager
        UdpLinkedListMember<PooledLogicalPacket> mCreatedLink;    // for created linked list in manager
};



        /////////////////////////////////////////////////////////////////////////
        // FixedLogicalPacket implementation
        /////////////////////////////////////////////////////////////////////////
template<int t_quickSize> FixedLogicalPacket<t_quickSize>::FixedLogicalPacket(const void *data, int dataLen)
{
    mDataLen = dataLen;
    if (data != nullptr)
        memcpy(mData, data, mDataLen);
}

template<int t_quickSize> void *FixedLogicalPacket<t_quickSize>::GetDataPtr()
{
    return(mData);
}

template<int t_quickSize> const void *FixedLogicalPacket<t_quickSize>::GetDataPtr() const
{
    return(mData);
}

template<int t_quickSize> int FixedLogicalPacket<t_quickSize>::GetDataLen() const
{
    return(mDataLen);
}

template<int t_quickSize> void FixedLogicalPacket<t_quickSize>::SetDataLen(int len)
{
    mDataLen = len;
}


        /////////////////////////////////////////////////////////////////////////
        // StructLogicalPacket implementation
        /////////////////////////////////////////////////////////////////////////
template<typename T> StructLogicalPacket<T>::StructLogicalPacket(T *initData)
{
    if (initData != nullptr)
        mStruct = *initData;
}

template<typename T> void *StructLogicalPacket<T>::GetDataPtr()
{
    return(&mStruct);
}

template<typename T> const void *StructLogicalPacket<T>::GetDataPtr() const
{
    return(&mStruct);
}

template<typename T> int StructLogicalPacket<T>::GetDataLen() const
{
    return(sizeof(mStruct));
}

template<typename T> void StructLogicalPacket<T>::SetDataLen(int len)
{
}

}   // namespace


#endif
