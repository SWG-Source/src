#ifndef AUCTIONTRANSFERENUM_H
#define AUCTIONTRANSFERENUM_H

#include <map>
#include <string>

#define DEFAULT_TIMEOUTSECS 60
//////////////////////////////////////////////////////////////////////////////////////
namespace AuctionTransfer
{
	enum CloseReason 
	{ 
		REASON_HOST_UNREACHABLE,
		REASON_LOCAL_DISCONNECT, 
		REASON_REMOTE_DISCONNECT, 
		REASON_SOCKET_FAILURE, 
		REASON_DNS_FAILURE  
	};

	enum ResponseCode
	{
		RESPONSE_FIRST = 0,
		RESPONSE_UNKNOWN = RESPONSE_FIRST,	// Initial state internally for call denotes no response yet
		RESPONSE_ACCEPTED = 1,
		RESPONSE_REJECTED,
		RESPONSE_FAILURE,
		RESPONSE_DUPLICATE,
		GAME_API_TIME_OUT,								//5
		TRANSFER_SERVER_TIME_OUT,
		AUCTION_API_TIME_OUT,
		RESPONSE_REJECTED_INVALID_XML,
		RESPONSE_REJECTED_USER_NOT_REGISTERED,
		RESPONSE_REJECTED_UNABLE_TO_VALIDATE_USER,		//10
		RESPONSE_REJECTED_NO_PAPER_DOLL_IMAGE,
		RESPONSE_REJECTED_INVALID_TRANSACTION_ID,
		RESPONSE_REJECTED_USER_NOT_AUTHORIZED,
		RESPONSE_REJECTED_HOLDING_AREA_FULL,
		RESPONSE_REJECTED_HOLDING_AREA_ASSET_TOO_OLD,	//15
		RESPONSE_REJECTED_CHARACTER_LIMIT_REACHED,
		RESPONSE_REJECTED_CHARACTER_NOT_AVAILABLE,
		RESPONSE_REJECTED_MAILBOX_FULL,
		RESPONSE_REJECTED_GAME_SERVER_DOWN,
		RESPONSE_NAME_INVALID,							//20
		RESPONSE_NAME_TAKEN,
		RESPONSE_FAILURE_DATABASE,
		RESPONSE_FAILURE_SERVER,
		RESPONSE_FAILURE_NETWORK,
		RESPONSE_INVALID_ALIGNMENT,						//25
		RESPONSE_WEB_SERVICE_TIMEOUT,
		RESPONSE_REJECTED_EMAIL_NOT_VALIDATED,

		//NOTE: THESE ALWAYS STAY AT THE END.  IF EVER ADD A VALUE AFTER THE LAST RESPONSE_*, THEN IT NEEDS TO BE ASSIGNED BELOW
		RESPONSE_LAST = RESPONSE_REJECTED_EMAIL_NOT_VALIDATED,
		RESPONSE_COUNT = RESPONSE_LAST - RESPONSE_FIRST + 1
	};

	enum RequestTypes
	{
		REQUEST_SET_API = 0,
		REQUEST_SET_SERVER_LIST,
		GAME_REQUEST_SEND_PREPARE_TRANSACTION,
		GAME_REQUEST_SEND_COMMIT,
		GAME_REQUEST_SEND_ABORT,
		GAME_REQUEST_RECEIVE_PREPARE_TRANSACTION,	//5
		GAME_REQUEST_RECEIVE_COMMIT,				
		GAME_REQUEST_RECEIVE_ABORT,
		GAME_REPLY_RECEIVE_PREPARE_TRANSACTION,
		GAME_REPLY_RECEIVE_COMMIT,
		GAME_REPLY_RECEIVE_ABORT,					//10
		GAME_REQUEST_RECEIVE_GET_CHARACTER_LIST,
		GAME_REPLY_RECEIVE_GET_CHARACTER_LIST,
		GAME_SEND_AUDIT_MESSAGE,
		GAME_REQUEST_GET_TRANSACTION_ID,
		GAME_REQUEST_SEND_PREPARE_TRANSACTION_COMPRESSED,

