#ifndef UDPLIBRARY_UDPMANAGER_H
#define UDPLIBRARY_UDPMANAGER_H

// Copyright 2004 Sony Online Entertainment, all rights reserved.
// Author: Jeff Petersen

#include "UdpPriority.h"
#include "UdpHashTable.h"
#include "UdpHelper.h"

namespace UdpLibrary
{

enum { cReliableChannelCount = 4 };
enum { cEncryptPasses = 2 };

    // encryption methods are allowed to change the packet-size, so raw packet level compression
    // would actually be implemented as a new encryption-method if needed
    // the user-supplied method requires that the both ends of the connection have the user-supplied
    // encrypt handler functions setup to correspond to each other.
enum EncryptMethod { cEncryptMethodNone
                    , cEncryptMethodUserSupplied        // use the UdpConnectionHandler::OnUserSuppliedEncrypt function
                    , cEncryptMethodUserSupplied2    // use the UdpConnectionHandler::OnUserSuppliedEncrypt2 function
                    , cEncryptMethodXorBuffer        // slower xor method, but slightly more encrypted
                    , cEncryptMethodXor                // faster using less memory, slightly less well encrypted, use this one as first choice typically
                    , cEncryptMethodCount };

enum ManagerRole { cManagerRoleDefault    // original default UdpLibrary settings
          , cManagerRoleInternalServer    // server process that is servicing multiple internal/local/high-bandwidth connections
          , cManagerRoleInternalClient    // client process that is connecting to internal servers (ie. local high-bandwidth connections)
          , cManagerRoleExternalServer    // server process that is servicing multiple external/relative-low-bandwidth connections
          , cManagerRoleExternalClient    // client process that is connection to an external server (ie. a typical end-user client setup, relatively low bandwidth)
          , cManagerRoleLfn               // highly specialized role for talking on a long-fat-network (super-high bandwidth, high-latency, slight packetloss, semi-dedicated pipe)
          };




class UdpManagerThread;

struct UdpManagerStatistics
{
    udp_int64 bytesSent;
    udp_int64 packetsSent;
    udp_int64 bytesReceived;
    udp_int64 packetsReceived;
    udp_int64 connectionRequests;
    udp_int64 crcRejectedPackets;
    udp_int64 orderRejectedPackets;
    udp_int64 duplicatePacketsReceived;
    udp_int64 resentPacketsAccelerated;         // number of times we have resent a packet due to receiving a later packet in the series
    udp_int64 resentPacketsTimedOut;            // number of times we have resent a packet due to the ack-timeout expiring
    udp_int64 priorityQueueProcessed;           // cumulative number of times a priority-queue entry has received processing time
    udp_int64 priorityQueuePossible;            // cumalative number of priority-queue entries that could have received processing time
    udp_int64 applicationPacketsSent;
    udp_int64 applicationPacketsReceived;
    udp_int64 iterations;                       // number of times GiveTime has been called
    udp_int64 corruptPacketErrors;              // number of misformed/corrupt packets
    udp_int64 socketOverflowErrors;             // number of times the socket buffer was full when a send was attempted.
    udp_int64 maxPollingTimeExceeded;           // number of times GiveTime has aborted due to time, before exhausting all data in the socket buffer
    udp_int64 maxDeliveryTimeExceeded;          // number of times DeliverEvents has aborted due to time, before exhausting all data in the event queue
    int connectionCount;          // number of connections currently being managed
    int disconnectPendingCount;   // number of connections that are pending disconnection
    int eventListCount;           // number of events that are in the callback event queue
    int eventListBytes;           // number of bytes of packet data that are in the callback event queue
    int poolCreated;        // number of packets created in the pool
    int poolAvailable;      // number of packets available in the pool
    int elapsedTime;        // how long these statistics have been gathered (in milliseconds), useful for figuring out averages
};


struct UdpReliableConfig
{
    int maxOutstandingBytes;         // maximum number of bytes that are allowed to be outstanding without an acknowledgement before more are sent (default=200k)
    int maxOutstandingPackets;       // maximum number of physical reliable packets that are allowed to be outstanding (default=400)
    int maxInstandingPackets;        // maximum number of incoming reliable packets it will queue for ordered delivery while waiting for the missing packet to arrive (should generally be same as maxOutstandingPackets setting on other side) (default=400)
    int fragmentSize;                // this is the size it should fragment large logical packets into (default=0=max allowed=maxRawPacketSize)
    int trickleSize;                 // maximum number of bytes to send per trickleRate period of time (default=0=max allowed=fragmentSize)
    int trickleRate;                 // how often trickleSize bytes are sent on the channel. (default=0=no trickle control)
    int resendDelayAdjust;           // amount of additional time (in ms) above the average ack-time before a packet should be deemed lost and resent (default=300)
    int resendDelayPercent;          // percent average ack-time it should use in calculating the resend delay (default = 125 or 125%)
    int resendDelayCap;              // maximum length of resend-delay that will ever be assigned to an outstanding packet.  (default=5000)
    int congestionWindowMinimum;     // the minimum size to allow the congestion-window to shrink.  This defaults to 0, though internally it the implementation will never let the window get smaller than a single raw packet (512 bytes by default).
                                     // This setting is more intended to allow the application to set a higher minimum than that, effectively allowing the application to tell the connection to refuse to slow itself
                                     // down as much.  See release notes for more details.

    int toleranceLossCount;          // the number of resend-accellerated packets in a frame that is severe enough to constitute a resetting of the flow control window.
                                     // typically this number is set in extremely high bandwidth (LFN) situations where some small amount of packetloss should not
                                     // reset the flow-control window.  (default = 0, which means that even is a single lost packet will reset the window, which matches previously UdpLibrary and TCP like behavior)
                                     // note: setting this too high can cause it to be unfriendly to other connections sharing the network.

    bool outOfOrder;                 // whether incoming packets on this channel should be allowed to be delivered out of order (default=false)
    bool coalesce;                   // whether the reliable-channel should attempt to coalesce data to reduce ack's needed (note: rarely change this to false)(default=true)
    bool ackDeduping;                // whether ack-packets stuck into the low-level multi-buffer should be deduped (note: rarely change this to false)(default=true)
};

    // these functions allow you to manipulate the line-simulation parameters on the fly
struct UdpSimulationParameters
{
    UdpSimulationParameters();


            // the following parameters are used to simulate various line conditions that may occur. This allows the application
            // program to test how well it performs under various conditions
            // simulating an incoming byte-rate is simply done by internally not polling the port for a certain period of time
            // after receiving a packet based on the amount of data in the last packet received
            // simulating outgoing byte-rate is a bit more difficult as it requires queuing the data to be sent and slowly trickling
            // it out to the socket.  Simulating is very much akin to simulating lag; however, it is not exactly the same thing.  It's
            // possible to have low-bandwidth, yet good or bad ping times depending on how many hops away you are, but this should good
            // enough for most testing.
            // default = 0
    float simulateOutgoingLossPercent;       // 0=no loss, 100=100% loss
    float simulateIncomingLossPercent;       // 0=no loss, 100=100% loss
    int simulateIncomingByteRate;            // simulates the incoming bandwidth specified (in bytes/second) (0=off)


        // simulates the outgoing bandwidth specified (in bytes/second) (0=off)
        // an interesting side-note of the outgoing byte-rate limit.  In order to accomplish
        // this we to queue stuff on the client and then pace the sending of it.  When the client
        // terminates the connection, the last thing it does is send a terminate packet to the other side
        // The UdpManager is playing the role of the operating system when byte-limiting is on insofar as it
        // is the UdpManager that is managing the virutal socket buffer.  Most of the time when the client
        // terminates, it also terminates the UdpManager.  When you are using the outgoing byte rate simulator,
        // terminating the UdpManager is akin to rebooting the box.  The net effect is that any outgong packets
        // in the queue are lost.  This includes the ever important 'terminate-packet' that is sent to the server
        // at the last moment.  This means that when you are simulating an outgoing byte rate, that the server
        // will never see the client properly terminate, UNLESS the client gives the UdpManager processing time
        // for several seconds (long enough to flush the queue at the specified byte-rate) after the connection
        // is destroyed, but before the UdpManager is destroyed.
        // note: when using this setting, it is critical that you also set the simulateOutgoingOverloadLevel as
        // well in order to simulate the limited-size socket buffer that the OS would have.
    int simulateOutgoingByteRate;


