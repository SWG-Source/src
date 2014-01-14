// ======================================================================
//
// TextAppearance.h
// Copyright 2001, Sony Online Entertainment Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_TextAppearance_H
#define INCLUDED_TextAppearance_H

// ======================================================================

#include "sharedObject/Appearance.h"

class VectorArgb;

// ======================================================================

class TextAppearance : public Appearance
{
public:

	TextAppearance ();
	virtual ~TextAppearance ()=0;
		
	virtual void setText(const char* text)=0;
	virtual void setColor(const VectorArgb& color)=0;

private:

	TextAppearance (const TextAppearance& rhs);
	TextAppearance& operator= (const TextAppearance& rhs);
};

// ======================================================================

#endif
