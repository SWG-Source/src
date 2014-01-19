// ======================================================================
//
// IoWin.h
// jeff grills
//
// copyright 1998 Bootprint Entertainment
//
// ======================================================================

#ifndef IO_WIN_H
#define IO_WIN_H

// ======================================================================

#include "sharedIoWin/IoWin.def"

// ======================================================================

class IoWin
{
	friend class IoWinManager;

private:

	// explicitly disable these routines
	IoWin(void);
	IoWin(const IoWin &);
	IoWin &operator =(const IoWin &);

private:

	char  *ioDebugName;
	IoWin *ioNext;

public:

	explicit IoWin(const char *debugName);
	virtual ~IoWin(void);

	virtual IoResult processEvent(IoEvent *event);
	virtual void     draw(void) const;

	void open(void);
	void close(void);
};

// ======================================================================

#endif