        // this is the number of bytes in the outgoing queue total before packets are simply lost
        // this number is used to simulate the condition where you can't simply throw tons of data at a modem
        // and expect it to not lose any of it.  Normally this data would queue up in the socket buffer for however
        // big your outgoing socket buffer is; however, if you are simulating limited outgoing bandwidth, then
        // the socket buffer never actually has any data in it, instead all the queuing is taking place in the
        // simulation queue.  In order to handle this properly, we have to cap the outgoing simulation queue size
        // total for all connection.
    int simulateOutgoingOverloadLevel;


        // this is the number of bytes in the outgoing queue/connection before packets are simply lost (0=infinite)
        // this number is used to simulate the condition where some router or terminal-server down
        // the line has a limited buffer size and once it gets overloaded it just throws away new incoming
        // packets.  The reason we have to do this is because otherwise our internal simulation queue
        // would grow forever.  The loss of packets is necessary for the flow control stuff to do its
        // job.  The incoming side uses the socket-buffer for this purpose and as such can simply
        // set the incoming socket buffer to the desired size and the OS will throw away stuff that comes
        // in yet doesn't fit.  Since this parameter is supposed to simulate a line condition downward
        // toward the destination and we may have multiple destinations in our queue, this handles it properly
        // by tracking the amount of data queued for each destination ip/port address (it does this by actually
        // back-link up to the UdpConnection object).
    int simulateDestinationOverloadLevel;


        // this simulates outgoing packet latency.  That is, if you set this value to 100, it will act as if every
        // packet sent lived on the wire for 100ms longer than it actually did.  That is, the library will hold the raw
        // packet for 100ms before putting it on the wire, such that it simulates the packet having been on the wire for
        // 100ms longer than it actually was.
    int simulateOutgoingLatency;

        // this simulates incoming packet latency.  That is, if you set this value to 100, it will act as if every
        // packet received off the wire took 100ms longer than it actually did.  That is, the library will hold the raw
        // packet for 100ms before processing it in any way.
    int simulateIncomingLatency;
};

struct UdpParams
{
    UdpParams(ManagerRole role = cManagerRoleDefault);        // constructor merely sets default values for the structure members

            // instead of specifying a callbackConnectRequest pointer, you can also specify a handler object to receive
            // the callback directly.  To have the UdpManager call your object directly when connection requests come in, you
            // simply need to derive your class (multiply if necessary) from UdpManagerHandler, then you can set this
            // pointer equal to your object and the UdpManager will call it as appropriate.  The UdpConnection object
            // also has a handler mechanism that replaces the other callback functions below, see UdpConnection::SetHandler
            // if a handler is specified, the callback function is ignored, even if specified.
            // default = nullptr (not used)
    UdpManagerHandler *handler;

            // this is the maximum number of connections that can be established by this manager, any incoming/outgoing connections
            // over this limit will be refused.  On the client side, this typically only needs to be set to 1, though there
            // is little harm in setting this number larger.
            // default = 10
    int maxConnections;

            // this is the port number that this manager will use for all incoming and outgoing data. On the client side
            // this is typically set to 0, which causes the manager object to randomly pick an available port.  On the server
            // side, this port should be set to a specific value as it will represent the port number that clients will use
            // to connect to the server (ie. the listening port).  It's generally a good idea to give the user on the client
            // side the option of fixing this port number at a specific value as well as it is often necessary for them to
            // do so in order to navigate company firewalls which may have specific port numbers open to them for this purpose.
            // default = 0
    int port;

            // if port is not zero, setting this to a value greater than zero causes it to randomly pick a port in the range
            // (port to port+portRange).  This is desireable when you wish to have the manager bind to a random port within
            // a specified range.  If you specify a portRange to use, then port must not be 0 (since 0 means to let the OS
            // pick the port to use).  0=don't use a range, bind only to the port specified in 'port' setting.
            // default = 0
    int portRange;

            // the size of the outgoing socket buffer.  The client will want to set this fairly small (32k or so), but the server
            // will want to set this fairly large (512k)
            // default = 64k
    int outgoingBufferSize;

            // the size of the incoming socket buffer.  The client will want to set this fairly small (32k or so), but the server
            // will want to set this fairly large (512k)
            // default = 64k
    int incomingBufferSize;                                

            // the purpose of the packet history is to make debugging easier.  Sometimes a processed packet will cause the server
            // to crash (due to a bug or possibly just corruption).  Typically the application will put an exception handler around
            // the main loop and call UdpManager::DumpPacketHistory when it is triggered.  This will dump a history of the last
            // packets received such that they can be analyzed by hand to determine if one of them caused the problem and why.
            // The packet history is done at the raw packet level (before logical packet re-assembly).  This is the number of raw
            // packets to buffer in the history.  Typically this can be set fairly small (maybe 1000) since packets older than
            // that have little debug value. (uses maxRawPacketSize * thisValue of memory)
            // default = 4
    int packetHistoryMax;

            // how long a connection will wait before sending a keep-alive packet to the other side of the connection
            // set this to 0 to never send keep-alive packets (typically the server will set this to 0 and never keep alive
            // the client, but the client will typically set this some value that will ensure that the server will not kick them
            // for inactivity.  Keep alive packets are only sent if no other data is being sent.
            // default = 0
    int keepAliveDelay;

            // this is very similar to the keep alive delay, but serves a completely different purpose and may not have the desired
            // effect on all platforms.  The purpose of the keepAliveDelay above is generally to keep data flowing to the server
            // so it knows that the client is still there.  In this manner if the server doesn't get data within a certain period
            // of time, it can know that the connection is probably dead.  Sometimes it is the case that the server does not need
            // to be kept alive, or at least kept alive very often (like for a chat server perhaps where nobody is talking much).
            // For some people, it may be necessary to send data more frequently in order to keep their NAT mapping fresh, or their
            // firewall software happy.  However, we don't want to be in a situation where our server is receiving a lot more data
            // than it needs to just so these people can keep their port open.  I have seen NAT's that lose mappings in as short
            // as 20 seconds.  What this feature does is a bit tricky.  It changes the time-to-live (TTL) for a special keep-alive
            // packet to some small value (4) which is enough for the packet to get past firewalls and NAT's, but not make it all
            // the way to our server.  In this manner, the port gets kept alive, but we don't waste bandwidth with these packets.
            // These special packets are not counted statistically in any way and they do not reset any timers of any kind.  Their 
            // sole purpose is to keep the port alive on the client side.  Any other data (include standard keepAlive packets will
            // reset the timer for this packet, so obviously the portAliveDelay must be smaller than the keepAliveDelay in order
            // to be meaningful.
            // default = 0 = off
    int portAliveDelay;

            // whether this UdpManager should send back an unreachable-connection packet when it receives incoming data from
            // a destination it does not have a UdpConnection object representing.  This is the equivalent of a port-unreachable
            // ICMP error packet, only taken up one level further to the virtual-connection object (UdpConnection).  Imagine a
            // server that has terminated a client's connection, but the client didn't get notice that such termination had occurred.
            // The client would still think the connection was good and continue to try and send data to the server, but the data
            // would simply be getting lost and queued up indefinately on the client.  The client doesn't get port unreachable errors
            // since the server is using the same port for lots of people.  Having this true causes the server to notify the client
            // that its connection is dead.  If a client finds out it is terminated in this manner, it will set its disconnect reason
            // to cDisconnectReasonUnreachableConnection.
            // default = true
    bool replyUnreachableConnection;

