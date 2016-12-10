// ======================================================================
//
// ConfigFile.h
// copyright 2001 Sony Online Entertainment
//
// ======================================================================

#ifndef CONFIG_FILE_H
#define CONFIG_FILE_H

// ======================================================================

class StringCompare;

#include "sharedFoundation/Tag.h"

// ======================================================================

/// Provide access to configuration file paramaters.
class ConfigFile
{
public:

	/** Represents a single value of a key/value pair
	 *
	 *  Stored as a string, but can be cast and returned as an int, bool, and float as well
	 */
	class Element
	{
	public:
		Element();
		explicit Element(const char *entry);
		virtual ~Element(void);
		Element(const Element&);
		Element &operator =(const Element&);
		int         getAsInt(void) const;
		bool        getAsBool(void) const;
		float       getAsFloat(void) const;
		const char *getAsString(void) const;
		Tag         getAsTag(void) const;

	private:
		/// a buffer holding the entry string, which may be recast to other types
		char       *m_entry;
	};
	typedef std::list<Element *> ElementList;

	// -----------------------------------------------------------------------

	///Represents a single key, which may hold multiple ordered Elements as values
	class Key
	{
	public:
		Key (const char *name, const char *value, bool lazyAdd = false);
		virtual ~Key(void);
		const char      *getName(void) const;
		bool             getLazyAdd(void) const;
		int              getAsInt(int index, int defaultValue) const;
		bool             getAsBool(int index, bool defaultValue) const;
		float            getAsFloat(int index, float defaultValue) const;
		const char      *getAsString(int index, const char *defaultValue) const;
		Tag              getAsTag(int index, Tag defaultValue) const;
		int              getCount(void) const;
		void             addValue(const char *value);
		void             dump(const char *keyName) const;

	private:
		/// disabled
		Key(void);
		/// disabled
		Key(const Key&);
		/// disabled
		Key &operator =(const Key&);

	private:
		///A list of element values, since are elements are unique and order is important
		ElementList *m_elements;
		///String representation of the key's name
		char* m_name;
		/// track whether this was added lazily (not explicitly in the config file)
		bool m_lazyAdd;
	};
	typedef std::map<const char *, Key *, StringCompare> KeyMap;

	// -----------------------------------------------------------------------

	/** A scoping mechanism for keys
	 *
	 * All keys exist inside a section, i.e. [Section]
	 *                                       a = 5
	 */
	class Section
	{
	public:
		explicit Section (const char *name);
		~Section(void);
		const char  *getName(void) const;
		Key         *findKey(const char *key) const;
		int          getKeyInt   (const char *key, int index, int defaultValue = 0) const;
		bool         getKeyBool  (const char *key, int index, bool defaultValue = false) const;
		float        getKeyFloat (const char *key, int index, float defaultValue = 0.f) const;
		const char  *getKeyString(const char *key, int index, const char *defaultValue = nullptr) const;
		Tag          getKeyTag   (const char *key, int index, Tag defaultValue = 0) const;
		int          getKeyCount(const char *key) const;
		void         addKey(const char *keyName, const char *value, bool lazyAdd = false);
		void         removeKey(const char *key);
		void         dump(void) const;
		bool         getKeyExists(const char *key) const;

	private:
		/// disabled
		Section(void);
		/// disabled
		Section(const Section&);
		/// disabled
		Section &operator =(const Section&);

	private:
		/** A map of the keys
		  *
		  * Use a map for speed, and since keys are unique
		  */
		KeyMap *m_keys;
		///String representation of the section's name
		char* m_name;
	};
	typedef std::map<const char *, Section *, StringCompare> SectionMap;

public:
	//public interface
	static void            install(void);
	static bool            isInstalled(void);
	static bool            isEmpty();
	static Section        *getSection(const char *name);
	static bool            loadFile(char const * fileName);
	static bool            loadFromBuffer(char const * buffer, int length);
	static bool            loadFromCommandLine(const char *buffer);
	static Section        *createSection(const char *name);
	static void            removeSection(const char *name);
	static void            dump(void);
	//accessors using indexes
	static int             getKeyInt   (const char *section, const char *key, int index, int defaultValue);
	static bool            getKeyBool  (const char *section, const char *key, int index, bool defaultValue);
	static float           getKeyFloat (const char *section, const char *key, int index, float defaultValue);
	static const char     *getKeyString(const char *section, const char *key, int index, const char *defaultValue);
	static Tag             getKeyTag   (const char *section, const char *key, int index, Tag defaultValue);
	//accessors defaulting to retrieving the last entered value
	static DLLEXPORT int   getKeyInt   (const char *section, const char *key, int defaultValue, bool overrideLazyAdds = false);
	static DLLEXPORT bool  getKeyBool  (const char *section, const char *key, bool defaultValue, bool overrideLazyAdds = false);
	static float           getKeyFloat (const char *section, const char *key, float defaultValue, bool overrideLazyAdds = false);
	static const char     *getKeyString(const char *section, const char *key, const char *defaultValue, bool overrideLazyAdds = false);
	static Tag             getKeyTag   (const char *section, const char *key, Tag defaultValue, bool overrideLazyAdds = false);

private:
	/// disabled
	ConfigFile(void);
	/// disabled
	ConfigFile(const ConfigFile &);
	/// disabled
	ConfigFile &operator =(const ConfigFile &);

private:
	//private functions
	static void        processLine(const char *line);
	static void        processKeys(const char *key);
	static void        remove(void);

private:
	//private data members
	static SectionMap *ms_sections;
	static Section    *ms_currentSection;
	static const char *ms_IncludeDelimiter;
	static bool        ms_installed;
	static bool        ms_logConfigSettings;
};

// ======================================================================

#endif


