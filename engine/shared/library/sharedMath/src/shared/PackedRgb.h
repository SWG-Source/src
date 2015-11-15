//===================================================================
//
// PackedRgb.h
// asommers 6-20-2000
//
// copyright 2000, verant interactive
//
//===================================================================

#ifndef INCLUDED_PackedRgb_H
#define INCLUDED_PackedRgb_H

//===================================================================

class VectorArgb;

//===================================================================

class PackedRgb
{
private:

	static const float oo255;

public:

	static const PackedRgb solidBlack;
	static const PackedRgb solidBlue; 
	static const PackedRgb solidCyan; 
	static const PackedRgb solidGray;
	static const PackedRgb solidGreen;
	static const PackedRgb solidRed;  
	static const PackedRgb solidMagenta;
	static const PackedRgb solidYellow;
	static const PackedRgb solidWhite;
	static const PackedRgb solidOrange;

public:

	uint8 r;
	uint8 g;
	uint8 b;

public:

	PackedRgb ();
	PackedRgb (uint8 newR, uint8 newG, uint8 newB);

	VectorArgb convert (float alpha=1.f) const;
	void       convert (const VectorArgb& color);

	uint32     asUint32() const;

	bool       operator== (const PackedRgb& rhs) const;
	bool       operator!= (const PackedRgb& rhs) const;

	static const PackedRgb linearInterpolate (const PackedRgb& color1, const PackedRgb& color2, float t);
};

//===================================================================

inline PackedRgb::PackedRgb () :
	r (0),
	g (0),
	b (0)
{
}

//-------------------------------------------------------------------

inline PackedRgb::PackedRgb (uint8 newR, uint8 newG, uint8 newB) :
	r (newR),
	g (newG),
	b (newB)
{
}

//-------------------------------------------------------------------

inline uint32 PackedRgb::asUint32() const
{
	return (static_cast<uint32>(r) << 16) | (static_cast<uint32>(g) << 8) | (static_cast<uint32>(b) << 0);
}

//===================================================================

#endif
