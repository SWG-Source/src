#ifndef	_ArchiveMutex_H
#define	_ArchiveMutex_H

#include <pthread.h>

namespace Archive
{

	class ArchiveMutex
	{

	public:
		ArchiveMutex();
		~ArchiveMutex();
		void enter();
		void leave();
		
		pthread_mutex_t &getInternalMutex() { return mutex; }
	private:
		ArchiveMutex(const ArchiveMutex &o);
		ArchiveMutex &operator =(const ArchiveMutex &o);

		pthread_mutex_t mutex;
		
	};
};
	
	

#endif
