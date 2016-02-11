// ======================================================================
//
// Profiler.cpp
//
// Copyright 2003 Sony Online Entertainment
// All Rights Reserved
//
// ======================================================================

#include "sharedDebug/FirstSharedDebug.h"
#include "sharedDebug/Profiler.h"

#include "sharedDebug/DebugFlags.h"
#include "sharedDebug/DebugKey.h"
#include "sharedDebug/ProfilerTimer.h"
#include "sharedFoundation/ConfigFile.h"
#include "sharedFoundation/ExitChain.h"
#include "sharedFoundation/MemoryBlockManager.h"
#include "sharedFoundation/MemoryBlockManagerMacros.h"

#include <vector>
#include <algorithm>

// ======================================================================

namespace ProfilerNamespace
{
	struct ProfilerEntry
	{
		typedef std::vector<int> Children;

		const char          *name;
		int                  parent;
		Children             children;
		int                  totalCalls;
		ProfilerTimer::Type  totalTime;
	};

	class VisibleExpandableEntry
	{
	public:

		VisibleExpandableEntry(char const * name);
		~VisibleExpandableEntry();

		char const *             getName() const;

		bool                     isExpanded() const;
		void                     setExpanded(bool opened);
		void                     toggleExpanded();

		void                     markAllChildrenInvisible();
		VisibleExpandableEntry * findOrAddExpandableChild(char const *name);
		bool                     pruneInvisibleChildren();

	private:

		typedef std::vector<VisibleExpandableEntry *> Children;

	private:

		static bool  ms_destroyedSelected;

	private:

		char const * m_name;
		Children     m_children;
		bool         m_expanded;
		bool         m_visible;
	};

	typedef std::vector<ProfilerEntry>   ProfilerEntries;
	typedef std::vector<ProfilerEntry *> SortedProfilerEntries;

	struct SortMultisiteEntries
	{
		bool operator()(ProfilerEntry const * lhs, ProfilerEntry const * rhs) const;
	};

	void formatInteger(int length, int value);
	void formatString(const char *string);
	bool formatEntry(int indent, ProfilerTimer::Type totalTime, int totalCalls, bool expandable, bool expanded, bool selected, char const *name);
	void processEntry(char const *rootName, int indent, ProfilerEntry const & entry, VisibleExpandableEntry * opened);
	void enterWithTime(char const *name, ProfilerTimer::Type time);
	void leaveWithTime(char const *name, ProfilerTimer::Type time);
	void generateReport();
	bool myCompare(const char *a, const char *b);


	std::vector<int>              ms_profilerEntryStack;
	ProfilerEntries *             ms_profilerEntriesCurrent;
	ProfilerEntries *             ms_profilerEntriesLast;
	ProfilerEntries               ms_profilerEntries1;
	ProfilerEntries               ms_profilerEntries2;

	char const *                  ms_rootProfilerName;

	VisibleExpandableEntry *      ms_rootVisibleExpandableEntry;
	VisibleExpandableEntry *      ms_beforeSelectedVisibleExpandableEntry;
	VisibleExpandableEntry *      ms_selectedVisibleExpandableEntry;
	VisibleExpandableEntry *      ms_afterSelectedVisibleExpandableEntry;
	VisibleExpandableEntry *      ms_previousVisibleExpandableEntry;

	std::vector<char>             ms_text;
	ProfilerTimer::Type           ms_timeDivisor;
	ProfilerTimer::Type           ms_percentageDivisor;
	ProfilerTimer::Type           ms_frequency;
	bool                          ms_enabled = true;
	bool                          ms_desiredEnabled = true;
	bool                          ms_debugKeyContext;
	bool                          ms_debugReportFlag;
	bool                          ms_debugReportLogFlag;
	bool                          ms_temporaryExpandAll;
	bool                          ms_multisiteCallerSummary;
	bool                          ms_setRoot;
	bool                          ms_selectedUp;
	bool                          ms_selectedDown;
	bool                          ms_selectedExpand;
	bool                          ms_selectedCollapse;
	bool                          ms_selectedToggle;
	int                           ms_displayPercentageMinimum;
	int                           ms_stackDepth;
	SortedProfilerEntries         ms_sortedProfilerEntries;

}
using namespace ProfilerNamespace;

