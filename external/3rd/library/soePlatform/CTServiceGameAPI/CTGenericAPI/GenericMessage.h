#if !defined (GENERICMESSAGE_H_)
#define GENERICMESSAGE_H_

//----------------------------------------
//
// WARNING: These files are NOT standard generic API files
// They have been modified for this project.
// Do NOT replace them with generic API files
//
//----------------------------------------

#include <time.h>
#include <Base/Archive.h>

#ifdef EXTERNAL_DISTRO
namespace NAMESPACE 
{
#endif

//-------------------------------------------
class GenericRequest
//-------------------------------------------
{
public:
	GenericRequest(short type, unsigned server_track = 0);
	virtual ~GenericRequest() {};

	virtual void			pack(Base::ByteStream &msg) = 0;
	short					getType() const					{ return m_type; }
	void					setTimeout(time_t t)			{ m_timeout = t; }
	time_t					getTimeout()					{ return m_timeout; }
	void					setTrack(unsigned t)			{ m_track = t; }
	unsigned				getTrack() const				{ return m_track; }
	inline const unsigned	getMappedServerTrack() const	{ return m_server_track; }
	inline void				setServerTrack(unsigned track)	{ m_server_track = track; }

protected:
	short		m_type;
	unsigned	m_track;
	time_t		m_timeout;
	unsigned	m_server_track;
};

// Basic response message from server. In the case that this response to a request
// submitted from this API, the response would have been generated at request submission
// time, and the timeout value filled in appropriatly
//-------------------------------------------
class GenericResponse
//-------------------------------------------
{
public:
	GenericResponse(short type, unsigned result, void *user);
	virtual ~GenericResponse() {};
	virtual void unpack(Base::ByteStream::ReadIterator &iter);

	short		getType() const			{ return m_type; }
	void		setTimeout(time_t t)	{ m_timeout = t; }
	time_t		getTimeout()			{ return m_timeout; }
	void		setTrack(unsigned t)	{ m_track = t; }
	unsigned	getTrack() const		{ return m_track; }
	unsigned	getResult() const		{ return m_result; }
	void		setResult(unsigned res) { m_result = res; }
	void *		getUser() const			{ return m_user; }
protected:
	short		m_type;
	unsigned	m_track;
	unsigned	m_result;
	void		*m_user;
	time_t		m_timeout;
};

#ifdef EXTERNAL_DISTRO
};
#endif
#endif
