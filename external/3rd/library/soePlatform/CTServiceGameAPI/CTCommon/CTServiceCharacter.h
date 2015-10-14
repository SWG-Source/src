#ifndef CTSERVICECHARACTER_H
#define CTSERVICECHARACTER_H

namespace CTService 
{

//--------------------------------------------------------
class CTServiceCharacter
//--------------------------------------------------------
{
	typedef unsigned short uchar_t;
	inline void copy_wide_string(uchar_t * target, const uchar_t * source, int length)
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
		enum { CHARACTER_SIZE=64, CHARACTER_BUFFER, REASON_SIZE=4096, REASON_BUFFER };

		CTServiceCharacter() : mCharacter(), mCanRename(true), mCanMove(true), mCanTransfer(true), mRenameReason(), mMoveReason(), mTransferReason() 
			{ mCharacter[0]		= 0;
			  mRenameReason[0]	= 0;
			  mMoveReason[0]	= 0;
			  mTransferReason[0]= 0; }

		CTServiceCharacter(const unsigned short *ch) : mCharacter(), mCanRename(true), mCanMove(true), mCanTransfer(true), mRenameReason(), mMoveReason(), mTransferReason() 
			{ copy_wide_string(mCharacter, ch, CHARACTER_BUFFER);
			  mRenameReason[0]	= 0;
			  mMoveReason[0]	= 0;
			  mTransferReason[0]= 0; }

		void			SetCharacter(const uchar_t * value)			{ copy_wide_string(mCharacter, value, CHARACTER_BUFFER); }
		const uchar_t * GetCharacter() const						{ return mCharacter; }

		void			SetCanRename(const bool value)				{ mCanRename = value; }
		const bool		GetCanRename() const 						{ return mCanRename; }
		void			SetCanMove(const bool value)				{ mCanMove = value; }
		const bool		GetCanMove() const							{ return mCanMove; }
		void			SetCanTransfer(const bool value)			{ mCanTransfer = value; }
		const bool		GetCanTransfer() const						{ return mCanTransfer; }

		void			SetRenameReason(const uchar_t * value)		{ copy_wide_string(mRenameReason, value, REASON_BUFFER);  }
		const uchar_t * GetRenameReason() const 					{ return mRenameReason; }
		void			SetMoveReason(const uchar_t * value)		{ copy_wide_string(mMoveReason, value, REASON_BUFFER);  }
		const uchar_t * GetMoveReason() const						{ return mMoveReason; }
		void			SetTransferReason(const uchar_t * value)	{ copy_wide_string(mTransferReason, value, REASON_BUFFER);  }
		const uchar_t * GetTransferReason() const					{ return mTransferReason; }

	private:
		uchar_t	mCharacter[CHARACTER_BUFFER];
		bool	mCanRename;
		bool	mCanMove;
		bool	mCanTransfer;
		uchar_t	mRenameReason[REASON_BUFFER];
		uchar_t	mMoveReason[REASON_BUFFER];
		uchar_t	mTransferReason[REASON_BUFFER];
	};

}; // namespace

#endif	//CTSERVICECHARACTER_H

