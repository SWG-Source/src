//========================================================================
//
// ScriptDictionary.h - interface class to pass dictionaries around. Must be 
// derived by a real class that hold the dictionary data. 
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_ScriptDictionary_H
#define _INCLUDED_ScriptDictionary_H

#include <vector>


namespace boost
{
	template <class T> class shared_ptr;
}


class ScriptDictionary
{
public:

	virtual ~ScriptDictionary();

	virtual void serialize(void) = 0;

	const std::vector<int8> & getSerializedData(void) const;
	uint32                    getCrc(void) const;

protected:

	ScriptDictionary(void);


	std::vector<int8> m_serializedData;
	uint32               m_crc;

private:
	ScriptDictionary(const ScriptDictionary &);
	ScriptDictionary & operator = (const ScriptDictionary &);
};
typedef std::shared_ptr<ScriptDictionary> ScriptDictionaryPtr;


inline ScriptDictionary::ScriptDictionary(void)
{
}

inline ScriptDictionary::~ScriptDictionary()
{
}

inline const std::vector<int8> & ScriptDictionary::getSerializedData(void) const
{
	return m_serializedData;
}

inline uint32 ScriptDictionary::getCrc(void) const
{
	return m_crc;
}


#endif	// _INCLUDED_ScriptDictionary_H
