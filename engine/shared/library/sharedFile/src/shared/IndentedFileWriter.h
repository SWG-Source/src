// ======================================================================
//
// IndentedFileWriter.h
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_IndentedFileWriter_H
#define INCLUDED_IndentedFileWriter_H

// ======================================================================

#include <stdio.h>

// ======================================================================

class IndentedFileWriter
{
public:

	static IndentedFileWriter *createWriter(char const *filename);

public:

	~IndentedFileWriter();

	void  indent();
	void  unindent();

	void  writeLine(char const *line) const;
	void  writeLineFormat(char const *format, ...) const;

private:

	IndentedFileWriter();
	bool createFile(char const *filename);

private:

	int   m_indentCount;
	FILE *m_file;

};

// ======================================================================

#endif