		WEB_REQUEST_SEND_PREPARE_TRANSACTION = 10000,
		WEB_REQUEST_SEND_COMMIT,
		WEB_REQUEST_SEND_ABORT,
		WEB_REQUEST_RECEIVE_PREPARE_TRANSACTION,
		WEB_REQUEST_RECEIVE_COMMIT,
		WEB_REQUEST_RECEIVE_ABORT,					//10005
		WEB_REPLY_RECEIVE_PREPARE_TRANSACTION,
		WEB_REPLY_RECEIVE_COMMIT,
		WEB_REPLY_RECEIVE_ABORT,
		WEB_REQUEST_SEND_GET_CHARACTER_LIST,
		WEB_REQUEST_SEND_GET_SERVER_LIST,			//10010
		WEB_REPLY_RECEIVE_AUDIT,
		WEB_REQUEST_RECEIVE_GET_TRANSACTION_ID,
		WEB_REPLY_RECEIVE_GET_TRANSACTION_ID,
		WEB_REQUEST_RECEIVE_PREPARE_TRANSACTION_COMPRESSED,
		WEB_REPLY_RECEIVE_PREPARE_TRANSACTION_COMPRESSED

	};

	#define response_text std::pair<unsigned,const char *>
	static const response_text _responseString[RESPONSE_COUNT] = 
	{  
		response_text(RESPONSE_UNKNOWN, "RESPONSE_UNKNOWN"),
			response_text(RESPONSE_ACCEPTED, "RESPONSE_ACCEPTED"),
			response_text(RESPONSE_REJECTED, "RESPONSE_REJECTED"),
			response_text(RESPONSE_FAILURE, "RESPONSE_FAILURE"),
			response_text(RESPONSE_DUPLICATE, "RESPONSE_DUPLICATE"),
			response_text(GAME_API_TIME_OUT, "GAME_API_TIME_OUT"),
			response_text(TRANSFER_SERVER_TIME_OUT, "TRANSFER_SERVER_TIME_OUT"),
			response_text(AUCTION_API_TIME_OUT, "AUCTION_API_TIME_OUT"),
			response_text(RESPONSE_REJECTED_INVALID_XML, "RESPONSE_REJECTED_INVALID_XML"),
			response_text(RESPONSE_REJECTED_USER_NOT_REGISTERED, "RESPONSE_REJECTED_USER_NOT_REGISTERED"),
			response_text(RESPONSE_REJECTED_UNABLE_TO_VALIDATE_USER, "RESPONSE_REJECTED_UNABLE_TO_VALIDATE_USER"),
			response_text(RESPONSE_REJECTED_NO_PAPER_DOLL_IMAGE, "RESPONSE_REJECTED_NO_PAPER_DOLL_IMAGE"),
			response_text(RESPONSE_REJECTED_INVALID_TRANSACTION_ID, "RESPONSE_REJECTED_INVALID_TRANSACTION_ID"),
			response_text(RESPONSE_REJECTED_USER_NOT_AUTHORIZED, "RESPONSE_REJECTED_USER_NOT_AUTHORIZED"),
			response_text(RESPONSE_REJECTED_HOLDING_AREA_FULL, "RESPONSE_REJECTED_HOLDING_AREA_FULL"),
			response_text(RESPONSE_REJECTED_HOLDING_AREA_ASSET_TOO_OLD, "RESPONSE_REJECTED_HOLDING_AREA_ASSET_TOO_OLD"),
			response_text(RESPONSE_REJECTED_CHARACTER_LIMIT_REACHED, "RESPONSE_REJECTED_CHARACTER_LIMIT_REACHED"),
			response_text(RESPONSE_REJECTED_CHARACTER_NOT_AVAILABLE, "RESPONSE_REJECTED_CHARACTER_NOT_AVAILABLE"),
			response_text(RESPONSE_REJECTED_MAILBOX_FULL, "RESPONSE_REJECTED_MAILBOX_FULL"),
			response_text(RESPONSE_REJECTED_GAME_SERVER_DOWN, "RESPONSE_REJECTED_GAME_SERVER_DOWN"),
			response_text(RESPONSE_NAME_INVALID, "RESPONSE_NAME_INVALID"),
			response_text(RESPONSE_NAME_TAKEN, "RESPONSE_NAME_TAKEN"),
			response_text(RESPONSE_FAILURE_DATABASE, "RESPONSE_FAILURE_DATABASE"),
			response_text(RESPONSE_FAILURE_SERVER, "RESPONSE_FAILURE_SERVER"),
			response_text(RESPONSE_FAILURE_NETWORK, "RESPONSE_FAILURE_NETWORK"),
			response_text(RESPONSE_INVALID_ALIGNMENT, "RESPONSE_INVALID_ALIGNMENT"),
			response_text(RESPONSE_WEB_SERVICE_TIMEOUT, "RESPONSE_WEB_SERVICE_TIMEOUT"),
			response_text(RESPONSE_REJECTED_EMAIL_NOT_VALIDATED, "RESPONSE_EMAIL_NOT_VALIDATED"),
	};
	static std::map<unsigned,const char *> ResponseString((const std::map<unsigned,const char *>::value_type *)&_responseString[0],(const std::map<unsigned,const char *>::value_type *)&_responseString[RESPONSE_COUNT]);

