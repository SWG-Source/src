// ======================================================================
//
// Branch.h
// copyright 2005 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_Branch_H
#define INCLUDED_Branch_H

#include <string>

class Branch
{
public:
	Branch();

	const std::string & getBranchName() const;

private:
	std::string m_branchName;
};

// ======================================================================

#endif