bool  VisibleExpandableEntry::ms_destroyedSelected;

// ======================================================================

VisibleExpandableEntry::VisibleExpandableEntry(char const * name)
:
	m_name(name),
	m_children(),
	m_expanded(false),
	m_visible(true)
{
}

// ----------------------------------------------------------------------

VisibleExpandableEntry::~VisibleExpandableEntry()
{
	// handle case where the selected item gets deleted
	if (this == ms_selectedVisibleExpandableEntry)
		ms_destroyedSelected = true;

	while (!m_children.empty())
	{
		VisibleExpandableEntry *back = m_children.back();
		m_children.pop_back();
		delete back;
	}
}

// ----------------------------------------------------------------------

inline char const * VisibleExpandableEntry::getName() const
{
	return m_name;
}

// ----------------------------------------------------------------------

inline bool VisibleExpandableEntry::isExpanded() const
{
	return m_expanded || ms_temporaryExpandAll;
}

// ----------------------------------------------------------------------

inline void VisibleExpandableEntry::setExpanded(bool expanded)
{
	m_expanded = expanded;
}

// ----------------------------------------------------------------------

inline void VisibleExpandableEntry::toggleExpanded()
{
	m_expanded = !m_expanded;
}

// ----------------------------------------------------------------------

void VisibleExpandableEntry::markAllChildrenInvisible()
{
	Children::iterator const iEnd = m_children.end();
	for (Children::iterator i = m_children.begin(); i != iEnd; ++i)
		(*i)->m_visible = false;
}

// ----------------------------------------------------------------------

VisibleExpandableEntry * VisibleExpandableEntry::findOrAddExpandableChild(char const *name)
{
	Children::iterator const iEnd = m_children.end();
	for (Children::iterator i = m_children.begin(); i != iEnd; ++i)
	{
		VisibleExpandableEntry * child = *i;
		if (child->m_name == name)
		{
			child->m_visible = true;
			return child;
		}
	}

	m_children.push_back(new VisibleExpandableEntry(name));
	return m_children.back();
}

// ----------------------------------------------------------------------

bool VisibleExpandableEntry::pruneInvisibleChildren()
{
	for (Children::iterator i = m_children.begin(); i != m_children.end(); )
	{
		if ((*i)->m_visible == false)
		{
			i = m_children.erase(i);
		}
		else
		{
			++i;
		}
	}
	bool const result = ms_destroyedSelected;
	ms_destroyedSelected = false;
	return result;
}

// ======================================================================

void Profiler::install()
{
	ExitChain::add(Profiler::remove, "Profiler::remove");

	ms_profilerEntriesCurrent = &ms_profilerEntries1;
	ms_profilerEntriesLast    = &ms_profilerEntries2;
	ms_rootVisibleExpandableEntry = new VisibleExpandableEntry(nullptr);
	ms_rootVisibleExpandableEntry->setExpanded(true);
	ms_selectedVisibleExpandableEntry = ms_rootVisibleExpandableEntry;
}

// ----------------------------------------------------------------------

void Profiler::remove()
{
	delete ms_rootVisibleExpandableEntry;
	ms_rootVisibleExpandableEntry = nullptr;
	ms_beforeSelectedVisibleExpandableEntry = nullptr;
	ms_selectedVisibleExpandableEntry = nullptr;
	ms_afterSelectedVisibleExpandableEntry = nullptr;
	ms_previousVisibleExpandableEntry = nullptr;
	ms_profilerEntriesCurrent = nullptr;
	ms_profilerEntriesLast = nullptr;
}

