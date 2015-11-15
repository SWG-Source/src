// ============================================================================
//
// TextIterator.cpp
// Copyright Sony Online Entertainment
//
// ============================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/TextIterator.h"

#include "UnicodeUtils.h"

// ============================================================================
//
// TextIterator
//
// ============================================================================

//-----------------------------------------------------------------------------
TextIterator::TextIterator(Unicode::String const &text)
 : m_rawText(text)
 , m_currentColorCode()
 , m_rawIndex(0)
 , m_iterPrintableIndex(m_rawText.begin())
{
}

//-----------------------------------------------------------------------------
void TextIterator::setRawText(Unicode::String const &text)
{
	m_rawText = text;
}

//-----------------------------------------------------------------------------
Unicode::String const &TextIterator::getRawText() const
{
	return m_rawText;
}

//-----------------------------------------------------------------------------
void TextIterator::skipUnPrintable()
{
	DEBUG_FATAL((m_iterPrintableIndex == m_rawText.begin()) && (m_rawIndex != 0), ("Index should be reset to zero: %d", m_rawIndex));

	for (; m_iterPrintableIndex != m_rawText.end();)
	{
		if ((*m_iterPrintableIndex) == '\\')
		{
			++m_iterPrintableIndex;
			++m_rawIndex;

			if ((*m_iterPrintableIndex) == '#')
			{
				++m_iterPrintableIndex;
				++m_rawIndex;

				if (m_iterPrintableIndex != m_rawText.end())
				{
					if ((*m_iterPrintableIndex) == '.')
					{
						// Color escape sequence: \#. set to default color

						++m_iterPrintableIndex;
						++m_rawIndex;
					}
					else if ((*m_iterPrintableIndex) == 'p')
					{
						// Color escape sequence: \#p<paletteentry>

						++m_iterPrintableIndex;
						++m_rawIndex;

						for (; m_iterPrintableIndex != m_rawText.end();)
						{
							++m_iterPrintableIndex;
							++m_rawIndex;

							if ((*m_iterPrintableIndex) == ' ')
							{
								break;
							}
						}
					}
					else
					{
						// Color code

						int count = 0;
						m_currentColorCode = Unicode::narrowToWide("\\#");

						for (; m_iterPrintableIndex != m_rawText.end();)
						{
							m_currentColorCode += (*m_iterPrintableIndex);

							++m_iterPrintableIndex;
							++m_rawIndex;

							if (++count == 6)
							{
								break;
							}
						}
					}
				}
			}
			else if ((*m_iterPrintableIndex) == '>')
			{
				// Indentation setting escape sequence \>xxx

				++m_iterPrintableIndex;
				++m_rawIndex;

				int count = 0;

				for (; m_iterPrintableIndex != m_rawText.end();)
				{
					++m_iterPrintableIndex;
					++m_rawIndex;

					if (++count == 3)
					{
						break;
					}
				}
			}
			else if ((*m_iterPrintableIndex) == '@')
			{
				// Tab setting escape sequence \@xx

				++m_iterPrintableIndex;
				++m_rawIndex;

				int count = 0;

				for (; m_iterPrintableIndex != m_rawText.end();)
				{
					++m_iterPrintableIndex;
					++m_rawIndex;

					if (++count == 2)
					{
						break;
					}
				}
			}
		}
		else
		{
			break;
		}
	}
}

//-----------------------------------------------------------------------------
Unicode::String TextIterator::getPrintableText()
{
	Unicode::String result;

	// Set the search to start at the beginning of the text

	m_iterPrintableIndex = m_rawText.begin();
	m_rawIndex = 0;

	// Move through the text keeping on printable text

	while (m_iterPrintableIndex != m_rawText.end())
	{
		skipUnPrintable();

		if (m_iterPrintableIndex != m_rawText.end())
		{
			result += (*m_iterPrintableIndex);
			++m_iterPrintableIndex;
			++m_rawIndex;
		}
	}

	return result;
}

//-----------------------------------------------------------------------------
void TextIterator::insertTextAtFirstPrintablePosition(Unicode::String const &text)
{
	// Set the search to start at the beginning of the text

	m_iterPrintableIndex = m_rawText.begin();
	m_rawIndex = 0;

	// Move to the first non-coded position

	skipUnPrintable();
	
	// Insert the text

	m_rawText.insert(m_rawIndex, text);
}

//-----------------------------------------------------------------------------
void TextIterator::insertTextAtPrintablePosition(int const position, Unicode::String const &text)
{
	// Set the search to start at the beginning of the text

	m_iterPrintableIndex = m_rawText.begin();
	m_rawIndex = 0;

	// Search for the specified printable position

	if ((position >= 0) &&
	    (position < static_cast<int>(m_rawText.size())))
	{
		int index = 0;

		skipUnPrintable();

		while (index != position)
		{
			if (m_iterPrintableIndex != m_rawText.end())
			{
				++m_iterPrintableIndex;
				++m_rawIndex;
				++index;

				skipUnPrintable();
			}
			else
			{
				break;
			}
		}

		// Insert the text

		if (m_iterPrintableIndex != m_rawText.end())
		{
			m_rawText.insert(m_rawIndex, text);
		}
	}
}

//-----------------------------------------------------------------------------
void TextIterator::insertCurrentColorCodeAtPrintablePosition(int const position)
{
	// Set the search to start at the beginning of the text

	m_iterPrintableIndex = m_rawText.begin();
	m_rawIndex = 0;

	// Search for the specified printable position

	if ((position >= 0) &&
	    (position < static_cast<int>(m_rawText.size())))
	{
		int index = 0;

		skipUnPrintable();

		while (index != position)
		{
			if (m_iterPrintableIndex != m_rawText.end())
			{
				++m_iterPrintableIndex;
				++m_rawIndex;
				++index;

				skipUnPrintable();
			}
			else
			{
				break;
			}
		}

		// Insert the current color code

		if (m_iterPrintableIndex != m_rawText.end())
		{
			m_rawText.insert(m_rawIndex, m_currentColorCode);
		}
	}
}

//-----------------------------------------------------------------------------
void TextIterator::appendText(Unicode::String const &text)
{
	m_rawText += text;
}

// ============================================================================