            // the UdpLibrary supports a feature whereby a connection can remap its ip/port on-the-fly.  How it works is, if a client
            // gets back a connection-unreachable error, it will send out a request for the server to remap its connection to the
            // new port that it is coming from.  This will happen if a NAT expires and the next outgoing packet causes a different
            // port-mapping to be selected.  The UdpLibrary can recover from this situation.  The icmpErrorRetryPeriod must be
            // set to a reasonably high value, such that the client has time to send data and request the remapping before icmp
            // errors are processed.  This value determines not only whether a server will honor a remapping request, but also
            // whether the client will attempt a remapping request.  The terms client as server are used loosely here, as it is
            // possible for the server to request the remapping (though it is extremely unlikely that the server end
            // of a connection will have its port changed on-the-fly).
            // default = true
    bool allowPortRemapping;        

            // this is the same as allowPortRemapping, only it allows the full IP address to be remapped on-the-fly.  I
            // recommend that you do NOT enable this, as it represents a fairly serious security loophole, whereby a hacker could
            // cause random people to be disconnected, and in theory possibly even hi-jack their connection.  The odds of this
            // actually being able to happen are incredibly rare unless the hacker has been snooping your packet stream, as they
            // would effectively have to guess two 32-bit random numbers (one generated by the client and one generated by the server)
            // default = false
    bool allowAddressRemapping;


            // how long (in milliseconds) the manager will ignore ICMP errors to a particular connection before it will act upon
            // them and terminate the connection.  When a packet is successfully received into the connection in question, the
            // retry period is reset, such that the next time an ICMP error comes in, it has another period of time for it
            // to resolve the issue.  This servers a couple purpose, 1) it can allow for momentary outages that can be recovered
            // from in short order, and 2) it is necessary for the port-remapping feature to work properly in situations where
            // the server may send data to the old-port before the remapping negotiations are completed.  In order for the
            // remapping feature to work properly, this value should be set to larger than the longest time a connection typically
            // goes without receiving data from the other side. (0=no grace period)
            // default = 5000
    int icmpErrorRetryPeriod;

            // how long a connection will go without receiving any data before it will terminate the connection
            // set this to 0 to never have the connection terminated automatically for receiving no data.
            // the application will receive an OnTerminated callback when the connection is terminated due to this timeout
            // this setting can be overridden on a per-connection basis by calling UdpConnection::SetNoDataTimeout
            // default = 0
    int noDataTimeout;
    
            // when reliable data is sent, it remains in the queue until it is acknowledged by the other side.  When you query
            // the reliable channel status, you can find out the age of the oldest piece of data in the queue that has been sent
            // yet has not been acknowledged yet.  As a general rule, if things are operating correctly, it should be very rare
            // for something that has been sent to not be acknowledged within a few seconds, even if resending had to occur.
            // Eventually, the sender could use this statistic to determine that the other side is no longer talking and terminate
            // the connection.  In past version of the UdpLibrary, the sending-application has checked this statistic itself.  This
            // parameter will cause the UdpLibrary to monitor this for you and automatically change the connection to a disconnected
            // state when the value goes over this setting (in milliseconds) (0 means do not perform this check at all)
            // The default is set fairly liberally, on client-side connections, you could safely set this to as low as 30 seconds
            // allowing for quicker realization of lost connections.  Often times the connection will realize it is dead much quicker
            // for other reasons.  When disconnected due to this, the disconnect reason is set to cDisconnectReasonUnacknowledgedTimeout
            // default = 90000
    int oldestUnacknowledgedTimeout;

            // maximum number of bytes that are allowed to be pending in all the reliable channels combined
            // before it will terminate the connection with a cDisconnectReasonReliableOverflow.  If this value is set to
            // zero, then it will never overflow (it will run out of memory and crash first).  If your application wants
            // to do something other than disconnect on this condition, then the application will have to periodically check the status
            // itself using UdpConnection::TotalPendingBytes and act as appropriate.
            // default = 0
    int reliableOverflowBytes;

            // how long a connection will hold onto outgoing data in hopes of bundling together future outgoing data in the same
            // raw packet (specified in milliseconds)
            // setting this to 0 will cause it to effectively flush at the end of every frame.  This is generally desireable in
            // cases where frame-rates are slow (less than 10 fps), or for internal LAN connections.
            // default = 50
    int maxDataHoldTime;

            // how much data a connection will hold onto before sending out a raw packet
            // (0=no multi-packet buffering, all application sends result in immediate raw packet sends)
            // (-1=use same value as maxRawPacketSize)
            // this value will be effectively ignored if it is larger than the maxRawPacketSize specified below)
            // default = -1
    int maxDataHoldSize;

            // this is the maximum raw physical packet size that the UdpLibrary will attempt to send or can receive.
            // In very old versions of the UdpLibrary, both sides needed to have this value configured identically.  In the
            // newer versions, this value is negotiated during connection negotiation and the actual size used is the smaller
            // value of what the client has configured and what the server has configured.  As a general rule, any connection
            // that talks off of the local-area-network should leave this at the default value of 512.  Internal server to server
            // connections should set this number to 1460 bytes, as that represents the maximum size that can fit in a normal
            // ethernet frame.  If you are unsure, set it to 512, since this value is guaranteed to work on any IP based network.
            // Larger values will make more efficient use of internal network resources if you know that all connectionsn are
            // going to be on our internal network.
            // default = 512, minimum = 64
    int maxRawPacketSize;

            // how large the hash-table is for looking up connections.  It takes 4*hashTableSize memory and it is recommended
            // you set it fairly large to prevent collisions (10 times maximum number of connections should be fine)
            // default = 100
    int hashTableSize;

            // whether a priority queue should be used.  If a priority queue is not used, then everytime
            // UdpManager::GiveTime is called, every UdpConnection object gets processing time as well.
            // It is thought that if traffic is heavy enough, that managing the priority queue may end up being more
            // cpu time than giving everybody time (as it is possible everybody would end up getting time anyways)
            // if not using a priority queue, it is recommended that you GiveTime only periodically (every 50ms for example)
            // the more often you GiveTime, the more critical the priority-queue is at reducing load compared to not using it
            // default = false
    bool avoidPriorityQueue;

            // how often the client synchronizes its timing-clock to the servers (0=never)(specified in ms).
            // the server-side MUST specify this as 0, or else the server will attempt to synchronize it's clock with
            // the client as well (which would just be a waste of packets generally, though would work)
            // the client should generally always set this feature on by setting it to something like 45000 ms.
            // the clock-sync is used to negotiate statistics as well, so if you are not using clock sync, then you
            // will not be able to get packetloss/lag statistics for the connection.  If you are using it, then you will
            // be able to get these statistics from either end of the connection.
            // default = 0
    int clockSyncDelay;

            // when the UdpManager is destroyed and the socket is closed, all pending data in the socket is destroyed
            // (because we are UDP and non-blocking, there is no lingering time).  The problem is, on the client side it
            // is often the case that connection and manager are destroyed simultaneously.  This can often result in the
            // final terminate-packet not making it to the server since the terminate packet will often get thrown away
            // when the socket buffer is destroyed.  This delay will cause the UdpManager to sleep momentarily (for however
            // long is specified in this option) in order to give the OS time to flush out that socket buffer before it
            // destroys it.  I wish there were a way to select on whether a socket buffer was empty, but there isn't, and there
            // is no way to query how full an outgoing socket buffer is.  As such, the only way to have a fair-chance of deliver
            // of this final packet is to simply give up our time-slice momentarily and hope the OS pumps out that final message
            // to the network before we wake up and destroy the socket.  By default we will have it sleep for 10ms, which does
            // little more than give up our time-slice, but should be enough to get the job done in most cases.  Setting this
            // value to 0 will cause it to not linger at all.  Again, however long you set this value to is how long the
            // UdpManager destructor will block.
            // default=10
    int lingerDelay;


