#ifndef CTSERVICEDBTRANSACTION_H
#define CTSERVICEDBTRANSACTION_H

#include <string.h>
namespace CTService 
{

//--------------------------------------------------------
	// This object is used to pass back a transaction to the webAPI. 
	//this objects intended use is for CSR functionality, see also DBTransaction.h(server only, not in webapi) and CTServiceWebAPITransaction.h(server and webapi, in common folder)
//--------------------------------------------------------
class CTServiceTransaction
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
		while (offset < length && (target[(offset+1)] = source[offset])){
			target[length-1] = 0;
			offset++;
		}
	}

	public:
		enum {	LANGUAGE_SIZE=2, LANGUAGE_BUFFER, 
				DATE_SIZE=20, DATE_BUFFER, 
				GAME_CODE_SIZE=25, GAME_CODE_BUFFER,
				NAME_SIZE=64, NAME_BUFFER,
				TRANSACTIONS_STATUS_SIZE=64, TRANSACTIONS_STATUS_BUFFER };

		CTServiceTransaction() : mTransactionID(0), mOrderID(0), mTransactionStatus(), mGameResultID(0),
									mSuggestedName(), mGameCode(), mSourceServer(), mDestServer(), 
									mSourceCharacter(), mDestCharacter(), mSourceUID(0), mDestStationName(),
									mDestUID(0), mWithItems(0), mOverride(0), mLanguage(), mTransferServerHost(), 
									mTransferServerPort(), mRetryCounter(0), mTimeoutDate(), mCreationDate(), 
									mModifyDate(), mCompletionDate()
		{
			mTransactionStatus[0] = 0; 
			mSuggestedName[0] = 0; 
			mGameCode[0] = 0; 
			mSourceServer[0] = 0; 
			mDestServer[0] = 0; 
			mSourceCharacter[0] = 0; 
			mDestCharacter[0] = 0; 
			mDestStationName[0] = 0; 
			mLanguage[0] = 0; 
			mTransferServerHost[0] = 0; 
			mTransferServerPort[0] = 0; 
		}

		// unsigned

		void			SetTransactionID(const unsigned value)				{ mTransactionID = value; }
		unsigned		GetTransactionID()									{ return mTransactionID; }

		void			SetOrderID(const unsigned value)					{ mOrderID = value; }
		unsigned		GetOrderID()										{ return mOrderID; }

		void			SetGameResultID(const unsigned value)				{ mGameResultID = value; }
		unsigned		GetGameResultID()									{ return mGameResultID; }

		void			SetSourceUID(const unsigned value)					{ mSourceUID = value; }
		unsigned		GetSourceUID()										{ return mSourceUID; }

		void			SetDestUID(const unsigned value)					{ mDestUID = value; }
		unsigned		GetDestUID()										{ return mDestUID; }

		void			SetWithItems(const unsigned value)					{ mWithItems = value; }
		unsigned		GetWithItems()										{ return mWithItems; }

		void			SetOverride(const unsigned value)					{ mOverride = value; }
		unsigned		GetOverride()										{ return mOverride; }
		
		void			SetRetryCounter(const unsigned value)				{ mRetryCounter = value; }
		unsigned		GetRetryCounter()									{ return mRetryCounter; }		

		// char

		void			SetTimeoutDate(const char * value)					{ if (value) strncpy(mTimeoutDate, value, DATE_BUFFER); else mTimeoutDate[0] = 0;  }
		const char *	GetTimeoutDate()									{ return mTimeoutDate; }

		void			SetCreationDate(const char * value)					{ if (value) strncpy(mCreationDate, value, DATE_BUFFER); else mCreationDate[0] = 0;  }
		const char *	GetCreationDate()									{ return mCreationDate; }

		void			SetModifyDate(const char * value)					{ if (value) strncpy(mModifyDate, value, DATE_BUFFER); else mModifyDate[0] = 0;  }
		const char *	GetModifyDate()										{ return mModifyDate; }

		void			SetCompletionDate(const char * value)				{ if (value) strncpy(mCompletionDate, value, DATE_BUFFER); else mCompletionDate[0] = 0;  }
		const char *	GetCompletionDate()									{ return mCompletionDate; }

		void			SetTransactionStatus(const char * value)			{ if (value) strncpy(mTransactionStatus, value, TRANSACTIONS_STATUS_BUFFER); else mTransactionStatus[0] = 0;  }
		const char *	GetTransactionStatus()								{ return mTransactionStatus; }

		void			SetGameCode(const char * value)						{ if (value) strncpy(mGameCode, value, GAME_CODE_BUFFER); else mGameCode[0] = 0;  }
		const char *	GetGameCode()										{ return mGameCode; }

		void			SetLanguage(const char * value)						{ if (value) strncpy(mLanguage, value, LANGUAGE_BUFFER); else mLanguage[0] = 0;  }
		const char *	GetLanguage()										{ return mLanguage; }

		void			SetTransferServerHost(const char * value)			{ if (value) strncpy(mTransferServerHost, value, NAME_BUFFER); else mTransferServerHost[0] = 0;  }
		const char *	GetTransferServerHost()								{ return mTransferServerHost; }

		void			SetTransferServerPort(const char * value)			{ if (value) strncpy(mTransferServerPort, value, NAME_BUFFER); else mTransferServerPort[0] = 0;  }
		const char *	GetTransferServerPort()								{ return mTransferServerPort; }

		// unsigned short

		void			SetSuggestedName(const unsigned short * value)		{ copy_wide_string(mSuggestedName, value, NAME_BUFFER); }
		unsigned short	*GetSuggestedName()									{ return mSuggestedName; }

		void			SetSourceServer(const unsigned short * value)		{ copy_wide_string(mSourceServer, value, NAME_BUFFER); }
		unsigned short	*GetSourceServer()									{ return mSourceServer; }

		void			SetDestServer(const unsigned short * value)			{ copy_wide_string(mDestServer, value, NAME_BUFFER); }
		unsigned short	*GetDestServer()									{ return mDestServer; }

		void			SetSourceCharacter(const unsigned short * value)	{ copy_wide_string(mSourceCharacter, value, NAME_BUFFER); }
		unsigned short	*GetSourceCharacter()								{ return mSourceCharacter; }

		void			SetDestCharacter(const unsigned short * value)		{ copy_wide_string(mDestCharacter, value, NAME_BUFFER); }
		unsigned short	*GetDestCharacter()									{ return mDestCharacter; }

		void			SetDestStationName(const unsigned short * value)	{ copy_wide_string(mDestStationName, value, NAME_BUFFER); }
		unsigned short	*GetDestStationName()								{ return mDestStationName; }


	private:
		unsigned		mTransactionID;
		unsigned		mOrderID;
		char			mTransactionStatus[TRANSACTIONS_STATUS_BUFFER];
		unsigned		mGameResultID;
		unsigned short	mSuggestedName[NAME_BUFFER];
		char			mGameCode[GAME_CODE_BUFFER];
		unsigned short	mSourceServer[NAME_BUFFER];
		unsigned short	mDestServer[NAME_BUFFER];
		unsigned short	mSourceCharacter[NAME_BUFFER];
		unsigned short	mDestCharacter[NAME_BUFFER];
		unsigned		mSourceUID;
		unsigned short	mDestStationName[NAME_BUFFER];
		unsigned		mDestUID;
		unsigned		mWithItems;
		unsigned		mOverride;
		char			mLanguage[LANGUAGE_BUFFER];
		char			mTransferServerHost[NAME_BUFFER];
		char			mTransferServerPort[NAME_BUFFER];
		unsigned		mRetryCounter;
		char			mTimeoutDate[DATE_BUFFER];
		char			mCreationDate[DATE_BUFFER];
		char			mModifyDate[DATE_BUFFER];
		char			mCompletionDate[DATE_BUFFER];
	};

}; // namespace

#endif	//CTSERVICEDBTRANSACTION_H