// ----------------------------------------------------------------------

void Profiler::registerDebugFlags()
{
	DebugKey::registerFlag(ms_debugKeyContext, "profiler");
	DebugFlags::registerFlag(ms_debugReportFlag,    "SharedDebug/Profiler", "report", printLastFrameData);
	DebugFlags::registerFlag(ms_desiredEnabled,     "SharedDebug/Profiler", "enabled");
	DebugFlags::registerFlag(ms_debugReportLogFlag, "SharedDebug/Profiler", "logNextReport");
	DebugFlags::registerFlag(ms_temporaryExpandAll, "SharedDebug/Profiler", "temporaryExpandAll");
	ms_displayPercentageMinimum = ConfigFile::getKeyInt("SharedDebug/Profiler", "displayPercentageMinimum", 0);
}

// ----------------------------------------------------------------------

void Profiler::enable(bool enable)
{
	ms_desiredEnabled = enable;
}

// ----------------------------------------------------------------------

void Profiler::enableProfilerOutput(bool enableOutput)
{
	ms_debugReportFlag = enableOutput;
}

// ----------------------------------------------------------------------

void ProfilerNamespace::formatInteger(int length, int value)
{
	// pad with spaces
	for (int i = 0; i < length; ++i)
		ms_text.push_back(' ');

	// get a pointer to the last character
	char *buffer = &ms_text[ms_text.size()-1];

	// write the value right to left
	do
	{
		*buffer = static_cast<char>('0' + (value % 10));
		--length;
		--buffer;
		value /= 10;
	} while (length && value);
}

// ----------------------------------------------------------------------

void ProfilerNamespace::formatString(char const *string)
{
	for ( ; *string; ++string)
		ms_text.push_back(*string);
}

// ----------------------------------------------------------------------

bool ProfilerNamespace::formatEntry(int indent, ProfilerTimer::Type totalTime, int totalCalls, bool expandable, bool expanded, bool selected, char const * name)
{
	int const percentage = static_cast<int>(totalTime / ms_percentageDivisor);

	if (percentage < ms_displayPercentageMinimum)
		return false;

	int const ticks = static_cast<int>(totalTime / ms_timeDivisor);

	// indent
	{
		for (int i = 0; i < indent; ++i)
			ms_text.push_back(' ');
	}

	// print number of ticks
	formatInteger(7, ticks);
	ms_text.push_back(' ');

	// pring % of frame time
	formatInteger(3, percentage);
	ms_text.push_back('%');
	ms_text.push_back(' ');

	// print the number of calls
	formatInteger(8, totalCalls);
	ms_text.push_back(' ');

	if (expandable)
	{
		// print the expanded state
		if (expanded)
			ms_text.push_back('-');
		else
			ms_text.push_back('+');

		// print the selection marker
		if (selected)
			ms_text.push_back('>');
		else
			ms_text.push_back(' ');
	}
	else
	{
		ms_text.push_back(' ');
		ms_text.push_back(' ');
	}

	// print the entry name
	formatString(name);
	ms_text.push_back('\n');

	return true;
}

// ----------------------------------------------------------------------

