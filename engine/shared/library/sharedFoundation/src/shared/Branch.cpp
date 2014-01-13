// ======================================================================
//
// Branch.cpp
//
// copyright 2005 Sony Online Entertainment
// All Rights Reserved
//
// ======================================================================

#include "sharedFoundation/FirstSharedFoundation.h"
#include "sharedFoundation/Branch.h"

//----------------------------------------------------------------------

namespace BranchNamespace
{
#ifdef _WIN32
	const char * cs_win32PathSeperator = "\\";
#endif
	const char * cs_pathSeperator = "/";
	const char * cs_branchIdentifierString = "/src/engine/shared";
}

using namespace BranchNamespace;

//----------------------------------------------------------------------

Branch::Branch()
{
	m_branchName = __FILE__;

	std::string::size_type slash = 0;

#ifdef _WIN32
	while ( ( slash = m_branchName.find( cs_win32PathSeperator, slash ) ) != std::string::npos )
	{
		m_branchName[ slash ] = *cs_pathSeperator;
	}
#endif

	slash = m_branchName.find( cs_branchIdentifierString );

	if ( slash != std::string::npos )
	{
		m_branchName.resize( slash );

		slash = m_branchName.rfind( cs_pathSeperator );

		if ( slash != std::string::npos )
		{
			m_branchName = m_branchName.substr( ++slash );
		}
	}
}

//----------------------------------------------------------------------

const std::string & Branch::getBranchName() const
{
	return m_branchName;
}

//----------------------------------------------------------------------
