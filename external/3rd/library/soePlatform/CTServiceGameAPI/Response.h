#if !defined (RESPONSE_H_)
#define RESPONSE_H_

#pragma warning (disable : 4786)

#include "CTGenericAPI/GenericMessage.h"
#include <Base/Archive.h>
#include "CTCommon/RequestStrings.h"

namespace CTService 
{


//----------- TEST CODE ONLY ------------
//-----------------------------------------
class ResTest : public GenericResponse
//-----------------------------------------
{
public:
	ResTest(void *user);
    virtual ~ResTest() {}

	void unpack(Base::ByteStream::ReadIterator &iter);

    inline unsigned	getValue()		 { return m_value; }

private:
	unsigned	m_value;
};

//-----------------------------------------
class ResReplyTest : public GenericResponse
//-----------------------------------------
{
public:
	ResReplyTest(void *user);
    virtual ~ResReplyTest() {}

private:
};
//----------- TEST CODE ONLY ------------



//-----------------------------------------
class ResReplyMoveStatus : public GenericResponse
//-----------------------------------------
{
public:
	ResReplyMoveStatus(void *user);
    virtual ~ResReplyMoveStatus() {}

private:
};

//-----------------------------------------
class ResReplyValidateMove : public GenericResponse
//-----------------------------------------
{
public:
	ResReplyValidateMove(void *user);
    virtual ~ResReplyValidateMove() {}

private:
};

//-----------------------------------------
class ResReplyMove : public GenericResponse
//-----------------------------------------
{
public:
	ResReplyMove(void *user);
    virtual ~ResReplyMove() {}

private:
};

//-----------------------------------------
class ResReplyDelete : public GenericResponse
//-----------------------------------------
{
public:
	ResReplyDelete(void *user);
    virtual ~ResReplyDelete() {}

private:
};

//-----------------------------------------
class ResReplyRestore : public GenericResponse
//-----------------------------------------
{
public:
	ResReplyRestore(void *user);
    virtual ~ResReplyRestore() {}

private:
};

//-----------------------------------------
class ResReplyTransferAccount : public GenericResponse
//-----------------------------------------
{
public:
	ResReplyTransferAccount(void *user);
    virtual ~ResReplyTransferAccount() {}

private:
};

//-----------------------------------------
class ResReplyCharacterList : public GenericResponse
//-----------------------------------------
{
public:
	ResReplyCharacterList(void *user);
    virtual ~ResReplyCharacterList() {}

private:
};

//-----------------------------------------
class ResReplyServerList : public GenericResponse
//-----------------------------------------
{
public:
	ResReplyServerList(void *user);
    virtual ~ResReplyServerList() {}

private:
};

//-----------------------------------------
class ResReplyDestinationServerList : public GenericResponse
//-----------------------------------------
{
public:
	ResReplyDestinationServerList(void *user);
    virtual ~ResReplyDestinationServerList() {}

private:
};

}; // namespace

#endif	//RESPONSE_H_