void ProfilerNamespace::processEntry(char const *rootName, int indent, ProfilerEntry const & entry, VisibleExpandableEntry * visibleExpandableEntry)
{
	if (rootName == entry.name)
		rootName = nullptr;

	if (!rootName)
	{
		if (!formatEntry(indent, entry.totalTime, entry.totalCalls, visibleExpandableEntry != nullptr, visibleExpandableEntry && visibleExpandableEntry->isExpanded(), visibleExpandableEntry == ms_selectedVisibleExpandableEntry, entry.name))
			return;
	}

	if (visibleExpandableEntry)
	{
		// keep track of before and after the selected entry
		if (ms_selectedVisibleExpandableEntry == visibleExpandableEntry)
			ms_beforeSelectedVisibleExpandableEntry = ms_previousVisibleExpandableEntry;
		if (!entry.children.empty())
		{
			if (ms_previousVisibleExpandableEntry == ms_selectedVisibleExpandableEntry)
				ms_afterSelectedVisibleExpandableEntry = visibleExpandableEntry;
			ms_previousVisibleExpandableEntry = visibleExpandableEntry;
		}

		// process the children
		visibleExpandableEntry->markAllChildrenInvisible();

			// indent if we have already seen the root
			if (!rootName)
				indent += 2;

			if (visibleExpandableEntry->isExpanded())
			{
				ProfilerEntry::Children::const_iterator iEnd = entry.children.end();
				for (ProfilerEntry::Children::const_iterator i = entry.children.begin(); i != iEnd; ++i)
				{
					ProfilerEntry const & child = (*ms_profilerEntriesLast)[*i];

					if (child.children.empty())
					{
						processEntry(rootName, indent, child, nullptr);
					}
					else
					{
						VisibleExpandableEntry * childVisibleExpandableEntry = visibleExpandableEntry->findOrAddExpandableChild(child.name);
						processEntry(rootName, indent, child, childVisibleExpandableEntry);
					}
				}
			}

		if (visibleExpandableEntry->pruneInvisibleChildren())
			ms_selectedVisibleExpandableEntry = visibleExpandableEntry;
	}
}

// ----------------------------------------------------------------------

inline bool SortMultisiteEntries::operator()(ProfilerEntry const * lhs, ProfilerEntry const * rhs) const
{
	return lhs->name < rhs->name;
}

// ----------------------------------------------------------------------