            // these two values control the number of packets that the UdpManager will create in its packet pool.  The packet pool
            // is a means by which the UdpManager avoid allocating logical packets for every send, by instead using them from the pool.
            // you need to specify the size of the packets that are in the pool.  Then, when somebody calls UdpManager::CreatePacket,
            // if the packet being created is small enough and there is room available, it grabs one from the pool, otherwise it
            // creates a new non-pooled one.  The largest the pool will ever grow is pooledPacketMax, and the memory used will be roughly
            // pooledPacketMax * pooledPacketSize.  You should be generous with the pool packet size and the pool max in order
            // to avoid having to do allocations as much as possible
            // pooledPacketMax default = 1000, (0 = don't use memory pooling at all)
    int pooledPacketMax;
            // pooledPacketInitial is the number of packets to allocate in the initial pool.  The only reason to set this to 
            // something other than the default, which is 0, is to avoid having your memory fragmented as the pool grows on demand.
    int pooledPacketInitial;
            // as a general rule, ou should leave this at -1.  This is critical.  If the pooledPacketSize is smaller than the
            // maxRawPacketSize, then all the coalescing that occurs in the reliable channel will result in allocations.  You
            // would just as well not have a pool if you don't set this at least as large as the maxRawPacketSize.  If your application
            // tends to send largish packets (larger than maxRawPacketSize), setting this large enough to cover those might buy you
            // some speed as well.
            // pooledPacketSize defaults to -1, which means use the same as maxRawPacketSize
    int pooledPacketSize;

            // the maximum number of entries to allow in the callback event pool.  The callback event pool is only used
            // if the application is queuing callback events for later delivery in the main thread.  This pool serves as
            // an optimization by providing a pool of these objects to use.  Generally, this number should be set as large
            // as the highest number of callback events that might come in between calls to have the events delivered.
            // We do not pre-create these things, they are just created on demand.
            // default = 5000
    int callbackEventPoolMax;

            // whether ICMP error packets should be used to determine if a connection has gone dead.  
            // when the destination machine is not available, or there is no process on the destination machine
            // talking on the port, then a ICMP error packet will sometimes be returned to the client when a packet is sent.
            // Processing ICMP errors will often allow the client machine to quickly determine that the other end of the
            // connection is no longer reachable, allowing it to quickly change to a disconnected state.  The downside
            // of having this feature enabled is that it is possible that if there is a network problem along the route, that
            // the connection will be terminated, even though the hardware along the route may correct the problem by re-routing
            // within a couple seconds.  If you are having problems with clients getting disconnected for ICMP errors (see
            // disconnect reason), and you know the server should have remained reachable the entire time, then you might
            // want to set this setting to false.  The only downside of setting this to false is that it might take the
            // client a bit longer to realize when a server goes down.
            // default = true
    bool processIcmpErrors;

            // whether ICMP errors should be used to terminate connection negotiations.  By default, this is set to 
            // false, since generally when you are trying to establish a new connection (ie. negotiating), you are
            // are willing to wait for timeout specified in the EstablishConnection call, since it may be a case
            // that the client process gets started slightly sooner than the server process.
            // default = false
    bool processIcmpErrorsDuringNegotiating;


            // during connection negotiation, the client sends connect-attempt packets on a periodic basis until the
            // server responds acknowledging the connection.  This value represents how often the client sends those packets.
            // By default this is set to 1000 or 1 second and generally should not be messed with.
    int connectAttemptDelay;

            // if you call UdpManager::ThreadStart to start a background thread going calling GiveTime, it will sleep for
            // this amount of time each iteration.  Generally there is no reason to change this value as going much faster
            // doesn't buy you much, and going much slower doesn't save you much either.
            // default = 20
    int threadSleepTime;


            // This settings allows you to bind the socket used by the library to a specific IP address on the machine.
            // Normally, and by default, the library will bind the socket to any address in the machine.  This setting should
            // not be messed with unless you really know what you are doing.  In multi-homed machines it is generally NOT
            // necessary to bind to a particular address, even if there are firewall issues involved, and even if you want
            // to limit traffic to a particular network (firewalls do a better job of serving that purpose).  If you are having
            // problems communicating with a server on a multi-homed machine and think this might solve the problem, think again.
            // You most likely need to configure the OS to route data appropriately, or make sure that internal network clients
            // are connecting to the machine via the machines internal IP address (or vice versa).
            // by default this string is empty meaning the socket is bound to any address.  To bind to specific IP address, it
            // should be entered in a.b.c.d form (DNS names are not allowed).  Figuring out what IP addresses are in the machine
            // and which one should be bound to is left as an exercise for the user.
    char bindIpAddress[32];

            // you need to specify the characteristics of the various reliable channels here, generally you will want
            // to make sure the client and server sides set these characteristics the same, though it is technically not
            // required.  Each channel decides locally whether it will accept out of order delivery on a particular channel or not.
            // (note: out of order delivery is a tiny optimization that simply lets the channel deliver the packet the moment it
            // arrives, even if previous packets have not yet arrived).  Likewise trickle-rates are for outgoing data only obviously.
            // reliable channel managers are not actually created internally until data is actually sent on the channel, so there
            // is no overhead associated with channels that are not used, and you need not specify characteristics for channels
            // that you know you will not be using.
            // default = 400 packets in&out/200k outstanding, ordered, no trickle (all channels)
    UdpReliableConfig reliable[cReliableChannelCount];


            // when user supplied encrypt and decrypt routines are specified, it becomes necessary to tell the UdpManager
            // how many bytes the encryption process could possibly expand the chunk of data it was given.  Often times this    
            // will simply be 0, but if the user supplied routines attempt compression, then it's possible that expansion could
            // could actually occur.  Typically I would have the compression routines put a leader-byte on the chunk of data
            // specifying whether it was compressed or not.  Then if the compression didn't pan out, it could alway abort and just
            // prepend the byte in question and the rest of the data.  In that sort of algorithm, you would set this value to 1
            // since the most it could ever expand beyond the original is the 1-byte indicator on the front.  It's possible that
            // a particular encryption routine might want to append a 4-byte encryption key on the end of the chunk of data, in
            // which case you would need to reserve 4 bytes.  This is necessary as it allows the udp library to guarantee that
            // no packet will be larger than maxRawPacketSize, and at the same time ensures that the destination buffers supplied
            // to the encrypt/decrypt functions will have enough room.  Obviously this value is ignored if the encryptMethod
            // is not set to cEncryptMethodUserSupplied.
            // default = 0
    int userSuppliedEncryptExpansionBytes;
    int userSuppliedEncryptExpansionBytes2;


            // pointer to driver-object to use for this manager.  Typically, this driver object will create
            // the UdpPlatformDriver object itself and chain the calls on through, plus do whatever else it wants;
            // however, that is not required.  The application maintains ownership of this object and the object must
            // not be destroyed by the application until the UdpManager using it is destroyed.
            // default = nullptr, meaning the UdpManager it will create it's own UdpPlatformDriver for use.
    UdpDriver *udpDriver;


            // whether the UdpManager should be operating in event-queuing mode.  This setting can be changed on the fly
            // by the application through the SetEventQueuing function call.  When in event queuing mode, all callbacks
            // to the application via either the UdpConnectionHandler or the UdpManagerHandler are queued and only sent
            // when the application makes a DeliverEvents call (or turns queuing mode off).  Queuing these events is not
            // quite as efficient as processing them inline, so should generally only be done when the threading benefits
            // are needed.  The main purpose of this feature is such that a background thread can GiveTime to the UdpManager
            // on a regulard basis, yet the applications main thread can call DeliverEvents only at times it is capable of
            // processing the packets.  If your application's main loop consistently spins fast enough (say 10 fps), then I
            // recommend not spinning a thread to give time to the UdpManager, and instead giving time in the main thread
            // and operating in non-queuing mode.  See SetEventQueuing function call for more details.
            // default = false
    bool eventQueuing;


