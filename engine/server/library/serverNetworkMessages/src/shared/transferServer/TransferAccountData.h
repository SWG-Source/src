// TransferAccountData.h
// Copyright 2000-05, Sony Online Entertainment Inc., all rights reserved. 
// Author: Vijay Thakkar

#ifndef	_INCLUDED_TransferAccountData_H
#define	_INCLUDED_TransferAccountData_H

//-----------------------------------------------------------------------
#include "sharedFoundation/StationId.h"
#include <string>
//-----------------------------------------------------------------------

typedef std::pair<std::string, std::string> AvatarData;

class TransferAccountData;

namespace Archive
{
	class ReadIterator;
	void get(ReadIterator & source, TransferAccountData & target);
}

class TransferAccountData
{
public:
	TransferAccountData(const TransferAccountData & rhs);
	TransferAccountData();

	~TransferAccountData();

	unsigned int                              getDestinationStationId       () const;
	unsigned int                              getSourceStationId            () const;
	unsigned int                              getTrack                      () const;
	unsigned int                              getTransactionId              () const;
	const std::string &                       getStartGalaxy                () const;
	bool                                      getDestinationHasAvatars      () const;
	const std::vector<AvatarData> &           getSourceAvatarData           () const;
	std::string                               toString                      () const;

	void                                      setDestinationStationId       (const unsigned int destinationStationId);
	void                                      setSourceStationId            (const unsigned int sourceStationId);
	void                                      setTrack                      (const unsigned int track);
	void                                      setTransactionId              (const unsigned int transactionId);
	void                                      setStartGalaxy                (const std::string & galaxy);
	void                                      setDestinationHasAvatars      (const bool avatars);
	void                                      setSourceAvatarData           (const std::vector<AvatarData> & avatarData);

private:
	friend void Archive::get(Archive::ReadIterator & source, TransferAccountData & target);

	unsigned int                        m_track;
	StationId                           m_sourceStationId;
	StationId                           m_destinationStationId;
	unsigned int                        m_transactionId;
	std::string                         m_startGalaxy;
	bool                                m_destinationHasAvatars;
	// avatarData elements: < clusterName, avatarName >
	std::vector<AvatarData>             m_sourceAvatarData;
};

//-----------------------------------------------------------------------

#endif	// _INCLUDED_TransferAccountData_H
