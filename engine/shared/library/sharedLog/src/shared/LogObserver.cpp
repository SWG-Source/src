// ======================================================================
//
// LogObserver.cpp
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#include "sharedLog/FirstSharedLog.h"
#include "sharedFoundation/NetworkIdArchive.h"
#include "sharedLog/LogObserver.h"
#include "sharedNetworkMessages/LogMessage.h"
#include <string>
#include <vector>

// ======================================================================

LogObserver::LogObserver() :
	m_filters(new std::vector<std::string>())
{
}

// ----------------------------------------------------------------------

LogObserver::~LogObserver()
{
	delete m_filters;
}

// ----------------------------------------------------------------------

void LogObserver::setFilter(std::string const &filter)
{
	// filter is a : separated list of filter rules.  split them and add
	// valid ones to the filter list
	m_filters->clear();
	size_t curPos = 0;
	while (curPos < filter.length())
	{
		size_t endPos = filter.find(":", curPos);
		if (endPos == std::string::npos)
			endPos = filter.length()+1;
		m_filters->push_back(filter.substr(curPos, endPos-curPos));
		curPos = endPos+1;
	}
}

// ----------------------------------------------------------------------

bool LogObserver::isFiltered(LogMessage const &msg) const
{
	bool filtered = false;
	// run through the list of filters comparing them
	std::vector<std::string>::const_iterator i;
	for (i = m_filters->begin(); i != m_filters->end(); ++i)
	{
		std::string const &filter = (*i);
		std::string const *source = 0;
		// source specifier (c or d - channel or data)
		if (filter[0] == 'c')
			source = &msg.getChannel();
		else if (filter[0] == 'd')
			source = &msg.getText();
		else if (filter[0] == 'p')
			source = &msg.getProcId();
		else
			continue; // invalid source specifier, skip filter
		// filter action (+ or -)
		bool action = false;
		if (filter[1] == '+')
			action = false;
		else if (filter[1] == '-')
			action = true;
		else
			continue; // invalid action specifier, skip filter
		// may be a comparison negation specifier
		size_t filterPos = 2;
		bool negateCompare = false;
		if (filter[2] == '!')
		{
			++filterPos;
			negateCompare = true;
		}
		bool match;
		// either '*' or a substring to search for, may change this to regex
		if (filter[filterPos] == '*')
			match = true;
		else
			match = (source->find(filter.c_str()+filterPos) != std::string::npos);
		if (negateCompare)
			match = !match;
		if (match)
			filtered = action;
	}
	return filtered;
}

// ----------------------------------------------------------------------

void LogObserver::flush()
{
}

//------------------------------------------------------------------------------------------

void LogObserver::update()
{
}

// ======================================================================