            // largest application packet to expect incoming on the wire.  This should generally be set liberally as it will
            // disconnect any connection that tries to send a larger packet than this.  The purpose of this setting it to
            // prevent hackers from fiddling with the packet-stream and make the server think it has an enomous (gigabytes)
            // packet coming in, as it will allocate a buffer to hold the incoming packet, which could lead to a server crash.
            // Since extremely large applications packets are rare, I have set the default limit to somewhat low, since most
            // applications won't bother to mess with this setting.  If you legitimately send super-big packets, you may need
            // to increase this number
            // default = 10mb
    int incomingLogicalPacketMax;


        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        // the following values are ignored by connections initiated by this manager (ie. client side) since the server will tell 
        // the client the values that are in effect during the connection initialization process.
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

            // how many additional bytes of CRC data is added to raw packets to ensure integrity (0 through 4 allowed)
            // default = 0
    int crcBytes;

            // which encryption method is to be used (see enumeration) (this occurs at the raw packet level)
            // default = cEncryptMethodNone
    EncryptMethod encryptMethod[cEncryptPasses];
};



    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // The purpose of the UdpManager is to manage a set of connections that are coming in on a particular port.
    // Typically an application will only have one UdpManager taking care of all incoming connections.  The
    // exception is if the application is talking to two distinct sets of individuals.  For example, the leaf
    // server application might have a UdpManager to manage the connections to all the users/players who will
    // be connecting.  It may then have a second UdpManager to manage its connection to a master server
    // someplace (though in theory it could use one UdpManager for everything).
    //
    // The UdpManager owns the solitary socket that all data being sent/received by any of the managed connections uses.
    // When the UdpManager is created, it is given a port-number that it uses for this purpose.  The UdpManager is capable
    // of establishing new connections to other UdpManager, or it is also capable of accepting new connections.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

class UdpManager : public UdpGuardedRefCount
{
    public:
        enum ErrorCondition { cErrorConditionNone, cErrorConditionCouldNotAllocateSocket, cErrorConditionCouldNotBindSocket };

        typedef UdpParams Params;                   // DEPRECATED: in here for backwards compatibility

        enum { cProtocolVersion = 2 };        // protocol version must match on both ends, or connect packets are simply ignored by the server
        enum { cHardMaxRawPacketSize = 8192 };
        enum { cHardMaxOutstandingPackets = 30000 };    // don't change this

        UdpManager(const UdpParams *params);

        void SetHandler(UdpManagerHandler *handler);
        UdpManagerHandler *GetHandler() const;
        void SetPassThroughData(void *passThroughData);
        void *GetPassThroughData() const;
        ErrorCondition GetErrorCondition() const;

            // previous UdpMisc stuff that now needs driver
        udp_ushort LocalSyncStampShort();                        // gets a local-clock based sync-stamp. (only good for timings up to 32 seconds)
        udp_uint LocalSyncStampLong();                            // gets a local-clock based sync-stamp. (good for timings up to 23 days)
        int ClockElapsed(UdpClockStamp stamp);                    // returns a elapsed time since stamp in milliseconds (if elapsed is over 23 days, it returns 23 days)
        UdpClockStamp Clock();
        int Random();
        void Sleep(int milliseconds);   // chains on through to the driver object


            // this function MUST be called on a regular basis in order to give the manager object time to service the socket
            // and give time to various connection objects that may need processing time to send/resend packets, etc.
            //
            // If you set maxPollingTime to 0, it will not attempt to receive any packets out of the socket.  If no
            // packets are gotten from the socket, then it is impossible for any packets to be forwarded to the application;
            // hence, this is a good way to give the manager processing time for outgoing packets in situations
            // where the application does not want to have to worry about processing incoming packets.
            //
            // If you set maxPollingTime to -1, it will poll at most 1 packet out of the socket at a time and process it.  You
            // should know that a single packet on the socket can possibly result in multiple data-packets being delivered to the application.
            // For example, the incoming packet could be a previous lost reliable packet, which would cause all the stalled reliable packets to be
            // delivered.  Multi-buffering could also result in multiple route-packet callbacks from one physical packet coming off the socket.
            // The purpose of this 1-packet feature is primarily to support the EstablishConnection process.  By processing only one packet at a
            // time, it gives the application a chance to check the status of a negotiating connection before any data is possibly delivered to
            // that connection.  This allows the client-app to loop on GetStatus checking for the connection to get established, then create all
            // the infrastructure necessary to support that connection once it is established, without fear of packets being delivered before said
            // infrastructure is in place.  This is safe since connection-confirmation packets do not multi-buffer up.
            //
            // The only reason you might want to avoid giving the connections time is if you are a fairly non-packet-time critical application
            // and are using the avoidPriorityQueue setting (and want to avoid using up too much CPU time while at the same
            // time you don't want to avoid processing the socket).
            // returns true if any incoming packets were processed during this time slice, otherwise returns false
        bool GiveTime(int maxPollingTime = 500, bool giveConnectionsTime = true);


            // when in event queuing mode, callback events are all queued by the GiveTime function for later deliver.  The application
            // must call DeliverEvents in order to get these events deliver.  If there are events queued and you turn off queuing mode,
            // the next call to GiveTime will cause the queued events to actually be delivered.
            // note: specifying 0 for maxProcessingTime causes it to deliver at most one event during that call.  This can be
            // used in situations where the application wants to get control back after each event processed.
        void DeliverEvents(int maxProcessingTime = 500);

            // gets/sets the event queuing mode.  By default the event queuing mode is off.  The application must explicity
            // call SetEventQueuing(true) to turn on event queuing.  Generally, event queuing is used if the application sets up
            // a worker thread to give the UdpManager processing time, but doesn't want events delivered during that GiveTime call.
            // The main application thread that does want to receive deliver of the packets then calls DeliverEvents explicitly when
            // it wants the events delivered.  The main loop need not ever call GiveTime if they have a worker-thread doing it.
        void SetEventQueuing(bool on);
        bool GetEventQueuing() const;

            // changes the UdpParams::maxDataHoldTime setting on the fly
        void SetMaxDataHoldTime(int milliseconds);
        int GetMaxDataHoldTime() const;

            // start/stop a thread from automatically calling GiveTime in the background.
            // you will generally want to make sure you are in event queuing mode if you enable threads, otherwise
            // you will receive callbacks asynchronously from the other thread.  Normally you will want to call
            // DeliverEvents instead of GiveTime to cause the queued events to be delivered.
        void ThreadStart();
        void ThreadStop();

            // used to establish a connection to a server that is listening at the specified address and port.
            // the serverAddress will do a DNS lookup as appropriate.  This call will block long enough to resolve
            // the DNS lookup, but then will return a UdpConnection object that will be in a cStatusNegotiating
            // state until the connection is actually established.  The application must give the manager
            // object time after calling EstablishConnection or else the negotiation process to establish the
            // connection will never have time to actually occur.  Typically the client establishing the connection
            // will call EstablishConnection, then sit in a loop calling UdpManager::GiveTime and checking to see
            // if the status of the returned UdpConnection object is changed from cStatusNegotiating.  This allows
            // the application to look for the ESC key or timeout an attempted connection.
            // This function will return nullptr if the manager object has exceeded its maximum number of connections
            // or if the serverAddress cannot be resolved to an IP address
            // as is noted in the declaration, it is the responsibility of the application establishing the connection to delete it
            // setting the timeout value (in milliseconds) to something greater than 0 will cause the UdpConnection object to change
            // from a cStatusNegotiating state to a cStatusDisconnected state after the timeout has expired.  It will also cause
            // the connect-complete callback to be called.
            // note: if serverPort is 0, then serverAddress MUST contain the port-number appended to the address in the form ":port"
            // for example: myserver.com:5000 or 127.0.0.1:5000
        UdpConnection *EstablishConnection(const char *serverAddress, int serverPort = 0, int timeout = 0);