	static const response_text _responseText[RESPONSE_COUNT] = 
	{  
		response_text(RESPONSE_UNKNOWN, "Unknown response code - default."),
			response_text(RESPONSE_ACCEPTED, "The operation completed successfully."),
			response_text(RESPONSE_REJECTED, "The operation was rejected by by the auction system."),
			response_text(RESPONSE_FAILURE, "There was a general failure processing the request."),
			response_text(RESPONSE_DUPLICATE, "This transaction was initiated previously."),
			response_text(GAME_API_TIME_OUT, "Unable to contact the Auction Transfer Server."),
			response_text(TRANSFER_SERVER_TIME_OUT, "The Auction Transfer Server could not Contact the Web Services."),
			response_text(AUCTION_API_TIME_OUT, "The SysEng Web Services did not respond."),
			response_text(RESPONSE_REJECTED_INVALID_XML, "The XML format was invalid."),
			response_text(RESPONSE_REJECTED_USER_NOT_REGISTERED, "The user has not registered in the Auction System yet."),
			response_text(RESPONSE_REJECTED_UNABLE_TO_VALIDATE_USER, "The user account was not able to be validated at this time."),
			response_text(RESPONSE_REJECTED_NO_PAPER_DOLL_IMAGE, "There was no image uploaded for this character yet."),
			response_text(RESPONSE_REJECTED_INVALID_TRANSACTION_ID, "The transaction id is not valid."),
			response_text(RESPONSE_REJECTED_USER_NOT_AUTHORIZED, "The station account is not authorized."),
			response_text(RESPONSE_REJECTED_HOLDING_AREA_FULL, "The user has too many items in their holding area."),
			response_text(RESPONSE_REJECTED_HOLDING_AREA_ASSET_TOO_OLD, "The user has an item in their holding area that is too old."),
			response_text(RESPONSE_REJECTED_CHARACTER_LIMIT_REACHED, "Too Many characters in Auctions."),
			response_text(RESPONSE_REJECTED_CHARACTER_NOT_AVAILABLE, "The character is not available for this transaction."),
			response_text(RESPONSE_REJECTED_MAILBOX_FULL, "The character mailbox is full."),
			response_text(RESPONSE_REJECTED_GAME_SERVER_DOWN, "The game server is unable to process the request."),
			response_text(RESPONSE_NAME_INVALID, "The new character name is invalid."),
			response_text(RESPONSE_NAME_TAKEN, "The new character name is taken."),
			response_text(RESPONSE_FAILURE_DATABASE, "There was a database failure during the transaction."),
			response_text(RESPONSE_FAILURE_SERVER, "There was a server failure during the transaction."),
			response_text(RESPONSE_FAILURE_NETWORK, "There was a network failure during the transaction."),
			response_text(RESPONSE_INVALID_ALIGNMENT, "The alignment of the character for this item is incorrect."),
			response_text(RESPONSE_WEB_SERVICE_TIMEOUT, "The AppEng web service failed to respond."),
			response_text(RESPONSE_REJECTED_EMAIL_NOT_VALIDATED, "The user has not validated their email with the auction system yet."),
	};
	static std::map<unsigned,const char *> ResponseText((const std::map<unsigned,const char *>::value_type *)&_responseText[0],(const std::map<unsigned,const char *>::value_type *)&_responseText[RESPONSE_COUNT]);


}; //namespace AuctionTransfer
#endif //AUCTIONTRANSFERENUM_H