void ProfilerNamespace::generateReport()
{
	PROFILER_AUTO_BLOCK_DEFINE("Profiler::generateReport");

	if (!ms_profilerEntriesLast->empty())
	{
		if (ms_debugKeyContext)
		{
			if (DebugKey::isPressed(1))
				ms_desiredEnabled = !ms_desiredEnabled;
			if (DebugKey::isPressed(2))
				ms_debugReportLogFlag = true;
			if (DebugKey::isPressed(3))
				ms_temporaryExpandAll = !ms_temporaryExpandAll;
			if (DebugKey::isPressed(4))
				ms_setRoot = true;
			if (DebugKey::isPressed(5))
				ms_multisiteCallerSummary = !ms_multisiteCallerSummary;
			if (DebugKey::isPressed(6))
				ms_displayPercentageMinimum = clamp(0, ms_displayPercentageMinimum - 1, 100);
			if (DebugKey::isPressed(7))
				ms_displayPercentageMinimum = clamp(0, ms_displayPercentageMinimum + 1, 100);
			if (DebugKey::isPressed(8))
				Profiler::selectionMoveUp();
			if (DebugKey::isPressed(9))
				Profiler::selectionMoveDown();
			if (DebugKey::isPressed(0))
				Profiler::selectionToggleExpanded();
		}

		// try to get the ticks to approximately 1 million per second
		ms_timeDivisor = ms_frequency / static_cast<ProfilerTimer::Type>(1000000);
		if (ms_timeDivisor == 0)
			ms_timeDivisor = 1;

		// figure out what to divide by to get the value in percent of total frame time
		ms_percentageDivisor = (*ms_profilerEntriesLast)[0].totalTime / static_cast<ProfilerTimer::Type>(100);
		if (ms_percentageDivisor == 0)
			ms_percentageDivisor = 1;

		formatString("Profiler (min ");
		formatInteger(3, ms_displayPercentageMinimum);
		formatString("%):\n");

		ProfilerEntry const & rootEntry = (*ms_profilerEntriesLast)[0];

		ms_rootVisibleExpandableEntry->markAllChildrenInvisible();
		ms_rootVisibleExpandableEntry->findOrAddExpandableChild(rootEntry.name);
		
		ms_previousVisibleExpandableEntry = nullptr;
		ms_beforeSelectedVisibleExpandableEntry = nullptr;
		ms_afterSelectedVisibleExpandableEntry = nullptr;
		processEntry(ms_rootProfilerName, 0, rootEntry, ms_rootVisibleExpandableEntry);

		if (ms_rootVisibleExpandableEntry->pruneInvisibleChildren())
			ms_selectedVisibleExpandableEntry = ms_rootVisibleExpandableEntry;

		if (ms_setRoot)
		{
			if (ms_rootProfilerName)
				ms_rootProfilerName = nullptr;
			else
				ms_rootProfilerName = ms_selectedVisibleExpandableEntry->getName();
			ms_setRoot = false;
		}

		if (ms_selectedUp)
		{
			if (ms_beforeSelectedVisibleExpandableEntry)
				ms_selectedVisibleExpandableEntry = ms_beforeSelectedVisibleExpandableEntry;
			ms_selectedUp = false;
		}

		if (ms_selectedDown)
		{
			if (ms_afterSelectedVisibleExpandableEntry)
				ms_selectedVisibleExpandableEntry = ms_afterSelectedVisibleExpandableEntry;
			ms_selectedDown = false;
		}
	
		if (ms_selectedExpand)
		{
			ms_selectedVisibleExpandableEntry->setExpanded(true);
			ms_selectedExpand= false;
		}

		if (ms_selectedCollapse)
		{
			ms_selectedVisibleExpandableEntry->setExpanded(false);
			ms_selectedCollapse = false;
		}

		if (ms_selectedToggle)
		{
			ms_selectedVisibleExpandableEntry->toggleExpanded();
			ms_selectedToggle = false;
		}
	}

	if (ms_multisiteCallerSummary)
	{
		// generate a vector of all entries
		{
			ms_sortedProfilerEntries.clear();
			ProfilerEntries::iterator const iEnd = ms_profilerEntriesLast->end();
			for (ProfilerEntries::iterator i = ms_profilerEntriesLast->begin(); i != iEnd; ++i)
				ms_sortedProfilerEntries.push_back(&(*i));
		}

		// sort them by name
		std::sort(ms_sortedProfilerEntries.begin(), ms_sortedProfilerEntries.end(), SortMultisiteEntries());

		// find multiple entries with the same name
		{
			formatString("Multisite entries:\n");

			SortedProfilerEntries::iterator i          = ms_sortedProfilerEntries.begin();
			SortedProfilerEntries::iterator const iEnd = ms_sortedProfilerEntries.end();
			do
			{
				char const * const iName          = (*i)->name;
				SortedProfilerEntries::iterator j = i + 1;
				bool display                      = false;
				int totalCalls                    = (*i)->totalCalls;
				ProfilerTimer::Type totalTime     = (*i)->totalTime;

				// find the span of entries with the same name
				while (j != iEnd && iName == (*j)->name)
				{
					display = true;
					totalCalls += (*j)->totalCalls;
					totalTime += (*j)->totalTime;
					++j;
				}

				// if the name had multiple entries, report so here
				if (display)
					formatEntry(2, totalTime, totalCalls, false, false, false, iName);

				i = j;

			} while (i != iEnd);
		}

	}

	ms_text.push_back('\0');
}

// ----------------------------------------------------------------------

char const *Profiler::getLastFrameData()
{
	if (ms_text.empty())
		generateReport();

	return &(ms_text[0]);
}

// ----------------------------------------------------------------------

void Profiler::printLastFrameData()
{
	if (ms_text.empty())
		generateReport();

	Report::setFlags(Report::RF_print | (ms_debugReportLogFlag ? Report::RF_log : 0));
	Report::puts(&ms_text[0]);
	ms_debugReportLogFlag = false;
}

// ----------------------------------------------------------------------

void Profiler::selectionMoveUp()
{
	ms_selectedUp = true;
}

// ----------------------------------------------------------------------

void Profiler::selectionMoveDown()
{
	ms_selectedDown = true;
}

