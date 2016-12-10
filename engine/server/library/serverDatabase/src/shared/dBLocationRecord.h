// ======================================================================
//
// dBLocationRecord.h
// copyright (c) 2004 Sony Online Entertainment
// Author: Doug Mellencamp
//
// ======================================================================

#ifndef INCLUDED_dBLocationRecord_H
#define INCLUDED_dBLocationRecord_H

// ======================================================================

#include "Unicode.h"
#include "sharedFoundation/NetworkId.h"

// ======================================================================

/**
 * Data structures for querying Location.
 */
struct LocationRecord
{
  NetworkId                  m_locationId;
  NetworkId                  m_ownerId;
  std::string                m_locationString;
  int                        m_salesTax;
  NetworkId                  m_salesTaxBankId;
  int                        m_emptyDate;
  int                        m_lastAccessDate;
  int                        m_inactiveDate;
  int                        m_status;
  bool			     m_searchEnabled;
  int                        m_entranceCharge;
};

// ======================================================================

typedef std::vector<LocationRecord> LocationList;

// ======================================================================

#endif
