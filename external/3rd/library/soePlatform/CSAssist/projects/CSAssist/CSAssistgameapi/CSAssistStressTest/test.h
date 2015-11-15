#ifndef TEST_H
#define TEST_H

#include "CSAssistgameapi.h"
#include "CSAssistgameobjects.h"

extern "C" {
#include <stdio.h>
};

using namespace CSAssist;

//---------------------------------------------
class apiTest : public CSAssistGameAPI
//---------------------------------------------
{
public:
	apiTest(const char *, const unsigned, const unsigned, const unsigned);
	virtual ~apiTest();

	void OnConnectCSAssist(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData);
	void OnDisconnectCSAssist(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData);
	void OnNewTicketActivity(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const unsigned NewActivityFlag, const unsigned HasTickets);
	void OnRegisterCharacter(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData);
	void OnUnRegisterCharacter(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData);
	void OnGetIssueHierarchy(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPIXML hierarchyBody, const unsigned modifyDate);
	void OnCreateTicket(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPITicketID ticket);
	void OnAppendTicketComment(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPITicketID ticket);
	void OnGetTicketByID(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPITicket *ticketBody);
	void OnGetTicketComments(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPITicketComment *comments, const unsigned numberRead);
	void OnGetTicketByCharacter(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const unsigned NumberReturned, const unsigned TotalNumber, const CSAssistGameAPITicket *ticketBody);
	void OnTicketChange(const CSAssistGameAPITicketID ticketID, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar *character);
	void OnMarkTicketRead(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData);
	void OnCancelTicket(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPITicketID ticket);
	void OnGetTicketCommentsCount(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPITicketID ticket, const unsigned count);
	void OnRequestGameLocation(const CSAssistGameAPITrack sourceTrack, const CSAssistGameAPIUID uid, const CSAssistUnicodeChar *character, const CSAssistGameAPIUID CSRUID);
	void OnGetDocumentList(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPIDocumentHeader *docList, const unsigned numberRead);
	void OnGetDocument(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistUnicodeChar *documentBody);
	void OnGetTicketXMLBlock(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPIXML XMLBody);
	void OnSearchKB(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPISearchResult *results, const unsigned numberRead);
	void OnGetKBArticle(const CSAssistGameAPITrack track, const CSAssistGameAPIResult result, const void *userData, const CSAssistGameAPIXML articleBody);
	void OnIssueHierarchyChanged(const CSAssistUnicodeChar *version, const CSAssistUnicodeChar *language);

private:


};

#endif
