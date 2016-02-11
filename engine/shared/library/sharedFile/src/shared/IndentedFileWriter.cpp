// ======================================================================
//
// IndentedFileWriter.cpp
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedFile/FirstSharedFile.h"
#include "sharedFile/IndentedFileWriter.h"
#include <cstdio>

// ======================================================================
// class IndentedFileWriter: PUBLIC STATIC
// ======================================================================

IndentedFileWriter *IndentedFileWriter::createWriter(char const *filename)
{
	IndentedFileWriter *const newWriter = new IndentedFileWriter();
	if (newWriter->createFile(filename))
		return newWriter;
	else
	{
		// Kill the new writer since we weren't able to open the file for writing.
		delete newWriter;
		return nullptr;
	}
}

// ======================================================================
// class IndentedFileWriter: PUBLIC
// ======================================================================

IndentedFileWriter::~IndentedFileWriter()
{
	if (m_file)
	{
		fclose(m_file);
		m_file = nullptr;
	}
}

// ----------------------------------------------------------------------

void IndentedFileWriter::indent()
{
	++m_indentCount;
}

// ----------------------------------------------------------------------

void IndentedFileWriter::unindent()
{
	if (m_indentCount <= 0)
	{
		WARNING(true, ("unindent() called when indentation level is [%d], ignoring unindent call.", m_indentCount));
		return;
	}
	--m_indentCount;
}

// ----------------------------------------------------------------------

void IndentedFileWriter::writeLine(char const *line) const
{
	FATAL(!m_file, ("writeLine(): m_file is nullptr, programmer error."));
	FATAL(!line, ("writeLine(): line argument is nullptr."));

	//-- Write one tab for each indentation level.
	for (int i = 0; i < m_indentCount; ++i)
		fputc(static_cast<int>('\t'), m_file);

	//-- Write the line of text.
	fputs(line, m_file);

	//-- Add a newline.
	fputc(static_cast<int>('\n'), m_file);
}

// ----------------------------------------------------------------------

void IndentedFileWriter::writeLineFormat(char const *format, ...) const
{
	va_list  varArgList;

	//-- Capture the variable arguments.
	va_start(varArgList, format);

	//-- Print the string to the buffer.
	char buffer[2048];

	int const formatCount = vsnprintf(buffer, sizeof(buffer), format, varArgList);
	UNREF(formatCount);

	//-- Ensure it's nullptr terminated.
	buffer[sizeof(buffer) - 1] = '\0';

	//-- Do the real print.
	writeLine(buffer);

	//-- Release the var args.
	va_end(varArgList);
}

// ======================================================================
// class IndentedFileWriter: PRIVATE
// ======================================================================

IndentedFileWriter::IndentedFileWriter() :
	m_indentCount(0),
	m_file(nullptr)
{
}

// ----------------------------------------------------------------------

bool IndentedFileWriter::createFile(char const *filename)
{
	if (m_file)
	{
		WARNING(true, ("m_file is already set, ignoring createFile() call."));
		return false;
	}

	m_file = fopen(filename, "w");
	return (m_file != nullptr);
}

// ======================================================================
