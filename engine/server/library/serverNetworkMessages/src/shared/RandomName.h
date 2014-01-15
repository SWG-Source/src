// RandomName.h
// copyright 2001 Verant Interactive

#ifndef	_INCLUDED_RandomName_H
#define	_INCLUDED_RandomName_H

//-----------------------------------------------------------------------

class RandomNameRequest: public GameNetworkMessage
{
public:
	explicit RandomNameRequest(uint32 stationId, const std::string &creatureTemplate);
	explicit RandomNameRequest(Archive::ReadIterator & source);
	~RandomNameRequest();

	const uint32 getStationId() const;
	const std::string &getCreatureTemplate() const;
private:
	Archive::AutoVariable<uint32> m_stationId;
	Archive::AutoVariable<std::string> m_creatureTemplate;

	RandomNameRequest(const RandomNameRequest & source);
	RandomNameRequest & operator = (const RandomNameRequest & other);
};

//--------------------------------------------------------------------

inline const uint32 RandomNameRequest::getStationId() const
{
	return m_stationId.get();
}

//--------------------------------------------------------------------

inline const std::string &RandomNameRequest::getCreatureTemplate() const
{
	return m_creatureTemplate.get();
}

//-----------------------------------------------------------------------

class RandomNameResponse: public GameNetworkMessage
{
public:
	explicit RandomNameResponse(uint32 stationId, const std::string &creatureTemplate, const Unicode::String &name, const StringId &errorMessage);
	explicit RandomNameResponse(Archive::ReadIterator & source);
	~RandomNameResponse();

	const uint32 getStationId() const;
	const std::string &getCreatureTemplate() const;
	const Unicode::String &getName() const;
	const StringId &getErrorMessage() const;
private:
	Archive::AutoVariable<uint32> m_stationId;
	Archive::AutoVariable<std::string> m_creatureTemplate;
	Archive::AutoVariable<Unicode::String> m_name;
	Archive::AutoVariable<StringId> m_errorMessage;

	RandomNameResponse(const RandomNameRequest & source);
	RandomNameResponse & operator = (const RandomNameRequest & other);
};

//--------------------------------------------------------------------

inline const std::string &RandomNameResponse::getCreatureTemplate() const
{
	return m_creatureTemplate.get();
}

//--------------------------------------------------------------------

inline const uint32 RandomNameResponse::getStationId() const
{
	return m_stationId.get();
}

//--------------------------------------------------------------------

inline const Unicode::String &RandomNameResponse::getName() const
{
	return m_name.get();
}

//--------------------------------------------------------------------

inline const StringId &RandomNameResponse::getErrorMessage() const
{
	return m_errorMessage.get();
}

// ======================================================================

#endif

