// ======================================================================
//
// ScriptListEntry.h
//
// Copyright 2003 Sony Online Entertainment
//
// ======================================================================

#ifndef _ScriptListEntry_H_
#define _ScriptListEntry_H_

// ======================================================================

struct ScriptData;

// ======================================================================

class ScriptListEntry
{
public:
	ScriptListEntry();
	ScriptListEntry(std::pair<const std::string, ScriptData> * data);
	bool operator==(ScriptListEntry const &rhs) const;
	bool isValid() const;
	std::string const &getScriptName() const;
	ScriptData &getScriptData() const;
private:
	std::pair<const std::string, ScriptData> *m_data;
};

// ----------------------------------------------------------------------

inline ScriptListEntry::ScriptListEntry() :
	m_data(0)
{
}

// ----------------------------------------------------------------------

inline ScriptListEntry::ScriptListEntry(std::pair<const std::string, ScriptData> * data) :
	m_data(data)
{
}

// ----------------------------------------------------------------------

inline bool ScriptListEntry::operator==(ScriptListEntry const &rhs) const
{
	return m_data == rhs.m_data;
}

// ----------------------------------------------------------------------

inline bool ScriptListEntry::isValid() const
{
	return m_data != nullptr;
}

// ======================================================================

#endif // _ScriptListEntry_H_

