// ======================================================================
//
// ConfigServerBase.h
// Copyright 2004, Sony Online Entertainment Inc., all rights reserved.
// Author: Doug Mellencamp
//
// ======================================================================

#ifndef	_ConfigServerBase_H
#define	_ConfigServerBase_H

//-----------------------------------------------------------------------

class ConfigServerBase
{
  public:
    struct Data
    {
	const char *  serverName;
	int           sleepTimePerFrameMs;
	int           defaultFrameRateLimit;
	bool          showAllDebugInfo;                   // show detailed debugging logging info
    };

	static const char *  getServerName                          ();
	static int           getSleepTimePerFrameMs                 ();
	static int           getDefaultFrameRateLimit               ();
	static const bool    getShowAllDebugInfo                    ();

	static void          install                                ();
	static void          remove                                 ();

  private:
    static Data *	data;
};

// ----------------------------------------------------------------------

#endif	// _ConfigServerBase_H
