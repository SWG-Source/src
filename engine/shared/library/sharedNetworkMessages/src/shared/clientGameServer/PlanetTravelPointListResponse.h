// ==================================================================
//
// PlanetTravelPointListResponse.h
// Copyright 2002 Sony Online Entertainment
//
// ==================================================================

#ifndef	INCLUDED_PlanetTravelPointListResponse_H
#define	INCLUDED_PlanetTravelPointListResponse_H

// ==================================================================

#include "Archive/AutoDeltaByteStream.h"
#include "sharedNetworkMessages/GameNetworkMessage.h"
#include "sharedMath/Vector.h"

// ==================================================================

class PlanetTravelPointListResponse : public GameNetworkMessage
{
public: 

	PlanetTravelPointListResponse (const std::string& planetName, const std::vector<std::string>& travelPointNameList, const std::vector<Vector>& travelPointPointList, const std::vector<int>& travelPointCostList, const std::vector<bool>& travelPointInterplanetaryList, int sequenceId);
	explicit PlanetTravelPointListResponse (Archive::ReadIterator &source);
	virtual ~PlanetTravelPointListResponse();

	const std::string&              getPlanetName                    () const;
	const std::vector<std::string>& getTravelPointNameList           () const;
	const std::vector<Vector>&      getTravelPointPointList          () const;
	const std::vector<int>&         getTravelPointCostList           () const;
	const std::vector<bool>&        getTravelPointInterplanetaryList () const;
	const int                       getSequenceId                    () const;

private:

	PlanetTravelPointListResponse ();

public:

	static const char* const cms_name;

private:

	Archive::AutoVariable<std::string> m_planetName;
	Archive::AutoArray<std::string>    m_travelPointNameList;
	Archive::AutoArray<Vector>         m_travelPointPointList;
	Archive::AutoArray<int>            m_travelPointCostList;
	Archive::AutoArray<bool>           m_travelPointInterplanetaryList;
	Archive::AutoVariable<int>         m_sequenceId;
};

// ==================================================================

#endif 
