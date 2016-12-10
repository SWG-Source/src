//===================================================================
//
// OptionManager.h
// asommers
//
// copyright 2002, sony online entertainment
//
//===================================================================

#ifndef INCLUDED_OptionManager_H
#define INCLUDED_OptionManager_H

//===================================================================

class Iff;

//===================================================================

class OptionManager
{
public:

	OptionManager ();
	~OptionManager ();

	static void setOptionManagersEnabled(bool enabled);

	void load (char const * fileName);
	void save (char const * fileName);

	void registerOption (bool & variable, char const * section, char const * name, const int version = 0);
	void registerOption (float & variable, char const * section, char const * name, const int version = 0);
	void registerOption (int & variable, char const * section, char const * name, const int version = 0);
	void registerOption (std::string & variable, char const * section, char const * name, const int version = 0);
	void registerOption (Unicode::String & variable, char const * section, char const * name, const int version = 0);

	float findFloat(char const * section, char const * name, float defaultValue, const int version = 0);

private:

	class Option
	{
		friend class OptionManager;

	public:

		enum Type
		{
			T_bool,
			T_float,
			T_int,
			T_stdString,
			T_unicodeString
		};

	public:

		explicit Option (Type type);
		~Option ();
		Option (Option const & rhs);

		Type getType () const;

		void debugDump (char const * operation) const;

		Option & operator= (Option const & rhs);

	public:

		int m_version;
		char const * m_name;
		char const * m_section;

		union
		{
			bool *        m_bool;
			float *       m_float;
			int *         m_int;
			std::string * m_stdString;
			void *        m_void;
			Unicode::String * m_unicodeString;
		};

	private:

		Option ();

	private:

		Type m_type;
	};

	typedef std::vector<Option> OptionList;

private:

	OptionManager (OptionManager const &);
	OptionManager & operator= (OptionManager const &);

	void load (Iff & iff);
	void save (Iff & iff);

	void loadVersion (Iff & iff, const int version);

	bool        findBool (char const * section, char const * name, bool defaultValue, const int version = 0);
	int         findInt (char const * section, char const * name, int defaultValue, const int version = 0);
	std::string findStdString (char const * section, char const * name, std::string const & defaultValue, const int version = 0);
	Unicode::String findUnicodeString (char const * section, char const * name, Unicode::String const & defaultValue, const int version = 0);
	void        copyOptionListIntersection (const OptionList & src, OptionList & dst);
	void        clearSavedLists ();

private:

	OptionList * const m_registeredOptionList;
	OptionList * const m_savedOptionList;

	typedef std::vector<char*> StringList;
	StringList * const m_stringList;

	typedef std::vector<bool*> BoolList;
	BoolList * const m_boolList;

	typedef std::vector<float*> FloatList;
	FloatList * const m_floatList;

	typedef std::vector<int*> IntList;
	IntList * const m_intList;

	typedef std::vector<std::string*> StdStringList;
	StdStringList * const m_stdStringList;

	typedef std::vector<Unicode::String*> UnicodeStringList;
	UnicodeStringList * const m_unicodeStringList;
};

//===================================================================

#endif
