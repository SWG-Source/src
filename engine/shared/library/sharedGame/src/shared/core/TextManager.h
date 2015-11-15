// ============================================================================
//
// TextManager.h
// Copyright Sony Online Entertainment
//
// ============================================================================

#ifndef INCLUDED_TextManager_H
#define INCLUDED_TextManager_H

class PackedRgb;
class RegexList;

// ----------------------------------------------------------------------------
class TextManager
{
public:

	static void install();

public:

	static Unicode::String filterText(Unicode::String const &text);
	static bool            isAppropriateText(Unicode::String const &text);

private:

	static void remove();

	TextManager();
	~TextManager();
};

// ============================================================================

#endif // INCLUDED_TextManager_H