            // gets statistical information about this manager, which covers all connections going through this manager 
            // (useful for getting server-wide statistics)
        void GetStats(UdpManagerStatistics *stats);
        void ResetStats();

            // this function will dump the packet history out to the specified filename.  If no packet history is being kept (see UdmManagerParams::packetHistoryMax)
            // then this function does nothing.
        void DumpPacketHistory(const char *filename) const;

            // returns the ip address of this machine.  If the machine is multi-homed, this value may be blank.
        UdpPlatformAddress GetLocalIp() const;

            // returns the port the manager is actually using.  This value will be the same as is specified in Params::port (or if Params::port was set to 0, this will be the dynamically assigned port number)
        int GetLocalPort() const;

            // return a string representing the local ip and port the manager is actually using
        char *GetLocalString(char *buf, int bufLen) const;

            // returns how long it has been (in milliseconds) since this manager last received data
        int LastReceive();

            // returns how long it has been (in milliseconds) since this manager last sent data
        int LastSend();

            // manually forces all live connections to flush their multi buffers immediately
        void FlushAllMultiBuffer();

            // forces all connections to disconnect.  If an application is not sure that all connections are disconnected
            // before destroying the UdpManager, it is recommended that they call this first, as it will generally clean
            // up the connections better by increasing the chances that the other side of the connection will receive a
            // a terminated packet.
        void DisconnectAll();

            // creates a logical packet and populates it with data.  data can be nullptr, in which case it gives you logical packet
            // of the size specified, but copies no data into it.  If you are using pool management (see Params::poolPacketMax),
            // it will give you a packet out of the pool if possible, otherwise it will create a packet for you.  When logical
            // are packets are needed internally for various things (like reliable channel sends that use the (void *, int) interface)
            // they are gotten from this function, so your application can likely take advantage of pooling, even if it never bothers
            // to explicitly call this function.
        LogicalPacket *CreatePacket(const void *data, int dataLen, const void *data2 = nullptr, int dataLen2 = 0);

        void GetSimulation(UdpSimulationParameters *simulationParameters) const;
        void SetSimulation(const UdpSimulationParameters *simulationParameters);

    protected:
        friend class PooledLogicalPacket;
        void PoolReturn(/*gift*/ PooledLogicalPacket *packet);        // so pooled packets can add themselves back to the pool.  Note: this steals the reference for thread-safety reasons.
        void PoolCreated(PooledLogicalPacket *packet);
        void PoolDestroyed(PooledLogicalPacket *packet);

    protected:
        friend class UdpReliableChannel;

        class PacketHistoryEntry
        {
            public:
                PacketHistoryEntry(int maxRawPacketSize);
                ~PacketHistoryEntry();

            public:
                udp_uchar *mBuffer;
                UdpPlatformAddress mIp;
                int mPort;
                int mLen;
        };

        UdpParams mParams;
        UdpSimulationParameters mSimulation;
        PacketHistoryEntry **mPacketHistory;
        int mPacketHistoryPosition;
        int mRandomSeed;
        int mEventListBytes;
        UdpManagerThread *mBackgroundThread;

        void *mPassThroughData;

        UdpLinkedList<UdpConnection> mConnectionList;
        UdpLinkedList<UdpConnection> mDisconnectPendingList;    // linked list of connections to be released when they go to cStatusDisconnected

        ObjectHashTable<UdpConnection, HashTableMember1<UdpConnection> > *mAddressHashTable;
        ObjectHashTable<UdpConnection, HashTableMember2<UdpConnection> > *mConnectCodeHashTable;
        PriorityQueue<UdpConnection, UdpClockStamp> *mPriorityQueue;
        UdpClockStamp mMinimumScheduledStamp;        // soonest anybody is allowed to schedule themselves for more processing time

        UdpDriver *mDriver;
        bool mInsideGiveTime;
        UdpClockStamp mLastReceiveTime;
        UdpClockStamp mLastSendTime;
        UdpClockStamp mLastEmptySocketBufferStamp;
        int mProcessingInducedLag;      // how long the currently being processed packet could have possibly been sitting in the socket-buffer waiting for processing (which is effectively how long it has been since we last quit polling packets from the socket queue)
        ErrorCondition mErrorCondition;
        UdpClockStamp mCachedClock;

        UdpPlatformGuardObject mCachedClockGuard;
        UdpPlatformGuardObject mThreadGuard;
        UdpPlatformGuardObject mPoolGuard;
        UdpPlatformGuardObject mEventListGuard;
        UdpPlatformGuardObject mAvailableEventGuard;
        UdpPlatformGuardObject mStatsGuard;
        UdpPlatformGuardObject mDisconnectPendingGuard;
        UdpPlatformGuardObject mSimulateGuard;
        mutable UdpPlatformGuardObject mGiveTimeGuard;
        mutable UdpPlatformGuardObject mConnectionGuard;
        mutable UdpPlatformGuardObject mHandlerGuard;

        UdpManagerStatistics mManagerStats;
        UdpClockStamp mManagerStatsResetTime;

            // stuff for managing the outgoing packet-rate simulation
        class SimulateQueueEntry
        {
            public:
                SimulateQueueEntry(const udp_uchar *data, int dataLen, UdpPlatformAddress ip, int port, UdpClockStamp queueTime);
                ~SimulateQueueEntry();
            public:
                UdpLinkedListMember<SimulateQueueEntry> mLink;
                udp_uchar *mData;
                int mDataLen;
                UdpPlatformAddress mIp;
                int mPort;
                UdpClockStamp mQueueTime;
        };
        friend class SimulateQueueEntry;
        int mSimulateOutgoingQueueBytes;
        UdpClockStamp mSimulateNextIncomingTime;
        UdpClockStamp mSimulateNextOutgoingTime;

        UdpLinkedList<SimulateQueueEntry> mSimulateIncomingList;
        UdpLinkedList<SimulateQueueEntry> mSimulateOutgoingList;

            // pool management
        UdpLinkedList<PooledLogicalPacket> mPoolCreatedList;
        UdpLinkedList<PooledLogicalPacket> mPoolAvailableList;

    protected:
        class CallbackEvent
        {
            public:
                enum CallbackEventType { cCallbackEventNone, cCallbackEventRoutePacket, cCallbackEventConnectComplete
                                      , cCallbackEventTerminated, cCallbackEventCrcReject, cCallbackEventPacketCorrupt
                                      , cCallbackEventConnectRequest };

                CallbackEvent();
                ~CallbackEvent();
                void SetEventData(CallbackEventType eventType, UdpConnection *con, const LogicalPacket *payload = nullptr);
                void ClearEventData();

                CallbackEventType mEventType;
                UdpConnection *mSource;
                const LogicalPacket *mPayload;
                UdpCorruptionReason mReason;    // only used by CorruptPacket event
                UdpLinkedListMember<CallbackEvent> mLink;
        };

            // event queuing
        CallbackEvent *AvailableEventBorrow();
        void AvailableEventReturn(CallbackEvent *ce);
        void EventListAppend(CallbackEvent *ce);
        CallbackEvent *EventListPop();

        UdpLinkedList<CallbackEvent> mAvailableEventList;
        UdpLinkedList<CallbackEvent> mEventList;


    protected:        // internal functions
        int AddressHashValue(UdpPlatformAddress ip, int port) const;
        UdpConnection *AddressGetConnection(UdpPlatformAddress ip, int port) const;   // NOTE: returns an AddRef'ed connection, caller must release it
        UdpConnection *ConnectCodeGetConnection(int connectCode) const;         // NOTE: returns an AddRef'ed connection, caller must release it

