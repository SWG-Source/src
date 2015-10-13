#ifndef CTSERVICEWEBAPITRANSACTION_H
#define CTSERVICEWEBAPITRANSACTION_H

#include <string.h>

namespace CTService 
{
//--------------------------------------------------------
	// this object is used by the webapi to send in a transaction to the CTServer.
	// see also DBTransaction.h(server only, not in webapi) and CTServicDBTransaction.h(server and webapi, in common folder)
//--------------------------------------------------------
class CTServiceWebAPITransaction
//--------------------------------------------------------
{
	inline void copy_wide_string(unsigned short * target, const unsigned short * source, int length)
	{
		if (!target)
			return;
		if (!source)
		{
			target[0] = 0;
			return;
		}

		int offset = 0;
		while (offset < length && (target[(offset+1)] = source[offset])) {
			target[length-1] = 0;
			offset++;
		}
	}
	typedef unsigned short uchar_t;

	public:
		enum {	CHARACTER_SIZE=64, CHARACTER_BUFFER, 
				GAMECODE_SIZE=25, GAMECODE_BUFFER, 
				SERVER_SIZE=64, SERVER_BUFFER, 
				REASON_SIZE=4096, REASON_BUFFER, 
				STATION_NAME_SIZE=30, STATION_NAME_BUFFER, 
				STATION_PASSWORD_SIZE=30, STATION_PASSWORD_BUFFER };
		CTServiceWebAPITransaction() : mGameCode(), mSourceServerName(), mDestServerName(), mSourceCharacterName(), mDestCharacterName(), mSourceUID(0), mDestStationName(), mDestStationPassword(), mWithItems(), mOverride()
			{ mGameCode[0]		= 0;
			  mSourceServerName[0]	= 0;
			  mDestServerName[0]	= 0;
			  mSourceCharacterName[0]	= 0;
			  mDestCharacterName[0]	= 0;
			  mDestStationName[0]	= 0;
			  mDestStationPassword[0]= 0;
			  mWithItems = false;
			  mOverride = false; }
		void			SetGameCode(const char * value)						{ if (value) strncpy(mGameCode, value, GAMECODE_BUFFER); else mGameCode[0] = 0;  }
	//	void			SetGameCode(const char * value)						{ strncpy(mGameCode, value, GAMECODE_BUFFER);  }
		const char *	GetGameCode() const									{ return mGameCode; }

		void			SetSourceServerName(const unsigned short * value)	{ copy_wide_string(mSourceServerName, value, SERVER_BUFFER); }
		const uchar_t * GetSourceServerName() const							{ return mSourceServerName; }

		void			SetDestServerName(const unsigned short * value)		{ copy_wide_string(mDestServerName, value, SERVER_BUFFER); }
		const uchar_t * GetDestServerName() const							{ return mDestServerName; }

		void			SetSourceCharacterName(const unsigned short * value){ copy_wide_string(mSourceCharacterName, value, CHARACTER_BUFFER); }
		const uchar_t * GetSourceCharacterName() const						{ return mSourceCharacterName; }

		void			SetDestCharacterName(const unsigned short * value)	{ copy_wide_string(mDestCharacterName, value, CHARACTER_BUFFER); }
		const uchar_t * GetDestCharacterName() const							{ return mDestCharacterName; }

		void			SetSourceUID(const unsigned value)					{ mSourceUID = value; }
		unsigned		GetSourceUID()										{ return mSourceUID; }

		void			SetDestStationName(const unsigned short * value)	{ copy_wide_string(mDestStationName, value, STATION_NAME_BUFFER); }
		const uchar_t * GetDestStationName() const							{ return mDestStationName; }

		void			SetDestStationPassword(const unsigned short * value){ copy_wide_string(mDestStationPassword, value, STATION_PASSWORD_BUFFER); }
		const uchar_t * GetDestStationPassword() const						{ return mDestStationPassword; }

		void			SetWithItems(const bool value)						{ mWithItems = value; }
		bool			GetWithItems()										{ return mWithItems; }

		void			SetOverride(const bool value)						{ mOverride = value; }
		bool			GetOverride()										{ return mOverride; }


	private:
		char  			mGameCode[GAMECODE_BUFFER];
		uchar_t			mSourceServerName[SERVER_BUFFER];
		uchar_t			mDestServerName[SERVER_BUFFER];
		uchar_t			mSourceCharacterName[CHARACTER_BUFFER];
		uchar_t			mDestCharacterName[CHARACTER_BUFFER];
		unsigned		mSourceUID;
		uchar_t			mDestStationName[STATION_NAME_BUFFER];
		uchar_t			mDestStationPassword[STATION_PASSWORD_BUFFER];
		bool			mWithItems;
		bool			mOverride;
	};

}; // namespace

#endif