// ----------------------------------------------------------------------

void Profiler::selectionExpand()
{
	ms_selectedExpand = true;
}

// ----------------------------------------------------------------------

void Profiler::selectionCollapse()
{
	ms_selectedCollapse = true;
}

// ----------------------------------------------------------------------

void Profiler::selectionToggleExpanded()
{
	ms_selectedToggle = true;
}

// ----------------------------------------------------------------------

void Profiler::setTemporaryExpandAll(bool temporaryExpandAll)
{
	ms_temporaryExpandAll = temporaryExpandAll;
}

// ----------------------------------------------------------------------

void Profiler::setDisplayPercentageMinimum(int percentage)
{
	ms_displayPercentageMinimum = percentage;
}

// ----------------------------------------------------------------------

bool ProfilerNamespace::myCompare(const char *a, const char *b)
{
	while (*a && *b && tolower(*a) == tolower(*b))
	{
		++a;
		++b;
	}

	return *a == '\0';
}

// ----------------------------------------------------------------------

void Profiler::handleOperation(char const *operation)
{
	if (_stricmp(operation, "up") == 0)
		selectionMoveUp();
	else if (_stricmp(operation, "down") == 0)
		selectionMoveDown();
	else if (_stricmp(operation, "toggle") == 0)
		selectionToggleExpanded();
	else if (_stricmp(operation, "expand") == 0)
		selectionExpand();
	else if (_stricmp(operation, "collapse") == 0)
		selectionCollapse();
	else if (_stricmp(operation, "enable") == 0)
		enable(true);
	else if (_stricmp(operation, "disable") == 0)
		enable(false);
	else if (_stricmp(operation, "enableOutput") == 0)
		enableProfilerOutput(true);
	else if (_stricmp(operation, "disableOutput") == 0)
		enableProfilerOutput(false);
	else if (_stricmp(operation, "showAll") == 0)
		setTemporaryExpandAll(true);
	else if (_stricmp(operation, "showNormal") == 0)
		setTemporaryExpandAll(false);
	else if (myCompare("displayMinimum", operation))
	{
		const char *result = strchr(operation, ' ');
		if (result)
			ms_displayPercentageMinimum = atoi(result+1);
	}
}

// ----------------------------------------------------------------------

void ProfilerNamespace::enterWithTime(char const *name, ProfilerTimer::Type time)
{
	if (ms_stackDepth == 0)
	{
		ms_enabled = ms_desiredEnabled;
		if (!ms_enabled)
		{
			ms_text.clear();
			ms_text.push_back('\0');
		}
	}
	++ms_stackDepth;
	if (!ms_enabled)
		return;

	// search for another call to this block from the parent
	int entryIndex = -1;
	ProfilerEntry *entry = nullptr;
	if (!ms_profilerEntryStack.empty())
	{
		ProfilerEntry &parent = (*ms_profilerEntriesCurrent)[ms_profilerEntryStack.back()];
		ProfilerEntry::Children::iterator iEnd = parent.children.end();
		for (ProfilerEntry::Children::iterator i = parent.children.begin(); i != iEnd; ++i)
		{
			entryIndex = *i;
			ProfilerEntry &check = (*ms_profilerEntriesCurrent)[entryIndex];
			if (check.name == name)
			{
				entry = &check;
				break;
			}
		}
	}

	// no previous call to this block, so create a new entry
	if (!entry)
	{
		entryIndex = ms_profilerEntriesCurrent->size();
		ms_profilerEntriesCurrent->push_back(ProfilerEntry());
		entry = &ms_profilerEntriesCurrent->back();
	
		entry->name = name;
		if (ms_profilerEntryStack.empty())
		{
			entry->parent = -1;
		}
		else
		{
			entry->parent = ms_profilerEntryStack.back();
			(*ms_profilerEntriesCurrent)[entry->parent].children.push_back(entryIndex);
		}
		entry->totalCalls = 0;
		entry->totalTime = 0;

	}

	// record the profiling time
	entry->totalCalls  += 1;
	entry->totalTime -= time;
	ms_profilerEntryStack.push_back(entryIndex);
}

