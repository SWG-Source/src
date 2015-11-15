#ifndef CTSERVICEDBORDER_H
#define CTSERVICEDBORDER_H

#include <string.h>
namespace CTService 
{

//--------------------------------------------------------

	// This object is used to pass back an order to the webAPI. 
	//this objects intended use is for CSR functionality, see also DBOrder.h(server only, not in webapi)
//--------------------------------------------------------
class CTServiceDBOrder
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
				ECOMM_ORDER_ID_SIZE=16, ECOMM_ORDER_ID_BUFFER, 
				ORDER_STATUS_SIZE=16, ORDER_STATUS_BUFFER, 
				CUSTOMER_NAME_SIZE=64, CUSTOMER_NAME_BUFFER,
				CUSTOMER_PHONE_SIZE=32, CUSTOMER_PHONE_BUFFER,
				CUSTOMER_EMAIL_SIZE=128, CUSTOMER_EMAIL_BUFFER,
				CARD_INFO_SIZE=255, CARD_INFO_BUFFER,
				COUNTRY_SIZE=2, COUNTRY_BUFFER,
				CARD_EXP_MONTH_SIZE=2, CARD_EXP_MONTH_BUFFER,
				CARD_EXP_YEAR_SIZE=4, CARD_EXP_YEAR_BUFFER,
				GAME_CODE_SIZE=25, GAME_CODE_BUFFER };
		CTServiceDBOrder() : mOrderID(0), mEcommOrderID(), mOrderStatus(), mLanguage(), mGameCode(),
					mStationID(0), mCustomerFirstName(), mCustomerLastName(), mCustomerPhone(),
					mCustomerEmail(), mCardName(), mCardAddress1(), mCardAddress2(), mCardCity(),
					mCardState(), mCardZip(), mCountry(), mCreationDate(), mModifyDate(), mCompletionDate(),
					mTransactionCount(0), mSuccessfulTransactions(0), mFailedTransactions(0)
		{ 				
			mEcommOrderID[0] = 0; 
			mOrderStatus[0] = 0; 
			mLanguage[0] = 0; 
			mGameCode[0] = 0;
			mCustomerFirstName[0] = 0; 
			mCustomerLastName[0] = 0;
			mCustomerPhone[0] = 0; 
			mCustomerEmail[0] = 0; 
			mCardName[0] = 0; 
			mCardAddress1[0] = 0;
			mCardAddress2[0] = 0; 
			mCardCity[0] = 0; 
			mCardState[0] = 0; 
			mCardZip[0] = 0;
			mCountry[0] = 0; 
		}

		// unsigned
		void			SetOrderID(const unsigned value)						{ mOrderID = value; }
		unsigned		GetOrderID()											{ return mOrderID; }
		
		void			SetStationID(const unsigned value)						{ mStationID = value; }
		unsigned		GetStationID()											{ return mStationID; }
		
		void			SetTransactionCount(const unsigned value)				{ mTransactionCount = value; }
		unsigned		GetTransactionCount()									{ return mTransactionCount; }
		
		void			SetSuccessfulTransactions(const unsigned value)			{ mSuccessfulTransactions = value; }
		unsigned		GetSuccessfulTransactions()								{ return mSuccessfulTransactions; }
		
		void			SetFailedTransactions(const unsigned value)				{ mFailedTransactions = value; }
		unsigned		GetFailedTransactions()									{ return mFailedTransactions; }		

		// char
		void			SetCreationDate(const char * value)						{ if (value) strncpy(mCreationDate, value, DATE_BUFFER); else mCreationDate[0] = 0;  }
		const char *	GetCreationDate()										{ return mCreationDate; }

		void			SetModifyDate(const char * value)						{ if (value) strncpy(mModifyDate, value, DATE_BUFFER); else mModifyDate[0] = 0;  }
		const char *	GetModifyDate()											{ return mModifyDate; }

		void			SetCompletionDate(const char * value)					{ if (value) strncpy(mCompletionDate, value, DATE_BUFFER); else mCompletionDate[0] = 0;  }
		const char *	GetCompletionDate()										{ return mCompletionDate; }

		void			SetEcommOrderID(const char * value)						{ if (value) strncpy(mEcommOrderID, value, ECOMM_ORDER_ID_BUFFER); else mEcommOrderID[0] = 0;  }
		const char *	GetEcommOrderID()										{ return mEcommOrderID; }

		void			SetOrderStatus(const char * value)						{ if (value) strncpy(mOrderStatus, value, ORDER_STATUS_BUFFER); else mOrderStatus[0] = 0;  }
		const char *	GetOrderStatus()										{ return mOrderStatus; }

		void			SetLanguage(const char * value)							{ if (value) strncpy(mLanguage, value, LANGUAGE_BUFFER); else mLanguage[0] = 0;  }
		const char *	GetLanguage()											{ return mLanguage; }

		void			SetGameCode(const char * value)							{ if (value) strncpy(mGameCode, value, GAME_CODE_BUFFER); else mGameCode[0] = 0;  }
		const char *	GetGameCode()											{ return mGameCode; }

		// unsigned short
		void			SetCustomerFirstName(const unsigned short * value)		{ copy_wide_string(mCustomerFirstName, value, CUSTOMER_NAME_BUFFER); }
		unsigned short	*GetCustomerFirstName()									{ return mCustomerFirstName; }

		void			SetCustomerLastName(const unsigned short * value)		{ copy_wide_string(mCustomerLastName, value, CUSTOMER_NAME_BUFFER); }
		unsigned short	*GetCustomerLastName()									{ return mCustomerLastName; }

		void			SetCustomerPhone(const unsigned short * value)			{ copy_wide_string(mCustomerPhone, value, CUSTOMER_PHONE_BUFFER); }
		unsigned short	*GetCustomerPhone()										{ return mCustomerPhone; }

		void			SetCustomerEmail(const unsigned short * value)			{ copy_wide_string(mCustomerEmail, value, CUSTOMER_EMAIL_BUFFER); }
		unsigned short	*GetCustomerEmail()										{ return mCustomerEmail; }

		void			SetCardName(const unsigned short * value)				{ copy_wide_string(mCardName, value, CARD_INFO_BUFFER); }
		unsigned short	*GetCardName()											{ return mCardName; }

		void			SetCardAddress1(const unsigned short * value)			{ copy_wide_string(mCardAddress1, value, CARD_INFO_BUFFER); }
		unsigned short	*GetCardAddress1()										{ return mCardAddress1; }

		void			SetCardAddress2(const unsigned short * value)			{ copy_wide_string(mCardAddress2, value, CARD_INFO_BUFFER); }
		unsigned short	*GetCardAddress2()										{ return mCardAddress2; }

		void			SetCardCity(const unsigned short * value)				{ copy_wide_string(mCardCity, value, CARD_INFO_BUFFER); }
		unsigned short	*GetCardCity()											{ return mCardCity; }

		void			SetCardState(const unsigned short * value)				{ copy_wide_string(mCardState, value, CARD_INFO_BUFFER); }
		unsigned short	*GetCardState()											{ return mCardState; }

		void			SetCardZip(const unsigned short * value)				{ copy_wide_string(mCardZip, value, CARD_INFO_BUFFER); }
		unsigned short	*GetCardZip()											{ return mCardZip; }

		void			SetCountry(const unsigned short * value)				{ copy_wide_string(mCountry, value, COUNTRY_BUFFER); }
		unsigned short	*GetCountry()											{ return mCountry; }

	private:
		unsigned		mOrderID;
		char			mEcommOrderID[ECOMM_ORDER_ID_BUFFER];
		char			mOrderStatus[ORDER_STATUS_BUFFER];
		char			mLanguage[LANGUAGE_BUFFER];
		char			mGameCode[GAME_CODE_BUFFER];
		unsigned		mStationID;
		unsigned short	mCustomerFirstName[CUSTOMER_NAME_BUFFER];
		unsigned short	mCustomerLastName[CUSTOMER_NAME_BUFFER];
		unsigned short	mCustomerPhone[CUSTOMER_PHONE_BUFFER];
		unsigned short	mCustomerEmail[CUSTOMER_EMAIL_BUFFER];
		unsigned short	mCardName[CARD_INFO_BUFFER];
		unsigned short	mCardAddress1[CARD_INFO_BUFFER];
		unsigned short	mCardAddress2[CARD_INFO_BUFFER];
		unsigned short	mCardCity[CARD_INFO_BUFFER];
		unsigned short	mCardState[CARD_INFO_BUFFER];
		unsigned short	mCardZip[CARD_INFO_BUFFER];
		unsigned short	mCountry[COUNTRY_BUFFER];
		char			mCreationDate[DATE_BUFFER];
		char			mModifyDate[DATE_BUFFER];
		char			mCompletionDate[DATE_BUFFER];
		unsigned		mTransactionCount;
		unsigned		mSuccessfulTransactions;
		unsigned		mFailedTransactions;

	};

}; // namespace

#endif	//CTSERVICEDBORDER_H
