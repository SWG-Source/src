// ==================================================================
//
// PlanetTravelPointListResponse.cpp
// Copyright 2002 Sony Online Entertainment
//
// ==================================================================

#include "sharedNetworkMessages/FirstSharedNetworkMessages.h"
#include "sharedNetworkMessages/PlanetTravelPointListResponse.h"

#include "sharedMathArchive/VectorArchive.h"

// ==================================================================

const char* const PlanetTravelPointListResponse::cms_name = "PlanetTravelPointListResponse";

// ==================================================================

PlanetTravelPointListResponse::PlanetTravelPointListResponse (const std::string& planetName, const std::vector<std::string>& travelPointNameList, const std::vector<Vector>& travelPointPointList, const std::vector<int>& travelPointCostList, const std::vector<bool>& travelPointInterplanetaryList, int sequenceId) :
GameNetworkMessage (cms_name),
m_planetName (planetName),
m_travelPointNameList (),
m_travelPointPointList (),
m_travelPointCostList (),
m_travelPointInterplanetaryList (),
m_sequenceId                    (sequenceId)
{
	m_travelPointNameList.set (travelPointNameList);
	m_travelPointPointList.set (travelPointPointList);
	m_travelPointCostList.set (travelPointCostList);
	m_travelPointInterplanetaryList.set (travelPointInterplanetaryList);
	
	addVariable (m_planetName);
	addVariable (m_travelPointNameList);
	addVariable (m_travelPointPointList);
	addVariable (m_travelPointCostList);
	addVariable (m_travelPointInterplanetaryList);
//	addVariable (m_sequenceId);
}

// ------------------------------------------------------------------

PlanetTravelPointListResponse::PlanetTravelPointListResponse (Archive::ReadIterator& source) :
GameNetworkMessage (cms_name),
m_planetName (),
m_travelPointNameList (),
m_travelPointPointList (),
m_travelPointCostList (),
m_travelPointInterplanetaryList (),
m_sequenceId                    (0)
{
	addVariable (m_planetName);
	addVariable (m_travelPointNameList);
	addVariable (m_travelPointPointList);
	addVariable (m_travelPointCostList);
	addVariable (m_travelPointInterplanetaryList);
//	addVariable (m_sequenceId);
	unpack (source);
}

// ------------------------------------------------------------------

PlanetTravelPointListResponse::~PlanetTravelPointListResponse ()
{
}

// ------------------------------------------------------------------

const std::string& PlanetTravelPointListResponse::getPlanetName () const
{
	return m_planetName.get ();
}

// ------------------------------------------------------------------

const std::vector<std::string>& PlanetTravelPointListResponse::getTravelPointNameList () const
{
	return m_travelPointNameList.get ();
}

// ------------------------------------------------------------------

const std::vector<Vector>& PlanetTravelPointListResponse::getTravelPointPointList () const
{
	return m_travelPointPointList.get ();
}

// ------------------------------------------------------------------

const std::vector<int>& PlanetTravelPointListResponse::getTravelPointCostList () const
{
	return m_travelPointCostList.get ();
}

// ------------------------------------------------------------------

const std::vector<bool>& PlanetTravelPointListResponse::getTravelPointInterplanetaryList () const
{
	return m_travelPointInterplanetaryList.get ();
}

//----------------------------------------------------------------------

const int PlanetTravelPointListResponse::getSequenceId                    () const
{
	return m_sequenceId.get ();
}

// ==================================================================
