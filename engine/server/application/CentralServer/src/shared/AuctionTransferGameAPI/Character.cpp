#include "Character.h"

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{
#endif


namespace Base
{

void get(ByteStream::ReadIterator & source, AuctionTransfer::Character & target)
{
	std::string tmpStr;
	unsigned tmpNum;
	
	//name
	get(source, tmpStr);
	target.setName(tmpStr);
	
	// id
	get(source, tmpNum);
	target.setID(tmpNum);

	// data
	get(source, tmpStr);
	target.setData(tmpStr);
}

void put(ByteStream & target, const AuctionTransfer::Character &source)
{
	put(target, source.getName());
	put(target, source.getID());
	put(target, source.getData());
}

};


#ifdef EXTERNAL_DISTRO
};
#endif