// ----------------------------------------------------------------------

void ProfilerNamespace::leaveWithTime(char const *name, ProfilerTimer::Type time)
{
	if (!ms_enabled)
		return;

	UNREF(name);
	if (ms_profilerEntryStack.empty())
	{
		WARNING(true, ("Profiler::leave stack underflow leaving %s", name));
		return;
	}
	--ms_stackDepth;
	
	// pop the current entry off the stack
	ProfilerEntry &entry= (*ms_profilerEntriesCurrent)[ms_profilerEntryStack.back()];
	ms_profilerEntryStack.pop_back();

	DEBUG_FATAL(entry.name != name, ("Profiler::leave exiting '%s' but expected '%s'", name, entry.name));

	// record the time in the entry
	if (ms_profilerEntryStack.empty())
	{
		// get the calibrated loop end time
		ProfilerTimer::getCalibratedTime(time, ms_frequency);
		entry.totalTime += time;

		// clear the text so we know the profile data must be regenerated if it is desired
		ms_text.clear();

		// swap the pointers
		std::vector<ProfilerEntry> * temp = ms_profilerEntriesCurrent;
		ms_profilerEntriesCurrent = ms_profilerEntriesLast;
		ms_profilerEntriesLast = temp;

		// clear the entries for next frame
		ms_profilerEntriesCurrent->clear();
	}
	else
	{
		entry.totalTime += time;
	}
}

// ----------------------------------------------------------------------

void Profiler::enter(char const *name)
{
	ProfilerTimer::Type time;
	ProfilerTimer::getTime(time);
	enterWithTime(name, time);
}

// ----------------------------------------------------------------------

void Profiler::leave(char const *name)
{
	ProfilerTimer::Type time;
	ProfilerTimer::getTime(time);
	leaveWithTime(name, time);
}

// ----------------------------------------------------------------------

void Profiler::transfer(char const *leaveName, char const *enterName)
{
	ProfilerTimer::Type time;
	ProfilerTimer::getTime(time);
	leaveWithTime(leaveName, time);
	enterWithTime(enterName, time);
}

// ----------------------------------------------------------------------

void Profiler::adjustForLostBlocks(char const *expectingName)
{
	// This deals with cases where we've either missed closing a block or closed a block that wasn't open.
	// We prefer to guess that we've failed to close a block, since that is the more common mistake, before
	// trying to deal with it as though a block was closed but not opened.

	if (!ms_profilerEntryStack.empty())
	{
		// if everything is correct, the back entry will be the expected one, so we don't have to do anything
		if ((*ms_profilerEntriesCurrent)[ms_profilerEntryStack.back()].name == expectingName)
			return;
		for (int i = ms_profilerEntryStack.size()-2; i >= 0; --i)
		{
			if ((*ms_profilerEntriesCurrent)[ms_profilerEntryStack[i]].name == expectingName)
			{
				// we've got unclosed blocks, so close them
				for (int j = ms_profilerEntryStack.size()-1; j > i; --j)
				{
					DEBUG_WARNING(true, ("Profiler adjusting for unclosed block '%s'.", (*ms_profilerEntriesCurrent)[ms_profilerEntryStack[j]].name));
					PROFILER_BLOCK_DEFINE(p, (*ms_profilerEntriesCurrent)[ms_profilerEntryStack[j]].name);
					PROFILER_BLOCK_LEAVE(p);
				}
				return;
			}
		}
	}

	DEBUG_WARNING(true, ("Profiler adjusting for unopened block '%s'.", expectingName));
	// we're about to close a block that was not open, so open it
	PROFILER_BLOCK_DEFINE(p, expectingName);
	PROFILER_BLOCK_ENTER(p);
}

// ======================================================================
