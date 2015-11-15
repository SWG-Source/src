//======================================================================
//
// HttpGetEncoder.cpp
// copyright (c) 2006 Sony Online Entertainment
//
//======================================================================

#include "sharedUtility/FirstSharedUtility.h"
#include "HttpGetEncoder.h"
//======================================================================

namespace HttpGetEncoderNamespace
{
	// note:  This is a pretty aggressive encoder.  A few more characters
	// (such as parens) could be added, however extra encoding is always legal.
	std::string encodeString(const std::string & toEncode)
	{
		char buf[3];
		std::string output;
		for(unsigned int i = 0; i < toEncode.length(); ++i)
		{
			// check to see if the char is valid.
			char current = toEncode[i];

			// if it is, append it.
			if((current >= 'a' && current <= 'z') ||
				(current >='A' && current <= 'Z')  ||
				(current >='0' && current <= '9') ||
				(current == '-'))
			{
				output = output + current;
			}

			// if it is a space, add a + sign.
			else if(current == ' ')
			{
				output = output + "+";
			}
			// if not, append the ascii code.
			else
			{
				output = output + "%";
				snprintf(buf, 3, "%02x", current);
				output = output + buf;
			}
		}

		return output;
	}
}

std::string HttpGetEncoder::getUrl(const std::string & urlBase, const GetParams & params)
{
	std::string outUrl = urlBase;
	std::string paramList;
	for(GetParams::const_iterator it = params.begin(); it != params.end(); ++it)
	{
		if(it->first != "" && it->second != "")
			paramList = paramList + HttpGetEncoderNamespace::encodeString(it->first) + "=" 
			+ HttpGetEncoderNamespace::encodeString(it->second) + "&";
	}
	// trim the trailing &
	if(paramList.length() > 0)
	{
		paramList = paramList.substr(0, paramList.length() - 1);
		// if we still have data, append it.
		if( paramList.length() > 0)
			outUrl = outUrl + "?" + paramList;
	}
	return outUrl;
}
