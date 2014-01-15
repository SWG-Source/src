

set(SHARED_SOURCES
	shared/core/FirstCrypto.h

	shared/original/config.h
	shared/original/cryptlib.cpp
	shared/original/cryptlib.h
	shared/original/filters.cpp
	shared/original/filters.h
	shared/original/iterhash.cpp
	shared/original/iterhash.h
	shared/original/md5.cpp
	shared/original/md5.h
	shared/original/misc.cpp
	shared/original/misc.h
	shared/original/mqueue.cpp
	shared/original/mqueue.h
	shared/original/queue.cpp
	shared/original/queue.h
	shared/original/smartptr.h
	shared/original/tftables.cpp
	shared/original/twofish.cpp
	shared/original/twofish.h
	shared/original/words.h

	shared/wrapper/CryptoBufferTransform.h
	shared/wrapper/Hash.cpp
	shared/wrapper/Hash.h
	shared/wrapper/MD5Hash.cpp
	shared/wrapper/MD5Hash.h
	shared/wrapper/TwofishCrypt.cpp
	shared/wrapper/TwofishCrypt.h
	shared/wrapper/TwofishDecryptor.cpp
	shared/wrapper/TwofishDecryptor.h
	shared/wrapper/TwofishEncryptor.cpp
	shared/wrapper/TwofishEncryptor.h
)

if(WIN32)
	set(PLATFORM_SOURCES
		win32/FirstCrypto.cpp
	)
else()
	set(PLATFORM_SOURCES "")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/shared)

add_library(crypto
	${SHARED_SOURCES}
	${PLATFORM_SOURCES}
)
