// ======================================================================
//
// ScriptBuffer.h
// copyright (c) 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_ScriptBuffer_H
#define INCLUDED_ScriptBuffer_H

// ======================================================================

#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "serverDatabase/AbstractTableBuffer.h"
#include "sharedFoundation/NetworkId.h"
#include "sharedLog/Log.h"

namespace DB
{
	class Session;
}

// ======================================================================

/**
 * A buffer to hold script attach & detach commands.
 * Unlike many of the other buffers, this does not attempt to combine
 * operations (e.g. match up a detach with an attach already in the buffer,
 * and so skip sending either to the database).  Instead, it just keeps a
 * list of everything to do.
 */
class ScriptBuffer : public AbstractTableBuffer
{
  public:
	void clearScripts(const NetworkId &objectId);

	virtual bool save(DB::Session *session);
	virtual bool load(DB::Session *session,const DB::TagSet &tags, const std::string &schema, bool usingGoldDatabase);
	virtual void removeObject(const NetworkId &object);

	void addObject(const NetworkId &objectId);

	void getScriptsForObject(const NetworkId &objectId, std::vector<std::string> &scripts) const;

	ScriptBuffer();
	virtual ~ScriptBuffer();
		
  private:
	class IndexKey
	{
	  public:
		NetworkId m_objectId;
		int m_index;

	  public:
		IndexKey(const NetworkId &objectId, int index);
		
	  public:
		bool operator<(const IndexKey &rhs) const;
	};
	
	std::set<NetworkId> m_objectsToClear;

	typedef std::map<IndexKey, std::string> LoadedDataType;
	LoadedDataType *m_loadedData; // data loaded from the database
};

// ======================================================================

#endif
