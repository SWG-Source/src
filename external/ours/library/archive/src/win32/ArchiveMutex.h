#ifndef	_ArchiveMutex_H
#define	_ArchiveMutex_H

#include <windows.h>

namespace Archive
{

	class ArchiveMutex
	{
	public:

		ArchiveMutex();
		~ArchiveMutex();
		void enter();
		void leave();

	private:
		ArchiveMutex(const ArchiveMutex &o);
		ArchiveMutex &operator =(const ArchiveMutex &o);

		CRITICAL_SECTION m_criticalSection;
	
	};
}

#endif
