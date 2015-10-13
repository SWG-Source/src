#include "ccspanutil.h"

#include <stdlib.h>
#include <string.h>

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif
namespace Base 
{

char * CSpanUtil::getDescriptionFromCardNum(const char *cardNum)
{
	// if the card number is less than this we wont be able to determine it, and we dont want to crash so . . .
	if(strlen(cardNum) < 8 )
		return "Credit Card";

	switch(cardNum[0])
	{
	case '3':
		if(cardNum[1] == '4')
			return "AMEX";

		if(cardNum[1] == '7')
		{
			// RG 5/9 changed to fix rrc=233.  We do not support OP anyway.
			// Card prefix check is unclear.
			//
			//			if(cardNum[3] == '7')
			//				return "OP";
			//			else
			return "AMEX";
		}
		if((cardNum[1] == '0') || (cardNum[1] == '6'))
			return "DINERS";
		if(cardNum[1] == '8')
		{
			if((cardNum[2] >= '1') && (cardNum[2] <= '8'))
				return "DINERS";
		}
		if(cardNum[1] == '5')
		{
			char temp[3];
			memcpy(temp, &cardNum[2], 2);
			temp[2] = 0;

			int value = atoi(temp);
			if((value >= 28) && (value <= 89))
				return "JCB";
		}
		break;
	case '4':
		// RG 5/6 changed to fix rrc=233.  We do not support SW anyway.
		// Card prefix check is unclear.
		//
		//		if(cardNum[1] == '9')
		//			return "SW";
		//		else
		return "VISA";
		break;
	case '5':
		if(cardNum[1] == '6')
			return "Switch/solo";
		if((cardNum[1] >= '1') && (cardNum[1] <= '5'))
			return "MC";
		if(memcmp(cardNum, "504990", 6) == 0)
			return "Bill me Later";
		break;
	case '6':
		{
			char temp[8];
			memcpy(temp, cardNum, 7);
			temp[7] = 0;
			temp[6] = 0;
			int value6 = atoi(temp);
			temp[5] = 0;
			int value5 = atoi(temp);

			// september 29 2005 added new card IIN ranges for Discover per email from
			// paymentTech email effective Oct 1, 2005
			if((value5 == 60110) ||
				(value5 == 60112) ||
				(value5 == 60113) ||
				(value5 == 60114) ||
				(value5 == 60115) ||
				(value5 == 60116) ||
				(value6 == 601174) ||
				(value6 >= 601177 && value6 <= 601179) ||
				(value6 >=601186 && value6 <= 600189) ||
				(value5 == 60119) ||
				(value6 >= 650000 && value6 <=650999))
				return "DISCOVER";

			if(value6 == 603571)
				return "Saved Value";

			return "Switch/Solo";
		}
		break;
	case '7':
		return "Switch/Solo";
		break;
	case '9':
		//if((cardNum[1] == '4') && (cardNum[1] == '5'))
		return "Carte Blanche";
		break;
	}

	return "Credit Card";
}


// generates a credit card span for an account number string up to a length of any size.
std::string CSpanUtil::generateCCSpan(std::string cardNumber)
{
	// according to the PCI standard on visa's web site, the max number of displayable digits for a cc are the first 6 and last 4
	// we will however require that at least the middle half of the card is not shown
	std::string ccSpan;
	const std::string asteriks = "*";

	// if the length of the card is an odd number we will display the larger portion of the card on the front viewable amount.
	// for Amex this will look like 1234********123
	// on a 16 digit card, the most digits will be displayed being 1234********1234
	// a 12 digit card would look like 123******123

	// as per qa request and devtrack bug 2440 masking is first 4 and las 4 visible

	unsigned displayableBeginningDigits = 4;
	unsigned displayableEndingDigits = 4;
	
	if(cardNumber.length() <= 8)
		return cardNumber;

	ccSpan = cardNumber.substr(0, displayableBeginningDigits);	
	
	unsigned totalMasked = cardNumber.length() - displayableBeginningDigits - displayableEndingDigits;
	
	// now add in characters to mask the middle numbers
	for(unsigned maskedDigits = 0; maskedDigits < totalMasked; maskedDigits++)
		ccSpan += asteriks;
	
	ccSpan += cardNumber.substr((totalMasked+displayableBeginningDigits));

	return ccSpan;
}

std::string CSpanUtil::generateCCDescription(std::string cardNumber)
{
	std::string ccDescription;

	// get the card type

	ccDescription = std::string(getDescriptionFromCardNum(cardNumber.c_str())) + std::string("(") + generateCCSpan(cardNumber) + std::string(")");

	return ccDescription;
}
};
#ifdef EXTERNAL_DISTRO
};
#endif