        PacketHistoryEntry *SimulationReceive();
        PacketHistoryEntry *ActualReceive();
        void ActualSendHelper(const udp_uchar *data, int dataLen, UdpPlatformAddress ip, int port);
        void ProcessRawPacket(const PacketHistoryEntry *e);
        void ProcessDisconnectPending();
        void CloseSocket();
        void CreateAndBindSocket(int usePort);
        void CallbackConnectRequest(UdpConnection *con);
        UdpClockStamp LastSendTime();

    protected:    // functions called by the UdpConnection
        friend class UdpConnection;

        void KeepUntilDisconnected(UdpConnection *con);
        void AddConnection(UdpConnection *con);
        void RemoveConnection(UdpConnection *con);
        void ActualSend(const udp_uchar *data, int dataLen, UdpPlatformAddress ip, int port);
        void SendPortAlive(UdpPlatformAddress ip, int port);
        void SetPriority(UdpConnection *con, UdpClockStamp stamp);
        void CallbackRoutePacket(UdpConnection *con, const udp_uchar *data, int dataLen);
        void CallbackConnectComplete(UdpConnection *con);
        void CallbackTerminated(UdpConnection *con);
        void CallbackCrcReject(UdpConnection *con, const udp_uchar *data, int dataLen);
        void CallbackPacketCorrupt(UdpConnection *con, const udp_uchar *data, int dataLen, UdpCorruptionReason reason);

        void IncrementCrcRejectedPackets() { UdpGuard sg(&mStatsGuard); mManagerStats.crcRejectedPackets++; }
        void IncrementOrderRejectedPackets() { UdpGuard sg(&mStatsGuard); mManagerStats.orderRejectedPackets++; }
        void IncrementDuplicatePacketsReceived() { UdpGuard sg(&mStatsGuard); mManagerStats.duplicatePacketsReceived++; }
        void IncrementResentPacketsAccelerated() { UdpGuard sg(&mStatsGuard); mManagerStats.resentPacketsAccelerated++; }
        void IncrementResentPacketsTimedOut() { UdpGuard sg(&mStatsGuard); mManagerStats.resentPacketsTimedOut++; }
        void IncrementApplicationPacketsSent() { UdpGuard sg(&mStatsGuard); mManagerStats.applicationPacketsSent++; }
        void IncrementApplicationPacketsReceived() { UdpGuard sg(&mStatsGuard); mManagerStats.applicationPacketsReceived++; }
        void IncrementCorruptPacketErrors() { UdpGuard sg(&mStatsGuard); mManagerStats.corruptPacketErrors++; }

        int EncryptUserSupplied(UdpConnection *con, udp_uchar *destData, const udp_uchar *sourceData, int sourceLen);
        int EncryptUserSupplied2(UdpConnection *con, udp_uchar *destData, const udp_uchar *sourceData, int sourceLen);
        int DecryptUserSupplied(UdpConnection *con, udp_uchar *destData, const udp_uchar *sourceData, int sourceLen);
        int DecryptUserSupplied2(UdpConnection *con, udp_uchar *destData, const udp_uchar *sourceData, int sourceLen);

            // CachedClock is only updated once when a real Clock call is made, but is faster than a regular Clock call since it does 
            // not have to call out to the driver.  The UdpLibrary implementation makes use of the cached clock whenever possible so as to
            // reduce the number of calls to the driver.  The main motivation for this was to reduce the amount of data that needed
            // to be stored by the UdpDriverRecord driver.
        UdpClockStamp CachedClock();
        int CachedClockElapsed(UdpClockStamp start);
    private:
        ~UdpManager();    // does not destroy UdpConnection objects since it does not own them; however, it has a pointer to all the active
                        // ones and it will notify them that it no longer exists and set their state to cStatusDisconnected
                        // typically it is recommended that all UdpConnection objects be destroyed before destroying this manager object
};


class UdpManagerThread : public UdpPlatformThreadObject
{
    public:
        UdpManagerThread(UdpManager *manager, int sleepTime);
        ~UdpManagerThread();
        virtual void Run();
        void Stop(bool waitUntilStopped);

    private:
        UdpManager *mUdpManager;
        bool mStop;
        int mSleepTime;
};


        /////////////////////////////////////////////////////////////////////////
        // inline implementations
        /////////////////////////////////////////////////////////////////////////

