#ifndef CTSERVICECUSTOMER_H
#define CTSERVICECUSTOMER_H

#include <string.h>

namespace CTService 
{

//--------------------------------------------------------
class CTServiceCustomer
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
		enum {	SHORT_NAME_SIZE=64,			SHORT_NAME_BUFFER,
				PHONE_SIZE=32,				PHONE_BUFFER, 
				EMAIL_SIZE=128,				EMAIL_BUFFER, 
				CARD_NAME_SIZE=255,			CARD_NAME_BUFFER, 
				CARD_ADDRESS1_SIZE=255,		CARD_ADDRESS1_BUFFER, 
				CARD_ADDRESS2_SIZE=255,		CARD_ADDRESS2_BUFFER, 
				CARD_CITY_SIZE=255,			CARD_CITY_BUFFER, 
				CARD_STATE_SIZE=255,		CARD_STATE_BUFFER, 
				CARD_ZIP_SIZE=255,			CARD_ZIP_BUFFER, 
				CARD_COUNTRY_SIZE=2,		CARD_COUNTRY_BUFFER, 
				CARD_NUMBER_SIZE=255,		CARD_NUMBER_BUFFER, 
				CARD_EXPIRATION_MONTH_SIZE=2,CARD_EXPIRATION_MONTH_BUFFER, 
				CARD_EXPIRATION_YEAR_SIZE=4,CARD_EXPIRATION_YEAR_BUFFER };
		CTServiceCustomer() : mFirstName(), mLastName(), mPhone(), mEmail(), mStationID(0), mCardName(), mCardAddress1(), mCardAddress2(), mCardCity(), mCardState(), mCardZip(), mCountry(), mCardNumber(), mCardExpirationMonth(), mCardExpirationYear()
			{  mFirstName[0]	= 0;
			  mLastName[0]	= 0;
			  mPhone[0]	= 0;
			  mEmail[0]	= 0;
			  mCardName[0]	= 0;
			  mCardAddress1[0]	= 0;
			  mCardAddress2[0]	= 0;
			  mCardCity[0]	= 0;
			  mCardState[0]	= 0;
			  mCardZip[0]	= 0;
			  mCountry[0]	= 0;
			  mCardNumber[0]	= 0;
			  mCardExpirationMonth[0]= 0;
			  mCardExpirationYear[0]= 0; }


		void			SetFirstName(const unsigned short * value)			{ copy_wide_string(mFirstName, value, SHORT_NAME_BUFFER); }
		unsigned short	*GetFirstName()										{ return mFirstName; }

		void			SetLastName(const unsigned short * value)			{ copy_wide_string(mLastName, value, SHORT_NAME_BUFFER); }
		unsigned short	*GetLastName()										{ return mLastName; }

		void			SetPhone(const unsigned short * value)				{ copy_wide_string(mPhone, value, PHONE_BUFFER); }
		unsigned short	*GetPhone()											{ return mPhone; }

		void			SetEmail(const unsigned short * value)				{ copy_wide_string(mEmail, value, EMAIL_BUFFER); }
		unsigned short	*GetEmail()											{ return mEmail; }

		void			SetStationID(const unsigned value)					{ mStationID = value; }
		unsigned		GetStationID()										{ return mStationID; }

		void			SetCardName(const unsigned short * value)			{ copy_wide_string(mCardName, value, CARD_NAME_BUFFER); }
		unsigned short	*GetCardName()										{ return mCardName; }

		void			SetCardAddress1(const unsigned short * value)		{ copy_wide_string(mCardAddress1, value, CARD_ADDRESS1_BUFFER); }
		unsigned short	*GetCardAddress1()									{ return mCardAddress1; }

		void			SetCardAddress2(const unsigned short * value)		{ copy_wide_string(mCardAddress2, value, CARD_ADDRESS2_BUFFER); }
		unsigned short	*GetCardAddress2()									{ return mCardAddress2; }

		void			SetCardCity(const unsigned short * value)			{ copy_wide_string(mCardCity, value, CARD_CITY_BUFFER); }
		unsigned short	*GetCardCity()										{ return mCardCity; }

		void			SetCardState(const unsigned short * value)			{ copy_wide_string(mCardState, value, CARD_STATE_BUFFER); }
		unsigned short	*GetCardState()										{ return mCardState; }

		void			SetCardZip(const unsigned short * value)			{ copy_wide_string(mCardZip, value, CARD_ZIP_BUFFER); }
		unsigned short	*GetCardZip()										{ return mCardZip; }

		void			SetCountry(const unsigned short * value)			{ copy_wide_string(mCountry, value, CARD_COUNTRY_BUFFER); }
		unsigned short	*GetCountry()										{ return mCountry; }

		void			SetCardNumber(const unsigned short * value)			{ copy_wide_string(mCardNumber, value, CARD_NUMBER_BUFFER); }
		unsigned short	*GetCardNumber()									{ return mCardNumber; }

		void			SetCardExpirationMonth(const unsigned short * value){ copy_wide_string(mCardExpirationMonth, value, CARD_EXPIRATION_MONTH_BUFFER); }
		unsigned short	*GetCardExpirationMonth()							{ return mCardExpirationMonth; }

		void			SetCardExpirationYear(const unsigned short * value)	{ copy_wide_string(mCardExpirationYear, value, CARD_EXPIRATION_YEAR_BUFFER); }
		unsigned short	*GetCardExpirationYear()							{ return mCardExpirationYear; }


	private:
		unsigned short	mFirstName[SHORT_NAME_BUFFER];
		unsigned short	mLastName[SHORT_NAME_BUFFER];
		unsigned short	mPhone[PHONE_BUFFER];
		unsigned short	mEmail[EMAIL_BUFFER];
		unsigned		mStationID;
		unsigned short	mCardName[CARD_NAME_BUFFER];
		unsigned short	mCardAddress1[CARD_ADDRESS1_BUFFER];
		unsigned short	mCardAddress2[CARD_ADDRESS2_BUFFER];
		unsigned short	mCardCity[CARD_CITY_BUFFER];
		unsigned short	mCardState[CARD_STATE_BUFFER];
		unsigned short	mCardZip[CARD_ZIP_BUFFER];
		unsigned short	mCountry[CARD_COUNTRY_BUFFER];
		unsigned short	mCardNumber[CARD_NUMBER_BUFFER];
		unsigned short	mCardExpirationMonth[CARD_EXPIRATION_MONTH_BUFFER];
		unsigned short	mCardExpirationYear[CARD_EXPIRATION_YEAR_BUFFER];
	};

}; // namespace

#endif

