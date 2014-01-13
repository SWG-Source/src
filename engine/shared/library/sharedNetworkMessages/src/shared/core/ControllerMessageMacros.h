// ======================================================================
//
// ControllerMessageMacros.h
// Copyright 2002, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_ControllerMessageMacros_H
#define INCLUDED_ControllerMessageMacros_H

// ======================================================================
#include "sharedFoundation/MemoryBlockManagerMacros.h"
#include "Archive/Archive.h"

#define CONTROLLER_MESSAGE_INTERFACE \
	MEMORY_BLOCK_MANAGER_INTERFACE_WITHOUT_INSTALL; \
	public: \
		static void                   install(); \
		static void                   pack(const MessageQueue::Data* data, Archive::ByteStream & target); \
		static MessageQueue::Data*    unpack(Archive::ReadIterator & source); \
	private: \
		static void                   remove(); 

#define CONTROLLER_MESSAGE_IMPLEMENTATION(className, message) \
	MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(className, true, 0, 0, 0); \
	void className::install() \
	{ \
		installMemoryBlockManager(); \
		ControllerMessageFactory::registerControllerMessageHandler(static_cast<int>(message), pack, unpack); \
		ExitChain::add(&remove, #className "::remove"); \
	} \
	void className::remove() \
	{ \
		removeMemoryBlockManager(); \
	} 

#define CONTROLLER_MESSAGE_ALLOW_FROM_CLIENT_IMPLEMENTATION(className, message) \
	MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(className, true, 0, 0, 0); \
	void className::install() \
	{ \
		installMemoryBlockManager(); \
		ControllerMessageFactory::registerControllerMessageHandler(static_cast<int>(message), pack, unpack, true); \
		ExitChain::add(&remove, #className "::remove"); \
	} \
	void className::remove() \
	{ \
		removeMemoryBlockManager(); \
	} 

#define CONTROLLER_MESSAGE_IMPLEMENTATION_WITH_ARCHIVE(className, message) \
	MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(className, true, 0, 0, 0); \
	void className::install() \
	{ \
		installMemoryBlockManager(); \
		ControllerMessageFactory::registerControllerMessageHandler(static_cast<int>(message), pack, unpack); \
		ExitChain::add(&remove, #className "::remove"); \
	} \
	void className::remove() \
	{ \
		removeMemoryBlockManager(); \
	} \
	void className::pack(const MessageQueue::Data* const data, Archive::ByteStream & target) \
	{ \
		className ## Archive::put (data, target); \
	} \
	MessageQueue::Data* className::unpack(Archive::ReadIterator & source) \
	{ \
		return className ## Archive::get (source); \
	}	

#define CONTROLLER_MESSAGE_ALLOW_FROM_CLIENT_IMPLEMENTATION_WITH_ARCHIVE(className, message) \
	MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(className, true, 0, 0, 0); \
	void className::install() \
	{ \
		installMemoryBlockManager(); \
		ControllerMessageFactory::registerControllerMessageHandler(static_cast<int>(message), pack, unpack, true); \
		ExitChain::add(&remove, #className "::remove"); \
	} \
	void className::remove() \
	{ \
		removeMemoryBlockManager(); \
	} \
	void className::pack(const MessageQueue::Data* const data, Archive::ByteStream & target) \
	{ \
		className ## Archive::put (data, target); \
	} \
	MessageQueue::Data* className::unpack(Archive::ReadIterator & source) \
	{ \
		return className ## Archive::get (source); \
	}	

#define CONTROLLER_MESSAGE_IMPLEMENTATION_NO_INSTALL(className) \
	MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(className, true, 0, 0, 0); \
	void className::remove() \
	{ \
		removeMemoryBlockManager(); \
	} 

#define CONTROLLER_MESSAGE_IMPLEMENTATION_WITH_ARCHIVE_NO_INSTALL(className) \
	MEMORY_BLOCK_MANAGER_IMPLEMENTATION_WITHOUT_INSTALL(className, true, 0, 0, 0); \
	void className::remove() \
	{ \
		removeMemoryBlockManager(); \
	} \
	void className::pack(const MessageQueue::Data* const data, Archive::ByteStream & target) \
	{ \
		className ## Archive::put (data, target); \
	} \
	MessageQueue::Data* className::unpack(Archive::ReadIterator & source) \
	{ \
		return className ## Archive::get (source); \
	}	

#endif