           // UdpManager
inline int UdpManager::LastReceive()
{
    UdpGuard guard(&mStatsGuard);
    return(CachedClockElapsed(mLastReceiveTime));
}

inline int UdpManager::LastSend()
{
    UdpGuard guard(&mStatsGuard);
    return(CachedClockElapsed(mLastSendTime));
}

inline UdpClockStamp UdpManager::LastSendTime()
{
    UdpGuard guard(&mStatsGuard);
    return(mLastSendTime);
}

inline int UdpManager::AddressHashValue(UdpPlatformAddress ip, int port) const
{
    return((int)((ip.GetHash() ^ port) & 0x7fffffff));
}

inline void UdpManager::SetPriority(UdpConnection *con, UdpClockStamp stamp)
{
    UdpGuard myGuard(&mConnectionGuard);

        // do not ever let anybody schedule themselves for processing time sooner then mMinimumScheduledStamp
        // otherwise, they could end up getting processing time multiple times in a single UdpManager::GiveTime iteration
        // of the priority queue, which under odd circumstances could result in an infinite loop
    if (stamp < mMinimumScheduledStamp)
        stamp = mMinimumScheduledStamp;

    if (mPriorityQueue != nullptr)
    {
        mPriorityQueue->Add(con, stamp);
    }
}

inline void UdpManager::SetHandler(UdpManagerHandler *handler)
{
    UdpGuard guard(&mHandlerGuard);
    mParams.handler = handler;
}

inline UdpManagerHandler *UdpManager::GetHandler() const
{
    UdpGuard guard(&mHandlerGuard);
    return(mParams.handler);
}

inline void UdpManager::PoolReturn(/*gift*/ PooledLogicalPacket *packet)
{
    UdpGuard guard(&mPoolGuard);
    if (mPoolAvailableList.Count() < mParams.pooledPacketMax)
    {
        mPoolAvailableList.InsertHead(packet);
    }
    else
    {
        packet->TrueRelease();
    }
}

inline void UdpManager::SetPassThroughData(void *passThroughData)
{
    UdpGuard guard(&mGiveTimeGuard);      // probably no reason we couldn't change this during a GiveTime, but it is so rarely called it won't matter
    mPassThroughData = passThroughData;
}

inline void *UdpManager::GetPassThroughData() const
{
    UdpGuard guard(&mGiveTimeGuard);
    return(mPassThroughData);
}

inline void UdpManager::SetMaxDataHoldTime(int milliseconds)
{
      // not need to thread-protect this, it can't really screw anything up no matter how it is set
    mParams.maxDataHoldTime = milliseconds;
}

inline int UdpManager::GetMaxDataHoldTime() const
{
    return(mParams.maxDataHoldTime);
}

inline UdpClockStamp UdpManager::Clock()
{
    UdpGuard guard(&mCachedClockGuard);
    return(mCachedClock = mDriver->Clock());
}

inline int UdpManager::ClockElapsed(UdpClockStamp stamp)
{
    return(UdpMisc::ClockDiff(stamp, Clock()));
}

inline udp_ushort UdpManager::LocalSyncStampShort()
{
    return((udp_ushort)(Clock() & 0xffff));
}

inline udp_uint UdpManager::LocalSyncStampLong()
{
    return((udp_uint)(Clock() & 0xffffffff));
}

inline int UdpManager::Random()
{
    return(UdpMisc::Random(&mRandomSeed));
}

inline void UdpManager::Sleep(int milliseconds)
{
    mDriver->Sleep(milliseconds);
}

inline UdpClockStamp UdpManager::CachedClock()
{
    UdpGuard guard(&mCachedClockGuard);
    return(mCachedClock);
}

inline int UdpManager::CachedClockElapsed(UdpClockStamp start)
{
    return(UdpMisc::ClockDiff(start, CachedClock()));
}

inline int UdpManager::EncryptUserSupplied(UdpConnection *con, udp_uchar *destData, const udp_uchar *sourceData, int sourceLen)
{
    UdpGuard guard(&mHandlerGuard);
    if (mParams.handler != nullptr)
    {
        return(mParams.handler->OnUserSuppliedEncrypt(con, destData, sourceData, sourceLen));
    }
    return(0);
}

inline int UdpManager::EncryptUserSupplied2(UdpConnection *con, udp_uchar *destData, const udp_uchar *sourceData, int sourceLen)
{
    UdpGuard guard(&mHandlerGuard);
    if (mParams.handler != nullptr)
    {
        return(mParams.handler->OnUserSuppliedEncrypt2(con, destData, sourceData, sourceLen));
    }
    return(0);
}

inline int UdpManager::DecryptUserSupplied(UdpConnection *con, udp_uchar *destData, const udp_uchar *sourceData, int sourceLen)
{
    UdpGuard guard(&mHandlerGuard);
    if (mParams.handler != nullptr)
    {
        return(mParams.handler->OnUserSuppliedDecrypt(con, destData, sourceData, sourceLen));
    }
    return(0);
}

inline int UdpManager::DecryptUserSupplied2(UdpConnection *con, udp_uchar *destData, const udp_uchar *sourceData, int sourceLen)
{
    UdpGuard guard(&mHandlerGuard);
    if (mParams.handler != nullptr)
    {
        return(mParams.handler->OnUserSuppliedDecrypt2(con, destData, sourceData, sourceLen));
    }
    return(0);
}




    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // UdpManager::Params initializations constructor (ie. default values)
    /////////////////////////////////////////////////////////////////////////////////////////////////////
inline UdpParams::UdpParams(ManagerRole role)
{
    handler = nullptr;
    outgoingBufferSize = 64 * 1024;
    incomingBufferSize = 64 * 1024;
    packetHistoryMax = 4;
    maxDataHoldTime = 50;
    maxDataHoldSize = -1;
    maxRawPacketSize = 512;
    hashTableSize = 100;
    avoidPriorityQueue = false;
    clockSyncDelay = 0;
    crcBytes = 0;
    encryptMethod[0] = cEncryptMethodNone;
    encryptMethod[1] = cEncryptMethodNone;
    keepAliveDelay = 0;
    portAliveDelay = 0;
    noDataTimeout = 0;
    maxConnections = 10;
    port = 0;
    portRange = 0;
    pooledPacketMax = 1000;
    pooledPacketSize = -1;
    pooledPacketInitial = 0;
    replyUnreachableConnection = true;
    allowPortRemapping = true;
    allowAddressRemapping = false;
    icmpErrorRetryPeriod = 5000;
    oldestUnacknowledgedTimeout = 90000;
    processIcmpErrors = true;
    processIcmpErrorsDuringNegotiating = false;
    connectAttemptDelay = 1000;
    reliableOverflowBytes = 0;
    lingerDelay = 10;
    bindIpAddress[0] = 0;
    udpDriver = nullptr;
    callbackEventPoolMax = 5000;
    eventQueuing = false;
    threadSleepTime = 20;
    incomingLogicalPacketMax = 20 * 1024 * 1024;

    userSuppliedEncryptExpansionBytes = 0;
    userSuppliedEncryptExpansionBytes2 = 0;

    reliable[0].maxInstandingPackets = 400;
    reliable[0].maxOutstandingBytes = 200 * 1024;
    reliable[0].maxOutstandingPackets = 400;
    reliable[0].outOfOrder = false;
    reliable[0].coalesce = true;
    reliable[0].ackDeduping = true;
    reliable[0].fragmentSize = 0;
    reliable[0].resendDelayAdjust = 300;
    reliable[0].resendDelayPercent = 125;
    reliable[0].resendDelayCap = 8000;
    reliable[0].toleranceLossCount = 0;
    reliable[0].congestionWindowMinimum = 0;
    reliable[0].trickleRate = 0;
    reliable[0].trickleSize = 0;

    switch (role)
    {
        case cManagerRoleInternalServer:
            outgoingBufferSize = 4 * 1024 * 1024;
            incomingBufferSize = 4 * 1024 * 1024;
            crcBytes = 2;
            icmpErrorRetryPeriod = 500;
            maxRawPacketSize = 1460;
            hashTableSize = 10000;
            keepAliveDelay = 30000;
            noDataTimeout = 90000;
            maxConnections = 2000;
            pooledPacketMax = 20000;
            pooledPacketInitial = 1000;
            allowPortRemapping = false;
            reliable[0].maxInstandingPackets = 1000;
            reliable[0].maxOutstandingBytes = 1024 * 1024;
            reliable[0].maxOutstandingPackets = 1000;
            reliable[0].congestionWindowMinimum = 8192;
            reliable[0].resendDelayAdjust = 150;
            break;
        case cManagerRoleInternalClient:
            outgoingBufferSize = 1024 * 1024;
            incomingBufferSize = 1024 * 1024;
            crcBytes = 2;
            icmpErrorRetryPeriod = 500;
            maxRawPacketSize = 1460;
            hashTableSize = 10;
            keepAliveDelay = 30000;
            noDataTimeout = 90000;
            maxConnections = 2;
            pooledPacketMax = 2000;
            pooledPacketInitial = 100;
            allowPortRemapping = false;
            reliable[0].maxInstandingPackets = 1000;
            reliable[0].maxOutstandingBytes = 1024 * 1024;
            reliable[0].maxOutstandingPackets = 1000;
            reliable[0].congestionWindowMinimum = 8192;
            reliable[0].resendDelayAdjust = 150;
            break;
        case cManagerRoleExternalServer:
            outgoingBufferSize = 2 * 1024 * 1024;
            incomingBufferSize = 2 * 1024 * 1024;
            crcBytes = 2;
            icmpErrorRetryPeriod = 2500;
            hashTableSize = 10000;
            keepAliveDelay = 30000;
            noDataTimeout = 90000;
            maxConnections = 2000;
            pooledPacketMax = 20000;
            pooledPacketInitial = 1000;
            break;
        case cManagerRoleExternalClient:
            crcBytes = 2;
            icmpErrorRetryPeriod = 2500;
            hashTableSize = 10;
            keepAliveDelay = 30000;
            noDataTimeout = 90000;
            maxConnections = 2;
            pooledPacketMax = 2000;
            pooledPacketInitial = 10;
            break;
        case cManagerRoleLfn:
            outgoingBufferSize = 16 * 1024 * 1024;
            incomingBufferSize = 16 * 1024 * 1024;
            crcBytes = 2;
            icmpErrorRetryPeriod = 1500;
            maxRawPacketSize = 1460;
            keepAliveDelay = 30000;
            noDataTimeout = 90000;
            maxConnections = 2;
            pooledPacketMax = 50000;
            pooledPacketInitial = 5000;
            allowPortRemapping = false;
            incomingLogicalPacketMax = 200 * 1024 * 1024;
            callbackEventPoolMax = 50000;
            reliable[0].maxInstandingPackets = 32000;
            reliable[0].maxOutstandingBytes = 50 * 1024 * 1024;
            reliable[0].maxOutstandingPackets = 32000;
            reliable[0].congestionWindowMinimum = 300000;
            reliable[0].toleranceLossCount = 100;
            break;
        case cManagerRoleDefault:
        default:
            break;
    }

        // all reliable channels should match main channel
    for (int j = 1; j < cReliableChannelCount; j++)
        reliable[j] = reliable[0];
}

}   // namespace

#endif
