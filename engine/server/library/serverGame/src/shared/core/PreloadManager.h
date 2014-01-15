// PreloadManager.h
// copyright 2003 Sony Online Entertainment


#ifndef	_Included_PreloadManager_H
#define	_Included_PreloadManager_H


class PreloadManager
{
public:

	static void install();
	static void remove();

private:
	PreloadManager();
	PreloadManager(const PreloadManager&);
	PreloadManager & operator= (const PreloadManager&);
	~PreloadManager();

	static bool m_installed;
};


#endif
