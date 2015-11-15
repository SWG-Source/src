#ifndef CTSERVICESERVER_H
#define CTSERVICESERVER_H

namespace CTService 
{

//--------------------------------------------------------
class CTServiceServer
//--------------------------------------------------------
{
	inline void copy_wide_string(unsigned short * target, const unsigned short * source, int length)
	{
		if (!target)
			return;
		if (!source)
		{
			target[0] = 0;
			return;
		}
	
		int offset = 0;
		while (offset < length && (target[(offset+1)] = source[offset])) {		
			target[length-1] = 0;
			offset++;
		}
	}
	public:
		enum { SERVER_SIZE=64, SERVER_BUFFER, REASON_SIZE=4096, REASON_BUFFER };

		CTServiceServer() : mServer(), mCanRename(true), mCanMove(true), mCanTransfer(true), mRenameReason(), mMoveReason(), mTransferReason() 
			{ mServer[0]		= 0;
			  mRenameReason[0]	= 0;
			  mMoveReason[0]	= 0;
			  mTransferReason[0]= 0; }

		CTServiceServer(const unsigned short *server) : mServer(), mCanRename(true), mCanMove(true), mCanTransfer(true), mRenameReason(), mMoveReason(), mTransferReason() 
			{ copy_wide_string(mServer, server, SERVER_BUFFER);
			  mRenameReason[0]	= 0;
			  mMoveReason[0]	= 0;
			  mTransferReason[0]= 0; }

		void			SetServer(const unsigned short * value)				{ copy_wide_string(mServer, value, SERVER_BUFFER); }
		unsigned short	*GetServer()										{ return mServer; }

		void			SetCanRename(const bool value)						{ mCanRename = value; }
		const bool		GetCanRename() const								{ return mCanRename; }
		void			SetCanMove(const bool value)						{ mCanMove = value; }
		const bool		GetCanMove() const									{ return mCanMove; }
		void			SetCanTransfer(const bool value)					{ mCanTransfer = value; }
		const bool		GetCanTransfer() const								{ return mCanTransfer; }

		void			SetRenameReason(const unsigned short * value)		{ copy_wide_string(mRenameReason, value, REASON_BUFFER);  }
		const unsigned short * GetRenameReason() const						{ return mRenameReason; }
		void			SetMoveReason(const unsigned short * value)			{ copy_wide_string(mMoveReason, value, REASON_BUFFER);  }
		const unsigned short * GetMoveReason() const						{ return mMoveReason; }
		void			SetTransferReason(const unsigned short * value)		{ copy_wide_string(mTransferReason, value, REASON_BUFFER);  }
		const unsigned short * GetTransferReason() const					{ return mTransferReason; }

	private:
		unsigned short	mServer[SERVER_BUFFER];
		bool			mCanRename;
		bool			mCanMove;
		bool			mCanTransfer;
		unsigned short	mRenameReason[REASON_BUFFER];
		unsigned short	mMoveReason[REASON_BUFFER];
		unsigned short	mTransferReason[REASON_BUFFER];
	};

}; // namespace

#endif	//CTSERVICESERVER_H

