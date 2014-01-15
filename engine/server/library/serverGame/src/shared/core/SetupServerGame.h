// ======================================================================
//
// SetupServerGame.h
// copyright (c) 2004 Sony Online Entertainment
//
// ======================================================================

#ifndef INCLUDED_SetupServerGame_H
#define INCLUDED_SetupServerGame_H

// ======================================================================

class SetupServerGame
{
public:

	static void install();
	static void remove();

private:
	SetupServerGame();
	SetupServerGame(const SetupServerGame &);
	SetupServerGame &operator =(const SetupServerGame &);
};

// ======================================================================

#endif
