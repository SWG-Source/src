#ifndef CHARACTER_H
#define CHARACTER_H

#include "Base/Archive.h"

namespace AuctionTransfer
{


class Character
{
public:
	// constructors
	Character()
		: m_name(""), m_id(0), m_XMLdata("") 
	{
	}

	Character(const std::string &name, unsigned id, const std::string &xmlData)
		: m_name(name), m_id(id), m_XMLdata(xmlData)
	{
	}

	Character(const Character &character)
		: m_name(character.getName()), m_id(character.getID()), m_XMLdata(character.getData())
	{
	}

	// destructor
	~Character() 
	{
	}

	// accessor methods
	std::string getName() const 			{ return m_name; }
	unsigned	getID() const				{ return m_id; }
	std::string getData() const				{ return m_XMLdata; }

	void setName(const std::string &name)	{ m_name = name; }
	void setID(unsigned id)					{ m_id = id; }
	void setData(const std::string &data)	{ m_XMLdata = data; }

protected:
	std::string m_name;
	unsigned m_id;
	std::string m_XMLdata;
};

}; // namespace AuctionTransfer

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{
#endif


namespace Base
{

	void get(ByteStream::ReadIterator & source, AuctionTransfer::Character & target);
	void put(ByteStream & target, const AuctionTransfer::Character & source);
};


#ifdef EXTERNAL_DISTRO
};
#endif

#endif //CHARACTER_H

