//===================================================================
//
// HttpGetEncoder.h
//
// copyright 2006, Sony Online Entertainment
//
//===================================================================

#ifndef INCLUDED_HttpGetEncoder_H
#define INCLUDED_HttpGetEncoder_H

#include <string>
#include <map>

class HttpGetEncoder
{
public:
	typedef std::map<std::string, std::string> GetParams;
	static std::string getUrl( const std::string & UrlBase, const GetParams & params);

private:
	HttpGetEncoder();
};

#endif // INCLUDED_HttpGetEncoder_H
