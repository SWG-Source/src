// ====================================================================== //
//
// CollideParameters.h
// asommers
//
// copyright 2004, sony online entertainment
//
// ======================================================================

#ifndef INCLUDED_CollideParameters_H
#define INCLUDED_CollideParameters_H

// ======================================================================

class CollideParameters
{
public:

	static CollideParameters const cms_default;
	static CollideParameters const cms_toolPickDefault;

public:

	enum Quality
	{
		Q_high,
		Q_medium,
		Q_low
	};

	enum Type
	{
		T_collidable,
		T_opaqueSolid
	};

public:

	//-- Default constructed CollideParameters will choose the first enum
	CollideParameters();
	CollideParameters(Quality q, Type t, bool isToolPickTest);
	~CollideParameters();

	Quality getQuality() const;
	void setQuality(Quality quality);

	Type getType() const;
	void setType(Type type);

	bool getToolPickTest() const;
	void setToolPickTest(bool isToolPickTest);

	bool typeShouldCollide(bool collidable, bool opaqueSolid) const;

private:

	Quality m_quality;
	Type m_type;

	bool m_isToolPickTest;
};

// ======================================================================

#endif
