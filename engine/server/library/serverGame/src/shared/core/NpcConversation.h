//========================================================================
//
// NpcConversation.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef INCLUDED_NpcConversation_H
#define INCLUDED_NpcConversation_H

#include "StringId.h"
#include "sharedGame/ProsePackage.h"
#include "sharedObject/CachedNetworkId.h"

#include <string>

class TangibleObject;
class StringId;

//----------------------------------------------------------------------

class NpcConversation
{
public:
	NpcConversation  (TangibleObject & player, TangibleObject & npc, const std::string & convoName, uint32 appearanceOverrideSharedTemplateCrc);
	~NpcConversation ();

	class Response
	{
	public:
		StringId     stringId;
		ProsePackage pp;

		bool         isValid () const { return !stringId.isInvalid () || !pp.stringId.isInvalid (); }

		bool operator == (const Response & rhs) const
		{
			return stringId == rhs.stringId && pp == rhs.pp;
		}
	};

	const CachedNetworkId & getPlayer        () const;
	const CachedNetworkId & getNPC           () const;
	const std::string &     getName          () const;

	void                    sendMessage      (const Response & response, const Unicode::String & oob);
	void                    addResponse      (const Response & response);
	void                    removeResponse   (const Response & response);
	void                    clearResponses   ();
	void                    sendResponses    ();

	int                     getResponseCount () const;
	const Response &        getResponse      (int index) const;
	uint32                  getAppearanceOverrideSharedTemplateCrc() const;

	void setFinalStringId(StringId const & stringId);
	StringId const & getFinalStringId() const;

	void setFinalProsePackage(Unicode::String const & oob);
	Unicode::String const & getFinalProsePackage() const;

private:

	std::string                      m_convoName;
	CachedNetworkId                  m_player;
	CachedNetworkId                  m_npc;

	typedef std::vector<Response> ResponseVector;
	ResponseVector *                 m_responses;
	uint32                           m_appearanceOverrideSharedTemplateCrc;


	StringId                         m_finalStringId;
	Unicode::String                  m_finalProsePackage;

private:
	// no copying
	NpcConversation(const NpcConversation &src);
	NpcConversation & operator =(const NpcConversation &src);
};

//----------------------------------------------------------------------

inline const CachedNetworkId & NpcConversation::getPlayer() const
{
	return m_player;
}

//----------------------------------------------------------------------

inline const CachedNetworkId & NpcConversation::getNPC() const
{
	return m_npc;
}

//----------------------------------------------------------------------

inline const std::string & NpcConversation::getName() const
{
	return m_convoName;
}

//----------------------------------------------------------------------

inline uint32 NpcConversation::getAppearanceOverrideSharedTemplateCrc() const
{
	return m_appearanceOverrideSharedTemplateCrc;
}

//----------------------------------------------------------------------

#endif	// INCLUDED_NpcConversation_H
