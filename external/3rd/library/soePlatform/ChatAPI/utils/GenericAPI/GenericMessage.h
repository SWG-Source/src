#if !defined (GENERICMESSAGE_H_)
#define GENERICMESSAGE_H_

#include <time.h>
#include <Base/Archive.h>

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{

#endif
namespace GenericAPI 
{

// Basic request message, implements a type, and a timeout mechanism
class GenericMessage
{
public:
	GenericMessage(short type);
	virtual ~GenericMessage() {};

	short getType() const { return m_type; }
protected:
	short m_type;
};

class GenericRequest : public GenericMessage
{
public:
	GenericRequest(short type);
	virtual ~GenericRequest() {};
#ifdef USE_SERIALIZE_LIB
    virtual const unsigned char *pack(unsigned &msgSize) = 0;
#else
	virtual void pack(Base::ByteStream &msg) = 0;
#endif

	const time_t &getTimeout() const { return m_timeout; }
	void setTimeout(time_t t) { m_timeout = t; }
	void setTrack(unsigned t) { m_track = t; }
	unsigned getTrack() const { return m_track; }

    virtual bool isInputValid() const { return true; }
    virtual unsigned getInputError() const { return 0; }

protected:
	unsigned m_track;
	time_t m_timeout;
};

// Basic response message from server. In the case that this response to a request
// submitted from this API, the response would have been generated at request submission
// time, and the timeout value filled in appropriatly
class GenericResponse : public GenericMessage
{
public:
	GenericResponse(short type, unsigned result, void *user);
	virtual ~GenericResponse() {};
#ifdef USE_SERIALIZE_LIB
    virtual void unpack(const unsigned char *data, unsigned dataLen) = 0;
#else
	virtual void unpack(Base::ByteStream::ReadIterator &iter) = 0;
#endif

	time_t getTimeout() const { return m_timeout; }
	void setTimeout(time_t t) { m_timeout = t; }
	void setTrack(unsigned t) { m_track = t; }
	unsigned getTrack() const { return m_track; }
	unsigned getResult() const { return m_result; }
	void setResult(unsigned res) { m_result = res; }
	void *getUser() const { return m_user; }
protected:
	unsigned m_track;
	unsigned m_result;
	void *m_user;
	time_t m_timeout;
};

};

#ifdef EXTERNAL_DISTRO
};
#endif
#endif
